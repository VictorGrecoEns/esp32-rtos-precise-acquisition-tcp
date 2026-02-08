/*
 05/02/2026 : création du fichier, développement de la configuration du timer matériel
*/

// =====================================================================================
// ========================== CONFIGURATION DU TIMER MATERIEL ==========================
// =====================================================================================

// =========================== Configuration du timer ===========================
// Constantes
static constexpr uint32_t TIMER_BASE_FREQ_HZ   = 1000000;   // 1 MHz
static constexpr uint32_t TIMER_ISR_FREQ_HZ    = 80;      // en Hz  <---------- fréquence d'échantillonnage à diviser par le surechantillonnage
static constexpr uint32_t TIMER_ISR_TICKS      = TIMER_BASE_FREQ_HZ / TIMER_ISR_FREQ_HZ;

// Variables globales
hw_timer_t                *timer          = nullptr;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE               timerMux       = portMUX_INITIALIZER_UNLOCKED;

// ====================== ISR (Interrupt Service Routine) ======================
volatile bool isrActivated = false;

// Configuration de la LED dans l'ISR
static constexpr uint8_t  PIN_LED           = 2;
bool                      ledState          = false;
volatile uint32_t         ledIsrCounter     = 0;
volatile bool             toggleLed         = false;

static constexpr uint32_t LED_ACQ_PERIOD_ms = 10;
static constexpr uint32_t LED_TICKS         = TIMER_ISR_FREQ_HZ * LED_ACQ_PERIOD_ms / 1000;

void isrBegin(void){
  timerSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(TIMER_BASE_FREQ_HZ);  // Timer à TIMER_BASE_FREQ_HZ
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, TIMER_ISR_TICKS, true, 0); // Déclenche le onTimer tous les TIMER_ISR_TICKS de TIMER_BASE_FREQ_HZ
}

void ARDUINO_ISR_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);

  // Changement de l'état de la LED plus lent que l'ISR
  ledIsrCounter += 1;
  if (ledIsrCounter >= LED_TICKS){
    toggleLed = true;
    ledIsrCounter = 0; 
  }
  
  portEXIT_CRITICAL_ISR(&timerMux);
  xSemaphoreGiveFromISR(timerSemaphore, nullptr);
}



// =====================================================================================
// ============================== CONFIGURATION DU WIFI ================================
// =====================================================================================
// Connexion au réseau du Pi
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid      = "grecoPi";
const char* password  = "170703Victor.";
const char* serverUrl = "http://192.168.17.7:5000/data";

IPAddress localIP(192,168,17,40);
IPAddress gateway(192,168,17,1);
IPAddress subnet(255,255,255,0);

// Buffer circulaire stockant les données
static constexpr uint16_t BLOCK_SIZE = 200;
static constexpr uint8_t BUFFER_BLOCKS = 4;

uint16_t buffer[BUFFER_BLOCKS][BLOCK_SIZE];
volatile uint8_t  writeBlock = 0;
volatile uint8_t  readBlock  = 0;
volatile uint16_t writeIndex = 0;
volatile bool blockReady[BUFFER_BLOCKS] = {false};




// =====================================================================================
// ========================== CONFIGURATION DE L'ACQUISITION ===========================
// =====================================================================================

// GPIO
static constexpr uint8_t PIN_V = 35; // pin du capteur de tension

// Variables constantes
static const float resoCaptV = 25.0/4095.0; // resolution du capteur de tension (inutile ici car envoi des donnees brutes)

// Suréchantillonnage
volatile uint32_t accData  = 0;
volatile uint8_t  accCount = 0;

// Gestion de l'acquisition (intialiser à l'arret)
volatile int nbBlocToSend = -1; 
volatile bool acquisitionDone = true;


void get_blocks_from_server() {
    HTTPClient http;
    http.begin("http://192.168.17.7:5000/config");

    int code = http.GET();
    if (code != 200) {
        http.end();
        esp_restart();  // sécurité
    }

    String payload = http.getString();
    http.end();

    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload);

    nbBlocToSend = doc["n_blocks"];
}

void send_init_to_server(uint32_t fe,
                         uint16_t block_size,
                         uint32_t n_blocks) {

    HTTPClient http;
    http.begin("http://192.168.17.7:5000/init");
    http.addHeader("Content-Type", "application/json");

    String payload = "{";
    payload += "\"fe\":" + String(fe) + ",";
    payload += "\"block_size\":" + String(block_size) + ",";
    payload += "\"n_blocks\":" + String(n_blocks);
    payload += "}";

    int code = http.POST(payload);
    http.end();

    if (code != 200) {
        esp_restart();
    }
}

void start_acquisition(void){
  acquisitionDone = false;
}

void end_acquisition(void){
  nbBlocToSend = -1;
  acquisitionDone = true;
}


// ================= SETUP =================
void setup()
{
  // Initialisation de la communication serie
  Serial.begin(115200);
  delay(100);

  // Initialisation des PINS
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_V, INPUT);
  digitalWrite(PIN_LED, LOW);

  // Initialisation de l'ISR
  isrBegin();
  Serial.println("Timer demarre");
  
  // Initialisation Wi-Fi
  WiFi.config(localIP, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Connexion Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ledState = !ledState;
    digitalWrite(PIN_LED, ledState);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connecté");

  // Récupération de la configuration auprès du serveur
  get_blocks_from_server();  // GET /config
  
  // Envoyer les paramètres d'acquisition
  send_init_to_server(TIMER_ISR_FREQ_HZ/4, BLOCK_SIZE, nbBlocToSend);  // POST /init
  
  // Lancement de l'acquisition
  start_acquisition();

  
  /*for (int i = 0; i < 4; i++){
    blockReady[i] = false;
  }*/

  // Definition de la tache parallele: envoi wifi
  xTaskCreate(sendDataTask, "WiFiTask", 16384, NULL, 1, NULL);

}

// ================= LOOPS =================

void sendDataTask(void * parameter) {
  for (;;) {
    if (blockReady[readBlock]) {

      uint64_t timestamp = esp_timer_get_time();
  
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");
  
      // Construction JSON
      String payload = "{";
      payload += "\"timestamp\":" + String(timestamp) + ",";
      payload += "\"bloc\":" + String(readBlock) + ",";
      payload += "\"bloc_size\":" + String(BLOCK_SIZE) + ",";
      payload += "\"fe\":" + String(TIMER_ISR_FREQ_HZ/4) + ",";
      payload += "\"samples\":[";
  
      for (int i = 0; i < BLOCK_SIZE; i++) {
        payload += String(buffer[readBlock][i]);
        if (i < BLOCK_SIZE - 1) payload += ",";
      }
      payload += "]}";
  
      int code = http.POST(payload);
      http.end();
  
      if (code <= 0) {
        Serial.print("Erreur HTTP, code d'erreur:");
        Serial.println(code);
      } else {
        Serial.print("Envoi du bloc : ");
        Serial.println(readBlock);
      }
  
      blockReady[readBlock] = false;
      readBlock = (readBlock + 1) % BUFFER_BLOCKS;

      // Décrémenter le nombre de bloc à envoyer
      nbBlocToSend -= 1;
      if (nbBlocToSend <= 0){
        end_acquisition();
      }
    }
  }
  vTaskDelay(2); // petite pause pour laisser le CPU à d'autres tasks
}



void loop()
{
  if (acquisitionDone){
    ledState = !ledState;
    digitalWrite(PIN_LED, ledState);
    delay(500);
  } else {
    // Faire à chaque ISR
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
    {
  
      accData  += analogRead(PIN_V); // ajout à la valeur moyenne de la data
      accCount += 1;
  
      if (accCount == 4) { // si 4 échantillons cumulés
        uint16_t avg = accData >> 2; // division par 4
        accData = 0; 
        accCount = 0;
    
        buffer[writeBlock][writeIndex] = avg; // ajout dans le buffer
        writeIndex += 1;
        
        if (writeIndex >= BLOCK_SIZE) { // si le bloc actuel du buffer est plein on change de bloc
          blockReady[writeBlock] = true;
          writeIndex = 0;
          writeBlock = (writeBlock + 1) % BUFFER_BLOCKS;
        }
        Serial.print(millis());
        Serial.print(" ms - writeBlock = ");
        Serial.print(writeBlock);
        Serial.print(" - tension = ");
        Serial.println(avg*resoCaptV);
      }
      
      if (toggleLed)
      {
        toggleLed = false;
        ledState = !ledState;
        digitalWrite(PIN_LED, ledState);
      }
    }
  }
}