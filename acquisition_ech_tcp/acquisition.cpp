#include "acquisition.h"
#include "storage.h"
#include "config.h"

#include <Arduino.h>

// ===== Timer =====
// Variables
static        hw_timer_t*   timer               = NULL;
static const  uint32_t      TIMER_BASE_FREQ_HZ  = 1000000;
static        uint32_t      TIMER_ISR_FREQ_HZ;
static        uint32_t      TIMER_ISR_TICKS; // nb de ticks à attendre avant de relancer une isr (onTimer())
// Prototypes
void onTimer();


// ===== Oversampling =====
// Variables
static        uint16_t      oversampleAcc[NB_CAPTEURS]  = {0};  // accumulation 16bit okay : max value (2^12-1)*4 < 2^16-1
static        uint8_t       oversampleCount             = 0;

// ==== Acquisition task ====
// Variables
static        TaskHandle_t  acquisitionTaskHandle = NULL;
// Prototypes
static        void          acquisitionTask(void* parameter);



// ================= INITIALISATION =================
void initAcquisition(uint16_t sampleFrequency)
{
    // Definition of the ISR frequency
    TIMER_ISR_FREQ_HZ   = sampleFrequency * OVERSAMPLING; 
    TIMER_ISR_TICKS     = TIMER_BASE_FREQ_HZ / TIMER_ISR_FREQ_HZ; // nb de ticks à attendre avant de relancer une isr (onTimer())

    // Oversampling initialized
    oversampleCount     = 0;

    // Acquisition task created and pinned on CORE 1
    xTaskCreatePinnedToCore(
        acquisitionTask,
        "Acquisition_Task",
        4096,
        NULL,
        2,                      // priorité > TCP
        &acquisitionTaskHandle,
        1                       // CORE 1
    );

    // Timer configuration
    timer = timerBegin(TIMER_BASE_FREQ_HZ);       // Timer at 1000000Hz : 1MHz
    timerAttachInterrupt(timer, &onTimer);        // onTimer() attached to the timer 
    timerWrite(timer, 0);
    timerAlarm(timer, TIMER_ISR_TICKS, true, 0);  // onTimer() executed each TIMER_ISR_TICKS ticks of the timer
    // Serial.printf("Initialisation timer à fe = %d Hz\n", sampleFrequency);
    
}


void stopAcquisition()
{
    if (timer != nullptr)
    {
        timerEnd(timer);
        timer = nullptr;
    }
    if (acquisitionTaskHandle != NULL)
    {
        vTaskDelete(acquisitionTaskHandle);
        acquisitionTaskHandle = NULL;
    }
}

/*uint16_t get_adc_value(uint8_t channel)
{
    return analogRead(ADC_PINS[channel]);

    // Plus tard possible :
    // return adc1_get_raw(...);
}*/
uint16_t get_adc_value(uint8_t channel)
{
    if (channel >= NB_CAPTEURS)
        return 0;

    int raw = 0;

    esp_err_t err = adc_oneshot_read(adc1_handle, adc_channels[channel], &raw);

    return (err == ESP_OK) ? (uint16_t)raw : 0;
}

// ======== ISR (Interrupt Service Routine)============
void IRAM_ATTR onTimer()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(acquisitionTaskHandle,       // Notifying the task that it can be executed
                           &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void acquisitionTask(void* parameter){
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Receiving the notification of the ISR

        // Visualisation de l'acquisition tous les OVERSAMPLING points
        //Serial.print(".");
        
        
        // Oversampling accumulation
        for (uint8_t ch = 0; ch < NB_CAPTEURS; ch++)
        {
            oversampleAcc[ch] += get_adc_value(ch);
        }
    
        oversampleCount++;
    
        // If the oversampling is not over, we go back to the accumulation
        if (oversampleCount < OVERSAMPLING)
            continue;
        
        
        // ===== Data treatment =====
    
        uint16_t* buffer = getWriteBuffer();              // get the buffer to write in
        uint16_t index   = getSampleIndex();              // get the index position in that buffer where to write the information
    
        if (index >= FRAME_SIZE){ // Security : if the index overflows the FRAME_SIZE we stop this acquisition
            continue;
        }
        
        
        for (uint8_t ch = 0; ch < NB_CAPTEURS; ch++)
        {
            
            buffer[index + ch] = oversampleAcc[ch] >> OVERSAMPLING_BITS; // Getting the average of the accumulation by dividing by OVERSAMPLING
    
            oversampleAcc[ch] = 0;
        }
    
        oversampleCount = 0;
    
        index += NB_CAPTEURS; // [data0_1,...,data0_NB_CAPTEURS]:index = 0 | [data1_1,...,data1_NB_CAPTEURS]:index = NB_CAPTEURS ==> index corresponding to the position of data(index)_1
        setSampleIndex(index);
    
        if (index >= FRAME_SIZE)
        {
            uint8_t  bufIdx  = getCurrentWriteBufferIndex();  // get the current buffer index     /!\ <---------------
            switchToNextWriteBuffer();
            notifyBufferReady(bufIdx);                        // notifying the tcp_task to send this buffer
        }
    
        // Serial.printf(" - Task core %d\n",xPortGetCoreID());
        // Serial.printf("%d", index);
        
    }
}
