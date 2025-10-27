/*
 * 🎵 Unit AudioPlayer AndyShuffle для M5Stack Cardputer v4.29.1 🍀
 * С информацией о треках, времени, случайным выбором и зацикливанием
 * Новое: Terminal Splash заставка 💚 + Matrix Rain визуализация 💚
 * 3 режима визуализации: Bars → Waves → Matrix!
 * Made by Andy+AI
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
uint16_t lastDisplayedTrack = 1;
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
const unsigned long timeUpdateInterval = 1000;
int currentVolume = 20;

// Умная логика для батареи (как в battery_test_v2_1)
int lastValidBatteryLevel = -1;
bool wasCharging = false;
unsigned long chargingStartTime = 0;

// Переменные для визуализации
unsigned long lastVisualizerUpdate = 0;
const unsigned long visualizerUpdateInterval = 150;
int barHeights[5] = {0, 0, 0, 0, 0};

// Полноэкранная визуализация
bool isFullscreenVisualizer = false;
int visualizerMode = 0; // 0 = выкл, 1 = столбики, 2 = волны, 3 = matrix
int fullscreenBarHeights[20] = {0}; // 20 баров для полного экрана

// Волны для второго режима визуализации
float wavePhase = 0.0;
int waveAmplitudes[240] = {0}; // Массив для волны по ширине экрана

// Matrix Rain визуализация (режим 3)
struct MatrixColumn {
    int y;              // текущая позиция Y
    int speed;          // скорость падения (1-3)
    char character;     // текущий символ
    int brightness;     // яркость (0-255)
    int trailLength;    // длина следа
};
MatrixColumn matrixColumns[40]; // 240px / 6px = 40 столбцов
const char matrixChars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz@#$%&*+-=<>[]{}()";
unsigned long lastMatrixUpdate = 0;
const unsigned long matrixUpdateInterval = 80; // 80ms = ~12 FPS
int matrixNoteCounter = 0; // счетчик для музыкальных символов

// Terminal Splash Screen
bool splashShown = false;
unsigned long splashStartTime = 0;

// Экран информации
bool isInfoScreen = false;

// Переменные для временного сообщения
unsigned long tempMessageTime = 0;
bool tempMessageActive = false;
const unsigned long tempMessageDuration = 2000;
String tempMessageText = "";
uint16_t tempMessageColor = WHITE;

// ========== ИКОНКИ ==========

void drawNoteIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.fillCircle(x+2, y+6, 2, color);
    M5Cardputer.Display.fillRect(x+4, y+1, 2, 6, color);
    M5Cardputer.Display.fillRect(x+4, y+1, 3, 2, color);
}

void drawPauseIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.fillRect(x+1, y+1, 2, 6, color);
    M5Cardputer.Display.fillRect(x+5, y+1, 2, 6, color);
}

void drawPlayIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.fillTriangle(x+1, y+1, x+1, y+7, x+7, y+4, color);
}

void drawSpeakerIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.fillRect(x+1, y+2, 2, 4, color);
    M5Cardputer.Display.fillTriangle(x+3, y+1, x+3, y+7, x+6, y+4, color);
    M5Cardputer.Display.drawLine(x+7, y+2, x+8, y+1, color);
    M5Cardputer.Display.drawLine(x+7, y+6, x+8, y+7, color);
}

void drawBatteryIcon(int x, int y, uint16_t color, int level) {
    M5Cardputer.Display.drawRect(x, y+1, 10, 6, color);
    M5Cardputer.Display.fillRect(x+10, y+2, 2, 4, color);
    int fillWidth = (level * 8) / 100;
    if (fillWidth > 0) {
        M5Cardputer.Display.fillRect(x+1, y+2, fillWidth, 4, color);
    }
}

void drawLightningIcon(int x, int y, uint16_t color) {
    int points[][2] = {{x+4,y}, {x+2,y+4}, {x+4,y+4}, {x+2,y+8}, {x+6,y+3}, {x+4,y+3}};
    for (int i = 0; i < 5; i++) {
        M5Cardputer.Display.drawLine(points[i][0], points[i][1], 
                                     points[i+1][0], points[i+1][1], color);
    }
}

void drawLoopIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.drawCircle(x+3, y+3, 3, color);
    M5Cardputer.Display.fillTriangle(x+6, y+2, x+6, y+4, x+8, y+3, color);
    M5Cardputer.Display.fillTriangle(x, y+3, x, y+5, x+2, y+4, color);
}

void drawShuffleIcon(int x, int y, uint16_t color) {
    M5Cardputer.Display.drawLine(x, y+2, x+4, y+6, color);
    M5Cardputer.Display.drawLine(x, y+6, x+4, y+2, color);
    M5Cardputer.Display.fillTriangle(x+5, y+1, x+5, y+3, x+7, y+2, color);
    M5Cardputer.Display.fillTriangle(x+5, y+5, x+5, y+7, x+7, y+6, color);
}

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

void drawConnected() {
    drawNoteIcon(10, 45, YELLOW);
    M5Cardputer.Display.setTextColor(YELLOW);
    M5Cardputer.Display.setCursor(22, 45);
    M5Cardputer.Display.print("Connected!");
}

void drawTempMessage(String message, uint16_t color) {
    M5Cardputer.Display.fillRect(105, 120, 135, 15, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(color);
    M5Cardputer.Display.setCursor(105, 120);
    M5Cardputer.Display.print(message);
    
    // Сохраняем для полноэкранного режима
    tempMessageText = message;
    tempMessageColor = color;
    tempMessageTime = millis();
    tempMessageActive = true;
}

void clearTempMessage() {
    if (tempMessageActive && (millis() - tempMessageTime > tempMessageDuration)) {
        M5Cardputer.Display.fillRect(105, 120, 135, 15, BLACK);
        tempMessageActive = false;
        
        // Восстанавливаем строку "Press I for Info"
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setTextColor(YELLOW, BLACK);
        M5Cardputer.Display.setCursor(10, 110);
        M5Cardputer.Display.print("Press I for Info");
    }
}

// ========== TERMINAL SPLASH SCREEN - ANDY+AI LOGO ==========

void drawAndyLogo() {
    // Полная отрисовка логотипа
    
    // Красная рамка
    for (int i = 0; i < 3; i++) {
        M5Cardputer.Display.drawRect(10 + i, 10 + i, 220 - (i*2), 115 - (i*2), RED);
    }
    
    // Зеленый лист (упрощенная версия) - центр экрана
    int cx = 120; // центр X
    int cy = 50;  // центр Y
    uint16_t green = GREEN;
    
    // Центральный лепесток (вверх)
    M5Cardputer.Display.fillTriangle(cx, cy-20, cx-6, cy, cx+6, cy, green);
    
    // Левый верхний лепесток
    M5Cardputer.Display.fillTriangle(cx-12, cy-15, cx-18, cy-8, cx-8, cy-2, green);
    
    // Правый верхний лепесток
    M5Cardputer.Display.fillTriangle(cx+12, cy-15, cx+18, cy-8, cx+8, cy-2, green);
    
    // Левый средний лепесток
    M5Cardputer.Display.fillTriangle(cx-18, cy, cx-28, cy+5, cx-12, cy+6, green);
    
    // Правый средний лепесток
    M5Cardputer.Display.fillTriangle(cx+18, cy, cx+28, cy+5, cx+12, cy+6, green);
    
    // Левый нижний лепесток
    M5Cardputer.Display.fillTriangle(cx-12, cy+6, cx-16, cy+14, cx-6, cy+12, green);
    
    // Правый нижний лепесток  
    M5Cardputer.Display.fillTriangle(cx+12, cy+6, cx+16, cy+14, cx+6, cy+12, green);
    
    // Стебель
    M5Cardputer.Display.fillRect(cx-2, cy+12, 4, 10, green);
    
    // Текст "Andy + AI" (желтый, pixelated стиль)
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    int textX = 60;
    int textY = 95;
    M5Cardputer.Display.setCursor(textX, textY);
    M5Cardputer.Display.print("Andy + AI");
}

void showTerminalSplash() {
    M5Cardputer.Display.fillScreen(BLACK);
    
    splashStartTime = millis();
    bool skipRequested = false;
    
    // Построчная прорисовка - терминальный стиль!
    // Рисуем блоками по 7 строк для скорости
    int linesPerBlock = 7;
    int totalLines = 125; // высота логотипа
    int currentLine = 0;
    
    while (currentLine < totalLines && !skipRequested) {
        // Черный фон
        M5Cardputer.Display.fillScreen(BLACK);
        
        // Рисуем логотип до текущей линии
        // Используем clipping через fillRect для создания эффекта "сканирования"
        
        // Сначала рисуем весь логотип
        drawAndyLogo();
        
        // Затем закрываем черным прямоугольником то, что еще не "отрисовалось"
        if (currentLine < totalLines) {
            M5Cardputer.Display.fillRect(0, 10 + currentLine, 240, totalLines - currentLine, BLACK);
        }
        
        // Линия сканирования (зеленая, как в терминале)
        if (currentLine < totalLines - 2) {
            M5Cardputer.Display.drawLine(10, 10 + currentLine, 230, 10 + currentLine, GREEN);
            M5Cardputer.Display.drawLine(10, 10 + currentLine + 1, 230, 10 + currentLine + 1, 0x07E0); // Светло-зеленый
        }
        
        // Подсказка
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setTextColor(0x7BEF, BLACK); // серый
        M5Cardputer.Display.setCursor(50, 127);
        M5Cardputer.Display.print("Press any key to skip");
        
        // Проверка на пропуск
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
            skipRequested = true;
        }
        
        currentLine += linesPerBlock;
        delay(35); // 35ms между блоками = быстро и энергично!
    }
    
    if (skipRequested) {
        M5Cardputer.Display.fillScreen(BLACK);
        splashShown = true;
        return;
    }
    
    // Финальная отрисовка без линии сканирования
    M5Cardputer.Display.fillScreen(BLACK);
    drawAndyLogo();
    
    // Подсказка
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(0x7BEF, BLACK);
    M5Cardputer.Display.setCursor(50, 127);
    M5Cardputer.Display.print("Press any key to skip");
    
    // Пауза для просмотра (можно пропустить)
    unsigned long pauseStart = millis();
    while (millis() - pauseStart < 700 && !skipRequested) {
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
            skipRequested = true;
        }
        delay(50);
    }
    
    // Очистка
    M5Cardputer.Display.fillScreen(BLACK);
    
    splashShown = true;
}

void setup()
{
    M5Cardputer.begin();
    
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(BLACK);
    
    randomSeed(analogRead(0));
    
    // Показываем Terminal заставку!
    if (!splashShown) {
        showTerminalSplash();
    }
    
    drawHeader();
    drawBatteryIndicator();
    drawVolumeIndicator();

    Serial.begin(115200);
    Serial.println("=== AudioPlayer v4.21 🍀 ===");
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
    audioplayer.setPlayMode(AUDIO_PLAYER_MODE_SINGLE_STOP);
    
    delay(500);
    totalTracks = audioplayer.getTotalAudioNumber();
    Serial.printf("Total tracks: %d\n", totalTracks);
    
    audioplayer.selectAudioNum(1);
    currentTrack = 1;
    lastDisplayedTrack = 1;
    
    // Track (увеличенный шрифт)
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(MAGENTA);
    M5Cardputer.Display.setCursor(10, 60);
    M5Cardputer.Display.print("Track:");
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
    
    // Time (увеличенный шрифт)
    M5Cardputer.Display.setTextColor(MAGENTA);
    M5Cardputer.Display.setCursor(10, 80);
    M5Cardputer.Display.print("Time:");
    M5Cardputer.Display.setTextColor(CYAN);
    M5Cardputer.Display.print("00:00");
    
    // Только подсказка INFO
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(10, 110);
    M5Cardputer.Display.print("Press I for Info");
    
    delay(1000);
}

void loop()
{
    M5Cardputer.update();

    // Экран информации - отдельная логика
    if (isInfoScreen) {
        // Проверяем клавиатуру для выхода
        if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isPressed()) {
                Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
                
                for (auto key : status.word) {
                    // I или ESC для выхода
                    if (key == 'i' || key == 'I' || key == '`' || key == '~') {
                        exitInfoScreen();
                        return;
                    }
                }
            }
        }
        
        // В INFO режиме продолжаем проверять автопереключение треков
        checkAutoNext();
        return;
    }

    // Полноэкранная визуализация - отдельная логика
    if (isFullscreenVisualizer) {
        // Обновляем визуализатор
        if (millis() - lastVisualizerUpdate >= visualizerUpdateInterval) {
            lastVisualizerUpdate = millis();
            updateFullscreenVisualizer();
            drawFullscreenVisualizer();
        }
        
        // Очищаем временное сообщение
        if (tempMessageActive && (millis() - tempMessageTime > tempMessageDuration)) {
            drawFullscreenTempMessage("", WHITE); // Очистка
            tempMessageActive = false;
        }
        
        // Проверяем клавиатуру для управления
        if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isPressed()) {
                Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
                
                for (auto key : status.word) {
                    // V - переключение режимов визуализации
                    if (key == 'v' || key == 'V') {
                        switchVisualizerMode();
                        return;
                    }
                    // ESC - выход
                    else if (key == '`' || key == '~') {
                        exitFullscreenVisualizer();
                        return;
                    }
                    else if (key == '+' || key == '=') {
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
                    else if (key == ';') {
                        int newTrack = currentTrack + 10;
                        if (newTrack > totalTracks) newTrack = totalTracks;
                        goToTrack(newTrack);
                        Serial.println("⬆️ Secret: +10 tracks");
                    }
                    else if (key == '.') {
                        int newTrack = currentTrack - 10;
                        if (newTrack < 1) newTrack = 1;
                        goToTrack(newTrack);
                        Serial.println("⬇️ Secret: -10 tracks");
                    }
                    else if (key == ',') {
                        executeAction(3);
                        Serial.println("⬅️ Secret: Previous track");
                    }
                    else if (key == '/') {
                        executeAction(2);
                        Serial.println("➡️ Secret: Next track");
                    }
                }
                
                if (status.space) {
                    executeAction(1);
                    Serial.println("⏯️ Secret: Play/Pause");
                }
            }
        }
        
        // Обработка кнопки A для play/pause
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
        
        if (clickCount > 0 && (millis() - lastClickTime > clickTimeout)) {
            executeAction(clickCount);
            clickCount = 0;
        }
        
        // В полноэкранном режиме продолжаем проверять автопереключение треков
        checkAutoNext();
        return;
    }

    // ОБЫЧНЫЙ РЕЖИМ
    
    if (millis() - lastBatteryUpdate >= batteryUpdateInterval) {
        lastBatteryUpdate = millis();
        drawBatteryIndicator();
    }
    
    if (!isInputMode && millis() - lastTimeUpdate >= timeUpdateInterval) {
        lastTimeUpdate = millis();
        updatePlayTime();
    }
    
    if (!isInputMode && millis() - lastVisualizerUpdate >= visualizerUpdateInterval) {
        lastVisualizerUpdate = millis();
        updateVisualizer();
        drawVisualizer();
    }
    
    clearTempMessage();

    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            
            if (isInputMode) {
                // РЕЖИМ ВВОДА НОМЕРА ТРЕКА
                
                if (status.enter) {
                    if (inputTrackNumber.length() > 0) {
                        int trackNum = inputTrackNumber.toInt();
                        goToTrack(trackNum);
                    }
                    isInputMode = false;
                    inputTrackNumber = "";
                    
                    M5Cardputer.Display.clear();
                    drawHeader();
                    drawBatteryIndicator();
                    drawVolumeIndicator();
                    drawLoopIndicator();
                    
                    M5Cardputer.Display.setTextSize(2);
                    M5Cardputer.Display.setTextColor(MAGENTA, BLACK);
                    M5Cardputer.Display.setCursor(10, 60);
                    M5Cardputer.Display.print("Track: ");
                    M5Cardputer.Display.setTextColor(WHITE, BLACK);
                    M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
                    
                    M5Cardputer.Display.setTextSize(2);
                    M5Cardputer.Display.setTextColor(MAGENTA, BLACK);
                    M5Cardputer.Display.setCursor(10, 80);
                    M5Cardputer.Display.print("Time: ");
                    M5Cardputer.Display.setTextColor(CYAN, BLACK);
                    M5Cardputer.Display.print("00:00");
                    
                    M5Cardputer.Display.setTextSize(1);
                    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
                    M5Cardputer.Display.setCursor(10, 110);
                    M5Cardputer.Display.print("Press I for Info");
                    
                    drawConnected();
                    drawStatus(isPlaying);
                        
                        return;
                }
                
                if (status.del && inputTrackNumber.length() > 0) {
                    inputTrackNumber.remove(inputTrackNumber.length() - 1);
                    drawInputMode();
                    return;
                }
                
                for (auto key : status.word) {
                    if (key == 'g' || key == 'G') {
                        isInputMode = false;
                        inputTrackNumber = "";
                        
                        M5Cardputer.Display.clear();
                        drawHeader();
                        drawBatteryIndicator();
                        drawVolumeIndicator();
                        drawLoopIndicator();
                        
                        M5Cardputer.Display.setTextSize(2);
                        M5Cardputer.Display.setTextColor(MAGENTA, BLACK);
                        M5Cardputer.Display.setCursor(10, 60);
                        M5Cardputer.Display.print("Track: ");
                        M5Cardputer.Display.setTextColor(WHITE, BLACK);
                        M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
                        
                        M5Cardputer.Display.setTextSize(2);
                        M5Cardputer.Display.setTextColor(MAGENTA, BLACK);
                        M5Cardputer.Display.setCursor(10, 80);
                        M5Cardputer.Display.print("Time: ");
                        M5Cardputer.Display.setTextColor(CYAN, BLACK);
                        M5Cardputer.Display.print("00:00");
                        
                        M5Cardputer.Display.setTextSize(1);
                        M5Cardputer.Display.setTextColor(YELLOW, BLACK);
                        M5Cardputer.Display.setCursor(10, 110);
                        M5Cardputer.Display.print("Press I for Info");
                        
                        drawConnected();
                        drawStatus(isPlaying);
                        
                        Serial.println("Input cancelled");
                        return;
                    }
                    
                    if (key >= '0' && key <= '9') {
                        if (inputTrackNumber.length() < 5) {
                            inputTrackNumber += key;
                            drawInputMode();
                        }
                    }
                }
            } else {
                // ОБЫЧНЫЙ РЕЖИМ
                
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
                        isInputMode = true;
                        inputTrackNumber = "";
                        drawInputMode();
                        Serial.println("Input mode activated");
                    }
                    else if (key == 'v' || key == 'V') {
                        // Полноэкранная визуализация
                        enterFullscreenVisualizer();
                        Serial.println("🎨 Fullscreen visualizer");
                    }
                    else if (key == 'i' || key == 'I') {
                        // Экран информации
                        enterInfoScreen();
                        Serial.println("ℹ️ Info screen");
                    }
                    else if (key == ';') {
                        int newTrack = currentTrack + 10;
                        if (newTrack > totalTracks) newTrack = totalTracks;
                        goToTrack(newTrack);
                        Serial.println("⬆️ Secret: +10 tracks");
                    }
                    else if (key == '.') {
                        int newTrack = currentTrack - 10;
                        if (newTrack < 1) newTrack = 1;
                        goToTrack(newTrack);
                        Serial.println("⬇️ Secret: -10 tracks");
                    }
                    else if (key == ',') {
                        executeAction(3);
                        Serial.println("⬅️ Secret: Previous track");
                    }
                    else if (key == '/') {
                        executeAction(2);
                        Serial.println("➡️ Secret: Next track");
                    }
                }
                
                if (status.space) {
                    executeAction(1);
                    Serial.println("⏯️ Secret: Play/Pause");
                }
            }
        }
    }

    if (!isInputMode) {
        checkAutoNext();
    }

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
        
        if (clickCount > 0 && (millis() - lastClickTime > clickTimeout)) {
            executeAction(clickCount);
            clickCount = 0;
        }
    }
}

void drawHeader() {
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(8, 6);
    M5Cardputer.Display.print("Audio Player");
    
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(50, 22);
    M5Cardputer.Display.print("v4.28.5 ");
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.print("\x01");
}

void updatePlayTime() {
    if (!isPlaying) return;
    
    unsigned long elapsedTime = (millis() - trackStartTime - totalPausedTime) / 1000;
    int minutes = elapsedTime / 60;
    int seconds = elapsedTime % 60;
    
    // Очищаем область (увеличенная высота для размера 2)
    M5Cardputer.Display.fillRect(10, 80, 220, 20, BLACK);
    
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(MAGENTA, BLACK);
    M5Cardputer.Display.setCursor(10, 80);
    M5Cardputer.Display.print("Time:");
    
    M5Cardputer.Display.setTextColor(CYAN, BLACK);
    M5Cardputer.Display.printf("%02d:%02d", minutes, seconds);
}

void volumeUp() {
    if (currentVolume < 30) {
        currentVolume++;
        audioplayer.setVolume(currentVolume);
        
        char msg[20];
        sprintf(msg, "Vol UP: %d%%", (currentVolume * 100) / 30);
        
        if (isFullscreenVisualizer) {
            drawFullscreenTempMessage(msg, GREEN);
        } else {
            drawVolumeIndicator();
            drawStatus(isPlaying);
            drawTempMessage(msg, GREEN);
        }
        
        Serial.printf("🔊 Volume UP: %d/30 (%d%%)\n", currentVolume, (currentVolume * 100) / 30);
    }
}

void volumeDown() {
    if (currentVolume > 0) {
        currentVolume--;
        audioplayer.setVolume(currentVolume);
        
        char msg[20];
        sprintf(msg, "Vol DOWN: %d%%", (currentVolume * 100) / 30);
        
        if (isFullscreenVisualizer) {
            drawFullscreenTempMessage(msg, YELLOW);
        } else {
            drawVolumeIndicator();
            drawStatus(isPlaying);
            drawTempMessage(msg, YELLOW);
        }
        
        Serial.printf("🔉 Volume DOWN: %d/30 (%d%%)\n", currentVolume, (currentVolume * 100) / 30);
    }
}

void drawInputMode() {
    M5Cardputer.Display.fillRect(0, 35, 240, 95, BLACK);
    
    M5Cardputer.Display.fillRect(20, 40, 200, 50, BLACK);
    M5Cardputer.Display.drawRect(20, 40, 200, 50, CYAN);
    M5Cardputer.Display.drawRect(21, 41, 198, 48, CYAN);
    
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(CYAN);
    M5Cardputer.Display.setCursor(30, 48);
    M5Cardputer.Display.print("Go to track:");
    
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(YELLOW);
    M5Cardputer.Display.setCursor(30, 65);
    
    if (inputTrackNumber.length() > 0) {
        M5Cardputer.Display.print(inputTrackNumber);
    } else {
        M5Cardputer.Display.setTextColor(DARKGREY);
        M5Cardputer.Display.print("___");
    }
    
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setCursor(25, 100);
    M5Cardputer.Display.print("Enter:OK Del:Back G:Cancel");
}

void toggleLoop() {
    isLoopEnabled = !isLoopEnabled;
    
    if (isLoopEnabled && isShuffleEnabled) {
        isShuffleEnabled = false;
        Serial.println("Shuffle disabled (Loop enabled)");
    }
    
    if (isFullscreenVisualizer) {
        if (isLoopEnabled) {
            drawFullscreenTempMessage("Loop: ON", YELLOW);
        } else {
            drawFullscreenTempMessage("Loop: OFF", YELLOW);
        }
    } else {
        drawLoopIndicator();
        drawStatus(isPlaying);
        if (isLoopEnabled) {
            drawTempMessage("Loop: ON", YELLOW);
        } else {
            drawTempMessage("Loop: OFF", YELLOW);
        }
    }
    
    Serial.println(isLoopEnabled ? "Loop enabled" : "Loop disabled");
}

void toggleShuffle() {
    isShuffleEnabled = !isShuffleEnabled;
    
    if (isShuffleEnabled && isLoopEnabled) {
        isLoopEnabled = false;
        Serial.println("Loop disabled (Shuffle enabled)");
    }
    
    if (isFullscreenVisualizer) {
        if (isShuffleEnabled) {
            drawFullscreenTempMessage("Shuffle: ON", CYAN);
        } else {
            drawFullscreenTempMessage("Shuffle: OFF", CYAN);
        }
    } else {
        drawLoopIndicator();
        drawStatus(isPlaying);
        if (isShuffleEnabled) {
            drawTempMessage("Shuffle: ON", CYAN);
        } else {
            drawTempMessage("Shuffle: OFF", CYAN);
        }
    }
    
    Serial.println(isShuffleEnabled ? "Shuffle enabled" : "Shuffle disabled");
}

void goToTrack(int trackNum) {
    if (trackNum < 1 || trackNum > totalTracks) {
        char msg[30];
        sprintf(msg, "Invalid! (1-%d)", totalTracks);
        
        if (isFullscreenVisualizer) {
            drawFullscreenTempMessage(msg, RED);
        } else {
            M5Cardputer.Display.fillRect(0, 120, 240, 15, BLACK);
            M5Cardputer.Display.setTextSize(1);
            M5Cardputer.Display.setTextColor(RED);
            M5Cardputer.Display.setCursor(10, 120);
            M5Cardputer.Display.print(msg);
            delay(1500);
            M5Cardputer.Display.fillRect(0, 120, 240, 15, BLACK);
        }
        
        Serial.printf("Invalid track: %d (must be 1-%d)\n", trackNum, totalTracks);
        return;
    }
    
    audioplayer.selectAudioNum(trackNum);
    delay(100);
    audioplayer.playAudio();
    isPlaying = true;
    trackStartTime = millis();
    
    char msg[20];
    sprintf(msg, "Go: %d", trackNum);
    
    if (isFullscreenVisualizer) {
        drawFullscreenTempMessage(msg, CYAN);
    } else {
        drawStatus(isPlaying);
        drawTempMessage(msg, CYAN);
    }
    
    Serial.printf("Go to track: %d\n", trackNum);
}

void randomTrack() {
    if (totalTracks == 0) return;
    
    uint16_t randomNum = random(1, totalTracks + 1);
    
    Serial.printf("🎲 Random track: %d\n", randomNum);
    
    audioplayer.selectAudioNum(randomNum);
    currentTrack = randomNum;
    
    trackStartTime = millis();
    totalPausedTime = 0;
    isPlaying = (lastPlayStatus == AUDIO_PLAYER_STATUS_PLAYING);
    
    if (lastPlayStatus == AUDIO_PLAYER_STATUS_PLAYING) {
        audioplayer.playAudio();
    }
    
    if (isFullscreenVisualizer) {
        drawFullscreenTempMessage("Random!", MAGENTA);
    } else {
        // Увеличенный шрифт для Track
        M5Cardputer.Display.fillRect(10, 60, 220, 20, BLACK);
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.setTextColor(MAGENTA);
        M5Cardputer.Display.setCursor(10, 60);
        M5Cardputer.Display.print("Track:");
        M5Cardputer.Display.setTextColor(WHITE);
        M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
        
        drawStatus(isPlaying);
        drawTempMessage("Random!", MAGENTA);
    }
    
    Serial.printf("Random track: %d/%d\n", currentTrack, totalTracks);
}

void drawBatteryIndicator() {
    // Получаем сырые данные о батарее
    int rawBatteryLevel = M5Cardputer.Power.getBatteryLevel();
    int batteryVoltage = M5Cardputer.Power.getBatteryVoltage();
    bool isCharging = (batteryVoltage > 4200);
    
    // Умная логика (как в battery_test_v2_1)
    int smartBatteryLevel = rawBatteryLevel;
    
    // Отслеживаем начало зарядки
    if (isCharging && !wasCharging) {
        chargingStartTime = millis();
        if (rawBatteryLevel < 100) {
            lastValidBatteryLevel = rawBatteryLevel;
        }
        Serial.println("🔌 Charging started");
    }
    
    // Отслеживаем окончание зарядки
    if (!isCharging && wasCharging) {
        lastValidBatteryLevel = rawBatteryLevel;
        Serial.println("🔋 Charging stopped");
    }
    
    // Умная логика: если заряжается И показывает 100%, используем последнее валидное значение
    if (isCharging && rawBatteryLevel == 100 && lastValidBatteryLevel != -1 && lastValidBatteryLevel < 95) {
        smartBatteryLevel = lastValidBatteryLevel;
        
        // Постепенный прирост со временем
        unsigned long chargingTime = millis() - chargingStartTime;
        int chargeIncrease = (chargingTime / 60000); // +1% каждую минуту
        smartBatteryLevel = min(lastValidBatteryLevel + chargeIncrease, 99);
    } else if (!isCharging) {
        smartBatteryLevel = rawBatteryLevel;
        lastValidBatteryLevel = rawBatteryLevel;
    }
    
    wasCharging = isCharging;
    
    // Используем smartBatteryLevel для отображения
    int batteryLevel = smartBatteryLevel;
    
    int x = 182;
    int y = 2;
    
    M5Cardputer.Display.fillRect(x, y, 58, 22, BLACK);
    
    M5Cardputer.Display.drawRect(x, y, 38, 16, WHITE);
    M5Cardputer.Display.fillRect(x + 38, y + 5, 3, 6, WHITE);
    
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
    
    if (isCharging) {
        M5Cardputer.Display.setTextColor(YELLOW, color);
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setCursor(x + 15, y + 4);
        M5Cardputer.Display.print("~");
    }
    
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(x + 9, y + 18);
    M5Cardputer.Display.printf("%d%%", batteryLevel);
}

void drawLoopIndicator() {
    M5Cardputer.Display.fillRect(170, 73, 70, 12, BLACK);
    M5Cardputer.Display.setTextSize(1);
    
    if (isLoopEnabled) {
        drawLoopIcon(170, 73, YELLOW);
        M5Cardputer.Display.setTextColor(YELLOW, BLACK);
        M5Cardputer.Display.setCursor(182, 73);
        M5Cardputer.Display.print("LOOP");
    } else if (isShuffleEnabled) {
        drawShuffleIcon(170, 73, CYAN);
        M5Cardputer.Display.setTextColor(CYAN, BLACK);
        M5Cardputer.Display.setCursor(182, 73);
        M5Cardputer.Display.print("SHUFFLE");
    }
}

void drawVisualizer() {
    int startX = 110;
    int startY = 55;
    int barWidth = 6;
    int barGap = 2;
    int maxBarHeight = 25;
    
    for (int i = 0; i < 5; i++) {
        int x = startX + i * (barWidth + barGap);
        
        M5Cardputer.Display.fillRect(x, startY - maxBarHeight, barWidth, maxBarHeight, BLACK);
        
        if (barHeights[i] > 0) {
            uint16_t color;
            
            if (i == 0) {
                color = 0xF800; // Красный
            } else if (i == 1) {
                color = 0xFD20; // Оранжевый
            } else if (i == 2) {
                color = 0xFFE0; // Желтый
            } else if (i == 3) {
                color = GREEN;
            } else {
                color = CYAN;
            }
            
            M5Cardputer.Display.fillRect(x, startY - barHeights[i], barWidth, barHeights[i], color);
        }
    }
}

void updateVisualizer() {
    if (!isPlaying) {
        for (int i = 0; i < 5; i++) {
            barHeights[i] = 0;
        }
        return;
    }
    
    barHeights[0] = random(15, 26);
    barHeights[1] = random(12, 25);
    barHeights[2] = random(10, 22);
    barHeights[3] = random(8, 20);
    barHeights[4] = random(5, 17);
}

void drawVolumeIndicator() {
    int x = 175;
    int y = 30;
    
    M5Cardputer.Display.fillRect(x, y, 65, 30, BLACK);
    
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(x, y + 2);
    
    M5Cardputer.Display.setTextColor(CYAN);
    M5Cardputer.Display.print("Vol:");
    
    int volumePercent = (currentVolume * 100) / 30;
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.printf("%d%%", volumePercent);
    
    int barWidth = (volumePercent * 47) / 100;
    M5Cardputer.Display.drawRect(x, y + 22, 49, 6, WHITE);
    
    uint16_t barColor;
    if (volumePercent > 70) {
        barColor = GREEN;
    } else if (volumePercent > 30) {
        barColor = YELLOW;
    } else if (volumePercent > 0) {
        barColor = ORANGE;
    } else {
        barColor = RED;
    }
    
    if (barWidth > 0) {
        M5Cardputer.Display.fillRect(x + 1, y + 23, barWidth, 4, barColor);
    }
    
    Serial.printf("Volume: %d/30 (%d%%)\n", currentVolume, volumePercent);
}

void checkAutoNext() {
    static unsigned long lastCheck = 0;
    
    if (millis() - lastCheck < 500) {
        return;
    }
    lastCheck = millis();
    
    uint8_t currentStatus = audioplayer.checkPlayStatus();
    
    if (lastPlayStatus == AUDIO_PLAYER_STATUS_PLAYING && 
        currentStatus == AUDIO_PLAYER_STATUS_STOPPED) {
        
        if (isLoopEnabled) {
            Serial.println("Track ended - Loop repeat");
            
            delay(100);
            audioplayer.playAudio();
            delay(100);
            
            trackStartTime = millis();
            totalPausedTime = 0;
            isPlaying = true;
            
            M5Cardputer.Display.fillRect(10, 75, 150, 12, BLACK);
            M5Cardputer.Display.setTextColor(WHITE, BLACK);
            M5Cardputer.Display.setCursor(10, 75);
            M5Cardputer.Display.print("Time: ");
            M5Cardputer.Display.setTextColor(CYAN, BLACK);
            M5Cardputer.Display.print("00:00");
            
            drawStatus(true);
            drawTempMessage("Loop repeat", YELLOW);
        } else if (isShuffleEnabled) {
            Serial.println("Track ended - Shuffle next");
            
            uint16_t randomNum = random(1, totalTracks + 1);
            audioplayer.selectAudioNum(randomNum);
            delay(200);
            audioplayer.playAudio();
            delay(100);
            
            trackStartTime = millis();
            totalPausedTime = 0;
            isPlaying = true;
            
            drawStatus(true);
            drawTempMessage("Shuffle next", CYAN);
            
            Serial.printf("Shuffle to track: %d\n", randomNum);
        } else {
            Serial.println("Track ended - Auto next");
            
            audioplayer.nextAudio();
            delay(200);
            audioplayer.playAudio();
            delay(100);
            
            trackStartTime = millis();
            totalPausedTime = 0;
            isPlaying = true;
            
            drawStatus(true);
            drawTempMessage("Auto next", WHITE);
            
            Serial.println("Next track started");
        }
    }
    
    uint16_t realTrackNumber = audioplayer.getCurrentAudioNumber();
    
    if (realTrackNumber < 1 || realTrackNumber > totalTracks) {
        Serial.printf("Invalid track number: %d (ignored)\n", realTrackNumber);
        return;
    }
    
    if (realTrackNumber != lastDisplayedTrack) {
        Serial.printf("Track changed: %d -> %d\n", lastDisplayedTrack, realTrackNumber);
        
        currentTrack = realTrackNumber;
        lastDisplayedTrack = realTrackNumber;
        
        if (isLoopEnabled) {
            isLoopEnabled = false;
            drawLoopIndicator();
            Serial.println("Loop disabled (track changed)");
        }
        
        trackStartTime = millis();
        totalPausedTime = 0;
        
        // Увеличенный шрифт для Track
        M5Cardputer.Display.fillRect(10, 60, 220, 20, BLACK);
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.setTextColor(MAGENTA);
        M5Cardputer.Display.setCursor(10, 60);
        M5Cardputer.Display.print("Track:");
        M5Cardputer.Display.setTextColor(WHITE);
        M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
        
        // Увеличенный шрифт для Time
        M5Cardputer.Display.fillRect(10, 80, 220, 20, BLACK);
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.setTextColor(MAGENTA, BLACK);
        M5Cardputer.Display.setCursor(10, 80);
        M5Cardputer.Display.print("Time:");
        M5Cardputer.Display.setTextColor(CYAN, BLACK);
        M5Cardputer.Display.print("00:00");
        
        Serial.printf("Display updated: Track %d/%d, Timer reset\n", currentTrack, totalTracks);
    }
    
    if (lastPlayStatus != currentStatus) {
        if (currentStatus == AUDIO_PLAYER_STATUS_PLAYING) {
            if (lastPlayStatus == AUDIO_PLAYER_STATUS_PAUSED) {
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
            if (isFullscreenVisualizer) {
                drawFullscreenTempMessage("Paused", YELLOW);
            } else {
                drawStatus(false);
            }
            Serial.println("Paused");
        } else {
            audioplayer.playAudio();
            if (playStatus == AUDIO_PLAYER_STATUS_STOPPED) {
                trackStartTime = millis();
                totalPausedTime = 0;
            } else {
                totalPausedTime += (millis() - pausedTime);
            }
            isPlaying = true;
            if (isFullscreenVisualizer) {
                drawFullscreenTempMessage("Playing", GREEN);
            } else {
                drawStatus(true);
            }
            Serial.println("Playing");
        }
    }
    else if (clicks == 2) {
        Serial.println("Action: Next track");
        
        audioplayer.nextAudio();
        delay(100);
        audioplayer.playAudio();
        
        if (isFullscreenVisualizer) {
            drawFullscreenTempMessage("Next track", WHITE);
        } else {
            drawStatus(isPlaying);
            drawTempMessage("Next track", WHITE);
        }
        
        Serial.println("Next track command sent");
    }
    else if (clicks == 3) {
        Serial.println("Action: Previous track");
        
        audioplayer.previousAudio();
        delay(100);
        audioplayer.playAudio();
        
        if (isFullscreenVisualizer) {
            drawFullscreenTempMessage("Prev track", WHITE);
        } else {
            drawStatus(isPlaying);
            drawTempMessage("Prev track", WHITE);
        }
        
        Serial.println("Previous track command sent");
    }
    else {
        Serial.printf("Ignored: %d clicks\n", clicks);
        if (!isFullscreenVisualizer) {
            M5Cardputer.Display.print("Too many clicks!");
        }
    }
}

// ========== ПОЛНОЭКРАННАЯ ВИЗУАЛИЗАЦИЯ ==========

void enterFullscreenVisualizer() {
    isFullscreenVisualizer = true;
    visualizerMode = 1; // Начинаем с режима 1 (столбики)
    M5Cardputer.Display.fillScreen(BLACK);
    
    drawVisualizerHeader();
    
    Serial.println("🎨 Entered fullscreen visualizer - Mode 1 (Bars)");
}

void drawVisualizerHeader() {
    // Очищаем верх и низ
    M5Cardputer.Display.fillRect(0, 0, 240, 20, BLACK);
    M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
    
    // Заголовок
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(CYAN, BLACK);
    M5Cardputer.Display.setCursor(45, 5);
    
    if (visualizerMode == 1) {
        M5Cardputer.Display.print("Visualizer: Bars");
    } else if (visualizerMode == 2) {
        M5Cardputer.Display.print("Visualizer: Waves");
    }
    
    // Подсказка
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(35, 127);
    M5Cardputer.Display.print("V:Switch  ESC:Exit");
}

void switchVisualizerMode() {
    visualizerMode++;
    
    if (visualizerMode > 3) {
        // Режим 4 = выход
        exitFullscreenVisualizer();
    } else {
        // Переключаем режим
        M5Cardputer.Display.fillScreen(BLACK);
        
        // Инициализируем Matrix если переключились на режим 3
        if (visualizerMode == 3) {
            initMatrixColumns();
            Serial.println("💚 Matrix Rain activated!");
        }
        
        drawVisualizerHeader();
        Serial.printf("🎨 Switched to visualizer mode %d\n", visualizerMode);
    }
}

void exitFullscreenVisualizer() {
    isFullscreenVisualizer = false;
    visualizerMode = 0;
    
    // Восстанавливаем весь интерфейс
    M5Cardputer.Display.fillScreen(BLACK);
    drawHeader();
    drawBatteryIndicator();
    drawVolumeIndicator();
    drawLoopIndicator();
    drawConnected();
    
    // Информация о треке (увеличенный шрифт)
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(MAGENTA);
    M5Cardputer.Display.setCursor(10, 60);
    M5Cardputer.Display.print("Track:");
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
    
    M5Cardputer.Display.setTextColor(MAGENTA);
    M5Cardputer.Display.setCursor(10, 80);
    M5Cardputer.Display.print("Time:");
    M5Cardputer.Display.setTextColor(CYAN);
    M5Cardputer.Display.print("00:00");
    
    // Только подсказка INFO
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(10, 110);
    M5Cardputer.Display.print("Press I for Info");
    
    drawStatus(isPlaying);
    
    Serial.println("🎨 Exited fullscreen visualizer");
}

void updateFullscreenVisualizer() {
    if (visualizerMode == 1) {
        // Режим 1: Столбики
        if (!isPlaying) {
            for (int i = 0; i < 20; i++) {
                fullscreenBarHeights[i] = 0;
            }
            return;
        }
        
        // Генерируем "умные" случайные высоты для 20 баров
        for (int i = 0; i < 6; i++) {
            fullscreenBarHeights[i] = random(60, 101);
        }
        for (int i = 6; i < 14; i++) {
            fullscreenBarHeights[i] = random(40, 90);
        }
        for (int i = 14; i < 20; i++) {
            fullscreenBarHeights[i] = random(20, 70);
        }
    } else if (visualizerMode == 2) {
        // Режим 2: Волны
        if (!isPlaying) {
            for (int i = 0; i < 240; i++) {
                waveAmplitudes[i] = 0;
            }
            wavePhase = 0.0;
            return;
        }
        
        // Обновляем фазу волны
        wavePhase += 0.15;
        if (wavePhase > 6.28) wavePhase = 0.0; // 2*PI
        
        // Генерируем волну
        float amplitude = random(20, 40); // Амплитуда волны
        float frequency = 0.05; // Частота волны
        
        for (int x = 0; x < 240; x++) {
            waveAmplitudes[x] = (int)(amplitude * sin(x * frequency + wavePhase));
        }
    } else if (visualizerMode == 3) {
        // Режим 3: Matrix Rain
        if (millis() - lastMatrixUpdate >= matrixUpdateInterval) {
            lastMatrixUpdate = millis();
            updateMatrixColumns();
        }
    }
}

void drawFullscreenVisualizer() {
    if (visualizerMode == 1) {
        // Режим 1: Столбики
        int barWidth = 11;
        int barGap = 1;
        int maxBarHeight = 105;
        int startY = 125;
        
        for (int i = 0; i < 20; i++) {
            int x = i * (barWidth + barGap);
            
            M5Cardputer.Display.fillRect(x, 20, barWidth, maxBarHeight, BLACK);
            
            if (fullscreenBarHeights[i] > 0) {
                uint16_t color;
                
                if (i < 4) {
                    color = 0xF800; // Красный
                } else if (i < 8) {
                    color = 0xFD20; // Оранжевый
                } else if (i < 12) {
                    color = 0xFFE0; // Желтый
                } else if (i < 16) {
                    color = GREEN; // Зеленый
                } else {
                    color = CYAN; // Голубой
                }
                
                M5Cardputer.Display.fillRect(x, startY - fullscreenBarHeights[i], barWidth, fullscreenBarHeights[i], color);
            }
        }
    } else if (visualizerMode == 2) {
        // Режим 2: Зелёные волны
        M5Cardputer.Display.fillRect(0, 20, 240, 105, BLACK);
        
        int centerY = 67; // Центр экрана (20 + 105/2)
        uint16_t green = 0x07E0; // Зелёный цвет
        
        // Рисуем волну
        for (int x = 0; x < 239; x++) {
            int y1 = centerY + waveAmplitudes[x];
            int y2 = centerY + waveAmplitudes[x + 1];
            
            // Ограничиваем координаты
            if (y1 < 20) y1 = 20;
            if (y1 > 125) y1 = 125;
            if (y2 < 20) y2 = 20;
            if (y2 > 125) y2 = 125;
            
            // Рисуем линию волны
            M5Cardputer.Display.drawLine(x, y1, x + 1, y2, green);
            
            // Заполняем область под волной для эффекта
            if (y1 < centerY) {
                M5Cardputer.Display.drawLine(x, y1, x, centerY, green);
            } else {
                M5Cardputer.Display.drawLine(x, centerY, x, y1, green);
            }
        }
    } else if (visualizerMode == 3) {
        // Режим 3: Matrix Rain
        drawMatrixRain();
        return; // drawMatrixRain() уже рисует заголовок
    }
    
    // Обновляем заголовок и подсказки
    drawVisualizerHeader();
    
    // Рисуем временное сообщение внизу (если есть)
    if (tempMessageActive) {
        int textWidth = tempMessageText.length() * 6;
        int x = (240 - textWidth) / 2;
        if (x < 0) x = 0;
        
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setTextColor(tempMessageColor, BLACK);
        M5Cardputer.Display.setCursor(x, 127);
        M5Cardputer.Display.print(tempMessageText);
    }
}

void drawFullscreenTempMessage(String message, uint16_t color) {
    // Сохраняем сообщение
    tempMessageText = message;
    tempMessageColor = color;
    tempMessageTime = millis();
    tempMessageActive = (message.length() > 0);
    
    // Полностью очищаем нижнюю область
    M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
    
    if (message.length() > 0) {
        // Центрируем текст
        int textWidth = message.length() * 6;
        int x = (240 - textWidth) / 2;
        if (x < 0) x = 0;
        
        // Рисуем текст с явным фоном
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setTextColor(color, BLACK);
        M5Cardputer.Display.setCursor(x, 127);
        M5Cardputer.Display.print(message);
    }
}

// ========== MATRIX RAIN ВИЗУАЛИЗАЦИЯ (Режим 3) ==========

void initMatrixColumns() {
    for (int i = 0; i < 40; i++) {
        matrixColumns[i].y = random(-50, 0); // Начинаем сверху
        matrixColumns[i].speed = random(1, 4); // Скорость 1-3
        matrixColumns[i].character = matrixChars[random(0, strlen(matrixChars))];
        matrixColumns[i].brightness = random(100, 255);
        matrixColumns[i].trailLength = random(5, 15);
    }
    matrixNoteCounter = 0;
}

void updateMatrixColumns() {
    // Базовая скорость зависит от того, играет ли музыка
    int speedMultiplier = isPlaying ? 2 : 1;
    
    for (int i = 0; i < 40; i++) {
        // Обновляем позицию
        matrixColumns[i].y += matrixColumns[i].speed * speedMultiplier;
        
        // Если столбец ушел за экран, перезапускаем его
        if (matrixColumns[i].y > 135 + matrixColumns[i].trailLength * 8) {
            matrixColumns[i].y = random(-50, -10);
            matrixColumns[i].speed = random(1, 4);
            matrixColumns[i].trailLength = random(5, 15);
            matrixColumns[i].brightness = random(150, 255);
            
            // Каждый ~20-й символ - музыкальный
            matrixNoteCounter++;
            if (matrixNoteCounter >= 20) {
                matrixColumns[i].character = (random(0, 2) == 0) ? 0x0F : 0x0E; // ♪ или ♫ в CP437
                matrixNoteCounter = 0;
            } else {
                matrixColumns[i].character = matrixChars[random(0, strlen(matrixChars))];
            }
        }
    }
}

void drawMatrixRain() {
    // Затемняем экран для эффекта следа
    M5Cardputer.Display.fillRect(0, 20, 240, 105, BLACK);
    
    M5Cardputer.Display.setTextSize(1);
    
    for (int i = 0; i < 40; i++) {
        int x = i * 6; // 6 пикселей на символ
        int y = matrixColumns[i].y;
        
        if (y >= 20 && y < 125) {
            // Рисуем голову столбца (яркая)
            uint16_t headColor = M5Cardputer.Display.color565(200, 255, 200); // Светло-зеленый
            M5Cardputer.Display.setTextColor(headColor, BLACK);
            M5Cardputer.Display.setCursor(x, y);
            
            // Специальная обработка музыкальных символов
            if (matrixColumns[i].character == 0x0F) {
                M5Cardputer.Display.print((char)0x0F); // ♪
            } else if (matrixColumns[i].character == 0x0E) {
                M5Cardputer.Display.print((char)0x0E); // ♫
            } else {
                M5Cardputer.Display.print(matrixColumns[i].character);
            }
            
            // Рисуем след (затухающий)
            for (int j = 1; j < matrixColumns[i].trailLength; j++) {
                int trailY = y - (j * 8); // 8 пикселей на символ высотой
                
                if (trailY >= 20 && trailY < 125) {
                    // Уменьшаем яркость по мере удаления
                    int brightness = 255 - (j * 255 / matrixColumns[i].trailLength);
                    uint16_t trailColor = M5Cardputer.Display.color565(0, brightness, 0);
                    
                    M5Cardputer.Display.setTextColor(trailColor, BLACK);
                    M5Cardputer.Display.setCursor(x, trailY);
                    
                    // Случайный символ для следа
                    char trailChar = matrixChars[random(0, strlen(matrixChars))];
                    M5Cardputer.Display.print(trailChar);
                }
            }
        }
    }
    
    // Обновляем заголовок и подсказки
    drawVisualizerHeader();
    
    // Рисуем временное сообщение внизу (если есть)
    if (tempMessageActive) {
        int textWidth = tempMessageText.length() * 6;
        int x = (240 - textWidth) / 2;
        if (x < 0) x = 0;
        
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setTextColor(tempMessageColor, BLACK);
        M5Cardputer.Display.setCursor(x, 127);
        M5Cardputer.Display.print(tempMessageText);
    }
}

// ========== ЭКРАН ИНФОРМАЦИИ ==========

void drawLogo() {
    // Рисуем зелёный лист (упрощённая версия)
    // Центр листа: x=120, y=35
    int cx = 120;
    int cy = 35;
    
    // Основной лист (7 лепестков)
    uint16_t green = 0x07E0; // Зелёный
    
    // Центральный лепесток (вверх)
    M5Cardputer.Display.fillTriangle(cx, cy-15, cx-5, cy, cx+5, cy, green);
    
    // Левый верхний
    M5Cardputer.Display.fillTriangle(cx-10, cy-10, cx-15, cy-5, cx-5, cy, green);
    
    // Правый верхний
    M5Cardputer.Display.fillTriangle(cx+10, cy-10, cx+15, cy-5, cx+5, cy, green);
    
    // Левый средний
    M5Cardputer.Display.fillTriangle(cx-15, cy, cx-20, cy+5, cx-10, cy+5, green);
    
    // Правый средний
    M5Cardputer.Display.fillTriangle(cx+15, cy, cx+20, cy+5, cx+10, cy+5, green);
    
    // Левый нижний
    M5Cardputer.Display.fillTriangle(cx-10, cy+5, cx-12, cy+10, cx-5, cy+10, green);
    
    // Правый нижний
    M5Cardputer.Display.fillTriangle(cx+10, cy+5, cx+12, cy+10, cx+5, cy+10, green);
    
    // Стебель
    M5Cardputer.Display.fillRect(cx-1, cy+10, 2, 8, green);
}

void enterInfoScreen() {
    isInfoScreen = true;
    M5Cardputer.Display.fillScreen(BLACK);
    
    // Рисуем логотип
    drawLogo();
    
    // Заголовок (центрировано)
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    String title = "Unit AudioPlayer";
    int titleWidth = title.length() * 6;
    M5Cardputer.Display.setCursor((240 - titleWidth) / 2, 55);
    M5Cardputer.Display.print(title);
    
    M5Cardputer.Display.setTextColor(GREEN, BLACK);
    String subtitle = "AndyShuffle";
    int subtitleWidth = subtitle.length() * 6;
    M5Cardputer.Display.setCursor((240 - subtitleWidth) / 2, 65);
    M5Cardputer.Display.print(subtitle);
    
    // Версия (центрировано)
    M5Cardputer.Display.setTextColor(CYAN, BLACK);
    String version = "v4.29.1";
    int versionWidth = version.length() * 6;
    M5Cardputer.Display.setCursor((240 - versionWidth) / 2, 75);
    M5Cardputer.Display.print(version);
    
    // Компания (центрировано)
    M5Cardputer.Display.setTextColor(MAGENTA, BLACK);
    String company = "Andy+AI";
    int companyWidth = company.length() * 6;
    M5Cardputer.Display.setCursor((240 - companyWidth) / 2, 85);
    M5Cardputer.Display.print(company);
    
    // Команды (центрировано)
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    String cmd1 = "BtnA: 1x=Play 2x=Next 3x=Prev";
    int cmd1Width = cmd1.length() * 6;
    M5Cardputer.Display.setCursor((240 - cmd1Width) / 2, 95);
    M5Cardputer.Display.print(cmd1);
    
    String cmd2 = "+/-:Vol R:Rnd L:Lp S:Sh G:Go";
    int cmd2Width = cmd2.length() * 6;
    M5Cardputer.Display.setCursor((240 - cmd2Width) / 2, 105);
    M5Cardputer.Display.print(cmd2);
    
    M5Cardputer.Display.setTextColor(ORANGE, BLACK);
    String cmd3 = "Secret: Space,./; V:3Viz I:Info";
    int cmd3Width = cmd3.length() * 6;
    M5Cardputer.Display.setCursor((240 - cmd3Width) / 2, 115);
    M5Cardputer.Display.print(cmd3);
    
    // Подсказка выхода (центрировано)
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    String exit = "Press I or ESC to exit";
    int exitWidth = exit.length() * 6;
    M5Cardputer.Display.setCursor((240 - exitWidth) / 2, 125);
    M5Cardputer.Display.print(exit);
    
    Serial.println("ℹ️ Entered info screen");
}

void exitInfoScreen() {
    isInfoScreen = false;
    
    // Восстанавливаем весь интерфейс
    M5Cardputer.Display.fillScreen(BLACK);
    drawHeader();
    drawBatteryIndicator();
    drawVolumeIndicator();
    drawLoopIndicator();
    drawConnected();
    
    // Информация о треке (увеличенный шрифт)
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(MAGENTA);
    M5Cardputer.Display.setCursor(10, 60);
    M5Cardputer.Display.print("Track:");
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.printf("%d/%d", currentTrack, totalTracks);
    
    M5Cardputer.Display.setTextColor(MAGENTA);
    M5Cardputer.Display.setCursor(10, 80);
    M5Cardputer.Display.print("Time:");
    M5Cardputer.Display.setTextColor(CYAN);
    M5Cardputer.Display.print("00:00");
    
    // Только подсказка INFO
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(10, 110);
    M5Cardputer.Display.print("Press I for Info");
    
    drawStatus(isPlaying);
    
    Serial.println("ℹ️ Exited info screen");
}
