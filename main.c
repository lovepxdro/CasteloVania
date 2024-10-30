#include "raylib.h"
#include <math.h>

#define GRAVITY 400.0f
#define JUMP_FORCE -300.0f
#define PLAYER_SPEED 200.0f
#define BULLET_SPEED 500.0f
#define ENEMY_SPEED 100.0f
#define MAX_BULLETS 10

typedef struct Bullet {
    Rectangle rect;
    bool active;
} Bullet;

int main(void)
{
    // Inicialização da tela
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "2D Game with Raylib");

    // Definição do jogador
    Rectangle player = { 100, screenHeight - 100, 50, 50 };
    Vector2 playerSpeed = { 0, 0 };
    bool isGrounded = false;

    // Definição do chão
    Rectangle floor = { 0, screenHeight - 50, screenWidth, 50 };

    // Configuração dos projéteis
    Bullet bullets[MAX_BULLETS] = { 0 };
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;

    // Definição do inimigo
    Rectangle enemy = { 600, screenHeight - 100, 50, 50 }; // Mantém a mesma altura do jogador
    int enemyHits = 0;
    bool enemyAlive = true;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Movimentação do jogador
        if (IsKeyDown(KEY_RIGHT)) player.x += PLAYER_SPEED * GetFrameTime();
        if (IsKeyDown(KEY_LEFT)) player.x -= PLAYER_SPEED * GetFrameTime();

        // Controle do pulo
        if (IsKeyPressed(KEY_SPACE) && isGrounded) {
            playerSpeed.y = JUMP_FORCE;
            isGrounded = false;
        }

        // Aplicação da gravidade
        playerSpeed.y += GRAVITY * GetFrameTime();
        player.y += playerSpeed.y * GetFrameTime();

        // Colisão com o chão
        if (CheckCollisionRecs(player, floor)) {
            player.y = floor.y - player.height;
            playerSpeed.y = 0;
            isGrounded = true;
        }

        // Colisão com as bordas da tela
        if (player.x <= 0) player.x = 0;
        if (player.x + player.width >= screenWidth) player.x = screenWidth - player.width;

        // Movimento do inimigo em direção ao jogador, se ele estiver vivo
        if (enemyAlive) {
            Vector2 direction = { player.x - enemy.x, player.y - enemy.y };
            float magnitude = sqrt(direction.x * direction.x + direction.y * direction.y);
            if (magnitude > 0) {
                direction.x /= magnitude; // Normaliza o vetor na direção x
                // Não muda a posição y do inimigo, ele sempre estará na altura do chão
                enemy.x += direction.x * ENEMY_SPEED * GetFrameTime();
            }
        }

        // Criação de um projétil ao pressionar a tecla F
        if (IsKeyPressed(KEY_F)) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].active) {
                    bullets[i].rect.x = player.x + player.width;
                    bullets[i].rect.y = player.y + player.height / 2;
                    bullets[i].rect.width = 10;
                    bullets[i].rect.height = 5;
                    bullets[i].active = true;
                    break;
                }
            }
        }

        // Atualização dos projéteis
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                bullets[i].rect.x += BULLET_SPEED * GetFrameTime();
                if (bullets[i].rect.x > screenWidth) bullets[i].active = false;

                // Verifica colisão entre projétil e inimigo
                if (enemyAlive && CheckCollisionRecs(bullets[i].rect, enemy)) {
                    bullets[i].active = false;
                    enemyHits++;
                    if (enemyHits >= 2) {
                        enemyAlive = false; // O inimigo "morre" após 2 acertos
                    }
                }
            }
        }

        // Verifica colisão entre jogador e inimigo
        if (enemyAlive && CheckCollisionRecs(player, enemy)) {
            DrawText("GAME OVER", screenWidth / 2 - 50, screenHeight / 2, 30, RED);
            break; // Encerra o jogo se o jogador encostar no inimigo
        }

        // Renderização
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Desenha o chão e o jogador
        DrawRectangleRec(floor, DARKGRAY);
        DrawRectangleRec(player, BLUE);

        // Desenha os projéteis ativos
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                DrawRectangleRec(bullets[i].rect, RED);
            }
        }

        // Desenha o inimigo se ele estiver "vivo"
        if (enemyAlive) {
            DrawRectangleRec(enemy, MAROON);
        }

        // Informações na tela
        DrawText("Use as setas para mover, ESPAÇO para pular, e F para disparar", 10, 10, 20, DARKGRAY);
        DrawText(isGrounded ? "No Chão" : "No Ar", 10, 40, 20, RED);
        if (!enemyAlive) {
            DrawText("Inimigo Derrotado!", 10, 70, 20, GREEN);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
