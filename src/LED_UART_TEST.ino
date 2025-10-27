/*
 * 💡 Unit AudioPlayer LED UART Test
 * Прямые UART команды для управления LED
 */

#include <M5Cardputer.h>
#include <unit_audioplayer.hpp>

AudioPlayerUnit audioplayer;

// Прямая отправка UART команды
void sendLEDCommand(uint8_t r, uint8_t g, uint8_t b) {
    // N9301 протокол:
    // Попробуем разные форматы команд
    
    // Вариант 1: Стандартная команда (если есть LED команда)
    uint8_t cmd1[] = {0xAA, 0x0D, r, g, b, 0x00}; // 0x0D = LED command (пример)
    cmd1[5] = 0xAA + 0x0D + r + g + b; // Checksum
    
    Serial.print("Sending LED: R=");
    Serial.print(r);
    Serial.print(" G=");
    Serial.print(g);
    Serial.print(" B=");
    Serial.println(b);
    
    Serial.print("CMD: ");
    for(int i = 0; i < sizeof(cmd1); i++) {
        Serial.print("0x");
        if(cmd1[i] < 0x10) Serial.print("0");
        Serial.print(cmd1[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    
    Serial1.write(cmd1, sizeof(cmd1));
    delay(100);
    
    // Читаем ответ
    if(Serial1.available()) {
        Serial.print("Response: ");
        while(Serial1.available()) {
            uint8_t b = Serial1.read();
            Serial.print("0x");
            if(b < 0x10) Serial.print("0");
            Serial.print(b, HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void testDirectGPIO() {
    // Попробуем через GPIO (если LED подключен к GPIO)
    // Обычно на ESP32 модулях LED на GPIO 2, 4, или 5
    
    Serial.println("\nTesting GPIO approach...");
    
    int testPins[] = {2, 4, 5, 13, 15, 16, 17, 18, 19};
    
    for(int pin : testPins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        
        M5Cardputer.Display.fillRect(10, 90, 220, 20, BLACK);
        M5Cardputer.Display.setCursor(10, 90);
        M5Cardputer.Display.setTextColor(YELLOW);
        M5Cardputer.Display.print("Testing GPIO ");
        M5Cardputer.Display.print(pin);
        
        Serial.print("GPIO ");
        Serial.print(pin);
        Serial.println(" HIGH");
        
        delay(500);
        digitalWrite(pin, LOW);
        pinMode(pin, INPUT); // Возвращаем в INPUT
    }
    
    Serial.println("GPIO test complete");
}

void setup() {
    M5Cardputer.begin();
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(BLACK);
    
    Serial.begin(115200);
    Serial.println("=== LED UART Direct Test ===");
    
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setCursor(10, 10);
    M5Cardputer.Display.print("LED UART Test");
    
    // Подключаемся
    M5Cardputer.Display.setCursor(10, 30);
    M5Cardputer.Display.print("Connecting...");
    
    while (!audioplayer.begin(&Serial1, 1, 2)) {
        Serial.println("Waiting...");
        delay(1000);
    }
    
    Serial.println("Connected!");
    M5Cardputer.Display.fillRect(10, 30, 220, 10, BLACK);
    M5Cardputer.Display.setCursor(10, 30);
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.print("Connected!");
    
    delay(1000);
    
    // Сначала пробуем GPIO
    M5Cardputer.Display.setCursor(10, 50);
    M5Cardputer.Display.setTextColor(CYAN);
    M5Cardputer.Display.print("Testing GPIO...");
    testDirectGPIO();
    
    M5Cardputer.Display.fillRect(10, 50, 220, 10, BLACK);
    M5Cardputer.Display.setCursor(10, 50);
    M5Cardputer.Display.setTextColor(YELLOW);
    M5Cardputer.Display.print("Testing UART commands...");
}

void loop() {
    M5Cardputer.update();
    
    static unsigned long lastTest = 0;
    static int testPhase = 0;
    
    if (millis() - lastTest > 2000) {
        lastTest = millis();
        
        M5Cardputer.Display.fillRect(10, 70, 220, 30, BLACK);
        M5Cardputer.Display.setCursor(10, 70);
        
        Serial.println("\n=== Test Phase " + String(testPhase) + " ===");
        
        switch(testPhase) {
            case 0:
                M5Cardputer.Display.setTextColor(RED);
                M5Cardputer.Display.print("Test: RED");
                sendLEDCommand(255, 0, 0);
                break;
                
            case 1:
                M5Cardputer.Display.setTextColor(GREEN);
                M5Cardputer.Display.print("Test: GREEN");
                sendLEDCommand(0, 255, 0);
                break;
                
            case 2:
                M5Cardputer.Display.setTextColor(BLUE);
                M5Cardputer.Display.print("Test: BLUE");
                sendLEDCommand(0, 0, 255);
                break;
                
            case 3:
                M5Cardputer.Display.setTextColor(YELLOW);
                M5Cardputer.Display.print("Test: YELLOW");
                sendLEDCommand(255, 255, 0);
                break;
                
            case 4:
                M5Cardputer.Display.setTextColor(MAGENTA);
                M5Cardputer.Display.print("Test: MAGENTA");
                sendLEDCommand(255, 0, 255);
                break;
                
            case 5:
                M5Cardputer.Display.setTextColor(CYAN);
                M5Cardputer.Display.print("Test: CYAN");
                sendLEDCommand(0, 255, 255);
                break;
                
            case 6:
                M5Cardputer.Display.setTextColor(WHITE);
                M5Cardputer.Display.print("Test: WHITE");
                sendLEDCommand(255, 255, 255);
                break;
                
            case 7:
                M5Cardputer.Display.setTextColor(0x7BEF);
                M5Cardputer.Display.print("Test: OFF");
                sendLEDCommand(0, 0, 0);
                break;
                
            case 8:
                // Пробуем другую команду
                M5Cardputer.Display.setTextColor(ORANGE);
                M5Cardputer.Display.print("Test: ALT CMD");
                
                uint8_t altCmd[] = {0xAA, 0x10, 0xFF, 0x00, 0x00, 0xAF}; // Другой вариант
                Serial.print("Trying alternative command: ");
                for(int i = 0; i < sizeof(altCmd); i++) {
                    Serial.print("0x");
                    if(altCmd[i] < 0x10) Serial.print("0");
                    Serial.print(altCmd[i], HEX);
                    Serial.print(" ");
                }
                Serial.println();
                Serial1.write(altCmd, sizeof(altCmd));
                break;
        }
        
        testPhase = (testPhase + 1) % 9;
    }
    
    // Выход
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        for (auto key : status.word) {
            if (key == '`' || key == '~') {
                M5Cardputer.Display.fillScreen(BLACK);
                M5Cardputer.Display.setCursor(60, 60);
                M5Cardputer.Display.setTextColor(YELLOW);
                M5Cardputer.Display.print("Test complete!");
                Serial.println("\n=== Test finished ===");
                delay(2000);
                ESP.restart();
            }
        }
    }
    
    delay(10);
}

