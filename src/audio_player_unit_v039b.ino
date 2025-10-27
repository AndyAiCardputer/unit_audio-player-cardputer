/*
 * 🎵 Unit AudioPlayer для M5Stack Cardputer v0.39c
 * С информацией о треках, времени, случайным выбором и зацикливанием
 * Новое: исправлены временные сообщения Next/Prev track
 */

#include <M5Cardputer.h>
#include <SPI.h>
#include <unit_audioplayer.hpp>

AudioPlayerUnit audioplayer;

// Переменные для отслеживания кликов
unsigned long lastClickTime = 0;
int clickCount = 0;
const unsigned long clickTimeout = 500;

// Переменные для отслеживания треков
uint8_t lastPlayStatus = AUDIO_PLAYER_STATUS_STOPPED;
uint16_t currentTrack = 1;
uint16_t lastDisplayedTrack = 1; // Последний отображенный трек
uint16_t totalTracks = 0;

// Переменные для времени воспроизведения
unsigned long trackStartTime = 0;
unsigned long pausedTime = 0;
unsigned long totalPausedTime = 0;
bool isPlaying = false;

// Переменные для зацикливания трека
bool isLoopEnabled = false;

// Переменные для Shuffle режима
bool isShuffleEnabled = false;

// Переменные для ручного ввода номера трека
bool isInputMode = false;
String inputTrackNumber = "";

// Переменные для батареи и громкости
unsigned long lastBatteryUpdate = 0;
const unsigned long batteryUpdateInterval = 5000;
unsigned long lastTimeUpdate = 0;
const unsigned long timeUpdateInterval = 1000; // Обновляем время каждую секунду
int currentVolume = 20;

// Переменные для визуализации
unsigned long lastVisualizerUpdate = 0;
const unsigned long visualizerUpdateInterval = 150; // Обновление каждые 150ms
int barHeights[5] = {0, 0, 0, 0, 0}; // 5 баров

// Переменные для временного сообщения
unsigned long tempMessageTime = 0;
bool tempMessageActive = false;
const unsigned long tempMessageDuration = 2000; // 2 секунды

// Переменные для эквалайзера
int currentEQMode = 5; // 0=Rock, 1=Pop, 2=Jazz, 3=Classical, 4=Bass, 5=Normal
const char* eqModeNames[] = {"Rock", "Pop", "Jazz", "Classical", "Bass", "Normal"};
const int eqModeCount = 6;

// ========== ИКОНКИ ==========
// Функции для рисования иконок

// Иконка ноты ♪ (8x8)
void drawNoteIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.fillCircle(x+2, y+6, 2, color);
    M5Cardputer.Display.fillRect(x+4, y+1, 2, 6, color);
    M5Cardputer.Display.fillRect(x+4, y+1, 3, 2, color);
}

// Иконка паузы ⏸
void drawPauseIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.fillRect(x+1, y+1, 2, 6, color);
    M5Cardputer.Display.fillRect(x+5, y+1, 2, 6, color);
}

// Иконка play ▶
void drawPlayIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.fillTriangle(x+1, y+1, x+1, y+7, x+7, y+4, color);
}

// Иконка динамика 🔊
void drawSpeakerIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.fillRect(x+1, y+2, 2, 4, color);
    M5Cardputer.Display.fillTriangle(x+3, y+1, x+3, y+7, x+6, y+4, color);
    M5Cardputer.Display.drawLine(x+7, y+2, x+8, y+1, color);
    M5Cardputer.Display.drawLine(x+7, y+6, x+8, y+7, color);
}

// Иконка батареи 🔋
void drawBatteryIcon(int x, int y, uint16_t color, int level) {
    // Корпус батареи
    M5Cardputer.Display.drawRect(x, y+1, 10, 6, color);
    M5Cardputer.Display.fillRect(x+10, y+2, 2, 4, color);
    // Заполнение по уровню (0-100%)
    int fillWidth = (level * 8) / 100;
    if (fillWidth > 0) {
        M5Cardputer.Display.fillRect(x+1, y+2, fillWidth, 4, color);
    }
}

// Иконка молнии ⚡ (зарядка)
void drawLightningIcon(int x, int y, uint16_t color) {
    int points[][2] = {{x+4,y}, {x+2,y+4}, {x+4,y+4}, {x+2,y+8}, {x+6,y+3}, {x+4,y+3}};
    for (int i = 0; i < 5; i++) {
        M5Cardputer.Display.drawLine(points[i][0], points[i][1], 
                                     points[i+1][0], points[i+1][1], color);
    }
}

// Иконка Loop 🔁
void drawLoopIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.drawCircle(x+3, y+3, 3, color);
    M5Cardputer.Display.fillTriangle(x+6, y+2, x+6, y+4, x+8, y+3, color);
    M5Cardputer.Display.fillTriangle(x, y+3, x, y+5, x+2, y+4, color);
}

// Иконка Shuffle 🔀
void drawShuffleIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.drawLine(x, y+2, x+4, y+6, color);
    M5Cardputer.Display.drawLine(x, y+6, x+4, y+2, color);
    M5Cardputer.Display.fillTriangle(x+5, y+1, x+5, y+3, x+7, y+2, color);
    M5Cardputer.Display.fillTriangle(x+5, y+5, x+5, y+7, x+7, y+6, color);
}

// Функция для отображения статуса с иконкой (слева)
void drawStatus(bool playing) {
    M5Cardputer.Display.fillRect(0, 120, 100, 15, BLACK);
    M5Cardputer.Display.setTextSize(1);
    
    if (playing) {
        drawPlayIcon(10, 120, GREEN);
        M5Cardputer.Display.setTextColor(GREEN);
        M5Cardputer.Display.setCursor(22, 120);
        M5Cardputer.Display.print("Playing");
    } else {
        drawPauseIcon(10, 120, YELLOW);
        M5Cardputer.Display.setTextColor(YELLOW);
        M5Cardputer.Display.setCursor(22, 120);
        M5Cardputer.Display.print("Paused");
    }
}

// Функция для отображения "Connected!" с иконкой
void drawConnected() {
    drawNoteIcon(10, 45, YELLOW);
    M5Cardputer.Display.setTextColor(YELLOW);
    M5Cardputer.Display.setCursor(22, 45);
    M5Cardputer.Display.print("Connected!");
}

// Функция для отображения временного сообщения справа
void drawTempMessage(String message, uint16_t color) {
    M5Cardputer.Display.fillRect(105, 120, 135, 15, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(color);
    M5Cardputer.Display.setCursor(105, 120);
    M5Cardputer.Display.print(message);
    
    // Запускаем таймер для очистки сообщения
    tempMessageTime = millis();
    tempMessageActive = true;
}

// Функция для очистки временного сообщения
void clearTempMessage() {
    if (tempMessageActive && (millis() - tempMessageTime > tempMessageDuration)) {
        M5Cardputer.Display.fillRect(105, 120, 135, 15, BLACK);
        tempMessageActive = false;
    }
}

// Функция для отображения режима эквалайзера
void drawEQMode() {
    // EQ режим справа на третьей строке
    M5Cardputer.Display.fillRect(165, 30, 75, 12, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(ORANGE);
    M5Cardputer.Display.setCursor(165, 30);
    M5Cardputer.Display.print(eqModeNames[currentEQMode]);
}

// Функция для переключения режима эквалайзера вверх
void eqModeUp() {
    currentEQMode = (currentEQMode + 1) % eqModeCount;
    applyEQMode();
    drawEQMode();
    drawStatus(isPlaying);
    
    char msg[30];
    sprintf(msg, "EQ: %s", eqModeNames[currentEQMode]);
    drawTempMessage(msg, ORANGE);
    
    Serial.printf("🎚️ EQ Mode: %s\n", eqModeNames[currentEQMode]);
}

// Функция для переключения режима эквалайзера вниз
void eqModeDown() {
    currentEQMode = (currentEQMode - 1 + eqModeCount) % eqModeCount;
    applyEQMode();
    drawEQMode();
    drawStatus(isPlaying);
    
    char msg[30];
    sprintf(msg, "EQ: %s", eqModeNames[currentEQMode]);
    drawTempMessage(msg, ORANGE);
    
    Serial.printf("🎚️ EQ Mode: %s\n", eqModeNames[currentEQMode]);
}

// Функция для применения режима эквалайзера
void applyEQMode() {
    // Unit AudioPlayer использует команды через UART
    // Команда EQ: 0x1A (26 decimal)
    // Формат: 0x7E 0xFF 0x06 0x1A 0x00 0x00 EQ_MODE 0xEF
    // EQ_MODE: 0=Normal, 1=Pop, 2=Rock, 3=Jazz, 4=Classical, 5=Bass
    
    // Маппинг наших режимов на команды модуля
    // 0=Rock->2, 1=Pop->1, 2=Jazz->3, 3=Classical->4, 4=Bass->5, 5=Normal->0
    uint8_t eqCommand = 0;
    switch(currentEQMode) {
        case 0: eqCommand = 2; break; // Rock
        case 1: eqCommand = 1; break; // Pop
        case 2: eqCommand = 3; break; // Jazz
        case 3: eqCommand = 4; break; // Classical
        case 4: eqCommand = 5; break; // Bass
        case 5: eqCommand = 0; break; // Normal
    }
    
    // Отправляем команду через Serial1
    uint8_t cmd[] = {0x7E, 0xFF, 0x06, 0x1A, 0x00, 0x00, eqCommand, 0xEF};
    Serial1.write(cmd, 8);
    Serial1.flush();
    
    Serial.printf("🎚️ Applied EQ command: %d\n", eqCommand);
}

void setup()
{
    M5Cardputer.begin();
    
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(BLACK);
    
    // Инициализируем random для визуализатора
    randomSeed(analogRead(0));
    
    // Рисуем красивый заголовок
    drawHeader();
    
    // Рисуем начальные индикаторы
    drawBatteryIndicator();
    drawVolumeIndicator();

    Serial.begin(115200);
    Serial.println("=== AudioPlayer v0.39c ===");
    Serial.println("PORT.A: G1(RX), G2(TX)");
    
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setCursor(10, 40);
    M5Cardputer.Display.print("Connecting...");
    
    while (!audioplayer.begin(&Serial1, 1, 2)) {
        Serial.println("Waiting for AudioPlayer...");
        delay(1000);
    }
    
    M5Cardputer.Display.fillRect(0, 40, 170, 80, BLACK);
    drawConnected();
    
    Serial.println("AudioPlayer is ready!");
    
    audioplayer.setVolume(currentVolume);
    audioplayer.setPlayMode(AUDIO_PLAYER_MODE_SINGLE_STOP); // Один трек и стоп
    
    // Применяем режим эквалайзера по умолчанию (Normal)
    delay(200);
    applyEQMode();
    
    // Получаем общее количество треков
    delay(500);
    totalTracks = audioplayer.getTotalAudioNumber();
    Serial.printf("Total tracks: %d\n", totalTracks);
    
    audioplayer.selectAudioNum(1);
    currentTrack = 1;
    lastDisplayedTrack = 1; // Инициализируем
    
    // Track (фиолетовый)
    M5Cardputer.Display.setTextColor(MAGENTA);
    M5Cardputer.Display.setCursor(10, 60);
    M5Cardputer.Display.print("Track: ");
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
    
    // Time (фиолетовый)
    M5Cardputer.Display.setTextColor(MAGENTA);
    M5Cardputer.Display.setCursor(10, 75);
    M5Cardputer.Display.print("Time: ");
    M5Cardputer.Display.setTextColor(CYAN);
    M5Cardputer.Display.print("00:00");
    
    // Подсказки кнопок (розовый для 1x,2x,3x)
    M5Cardputer.Display.setTextColor(0xF81F); // Розовый (Pink)
    M5Cardputer.Display.setCursor(10, 90);
    M5Cardputer.Display.print("1x:");
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.print("Play ");
    M5Cardputer.Display.setTextColor(0xF81F);
    M5Cardputer.Display.print("2x:");
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.print("Next ");
    M5Cardputer.Display.setTextColor(0xF81F);
    M5Cardputer.Display.print("3x:");
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.print("Prev");
    
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setCursor(10, 105);
    M5Cardputer.Display.print("+/-:Vol R:Rnd L:Lp S:Sh G:Go ");
    M5Cardputer.Display.setTextColor(ORANGE);
    M5Cardputer.Display.print("^v:");
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.print("EQ");
    
    // Отображаем режим эквалайзера
    drawEQMode();
    
    delay(1000);
}

void loop()
{
    M5Cardputer.update();

    // Обновляем индикаторы
    if (millis() - lastBatteryUpdate >= batteryUpdateInterval) {
        lastBatteryUpdate = millis();
        drawBatteryIndicator();
    }
    
    // Обновляем время воспроизведения (только если не в режиме ввода)
    if (!isInputMode && millis() - lastTimeUpdate >= timeUpdateInterval) {
        lastTimeUpdate = millis();
        updatePlayTime();
    }
    
    // Обновляем визуализатор (только если не в режиме ввода)
    if (!isInputMode && millis() - lastVisualizerUpdate >= visualizerUpdateInterval) {
        lastVisualizerUpdate = millis();
        updateVisualizer();
        drawVisualizer();
    }
    
    // Очищаем временное сообщение через 2 секунды
    clearTempMessage();

    // Проверяем клавиатуру для громкости и других команд
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            
            if (isInputMode) {
                // РЕЖИМ ВВОДА НОМЕРА ТРЕКА
                
                // Enter - подтверждение
                if (status.enter) {
                    if (inputTrackNumber.length() > 0) {
                        int trackNum = inputTrackNumber.toInt();
                        goToTrack(trackNum);
                    }
                    isInputMode = false;
                    inputTrackNumber = "";
                    
                    // Перерисовываем весь интерфейс
                    M5Cardputer.Display.clear();
                    drawHeader();
                    drawBatteryIndicator();
                    drawVolumeIndicator();
                    drawLoopIndicator();
                    
                    // Информация о треке
                    M5Cardputer.Display.setTextSize(1);
                    M5Cardputer.Display.setTextColor(MAGENTA);
                    M5Cardputer.Display.setCursor(10, 60);
                    M5Cardputer.Display.print("Track: ");
                    M5Cardputer.Display.setTextColor(WHITE);
                    M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
                    
                    M5Cardputer.Display.setTextColor(MAGENTA);
                    M5Cardputer.Display.setCursor(10, 75);
                    M5Cardputer.Display.print("Time: ");
                    M5Cardputer.Display.setTextColor(CYAN);
                    M5Cardputer.Display.print("00:00");
                    
                    // Подсказки (розовый для 1x,2x,3x)
                    M5Cardputer.Display.setTextColor(0xF81F);
                    M5Cardputer.Display.setCursor(10, 90);
                    M5Cardputer.Display.print("1x:");
                    M5Cardputer.Display.setTextColor(WHITE);
                    M5Cardputer.Display.print("Play ");
                    M5Cardputer.Display.setTextColor(0xF81F);
                    M5Cardputer.Display.print("2x:");
                    M5Cardputer.Display.setTextColor(WHITE);
                    M5Cardputer.Display.print("Next ");
                    M5Cardputer.Display.setTextColor(0xF81F);
                    M5Cardputer.Display.print("3x:");
                    M5Cardputer.Display.setTextColor(WHITE);
                    M5Cardputer.Display.print("Prev");
                    
                    M5Cardputer.Display.setTextColor(WHITE);
                    M5Cardputer.Display.setCursor(10, 105);
                    M5Cardputer.Display.print("+/-:Vol R:Rnd L:Lp S:Sh G:Go");
                    
                    // Восстанавливаем "Connected!"
                    drawConnected();
                    
                    // Восстанавливаем статус внизу
                    drawStatus(isPlaying);
                        
                        return;
                }
                
                // Del - удалить последнюю цифру
                if (status.del && inputTrackNumber.length() > 0) {
                    inputTrackNumber.remove(inputTrackNumber.length() - 1);
                    drawInputMode();
                    return;
                }
                
                // Проверяем клавиши
                for (auto key : status.word) {
                    // G - отмена
                    if (key == 'g' || key == 'G') {
                        isInputMode = false;
                        inputTrackNumber = "";
                        
                        // Перерисовываем весь интерфейс
                        M5Cardputer.Display.clear();
                        drawHeader();
                        drawBatteryIndicator();
                        drawVolumeIndicator();
                        drawLoopIndicator();
                        
                        // Информация о треке
                        M5Cardputer.Display.setTextSize(1);
                        M5Cardputer.Display.setTextColor(MAGENTA);
                        M5Cardputer.Display.setCursor(10, 60);
                        M5Cardputer.Display.print("Track: ");
                        M5Cardputer.Display.setTextColor(WHITE);
                        M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
                        
                        M5Cardputer.Display.setTextColor(MAGENTA);
                        M5Cardputer.Display.setCursor(10, 75);
                        M5Cardputer.Display.print("Time: ");
                        M5Cardputer.Display.setTextColor(CYAN);
                        M5Cardputer.Display.print("00:00");
                        
                        // Подсказки (розовый для 1x,2x,3x)
                        M5Cardputer.Display.setTextColor(0xF81F);
                        M5Cardputer.Display.setCursor(10, 90);
                        M5Cardputer.Display.print("1x:");
                        M5Cardputer.Display.setTextColor(WHITE);
                        M5Cardputer.Display.print("Play ");
                        M5Cardputer.Display.setTextColor(0xF81F);
                        M5Cardputer.Display.print("2x:");
                        M5Cardputer.Display.setTextColor(WHITE);
                        M5Cardputer.Display.print("Next ");
                        M5Cardputer.Display.setTextColor(0xF81F);
                        M5Cardputer.Display.print("3x:");
                        M5Cardputer.Display.setTextColor(WHITE);
                        M5Cardputer.Display.print("Prev");
                        
                        M5Cardputer.Display.setTextColor(WHITE);
                        M5Cardputer.Display.setCursor(10, 105);
                        M5Cardputer.Display.print("+/-:Vol R:Rnd L:Lp S:Sh G:Go");
                        
                        // Восстанавливаем "Connected!"
                        drawConnected();
                        
                        // Восстанавливаем статус внизу
                        drawStatus(isPlaying);
                        
                        Serial.println("Input cancelled");
                        return;
                    }
                    
                    // Цифры 0-9
                    if (key >= '0' && key <= '9') {
                        if (inputTrackNumber.length() < 5) { // Максимум 5 цифр
                            inputTrackNumber += key;
                            drawInputMode();
                        }
                    }
                }
            } else {
                // ОБЫЧНЫЙ РЕЖИМ
                
                // Проверяем клавиши
                for (auto key : status.word) {
                    if (key == '+' || key == '=') {
                        volumeUp();
                    }
                    else if (key == '-') {
                        volumeDown();
                    }
                    else if (key == 'r' || key == 'R') {
                        randomTrack();
                    }
                    else if (key == 'l' || key == 'L') {
                        toggleLoop();
                    }
                    else if (key == 's' || key == 'S') {
                        toggleShuffle();
                    }
                    else if (key == 'g' || key == 'G') {
                        // Включаем режим ввода
                        isInputMode = true;
                        inputTrackNumber = "";
                        drawInputMode();
                        Serial.println("Input mode activated");
                    }
                    else if (key == ';') {
                        // Стрелка вверх - следующий режим EQ
                        eqModeUp();
                    }
                    else if (key == '.') {
                        // Стрелка вниз - предыдущий режим EQ
                        eqModeDown();
                    }
                    else if (key == ',') {
                        // Стрелка влево - предыдущий трек (секретная кнопка)
                        executeAction(3);
                        Serial.println("⬅️ Secret: Previous track");
                    }
                    else if (key == '/') {
                        // Стрелка вправо - следующий трек (секретная кнопка)
                        executeAction(2);
                        Serial.println("➡️ Secret: Next track");
                    }
                }
                
                // Пробел - Play/Pause (секретная кнопка)
                if (status.space) {
                    executeAction(1);
                    Serial.println("⏯️ Secret: Play/Pause");
                }
            }
        }
    }

    // Проверяем статус воспроизведения для автопереключения (только если не в режиме ввода)
    if (!isInputMode) {
        checkAutoNext();
    }

    // Обработка кнопки A (только если не в режиме ввода)
    if (!isInputMode) {
        if (M5Cardputer.BtnA.wasPressed()) {
            unsigned long currentTime = millis();
            
            if (currentTime - lastClickTime < clickTimeout) {
                clickCount++;
            } else {
                clickCount = 1;
            }
            
            lastClickTime = currentTime;
            Serial.printf("Click count: %d\n", clickCount);
        }
        
        // Проверяем timeout для выполнения действия
        if (clickCount > 0 && (millis() - lastClickTime > clickTimeout)) {
            executeAction(clickCount);
            clickCount = 0;
        }
    }
}

void drawHeader() {
    // Зеленый текст "Unit AudioPlayer"
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(8, 6);
    M5Cardputer.Display.print("Unit AudioPlayer");
    
    // Версия на второй строке: "AndyShuffle v0.39c"
    M5Cardputer.Display.setTextColor(CYAN);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(8, 18);
    M5Cardputer.Display.print("AndyShuffle ");
    M5Cardputer.Display.setTextColor(YELLOW);
    M5Cardputer.Display.print("v0.39c");
}

void updatePlayTime() {
    if (!isPlaying) return;
    
    // Вычисляем прошедшее время
    unsigned long elapsedTime = (millis() - trackStartTime - totalPausedTime) / 1000;
    int minutes = elapsedTime / 60;
    int seconds = elapsedTime % 60;
    
    // Очищаем ВСЮ строку времени
    M5Cardputer.Display.fillRect(10, 75, 150, 12, BLACK);
    
    // Рисуем заново ВСЮ строку
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(MAGENTA, BLACK); // Фиолетовый
    M5Cardputer.Display.setCursor(10, 75);
    M5Cardputer.Display.print("Time: ");
    
    M5Cardputer.Display.setTextColor(CYAN, BLACK);
    M5Cardputer.Display.printf("%02d:%02d", minutes, seconds);
}

void volumeUp() {
    if (currentVolume < 30) {
        currentVolume++;
        audioplayer.setVolume(currentVolume);
        drawVolumeIndicator();
        
        // Показываем уведомление
        drawStatus(isPlaying);  // Обновляем статус слева
        char msg[20];
        sprintf(msg, "Vol UP: %d%%", (currentVolume * 100) / 30);
        drawTempMessage(msg, GREEN);
        
        Serial.printf("🔊 Volume UP: %d/30 (%d%%)\n", currentVolume, (currentVolume * 100) / 30);
    }
}

void volumeDown() {
    if (currentVolume > 0) {
        currentVolume--;
        audioplayer.setVolume(currentVolume);
        drawVolumeIndicator();
        
        // Показываем уведомление
        drawStatus(isPlaying);  // Обновляем статус слева
        char msg[20];
        sprintf(msg, "Vol DOWN: %d%%", (currentVolume * 100) / 30);
        drawTempMessage(msg, YELLOW);
        
        Serial.printf("🔉 Volume DOWN: %d/30 (%d%%)\n", currentVolume, (currentVolume * 100) / 30);
    }
}

void drawInputMode() {
    // Полностью очищаем область ввода
    M5Cardputer.Display.fillRect(0, 35, 240, 95, BLACK);
    
    // Рисуем окно
    M5Cardputer.Display.fillRect(20, 40, 200, 50, BLACK);
    M5Cardputer.Display.drawRect(20, 40, 200, 50, CYAN);
    M5Cardputer.Display.drawRect(21, 41, 198, 48, CYAN);
    
    // Заголовок
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(CYAN);
    M5Cardputer.Display.setCursor(30, 48);
    M5Cardputer.Display.print("Go to track:");
    
    // Введенное число
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(YELLOW);
    M5Cardputer.Display.setCursor(30, 65);
    
    if (inputTrackNumber.length() > 0) {
        M5Cardputer.Display.print(inputTrackNumber);
    } else {
        M5Cardputer.Display.setTextColor(DARKGREY);
        M5Cardputer.Display.print("___");
    }
    
    // Подсказка
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setCursor(25, 100);
    M5Cardputer.Display.print("Enter:OK Del:Back G:Cancel");
}

void toggleLoop() {
    isLoopEnabled = !isLoopEnabled;
    
    // Если Loop включается, выключаем Shuffle
    if (isLoopEnabled && isShuffleEnabled) {
        isShuffleEnabled = false;
        Serial.println("Shuffle disabled (Loop enabled)");
    }
    
    drawLoopIndicator();
    drawStatus(isPlaying);  // Обновляем статус справа
    
    if (isLoopEnabled) {
        drawTempMessage("Loop: ON", YELLOW);
        Serial.println("Loop enabled");
    } else {
        drawTempMessage("Loop: OFF", YELLOW);
        Serial.println("Loop disabled");
    }
}

void toggleShuffle() {
    isShuffleEnabled = !isShuffleEnabled;
    
    // Если Shuffle включается, выключаем Loop
    if (isShuffleEnabled && isLoopEnabled) {
        isLoopEnabled = false;
        Serial.println("Loop disabled (Shuffle enabled)");
    }
    
    drawLoopIndicator();
    drawStatus(isPlaying);  // Обновляем статус справа
    
    if (isShuffleEnabled) {
        drawTempMessage("Shuffle: ON", CYAN);
        Serial.println("Shuffle enabled");
    } else {
        drawTempMessage("Shuffle: OFF", CYAN);
        Serial.println("Shuffle disabled");
    }
}

void goToTrack(int trackNum) {
    if (trackNum < 1 || trackNum > totalTracks) {
        // Неверный номер трека
        M5Cardputer.Display.fillRect(0, 120, 240, 15, BLACK);
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setTextColor(RED);
        M5Cardputer.Display.setCursor(10, 120);
        M5Cardputer.Display.printf("Invalid! (1-%d)", totalTracks);
        
        Serial.printf("Invalid track: %d (must be 1-%d)\n", trackNum, totalTracks);
        delay(1500);
        
        // Очищаем сообщение
        M5Cardputer.Display.fillRect(0, 120, 240, 15, BLACK);
        return;
    }
    
    // Переходим на трек
    audioplayer.selectAudioNum(trackNum);
    delay(100);
    audioplayer.playAudio();
    
    M5Cardputer.Display.fillRect(0, 120, 240, 15, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(CYAN);
    M5Cardputer.Display.setCursor(10, 120);
    M5Cardputer.Display.printf("Go to track: %d", trackNum);
    
    Serial.printf("Go to track: %d\n", trackNum);
}

void randomTrack() {
    if (totalTracks == 0) return;
    
    // Генерируем случайный номер трека (1 до totalTracks)
    uint16_t randomNum = random(1, totalTracks + 1);
    
    Serial.printf("🎲 Random track: %d\n", randomNum);
    
    audioplayer.selectAudioNum(randomNum);
    currentTrack = randomNum;
    
    // Сбрасываем таймер
    trackStartTime = millis();
    totalPausedTime = 0;
    isPlaying = (lastPlayStatus == AUDIO_PLAYER_STATUS_PLAYING);
    
    // Если было воспроизведение - запускаем случайный трек
    if (lastPlayStatus == AUDIO_PLAYER_STATUS_PLAYING) {
        audioplayer.playAudio();
    }
    
    // Обновляем информацию о треке
    M5Cardputer.Display.fillRect(10, 60, 160, 15, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(MAGENTA);
    M5Cardputer.Display.setCursor(10, 60);
    M5Cardputer.Display.print("Track: ");
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
    
    M5Cardputer.Display.fillRect(0, 120, 170, 15, BLACK);
    M5Cardputer.Display.setTextColor(MAGENTA);
    M5Cardputer.Display.setCursor(10, 120);
    M5Cardputer.Display.print("Random!");
    
    Serial.printf("Random track: %d/%d\n", currentTrack, totalTracks);
}

void drawBatteryIndicator() {
    // Получаем данные о батарее
    int batteryLevel = M5Cardputer.Power.getBatteryLevel();
    int batteryVoltage = M5Cardputer.Power.getBatteryVoltage();
    bool isCharging = (batteryVoltage > 4200);
    
    // Позиция в правом верхнем углу
    int x = 182;
    int y = 2;
    
    // Очищаем область
    M5Cardputer.Display.fillRect(x, y, 58, 22, BLACK);
    
    // Рисуем батарею
    M5Cardputer.Display.drawRect(x, y, 38, 16, WHITE);
    M5Cardputer.Display.fillRect(x + 38, y + 5, 3, 6, WHITE); // Контакт
    
    // Заливка по уровню
    int fillWidth = (batteryLevel * 34) / 100;
    
    uint16_t color;
    if (batteryLevel > 60) {
        color = GREEN;
    } else if (batteryLevel > 20) {
        color = YELLOW;
    } else {
        color = RED;
    }
    
    if (fillWidth > 0) {
        M5Cardputer.Display.fillRect(x + 2, y + 2, fillWidth, 12, color);
    }
    
    // Иконка зарядки
    if (isCharging) {
        M5Cardputer.Display.setTextColor(YELLOW, color);
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setCursor(x + 15, y + 4);
        M5Cardputer.Display.print("~");
    }
    
    // Процент под батареей
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(x + 9, y + 18);
    M5Cardputer.Display.printf("%d%%", batteryLevel);
    
    Serial.printf("Battery: %d%% | %.2fV | %s\n", 
                  batteryLevel, 
                  batteryVoltage / 1000.0,
                  isCharging ? "Charging" : "On Battery");
}

void drawLoopIndicator() {
    // Индикатор Loop/Shuffle справа на четвертой строке (под EQ)
    M5Cardputer.Display.fillRect(165, 42, 75, 12, BLACK);
    M5Cardputer.Display.setTextSize(1);
    
    if (isLoopEnabled) {
        drawLoopIcon(165, 42, YELLOW);
        M5Cardputer.Display.setTextColor(YELLOW, BLACK);
        M5Cardputer.Display.setCursor(177, 42);
        M5Cardputer.Display.print("LOOP");
    } else if (isShuffleEnabled) {
        drawShuffleIcon(165, 42, CYAN);
        M5Cardputer.Display.setTextColor(CYAN, BLACK);
        M5Cardputer.Display.setCursor(177, 42);
        M5Cardputer.Display.print("SHUFFLE");
    }
}

void drawVisualizer() {
    // Визуализатор под "Connected!" слева
    // Позиция: x=75, y=55, 5 баров по 6px ширина, 2px промежуток
    int startX = 75;  // Справа от "Connected!"
    int startY = 55;  // На той же высоте что и Connected
    int barWidth = 6;
    int barGap = 2;
    int maxBarHeight = 25;
    
    for (int i = 0; i < 5; i++) {
        int x = startX + i * (barWidth + barGap);
        
        // Очищаем область бара
        M5Cardputer.Display.fillRect(x, startY - maxBarHeight, barWidth, maxBarHeight, BLACK);
        
        // Рисуем бар снизу вверх
        if (barHeights[i] > 0) {
            // Цвет зависит от частоты (позиции бара)
            uint16_t color;
            
            // Бары 0-1: Низкие частоты (басы) - красный/оранжевый
            if (i == 0) {
                color = 0xF800; // Красный (басы)
            } else if (i == 1) {
                color = 0xFD20; // Оранжевый
            }
            // Бары 2-3: Средние частоты - желтый/зеленый
            else if (i == 2) {
                color = 0xFFE0; // Желтый
            } else if (i == 3) {
                color = GREEN; // Зеленый
            }
            // Бар 4: Высокие частоты - голубой
            else {
                color = CYAN; // Голубой (высокие частоты)
            }
            
            M5Cardputer.Display.fillRect(x, startY - barHeights[i], barWidth, barHeights[i], color);
        }
    }
}

void updateVisualizer() {
    // Обновляем высоту баров только если играет музыка
    if (!isPlaying) {
        // Если не играет - обнуляем бары
        for (int i = 0; i < 5; i++) {
            barHeights[i] = 0;
        }
        return;
    }
    
    // Генерируем "умные" случайные высоты, имитирующие спектр
    // Высоты увеличены в 2.5 раза (было 2-10px, стало 5-25px)
    
    // Бары 0-1: Низкие частоты (басы) - обычно выше и стабильнее
    barHeights[0] = random(15, 26);  // 15-25px (высокие)
    barHeights[1] = random(12, 25);  // 12-24px
    
    // Бары 2-3: Средние частоты - средняя высота, более динамичные
    barHeights[2] = random(10, 22);  // 10-21px
    barHeights[3] = random(8, 20);   // 8-19px
    
    // Бар 4: Высокие частоты - ниже, быстро меняются
    barHeights[4] = random(5, 17);   // 5-16px (низкие)
}

void drawVolumeIndicator() {
    // Позиция на второй строке после версии
    int x = 120;
    int y = 18;
    
    // Очищаем область
    M5Cardputer.Display.fillRect(x, y, 60, 12, BLACK);
    
    // Рисуем "Vol:" для Volume
    M5Cardputer.Display.setTextColor(CYAN);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(x, y);
    M5Cardputer.Display.print("Vol:");
    
    // Процент громкости
    int volumePercent = (currentVolume * 100) / 30;
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.printf("%d%%", volumePercent);
    
    Serial.printf("Volume: %d/30 (%d%%)\n", currentVolume, volumePercent);
}

void checkAutoNext() {
    static unsigned long lastCheck = 0;
    
    // Проверяем каждые 500ms
    if (millis() - lastCheck < 500) {
        return;
    }
    lastCheck = millis();
    
    uint8_t currentStatus = audioplayer.checkPlayStatus();
    
    // 1. Автопереключение треков или повтор
    if (lastPlayStatus == AUDIO_PLAYER_STATUS_PLAYING && 
        currentStatus == AUDIO_PLAYER_STATUS_STOPPED) {
        
        if (isLoopEnabled) {
            // Режим Loop - повторяем текущий трек
            Serial.println("Track ended - Loop repeat");
            
            delay(100);
            audioplayer.playAudio();
            delay(100);
            
            // Сбрасываем таймер для повтора
            trackStartTime = millis();
            totalPausedTime = 0;
            isPlaying = true;  // ВАЖНО: устанавливаем флаг воспроизведения!
            
            // Сбрасываем время на экране
            M5Cardputer.Display.fillRect(10, 75, 150, 12, BLACK);
            M5Cardputer.Display.setTextColor(WHITE, BLACK);
            M5Cardputer.Display.setCursor(10, 75);
            M5Cardputer.Display.print("Time: ");
            M5Cardputer.Display.setTextColor(CYAN, BLACK);
            M5Cardputer.Display.print("00:00");
            
            drawStatus(true);  // Показываем статус Playing слева
            drawTempMessage("Loop repeat", YELLOW);  // Сообщение справа
        } else if (isShuffleEnabled) {
            // Режим Shuffle - случайный трек
            Serial.println("Track ended - Shuffle next");
            
            uint16_t randomNum = random(1, totalTracks + 1);
            audioplayer.selectAudioNum(randomNum);
            delay(200);
            audioplayer.playAudio();
            delay(100);
            
            // Сбрасываем таймер сразу
            trackStartTime = millis();
            totalPausedTime = 0;
            isPlaying = true;
            
            drawStatus(true);  // Показываем статус Playing слева
            drawTempMessage("Shuffle next", CYAN);  // Сообщение справа
            
            Serial.printf("Shuffle to track: %d\n", randomNum);
        } else {
            // Обычный режим - следующий трек
            Serial.println("Track ended - Auto next");
            
            audioplayer.nextAudio();
            delay(200);
            audioplayer.playAudio();
            delay(100);
            
            // Сбрасываем таймер сразу
            trackStartTime = millis();
            totalPausedTime = 0;
            isPlaying = true;
            
            drawStatus(true);  // Показываем статус Playing слева
            drawTempMessage("Auto next", WHITE);  // Сообщение справа
            
            Serial.println("Next track started");
        }
    }
    
    // 2. Получаем текущий номер трека от модуля
    uint16_t realTrackNumber = audioplayer.getCurrentAudioNumber();
    
    // 3. Проверяем валидность номера трека (должен быть от 1 до totalTracks)
    if (realTrackNumber < 1 || realTrackNumber > totalTracks) {
        // Невалидное значение - игнорируем!
        Serial.printf("Invalid track number: %d (ignored)\n", realTrackNumber);
        return;
    }
    
    // 4. Если номер трека изменился - обновляем экран и сбрасываем таймер!
    if (realTrackNumber != lastDisplayedTrack) {
        Serial.printf("Track changed: %d -> %d\n", lastDisplayedTrack, realTrackNumber);
        
        currentTrack = realTrackNumber;
        lastDisplayedTrack = realTrackNumber;
        
        // ОТКЛЮЧАЕМ LOOP только если трек реально изменился (не Loop повтор)
        // Loop повтор не меняет номер трека, поэтому мы сюда не попадем
        if (isLoopEnabled) {
            isLoopEnabled = false;
            drawLoopIndicator(); // Убираем индикатор
            Serial.println("Loop disabled (track changed)");
        }
        
        // СБРОС ТАЙМЕРА при смене трека
        trackStartTime = millis();
        totalPausedTime = 0;
        
        // Обновляем номер трека на экране
        M5Cardputer.Display.fillRect(10, 60, 160, 15, BLACK);
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setTextColor(MAGENTA);
        M5Cardputer.Display.setCursor(10, 60);
        M5Cardputer.Display.print("Track: ");
        M5Cardputer.Display.setTextColor(WHITE);
        M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
        
        // Сбрасываем время на экране
        M5Cardputer.Display.fillRect(10, 75, 150, 12, BLACK);
        M5Cardputer.Display.setTextColor(MAGENTA, BLACK);
        M5Cardputer.Display.setCursor(10, 75);
        M5Cardputer.Display.print("Time: ");
        M5Cardputer.Display.setTextColor(CYAN, BLACK);
        M5Cardputer.Display.print("00:00");
        
        Serial.printf("Display updated: Track %d/%d, Timer reset\n", currentTrack, totalTracks);
    }
    
    // 4. Отслеживаем статус для паузы/воспроизведения
    if (lastPlayStatus != currentStatus) {
        if (currentStatus == AUDIO_PLAYER_STATUS_PLAYING) {
            if (lastPlayStatus == AUDIO_PLAYER_STATUS_PAUSED) {
                // Возобновление с паузы
                totalPausedTime += (millis() - pausedTime);
            }
            isPlaying = true;
        } else if (currentStatus == AUDIO_PLAYER_STATUS_PAUSED) {
            pausedTime = millis();
            isPlaying = false;
        } else if (currentStatus == AUDIO_PLAYER_STATUS_STOPPED) {
            isPlaying = false;
        }
    }
    
    lastPlayStatus = currentStatus;
}

void executeAction(int clicks) {
    if (clicks == 1) {
        Serial.println("Action: Play/Pause");
        
        uint8_t playStatus = audioplayer.checkPlayStatus();
        
        if (playStatus == AUDIO_PLAYER_STATUS_PLAYING) {
            audioplayer.pauseAudio();
            pausedTime = millis();
            isPlaying = false;
            drawStatus(false);
            Serial.println("Paused");
        } else {
            audioplayer.playAudio();
            if (playStatus == AUDIO_PLAYER_STATUS_STOPPED) {
                // Начало нового трека
                trackStartTime = millis();
                totalPausedTime = 0;
            } else {
                // Возобновление
                totalPausedTime += (millis() - pausedTime);
            }
            isPlaying = true;
            drawStatus(true);
            Serial.println("Playing");
        }
    }
    else if (clicks == 2) {
        Serial.println("Action: Next track");
        
        audioplayer.nextAudio();
        delay(100);
        audioplayer.playAudio();
        
        drawStatus(isPlaying);  // Обновляем статус слева
        drawTempMessage("Next track", WHITE);  // Сообщение справа
        
        Serial.println("Next track command sent");
        // checkAutoNext() обновит номер трека и таймер
    }
    else if (clicks == 3) {
        Serial.println("Action: Previous track");
        
        audioplayer.previousAudio();
        delay(100);
        audioplayer.playAudio();
        
        drawStatus(isPlaying);  // Обновляем статус слева
        drawTempMessage("Prev track", WHITE);  // Сообщение справа
        
        Serial.println("Previous track command sent");
        // checkAutoNext() обновит номер трека и таймер
    }
    else {
        Serial.printf("Ignored: %d clicks\n", clicks);
        M5Cardputer.Display.print("Too many clicks!");
    }
}
