# ESP32 RTOS Precise Acquisition over TCP

## Overview

This project implements a **deterministic real-time data acquisition system** on an ESP32-WROOM dual-core microcontroller.  

Key features:

- Hardware timer interrupts (`hw_timer_t`) for precise timing
- FreeRTOS task notifications from ISR
- Dual-core task separation: Core 1 = acquisition, Core 0 = TCP
- Oversampling with accumulation
- Double buffering with timestamps
- TCP streaming to a Python server
- Configurable ADC pins, sampling frequency, buffers, and server

---

## System Architecture

| Core | Task |
|------|------|
| Core 1 | Acquisition |
| Core 0 | TCP Communication |

**Acquisition task**: triggered by ISR, performs ADC reads, applies oversampling, writes to double buffers, signals TCP task.  

**TCP task**: sends full buffers to the server, handles acquisition start commands, stops acquisition when done.  

**Buffers**: double-buffered, each frame has `N_SAMPLES * NB_CAPTEURS` values, timestamped using `esp_timer_get_time()`.  

---

## Configuration Parameters (`config.h`)

- LED pin: `PIN_LED = 2`  
- Number of ADC channels: `NB_CAPTEURS = 2`  
- ADC pins: `ADC_PINS[NB_CAPTEURS] = {34, 35}`  
- Oversampling: `OVERSAMPLING_BITS = 2` → 4 samples per measurement  
- Buffer: `NB_BUFFERS = 4`, `N_SAMPLES = 256`  
- TCP Server: `SERVER_IP` and `SERVER_PORT`  

All parameters are editable to adapt to different hardware setups and acquisition needs.

---

## Acquisition Flow

1. **Hardware timer ISR** triggers task notification (`onTimer()`)  
2. **Acquisition task** (Core 1):
   - Reads ADC channels via `adc_oneshot_read`
   - Accumulates oversamples
   - Computes averaged sample
   - Writes into current buffer
   - Switches buffer when full and notifies TCP task
3. **TCP task** (Core 0):
   - Maintains WiFi connection
   - Receives acquisition parameters from server
   - Sends completed buffers with timestamps
   - Stops acquisition when requested buffers are sent

**Frame format sent over TCP**:
[uint64_t timestamp][uint16_t ADC samples buffer]

---

## Performance

- Deterministic sampling below 1 kHz  
- Stable up to 1 kHz (dependent on WiFi latency & ADC jitter)  
- Minimal ISR workload to reduce jitter  
- Double buffering prevents data loss during transmission  

---

## Technical Stack

- ESP32-WROOM (dual core)  
- FreeRTOS  
- Hardware timer (`hw_timer_t`)  
- ADC one-shot driver  
- WiFi TCP client  
- Python TCP server  

---

## Applications

- Remote sensor monitoring  
- IoT instrumentation  
- Embedded real-time data logging  
- Educational demonstrations  

---

## Why This Project

Provides a **reusable template** for deterministic ESP32 acquisition systems with:

- Core separation for real-time constraints  
- Minimal ISR design  
- Oversampling and double-buffering for reliable measurements  
- TCP streaming for remote monitoring  

---