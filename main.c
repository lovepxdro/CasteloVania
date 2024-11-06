#include "raylib.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define GRAVITY 500.0f
#define JUMP_FORCE -300.0f
#define PLAYER_SPEED 200.0f
#define BULLET_SPEED 500.0f
#define ENEMY_SPEED 100.0f
#define MAX_BULLETS 10
#define MAX_ENEMY_BULLETS 5

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

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "CasteloVania");

    // Cria e conecta salas
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
    int playerLife = 3; // Vida do jogador

    Bullet bullets[MAX_BULLETS] = {0};
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;

    srand(time(NULL)); // Seed para movimentos aleatórios do inimigo
    SetTargetFPS(60);

    bool movingRight = true; // Controla direção do boss

    while (!WindowShouldClose()) {
        // Verifica se o jogador ainda tem vidas
        if (playerLife <= 0) {
            DrawText("GAME OVER", screenWidth / 2 - 50, screenHeight / 2, 30, RED);
            break; // Encerra o jogo
        }

        // Movimentação do jogador
        if (IsKeyDown(KEY_D)) player.x += PLAYER_SPEED * GetFrameTime();
        if (IsKeyDown(KEY_A)) player.x -= PLAYER_SPEED * GetFrameTime();

        // Controle do pulo
        if (IsKeyPressed(KEY_W) && isGrounded) {
            playerSpeed.y = JUMP_FORCE;
            isGrounded = false;
        }

        // Aplicação da gravidade
        playerSpeed.y += GRAVITY * GetFrameTime();
        player.y += playerSpeed.y * GetFrameTime();

        // Colisão com o chão
        if (CheckCollisionRecs(player, salaAtual->floor)) {
            player.y = salaAtual->floor.y - player.height;
            playerSpeed.y = 0;
            isGrounded = true;
        }

        // Verifica transição de salas
        if (player.x + player.width > screenWidth) {  // Sai pela direita
            if (salaAtual->direita) {
                salaAtual = salaAtual->direita;
                player.x = 0;
            } else {
                player.x = screenWidth - player.width;
            }
        } else if (player.x < 0) {  // Sai pela esquerda
            if (salaAtual->esquerda) {
                salaAtual = salaAtual->esquerda;
                player.x = screenWidth - player.width;
            } else {
                player.x = 0;
            }
        }

        // Movimento do inimigo
        if (salaAtual->enemyAlive) {
            if (salaAtual->id == 5) { // Última sala: movimento horizontal contínuo no teto
                if (movingRight) {
                    salaAtual->enemy.x += ENEMY_SPEED * GetFrameTime();
                    if (salaAtual->enemy.x + salaAtual->enemy.width >= screenWidth) {
                        movingRight = false;
                    }
                } else {
                    salaAtual->enemy.x -= ENEMY_SPEED * GetFrameTime();
                    if (salaAtual->enemy.x <= 0) {
                        movingRight = true;
                    }
                }

                // Disparo aleatório do boss para baixo
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
            } else { // Outras salas: segue o jogador
                Vector2 direction = {player.x - salaAtual->enemy.x, player.y - salaAtual->enemy.y};
                float magnitude = sqrt(direction.x * direction.x + direction.y * direction.y);
                if (magnitude > 0) {
                    direction.x /= magnitude;
                    salaAtual->enemy.x += direction.x * ENEMY_SPEED * GetFrameTime();
                }
            }

            // Dano ao jogador ao encostar no inimigo
            if (CheckCollisionRecs(player, salaAtual->enemy)) {
                playerLife--;
                // Reposiciona o jogador para evitar perda de vida rápida em colisões contínuas
                player.x -= 50; 
            }
        }

        // Atualização dos projéteis do inimigo
        for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
            if (salaAtual->enemyBullets[i].active) {
                salaAtual->enemyBullets[i].rect.y += salaAtual->enemyBullets[i].speed.y * GetFrameTime();
                if (salaAtual->enemyBullets[i].rect.y > screenHeight) {
                    salaAtual->enemyBullets[i].active = false;
                }

                // Colisão do projétil do inimigo com o jogador
                if (CheckCollisionRecs(salaAtual->enemyBullets[i].rect, player)) {
                    playerLife--; // Reduz vida do jogador
                    salaAtual->enemyBullets[i].active = false;
                    if (playerLife <= 0) break;
                }
            }
        }

        // Criação de projétil do jogador
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
                        bullets[i].rect.height = 10; // Disparo vertical
                    }
                    break;
                }
            }
        }

        // Atualização dos projéteis do jogador
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                bullets[i].rect.x += bullets[i].speed.x * GetFrameTime();
                bullets[i].rect.y += bullets[i].speed.y * GetFrameTime();
                if (bullets[i].rect.x > screenWidth || bullets[i].rect.x < 0 || bullets[i].rect.y < 0) {
                    bullets[i].active = false;
                }

                // Colisão do projétil do jogador com o inimigo
                if (salaAtual->enemyAlive && CheckCollisionRecs(bullets[i].rect, salaAtual->enemy)) {
                    salaAtual->enemyLife -= 1;
                    bullets[i].active = false;
                    if (salaAtual->enemyLife <= 0) {
                        salaAtual->enemyAlive = false;
                    }
                }
            }
        }

        // Renderização
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Desenha o chão, teto, jogador e projéteis
        DrawRectangleRec(salaAtual->floor, DARKGRAY);
        DrawRectangleRec(salaAtual->ceiling, DARKGRAY);
        DrawRectangleRec(player, BLUE);

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                DrawRectangleRec(bullets[i].rect, RED);
            }
        }

        // Desenha o inimigo e seus projéteis
        if (salaAtual->enemyAlive) {
            DrawRectangleRec(salaAtual->enemy, MAROON);
            DrawText(TextFormat("Vida do inimigo: %d", salaAtual->enemyLife), salaAtual->enemy.x, salaAtual->enemy.y - 20, 10, BLACK);
        }
        for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
            if (salaAtual->enemyBullets[i].active) {
                DrawRectangleRec(salaAtual->enemyBullets[i].rect, DARKPURPLE);
            }
        }

        // Exibe a vida do jogador
        DrawText(TextFormat("Vida do jogador: %d", playerLife), 10, 10, 20, RED);

        EndDrawing();
    }

    liberaSalas(sala1);
    CloseWindow();

    return 0;
}
