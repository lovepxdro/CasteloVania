#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define VOLUME 0.5f
#define MAX_OPTIONS 4
#define GRAVITY 500.0f
#define JUMP_FORCE -300.0f
#define PLAYER_SPEED 200.0f
#define BULLET_SPEED 500.0f
#define ENEMY_SPEED 160.0f
#define BOSS_SPEED 100.0f
#define MAX_SCORE 10
#define MAX_BULLETS 10
#define MAX_ENEMY_BULLETS 100

typedef enum {
    MENU = 0,   
    RANKING,
    JOGO,
    SAIR
} GameScreen;

typedef struct PlayerScore{
    char name[51];
    int score;
} PlayerScore;

typedef struct Bullet {
    Rectangle rect;
    Vector2 speed;
    bool active;
} Bullet;

typedef struct Sala {
    int id;
    struct Sala* esquerda;
    struct Sala* direita;
    Texture2D background;
    Rectangle floor;
    Rectangle ceiling;
    Rectangle enemy;
    bool enemyAlive;
    int enemyLife;
    Bullet enemyBullets[MAX_ENEMY_BULLETS];
} Sala;

char playerName[51] = "";
int charIndex = 0;
PlayerScore ranking[MAX_SCORE];

void saveScore(const char *name, int score) {
    if (name == NULL) {
        return;
    }

    // Open file for reading
    FILE *file = fopen("Ranking.txt", "r");
    if (!file) {
        printf("Oh no\n");
        return;
    }

    PlayerScore temp;
    bool nameExists = false;
    int currentCount = 0;

    // Temporary list to hold scores for rewriting
    PlayerScore scores[MAX_SCORE];

    // Read scores from file and check if the name exists
    while (fscanf(file, "%50s %d", temp.name, &temp.score) == 2) {
        scores[currentCount++] = temp;
        if (strcmp(temp.name, name) == 0) {
            nameExists = true;
            // Update score if the new one is higher
            if (score > temp.score) {
                scores[currentCount - 1].score = score;
            }
        }
    }
    fclose(file);

    // If the name doesn’t exist, add it to the list
    if (!nameExists) {
        strcpy(scores[currentCount].name, name);
        scores[currentCount].score = score;
        currentCount++;
    }

    // Order ranking with Bubble sort
    for (int i = 0; i < currentCount - 1; i++) {
        for (int j = 0; j < currentCount - 1 - i; j++) {
            if (scores[j].score < scores[j + 1].score) {
                // Switch positions
                PlayerScore temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }

    // Rewrite the file with updated scores
    file = fopen("Ranking.txt", "w");
    if (!file) {
        printf(":(\n");
        return;
    }

    for (int i = 0; i < currentCount && i < MAX_SCORE; i++) {
        if (strcmp(scores[i].name, "") == 0 || strcmp(scores[i].name, " ") == 0) {
            break;
        }
        fprintf(file, "%s %d\n", scores[i].name, (int)scores[i].score);
        printf("Saving %s %d to file\n", scores[i].name, (int)scores[i].score);  // Debugging print
    }
    fclose(file);
}


int contScore(PlayerScore *ranking) {
    FILE *file = fopen("Ranking.txt", "r");  // Open in read mode
    if (!file) {
        printf("Oh oh\n");
        return 0;
    }

    int cont = 0;
    while (cont < MAX_SCORE && fscanf(file, "%50s %d", ranking[cont].name, &ranking[cont].score) == 2) {
        cont++;
    }
    fclose(file);

    // Order using bubble sort
    for (int i = 0; i < cont - 1; i++) {
        for (int j = 0; j < cont - 1 - i; j++) {
            if (ranking[j].score < ranking[j + 1].score) {
                // Switch positions
                PlayerScore temp = ranking[j];
                ranking[j] = ranking[j + 1];
                ranking[j + 1] = temp;
            }
        }
    }

    return cont;
}

void Transition(Texture2D image1, Texture2D image2, float delayTime) {
    float alpha = 0.0f; // Alpha para a transição
    bool transitioning = false;
    float timer = 0.0f;

    while (!WindowShouldClose()) {
        // Atualiza o timer para o delay da imagem inicial
        if (!transitioning) {
            timer += GetFrameTime();
            if (timer >= delayTime) {
                transitioning = true;  // Inicia a transição após o tempo de espera
            }
        }

        // Atualiza o valor de alpha durante a transição
        if (transitioning) {
            alpha += 0.01f; // Velocidade da transição
            if (alpha >= 1.0f) {
                alpha = 1.0f;
                break; // Termina a transição e sai do loop
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Desenha as duas imagens com alpha ajustado
        DrawTexture(image1, 0, 0, WHITE);
        DrawTexture(image2, 0, 0, Fade(WHITE, alpha)); // A próxima imagem vai aparecendo

        EndDrawing();
    }
}


GameScreen Menu(void) {
    InitAudioDevice();
    
    Texture2D image1 = LoadTexture("./images/castelo.png");
    Texture2D image2 = LoadTexture("./images/AAAAAAAAAAAAAAAAAAA.png");
    Transition(image1, image2, 2.0f);

    // Libera as texturas usadas na transição
    UnloadTexture(image1);
    UnloadTexture(image2);
    
    Music menuMusic = LoadMusicStream("./music/Castlevania (NES) Music - Boss Battle Poison Mind.mp3");
    SetMusicVolume(menuMusic, 0.5f);
    PlayMusicStream(menuMusic);
    const int screenWidth = 800;
    const int screenHeight = 420;
    int selectedOption = 0;
    int numRanking = contScore(ranking);
    const char *menuOptions[MAX_OPTIONS] = { "Iniciar", "Instruções", "Ranking", "Sair" };
    
    Color semiTransparent = (Color){255, 255, 255, 128};
    
    Texture2D menuBackground = LoadTexture("./images/AAAAAAAAAAAAAAAAAAA.png");
    
    while (!WindowShouldClose()) {
        UpdateMusicStream(menuMusic);
        // Controle de navegação no menu
        if (IsKeyPressed(KEY_DOWN)) selectedOption = (selectedOption + 1) % MAX_OPTIONS;
        if (IsKeyPressed(KEY_UP)) selectedOption = (selectedOption - 1 + MAX_OPTIONS) % MAX_OPTIONS;

        // Verifica se a opção foi selecionada com Enter
        if (IsKeyPressed(KEY_ENTER)) {
            if (selectedOption == 0) {
                StopMusicStream(menuMusic);  // Para a música antes de iniciar o jogo
                UnloadMusicStream(menuMusic); // Descarrega o stream de música do menu
                UnloadTexture(menuBackground);
                CloseAudioDevice();
                return JOGO;  // Selecionou "Iniciar", encerra o menu e inicia o jogo
            } else if (selectedOption == 3) {
                StopMusicStream(menuMusic);  // Para a música antes de iniciar o jogo
                UnloadMusicStream(menuMusic); // Descarrega o stream de música do menu
                UnloadTexture(menuBackground);
                CloseAudioDevice();
                return SAIR;  // Selecionou "Sair", encerra o programa
            }
        }

        // Desenho do menu dividido
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            DrawTexture(menuBackground, 0, 0, WHITE);

            // Divisão da tela
            DrawRectangle(0, 0, screenWidth / 2, screenHeight, semiTransparent);
            DrawRectangle(screenWidth / 2, 0, screenWidth / 2, screenHeight, semiTransparent);

            // Exibe o título do menu
            DrawText("Menu Principal", screenWidth / 4 - MeasureText("Menu Principal", 30) / 2, 50, 30, DARKBLUE);

            // Exibe as opções do menu na área esquerda
            for (int i = 0; i < MAX_OPTIONS; i++) {
                Color color = (i == selectedOption) ? RED : BLACK;
                DrawText(menuOptions[i], screenWidth / 4 - MeasureText(menuOptions[i], 20) / 2, 150 + i * 40, 20, color);
            }

            // Exibe informações na área direita automaticamente para "Instruções" e "Ranking"
            if (selectedOption == 1) {  // Instruções
                DrawText("Instruções", screenWidth * 3 / 4 - MeasureText("Instruções", 30) / 2, 50, 30, DARKBLUE);
                DrawText("Use W-A-D para movimentar o personagem.", screenWidth / 2 + 20, 150, 17, BLACK);
                DrawText("Use as setas para disparar.", screenWidth / 2 + 20, 190, 17, BLACK);
                DrawText("Aperte ESC para parar de jogar.", screenWidth / 2 + 20, 230, 17, BLACK);
            } else if (selectedOption == 2) {  // Ranking
                DrawText("Ranking", screenWidth * 3 / 4 - MeasureText("Ranking", 30) / 2, 50, 30, DARKBLUE);
                DrawText("Top 10 melhores pontuações!", screenWidth / 2 + 20, 130, 17, BLACK);
                for(int i = 0; i < numRanking; i++){
                    char text[100];
                    snprintf(text, sizeof(text), "%d. %s - %d", i + 1, ranking[i].name, ranking[i].score);
                    DrawText(text, screenWidth / 2 + 20, 160 + i * 25, 17, BLACK);
                }
            }
        EndDrawing();
    }
    UnloadMusicStream(menuMusic);
    CloseAudioDevice();
    UnloadTexture(menuBackground);
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
        sala->enemyAlive = true;
        sala->background = LoadTexture("./images/scenesample.gif");
        sala->enemy = (Rectangle){0, sala->ceiling.y + sala->ceiling.height, 50, 50}; // Posição inicial perto do teto
        sala->enemyLife = 130;
    } else {
        sala->background = LoadTexture("./images/8d830da54b4e5a98f5734a62fcae4be1ebc505db_2_1035x582.gif");
        if(id>1){
            sala->enemyAlive = true;
            sala->enemy = (Rectangle){600, 450 - 100, 50, 50}; // Inimigo no chão
            sala->enemyLife = 30;
        }else if(id == 1){
            sala->enemyAlive = false;
        }
    }

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

void GetPlayerName(){
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawText("Coloque o seu nome:", screenWidth / 2 - MeasureText("Coloque o seu nome:", 20) / 2, screenHeight / 2 - 50, 20, DARKBLUE);
        DrawText(playerName, screenWidth / 2 - MeasureText(playerName, 20) / 2, screenHeight / 2, 20, BLACK);
        
        EndDrawing();
        
        int key = GetKeyPressed();
        if(key >= 32 && key <= 125 && charIndex < 50){
            playerName[charIndex++] = (char)key;
            playerName[charIndex] = '\0';
        }
        if(IsKeyPressed(KEY_BACKSPACE) && charIndex > 0){
            playerName[--charIndex] = '\0';
        }
        if(IsKeyPressed(KEY_ENTER)){
            break;
        }
    }
}

int GameLoop(void) {
    InitAudioDevice();
    Music lvlMusic;
    Music musicBoss = LoadMusicStream("./music/Castlevania (NES)_ Black Night (Extended) [ ezmp3.cc ].mp3");
    Music music = LoadMusicStream("./music/Castlevania (NES)_ Wicked Child (Extended) [ ezmp3.cc ].mp3");
    Sound enemyMage = LoadSound("./sounds/Retro Event 19.wav");
    Sound enemyHowl = LoadSound("./sounds/Retro Wolf B 02.wav");
    Sound playerShoot = LoadSound("./sounds/Retro Gun SingleShot 04.mp3");
    SetSoundVolume(playerShoot, VOLUME);
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
    int enemyDiedCount = 0;
    int score;
    bool isGrounded = false;
    int playerLife = 1;

    Bullet bullets[MAX_BULLETS] = {0};
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;

    srand(time(NULL));
    SetTargetFPS(60);
    bool movingRight = true;
    double countdownTime = 120.0;
    
    PlayMusicStream(music);
    
    lvlMusic = music;

    while (!WindowShouldClose()) {
        UpdateMusicStream(lvlMusic);
        if (playerLife <= 0) {
        StopMusicStream(lvlMusic);
        ClearBackground(RAYWHITE);
        BeginDrawing();
        
        const char *gameOverText = "GAME OVER";
        int textWidth = MeasureText(gameOverText, 30);
        DrawText(gameOverText, (800 - textWidth) / 2, 420 / 2, 30, RED);

        EndDrawing();
        sleep(3);
        break;
    }
        
         countdownTime -= GetFrameTime();
        // countdownTime -= 5;

         if (countdownTime <= 0) {
            StopMusicStream(lvlMusic);
            ClearBackground(RAYWHITE);
            BeginDrawing();

            const char *timeoutText = "TEMPO ESGOTADO";
            int textWidth = MeasureText(timeoutText, 30);
            DrawText(timeoutText, (800 - textWidth) / 2, 420 / 2, 30, RED);

            EndDrawing();
            sleep(3);
            break;
        }
        
        if (salaAtual->enemyAlive == false && salaAtual == sala5){
            StopMusicStream(lvlMusic);
            ClearBackground(RAYWHITE);
            BeginDrawing();
            DrawText("VITÓRIA!", screenWidth / 2 - 100, screenHeight / 2, 30, GREEN);
            //TODO: Animação de Vitória
            EndDrawing();
            sleep(3);
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
        
        if(salaAtual->enemyAlive == false){
            if (player.x + player.width > screenWidth) {
                if (salaAtual->direita) {
                    salaAtual = salaAtual->direita;
                    if((salaAtual->enemyAlive == true) && (salaAtual != sala5)){
                        PlaySound(enemyHowl);
                    }else if((salaAtual->enemyAlive == true) && (salaAtual == sala5)){
                        PlaySound(enemyMage);
                    }
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
        }

        if (salaAtual->enemyAlive) {
            
            if(player.x < 0) player.x = 0;
            if(player.x > screenWidth - player.width) player.x = screenWidth - player.width;
            
            if (salaAtual->id == 5) {
                if(IsMusicStreamPlaying(music)){
                    StopMusicStream(lvlMusic);
                    lvlMusic = musicBoss;
                    PlayMusicStream(lvlMusic);
                }
                if (movingRight) {
                    salaAtual->enemy.x += BOSS_SPEED * GetFrameTime();
                    if (salaAtual->enemy.x + salaAtual->enemy.width >= screenWidth) movingRight = false;
                } else {
                    salaAtual->enemy.x -= BOSS_SPEED * GetFrameTime();
                    if (salaAtual->enemy.x <= 0) movingRight = true;
                }

                if (rand() % 8 == 0) {
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
            PlaySound(playerShoot);
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
                    if (salaAtual->enemyLife <= 0){
                      salaAtual->enemyAlive = false;
                      enemyDiedCount += 1;
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        if(salaAtual == sala5){
            DrawTextureEx(salaAtual->background, (Vector2){-20, 10}, 0.0f, 1.8f, RAYWHITE);
        }else{
            DrawTexture(salaAtual->background, 0, -90, RAYWHITE);
        }
        
        DrawRectangleLines(0, 450 - 50, 800, 50, DARKGRAY);
        DrawRectangleRec(player, BLUE);
        if (salaAtual->enemyAlive) DrawRectangleRec(salaAtual->enemy, RED);
        for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].active) DrawRectangleRec(bullets[i].rect, BLACK);
        for (int i = 0; i < MAX_ENEMY_BULLETS; i++) if (salaAtual->enemyBullets[i].active) DrawRectangleRec(salaAtual->enemyBullets[i].rect, WHITE);
        
        int minutes = (int)(countdownTime / 60);
        int seconds = (int)(countdownTime) % 60;
        DrawText(TextFormat("Tempo: %02d:%02d", minutes, seconds), screenWidth - 150, 10, 20, WHITE);

        DrawText(TextFormat("Vidas: %d", playerLife), 10, 10, 20, WHITE);
        EndDrawing();
    }
    
    score = enemyDiedCount * (int)countdownTime;
    GetPlayerName();
    
    liberaSalas(sala1);
    UnloadSound(enemyMage);
    UnloadSound(enemyHowl);
    UnloadSound(playerShoot);
    UnloadMusicStream(musicBoss);
    UnloadMusicStream(music);
    CloseAudioDevice();
    
    return score;
    
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 420;
    InitWindow(screenWidth, screenHeight, "CasteloVania");

    SetTargetFPS(60);
    
    //TODO: Animação de Entrada
    
    GameScreen currentScreen = MENU;
    while (currentScreen == MENU) {
        currentScreen = Menu();
    }

    if (currentScreen == SAIR) {
        CloseWindow();
        return 0;
    }
    
    int score = GameLoop();  // Just call GameLoop once and get the score
    
    // Only save if we got a valid score (greater than 0)
    saveScore(playerName, score);

    CloseWindow();
    return 0;
}