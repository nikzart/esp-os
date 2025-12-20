#ifndef CONFIG_H
#define CONFIG_H

// I2C Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define I2C_SDA 21
#define I2C_SCL 22

// Button GPIO Pin Definitions
// Wire each: GPIO -> Button -> GND (internal pull-ups enabled)
#define BTN_PIN_LEFT  32
#define BTN_PIN_RIGHT 33
#define BTN_PIN_UP    25
#define BTN_PIN_DOWN  26
#define BTN_PIN_A     27
#define BTN_PIN_B     14
#define BTN_PIN_C     13
#define BTN_PIN_D     4

// Button indices (for arrays)
#define BTN_LEFT  0
#define BTN_RIGHT 1
#define BTN_UP    2
#define BTN_DOWN  3
#define BTN_A     4
#define BTN_B     5
#define BTN_C     6
#define BTN_D     7
#define NUM_BUTTONS 8

// Debounce time in milliseconds
#define DEBOUNCE_MS 50

// Buzzer
#define BUZZER_PIN 15
#define BEEP_FREQ 2000
#define BEEP_DURATION 50

// DHT11 Sensor
#define DHT_PIN 17
#define DHT_TYPE DHT11
#define DHT_READ_INTERVAL 2000

// NVS Namespace
#define NVS_NAMESPACE "esp_os"

// API Configuration
#define OPENWEATHER_API_KEY "4bab8ccf06439eac65872a1eab4d4490"
#define NEWS_API_KEY "a7fb3c78c9e94f49945b1a74b49d2186"
#define DEFAULT_CITY "Kollam"

// App count
#define NUM_APPS 12

#endif
