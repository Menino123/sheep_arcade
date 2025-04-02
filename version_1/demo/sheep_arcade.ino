#include <U8glib.h>
#include <EEPROM.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0);

#define JUMP_BUTTON 2    // Botão de salto no pino 2
#define START_BUTTON 3   // Botão de início no pino 3
#define BUZZER_PIN 4     // Pino do buzzer

#define DEBOUNCE_TIME 300  // Tempo de debounce do botão (ms)

// Variáveis do Dino
long dinoY = 50;
long dinoVelocity = 0;
long dinoGravity = 1;
long jumpHeight = 6;
bool isJumping = false;

// Variáveis do Obstáculo
long obstacleX = 128;
long obstacleY = 50;
long obstacleSpeed = 5;

// Estado do jogo
bool gameStarted = false;
bool gameOver = false;
long score = 0;
long highScore = 0;

unsigned long lastJumpTime = 0;
unsigned long lastStartTime = 0;

void setup() {
    pinMode(JUMP_BUTTON, INPUT_PULLUP);
    pinMode(START_BUTTON, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    Serial.begin(9600);
    EEPROM.get(0, highScore);
    displayStartScreen();
}

void loop() {
    if (debounceButton(START_BUTTON, lastStartTime) && !gameStarted && !gameOver) {
        gameStarted = true;
        score = 0;
        displayLoadingScreen();
    }

    if (gameStarted && !gameOver) {
        handleJump();
        updateGameObjects();
        detectCollisions();
        renderGame();
    }
}

bool debounceButton(int pin, unsigned long &lastPressTime) {
    if (digitalRead(pin) == LOW) {
        if (millis() - lastPressTime > DEBOUNCE_TIME) {
            lastPressTime = millis();
            return true;
        }
    }
    return false;
}

void handleJump() {
    if (debounceButton(JUMP_BUTTON, lastJumpTime) && !isJumping) {
        isJumping = true;
        dinoVelocity = -jumpHeight;
        tone(BUZZER_PIN, 1000, 100);
    }

    if (isJumping) {
        dinoY += dinoVelocity;
        dinoVelocity += dinoGravity;

        if (dinoY >= 50) {
            dinoY = 50;
            isJumping = false;
        }
    }
}

void updateGameObjects() {
    obstacleX -= obstacleSpeed;
    
    // Quando o obstáculo sai da tela, ele volta à direita com uma distância aleatória
    if (obstacleX < 0) {
        obstacleX = 128 + random(0, 100);  // Lógica para reposicionar o obstáculo com uma distância aleatória
        score++;
        
        if (score % 5 == 0) obstacleSpeed++;  // Aumenta a velocidade do obstáculo a cada 10 pontos
        
        if (score > highScore) {
            highScore = score;
            EEPROM.put(0, highScore);
        }
    }
}

void detectCollisions() {
    if (obstacleX < 45 && obstacleX + 10 > 35 && dinoY + 10 > obstacleY) {
        gameOver = true;
        displayGameOver();
        delay(4000); // Mantém a tela de Game Over por 4 segundos
        return resetGame();
    }
}

void renderGame() {
    u8g.firstPage();
    do {
        drawDino(40, dinoY);
        drawObstacle();
        drawScore();
    } while (u8g.nextPage());
}

void drawDino(int x, int y) {
    u8g.drawBox(x, y, 10, 10);
    u8g.drawBox(x + 7, y - 5, 5, 5);
}

void drawObstacle() {
    u8g.drawBox(obstacleX, obstacleY, 10, 10);
}

void drawScore() {
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(5, 10, "Score:");
    u8g.setPrintPos(55, 10);
    u8g.print(score);
    u8g.drawStr(5, 20, "High Score:");
    u8g.setPrintPos(85, 20);
    u8g.print(highScore);
}

void displayGameOver() {
    u8g.firstPage();
    do {
        u8g.setFont(u8g_font_6x10);
        u8g.drawStr(30, 30, "Game Over!");
        u8g.drawStr(10, 50, "Pressione Start!");
    } while (u8g.nextPage());
    tone(BUZZER_PIN, 400, 500);
}

void resetGame() {
    obstacleX = 128;
    dinoY = 50;
    dinoVelocity = 0;
    isJumping = false;
    score = 0;
    gameStarted = false;
    gameOver = false;
    noTone(BUZZER_PIN); // Certifica-se de desligar o buzzer ao reiniciar
    return displayStartScreen();
}

void displayStartScreen() {
    u8g.firstPage();
    do {
        u8g.setFont(u8g_font_6x10);
        u8g.drawStr(32, 40, "SHEEP GAME");
    } while (u8g.nextPage());
}

void displayLoadingScreen() {
    for (int i = 0; i <= 3; i++) {
        u8g.firstPage();
        do {
            u8g.setFont(u8g_font_6x10);
            u8g.drawStr(20, 30, "Loading");
            for (int j = 0; j < i; j++) {
                u8g.drawStr(70 + j * 6, 30, ".");
            }
        } while (u8g.nextPage());
        delay(500);
    }
}
