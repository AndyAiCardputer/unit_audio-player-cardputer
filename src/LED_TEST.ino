/*
 * 💡 Unit AudioPlayer LED Test
 * Проверяем доступ к RGB LED на Unit AudioPlayer
 */

#include <M5Cardputer.h>
#include <unit_audioplayer.hpp>

AudioPlayerUnit audioplayer;

void setup() {
    M5Cardputer.begin();
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(BLACK);
    
    Serial.begin(115200);
    Serial.println("=== LED Test для Unit AudioPlayer ===");
    
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setCursor(10, 10);
    M5Cardputer.Display.print("LED Test");
    
    // Подключаемся к AudioPlayer
    while (!audioplayer.begin(&Serial1, 1, 2)) {
        Serial.println("Waiting...");
        delay(1000);
    }
    
    Serial.println("Connected!");
    M5Cardputer.Display.setCursor(10, 30);
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.print("Connected!");
    
    // Проверяем доступные методы
    M5Cardputer.Display.setCursor(10, 50);
    M5Cardputer.Display.setTextColor(YELLOW);
    M5Cardputer.Display.print("Testing LED...");
    
    delay(1000);
}

void loop() {
    M5Cardputer.update();
    
    // Тестируем разные варианты LED команд
    
    // Вариант 1: Попробуем через прямую UART команду
    // N9301 использует команды типа: 0xAA 0x0C ...
    
    static unsigned long lastTest = 0;
    static int testPhase = 0;
    
    if (millis() - lastTest > 2000) {
        lastTest = millis();
        
        M5Cardputer.Display.fillRect(10, 70, 220, 50, BLACK);
        M5Cardputer.Display.setCursor(10, 70);
        
        switch(testPhase) {
            case 0:
                // Попробуем setLED если есть
                M5Cardputer.Display.setTextColor(RED);
                M5Cardputer.Display.print("Test: LED RED");
                Serial.println("Testing: LED RED");
                // audioplayer.setLED(255, 0, 0); // если метод существует
                break;
                
            case 1:
                M5Cardputer.Display.setTextColor(GREEN);
                M5Cardputer.Display.print("Test: LED GREEN");
                Serial.println("Testing: LED GREEN");
                // audioplayer.setLED(0, 255, 0);
                break;
                
            case 2:
                M5Cardputer.Display.setTextColor(BLUE);
                M5Cardputer.Display.print("Test: LED BLUE");
                Serial.println("Testing: LED BLUE");
                // audioplayer.setLED(0, 0, 255);
                break;
                
            case 3:
                M5Cardputer.Display.setTextColor(WHITE);
                M5Cardputer.Display.print("Test: LED OFF");
                Serial.println("Testing: LED OFF");
                // audioplayer.setLED(0, 0, 0);
                break;
        }
        
        testPhase = (testPhase + 1) % 4;
    }
    
    // Выход по ESC
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        for (auto key : status.word) {
            if (key == '`' || key == '~') {
                M5Cardputer.Display.fillScreen(BLACK);
                M5Cardputer.Display.setCursor(60, 60);
                M5Cardputer.Display.setTextColor(YELLOW);
                M5Cardputer.Display.print("Test complete!");
                delay(2000);
                ESP.restart();
            }
        }
    }
    
    delay(10);
}

