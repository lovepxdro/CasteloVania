#include "raylib.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_OPTIONS 4
#define GRAVITY 500.0f
#define JUMP_FORCE -300.0f
#define PLAYER_SPEED 200.0f
#define BULLET_SPEED 500.0f
#define ENEMY_SPEED 100.0f
#define MAX_BULLETS 10
#define MAX_ENEMY_BULLETS 5

typedef enum {
    MENU = 0,
    JOGO,
    SAIR
} GameScreen;

typedef struct Bullet {
    Rectangle rect;
    Vector2 speed;
    bool active;
} Bullet;

typedef struct Sala {
    int id;
    struct Sala* esquerda;
    struct Sala* direita;
    Rectangle floor;
    Rectangle ceiling;
    Rectangle enemy;
    bool enemyAlive;
    int enemyLife;
    Bullet enemyBullets[MAX_ENEMY_BULLETS];
} Sala;

GameScreen Menu(void) {
    const int screenWidth = 800;
    const int screenHeight = 420;
    int selectedOption = 0;
    const char *menuOptions[MAX_OPTIONS] = { "Iniciar", "Instruções", "Ranking", "Sair" };

    while (!WindowShouldClose()) {
        // Controle de navegação no menu
        if (IsKeyPressed(KEY_DOWN)) selectedOption = (selectedOption + 1) % MAX_OPTIONS;
        if (IsKeyPressed(KEY_UP)) selectedOption = (selectedOption - 1 + MAX_OPTIONS) % MAX_OPTIONS;

        // Verifica se a opção foi selecionada com Enter
        if (IsKeyPressed(KEY_ENTER)) {
            if (selectedOption == 0) {
                return JOGO;  // Selecionou "Iniciar", encerra o menu e inicia o jogo
            } else if (selectedOption == 3) {
                return SAIR;  // Selecionou "Sair", encerra o programa
            }
        }

        // Desenho do menu dividido
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Divisão da tela
            DrawRectangle(0, 0, screenWidth / 2, screenHeight, LIGHTGRAY);
            DrawRectangle(screenWidth / 2, 0, screenWidth / 2, screenHeight, RAYWHITE);

            // Exibe o título do menu
            DrawText("Menu Principal", screenWidth / 4 - MeasureText("Menu Principal", 30) / 2, 50, 30, DARKBLUE);

            // Exibe as opções do menu na área esquerda
            for (int i = 0; i < MAX_OPTIONS; i++) {
                Color color = (i == selectedOption) ? RED : DARKGRAY;
                DrawText(menuOptions[i], screenWidth / 4 - MeasureText(menuOptions[i], 20) / 2, 150 + i * 40, 20, color);
            }

            // Exibe informações na área direita automaticamente para "Instruções" e "Ranking"
            if (selectedOption == 1) {  // Instruções
                DrawText("Instruções", screenWidth * 3 / 4 - MeasureText("Instruções", 30) / 2, 50, 30, DARKBLUE);
                DrawText("Use W-A-D para movimentar o personagem.", screenWidth / 2 + 20, 150, 17, DARKGRAY);
                DrawText("Use as setas para disparar.", screenWidth / 2 + 20, 190, 17, DARKGRAY);
            } else if (selectedOption == 2) {  // Ranking
                DrawText("Ranking", screenWidth * 3 / 4 - MeasureText("Ranking", 30) / 2, 50, 30, DARKBLUE);
                DrawText("Veja as pontuações mais altas aqui!", screenWidth / 2 + 20, 150, 17, DARKGRAY);
                DrawText("Ainda não há pontuações disponíveis.", screenWidth / 2 + 20, 190, 17, DARKGRAY);
            }

        EndDrawing();
    }

    return SAIR;
}

Sala* criaSala(int id) {
    Sala* sala = (Sala*)malloc(sizeof(Sala));
    sala->id = id;
    sala->esquerda = NULL;
    sala->direita = NULL;
    sala->floor = (Rectangle){0, 450 - 50, 800, 50};
    sala->ceiling = (Rectangle){0, 0, 800, 50}; // Teto

    if (id == 5) {  // Última sala (boss)
        sala->enemy = (Rectangle){0, sala->ceiling.y + sala->ceiling.height, 50, 50}; // Posição inicial perto do teto
        sala->enemyLife = 100;
    } else if(id>1) {
        sala->enemy = (Rectangle){600, 450 - 100, 50, 50}; // Inimigo no chão
        sala->enemyLife = 10;
    }
    sala->enemyAlive = true;

    // Inicializa os projéteis do inimigo
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        sala->enemyBullets[i].active = false;
    }
    return sala;
}

void liberaSalas(Sala* sala) {
    while (sala) {
        Sala* prox = sala->direita;
        free(sala);
        sala = prox;
    }
}

void GameLoop(void) {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "CasteloVania");

    Sala* sala1 = criaSala(1);
    Sala* sala2 = criaSala(2);
    Sala* sala3 = criaSala(3);
    Sala* sala4 = criaSala(4);
    Sala* sala5 = criaSala(5);
    sala1->direita = sala2;
    sala2->esquerda = sala1;
    sala2->direita = sala3;
    sala3->esquerda = sala2;
    sala3->direita = sala4;
    sala4->esquerda = sala3;
    sala4->direita = sala5;
    sala5->esquerda = sala4;

    Sala* salaAtual = sala1;

    Rectangle player = {100, screenHeight - 100, 50, 50};
    Vector2 playerSpeed = {0, 0};
    bool isGrounded = false;
    int playerLife = 3;

    Bullet bullets[MAX_BULLETS] = {0};
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;

    srand(time(NULL));
    SetTargetFPS(60);
    bool movingRight = true;

    while (!WindowShouldClose()) {
        if (playerLife <= 0) {
            DrawText("GAME OVER", screenWidth / 2 - 50, screenHeight / 2, 30, RED);
            break;
        }

        if (IsKeyDown(KEY_D)) player.x += PLAYER_SPEED * GetFrameTime();
        if (IsKeyDown(KEY_A)) player.x -= PLAYER_SPEED * GetFrameTime();

        if (IsKeyPressed(KEY_W) && isGrounded) {
            playerSpeed.y = JUMP_FORCE;
            isGrounded = false;
        }

        playerSpeed.y += GRAVITY * GetFrameTime();
        player.y += playerSpeed.y * GetFrameTime();

        if (CheckCollisionRecs(player, salaAtual->floor)) {
            player.y = salaAtual->floor.y - player.height;
            playerSpeed.y = 0;
            isGrounded = true;
        }

        if (player.x + player.width > screenWidth) {
            if (salaAtual->direita) {
                salaAtual = salaAtual->direita;
                player.x = 0;
            } else {
                player.x = screenWidth - player.width;
            }
        } else if (player.x < 0) {
            if (salaAtual->esquerda) {
                salaAtual = salaAtual->esquerda;
                player.x = screenWidth - player.width;
            } else {
                player.x = 0;
            }
        }

        if (salaAtual->enemyAlive) {
            if (salaAtual->id == 5) {
                if (movingRight) {
                    salaAtual->enemy.x += ENEMY_SPEED * GetFrameTime();
                    if (salaAtual->enemy.x + salaAtual->enemy.width >= screenWidth) movingRight = false;
                } else {
                    salaAtual->enemy.x -= ENEMY_SPEED * GetFrameTime();
                    if (salaAtual->enemy.x <= 0) movingRight = true;
                }

                if (rand() % 50 == 0) {
                    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
                        if (!salaAtual->enemyBullets[i].active) {
                            salaAtual->enemyBullets[i].rect = (Rectangle){salaAtual->enemy.x + salaAtual->enemy.width / 2, salaAtual->enemy.y + salaAtual->enemy.height, 5, 10};
                            salaAtual->enemyBullets[i].speed = (Vector2){0, BULLET_SPEED};
                            salaAtual->enemyBullets[i].active = true;
                            break;
                        }
                    }
                }
            } else {
                Vector2 direction = {player.x - salaAtual->enemy.x, player.y - salaAtual->enemy.y};
                float magnitude = sqrt(direction.x * direction.x + direction.y * direction.y);
                if (magnitude > 0) {
                    direction.x /= magnitude;
                    salaAtual->enemy.x += direction.x * ENEMY_SPEED * GetFrameTime();
                }
            }

            if (CheckCollisionRecs(player, salaAtual->enemy)) {
                playerLife--;
                player.x -= 50;
            }
        }

        for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
            if (salaAtual->enemyBullets[i].active) {
                salaAtual->enemyBullets[i].rect.y += salaAtual->enemyBullets[i].speed.y * GetFrameTime();
                if (salaAtual->enemyBullets[i].rect.y > screenHeight) salaAtual->enemyBullets[i].active = false;

                if (CheckCollisionRecs(salaAtual->enemyBullets[i].rect, player)) {
                    playerLife--;
                    salaAtual->enemyBullets[i].active = false;
                    if (playerLife <= 0) break;
                }
            }
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_UP)) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].active) {
                    bullets[i].rect.x = player.x + player.width / 2;
                    bullets[i].rect.y = player.y + player.height / 2;
                    bullets[i].rect.width = 10;
                    bullets[i].rect.height = 5;

                    if (IsKeyPressed(KEY_RIGHT)) {
                        bullets[i].active = true;
                        bullets[i].speed = (Vector2){BULLET_SPEED, 0};
                    } else if (IsKeyPressed(KEY_LEFT)) {
                        bullets[i].active = true;
                        bullets[i].speed = (Vector2){-BULLET_SPEED, 0};
                    } else if (IsKeyPressed(KEY_UP)) {
                        bullets[i].active = true;
                        bullets[i].speed = (Vector2){0, -BULLET_SPEED};
                        bullets[i].rect.height = 10;
                    }
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                bullets[i].rect.x += bullets[i].speed.x * GetFrameTime();
                bullets[i].rect.y += bullets[i].speed.y * GetFrameTime();
                if (bullets[i].rect.x > screenWidth || bullets[i].rect.x < 0 || bullets[i].rect.y < 0) bullets[i].active = false;

                if (salaAtual->enemyAlive && CheckCollisionRecs(bullets[i].rect, salaAtual->enemy)) {
                    salaAtual->enemyLife -= 1;
                    bullets[i].active = false;
                    if (salaAtual->enemyLife <= 0) salaAtual->enemyAlive = false;
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangleRec(salaAtual->floor, DARKGRAY);
        DrawRectangleRec(player, BLUE);
        if (salaAtual->enemyAlive) DrawRectangleRec(salaAtual->enemy, RED);
        for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].active) DrawRectangleRec(bullets[i].rect, BLACK);
        for (int i = 0; i < MAX_ENEMY_BULLETS; i++) if (salaAtual->enemyBullets[i].active) DrawRectangleRec(salaAtual->enemyBullets[i].rect, PURPLE);

        DrawText(TextFormat("Vidas: %d", playerLife), 10, 10, 20, RED);
        EndDrawing();
    }

    liberaSalas(sala1);
    CloseWindow();
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 420;
    InitWindow(screenWidth, screenHeight, "CasteloVania");

    SetTargetFPS(60);

    // Continua chamando o menu até que a opção "Iniciar" seja escolhida
    GameScreen currentScreen = MENU;
    while (currentScreen == MENU) {
        currentScreen = Menu();
    }

    // Se a opção selecionada foi "Sair", fecha a janela
    if (currentScreen == SAIR) {
        CloseWindow();
        return 0;
    }

    // Lógica do jogo principal
    while (!WindowShouldClose()) {
        // Volta ao menu principal ao pressionar ESC
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentScreen = Menu();
            if (currentScreen == SAIR) break;
        }
        CloseWindow();
        GameLoop();
    }

    CloseWindow();
    return 0;
}
