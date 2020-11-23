#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include "Map.h"
#include "Util.h"
#include "Scene.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"

#include "Menu.h"


#include <vector>

// Configuration
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.016666f

#define LEVEL1_WIDTH 22
#define LEVEL1_HEIGHT 8



Scene* currentScene;
Scene* sceneArray[4];

void SwitchToScene(Scene* scene) {
    currentScene = scene;
    currentScene->Initialize();
}

SDL_Window *displayWindow;
bool gameIsRunning = true;
GLuint fontTextureID;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

enum GameStatus
{
    GAME_MENU,
    GAME_PLAY,
    GAME_LOSE,
    GAME_WIN
};

enum Scenes
{
    SCENE_MENU = 0,
    SCENE_LEVEL_1 = 1,
    SCENE_LEVEL_2 = 2,
    SCENE_LEVEL_3 = 3
};

GameStatus gameStatus = GAME_MENU;
Mix_Music* music;
Mix_Chunk* jump;
Mix_Chunk* sword_swing;

unsigned int playerLives = 3;

void Initialize()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("Hack and Slash Sidescroller", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    sceneArray[SCENE_MENU] = new Menu();
    sceneArray[SCENE_LEVEL_1] = new Level1();
    sceneArray[SCENE_LEVEL_2] = new Level2();
    sceneArray[SCENE_LEVEL_3] = new Level3();
    SwitchToScene(sceneArray[0]);

    // Audio
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    music = Mix_LoadMUS("assets/Alexander_Foerster_-_Fantasy_Frontier (CC-BY-NC-SA).mp3");
    Mix_PlayMusic(music, -1);
    jump = Mix_LoadWAV("assets/jump_effect.wav");
    sword_swing = Mix_LoadWAV("assets/sword_swing.wav");


    // Font Texture
    fontTextureID = Util::LoadTexture("assets/font1.png");
}

void ProcessInput()
{

    SDL_Event event;
    Entity* player = currentScene->state.player;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
                if (gameStatus == GAME_PLAY && player->collidedBottom && player->moveState != ATTACK)
                {
                    player->SetMoveState(JUMP);
                    Mix_PlayChannel(-1, jump, 0);
                }
                break;
            case SDLK_SPACE:
                if (gameStatus == GAME_PLAY && player->collidedBottom && player->moveState != ATTACK) {
                    player->SetMoveState(ATTACK);
                    Mix_PlayChannel(-1, sword_swing, 0);
                }
                break;
            case SDLK_RETURN:
                if (currentScene == sceneArray[0]) {
                    gameStatus = GAME_PLAY;
                    currentScene->state.nextScene = 1;
                }
            }

            break; // SDL_KEYDOWN
        }
    }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (gameStatus == GAME_PLAY && currentScene != sceneArray[0]) {
        // Disable other controls during attack or jump
        if (player->moveState == ATTACK || !(player->collidedBottom))
            return;

        if (keys[SDL_SCANCODE_LEFT])
        {
            if (player->direction != LEFT)
                player->direction = LEFT;
            if (player->moveState != RUN)
                player->SetMoveState(RUN);
        }
        else if (keys[SDL_SCANCODE_RIGHT])
        {
            if (player->direction != RIGHT)
            {
                player->direction = RIGHT;
            }
            if (player->moveState != RUN)
                player->SetMoveState(RUN);
        }
        else
        {
            if (player->moveState != IDLE)
                player->SetMoveState(IDLE);
        }
    }
}

float lastTicks = 0.0f;
float accumulator = 0.0f;

void Update()
{

    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        accumulator = deltaTime;
        return;
    }
    while (deltaTime >= FIXED_TIMESTEP)
    {
        currentScene->Update(FIXED_TIMESTEP);
        // Show game win text if player kills all enemies on last scene
        if (currentScene->state.aliveEnemyNum == 0 && currentScene == sceneArray[3])
                gameStatus = GAME_WIN;
        deltaTime -= FIXED_TIMESTEP;
    }
    accumulator = deltaTime;


    // side scrolling
    viewMatrix = glm::mat4(1.0f);

    if (currentScene->state.player->position.x > 5)
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-(currentScene->state.player->position.x), 3.75, 0));
    else
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 3.75, 0));

    // End game when player dies
    if (!(currentScene->state.player->alive))
    {   
        if (--playerLives == 0) gameStatus = GAME_LOSE;
        // respawn
        else currentScene->Initialize();
    }
}
const char* gameEndText;
void Render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    program.SetViewMatrix(viewMatrix);
    currentScene->Render(&program);

    // Display Text when Game Ends
    switch (gameStatus)
    {
        case GAME_LOSE:
            gameEndText = "You Lose";
            break;
        case GAME_WIN:
            gameEndText = "You Win";
            break;
    }
    if (gameEndText)
        Util::DrawText(&program, fontTextureID, gameEndText, 1.0f, -0.5f, glm::vec3(currentScene->state.player->position.x - 1.0f, -0.8f, 0.0f));


    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown()
{
    // Free memory used for audio
    Mix_FreeChunk(jump);
    Mix_FreeChunk(sword_swing);
    Mix_FreeMusic(music);
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    Initialize();

    while (gameIsRunning)
    {
        ProcessInput();
        // Extra check for SCENE_MENU due to ProcessInput changing gameStatus on Enter during Menu
        if (gameStatus == GAME_PLAY && currentScene != sceneArray[SCENE_MENU])
            Update();
        if (currentScene->state.nextScene != -1) SwitchToScene(sceneArray[currentScene->state.nextScene]);
        Render();
    }

    Shutdown();
    return 0;
}
