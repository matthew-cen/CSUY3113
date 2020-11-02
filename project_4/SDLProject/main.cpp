#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include "Map.h"
#include "Util.h"

#include <vector>

// Configuration
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.016666f

#define LEVEL1_WIDTH 16
#define LEVEL1_HEIGHT 8

enum GameMode
{
    GAME_PLAY,
    GAME_LOSE,
    GAME_WIN
};

struct GameState
{
    std::vector<Entity *> entities;
    Entity *player;
    GameMode gameMode = GAME_PLAY;
    Map *map;
};

GameState state;

unsigned int level1_data[] =
    {
        5, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 5,
        5, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 5,
        5, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 5,
        5, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 0, 1, 1, 1, 5,
        5, 15, 15, 15, 15, 15, 15, 15, 0, 1, 1, 5, 5, 5, 5, 5,
        5, 15, 15, 15, 15, 0, 1, 1, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 1, 1, 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

SDL_Window *displayWindow;
bool gameIsRunning = true;
GLuint fontTextureID;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

void Initialize()
{
    SDL_Init(SDL_INIT_VIDEO);
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

    // Font
    fontTextureID = Util::LoadTexture("assets/font1.png");

    // Load Map
    GLuint mapTilesTextureID = Util::LoadTexture("assets/Tileset.png"); // 6x8 Tile Set
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTilesTextureID, 1.0f, 8, 6);

    // Load Sprites
    GLuint playerTextureID = Util::LoadTexture("assets/adventurer.png"); // 6x8 Tile Set
    state.player = new Entity(PLAYER, playerTextureID);
    state.player->animIdleLeft = new std::vector<int>{18, 19, 20};
    state.player->animIdleRight = new std::vector<int>{0, 1, 2};
    state.player->animMoveLeft = new std::vector<int>{25, 26, 27, 28, 29, 30};
    state.player->animMoveRight = new std::vector<int>{7, 8, 9, 10, 11, 12};
    state.player->animAttackLeft = new std::vector<int>{31, 32, 33, 34, 35};
    state.player->animAttackRight = new std::vector<int>{13, 14, 15, 16, 17};

    state.player->animIndex = 0;
    state.player->animCols = 18;
    state.player->animRows = 2;
    state.player->acceleration.y = -9.81f;
    state.player->position.x = 1.0f;
    state.player->position.y = -1.0f;
    state.player->direction = RIGHT;
    state.player->SetMoveState(IDLE);

    state.entities.push_back(state.player);

    GLuint houndTextureID = Util::LoadTexture("assets/enemies/hell-hound-idle-1-USE.png"); // 6x8 Tile Set
    Entity *enemy = new Entity(ENEMY, houndTextureID);
    enemy->animIdleLeft = new std::vector<int>{0};
    enemy->animIdleRight = new std::vector<int>{0};
    enemy->animMoveLeft = new std::vector<int>{0};
    enemy->animMoveRight = new std::vector<int>{0};
    enemy->animAttackLeft = new std::vector<int>{0};
    enemy->animAttackRight = new std::vector<int>{0};
    enemy->animIndices = enemy->animIdleRight;
    enemy->animIndex = 0;
    enemy->animCols = 1;
    enemy->animRows = 1;
    enemy->acceleration.y = -9.81f;
    enemy->position.x = 5.5f;
    enemy->position.y = -0.5f;
    enemy->direction = RIGHT;
    enemy->aiState = JUMPING;
    enemy->SetMoveState(IDLE);
    state.entities.push_back(enemy);

    enemy = new Entity(ENEMY, houndTextureID);
    enemy->animIdleLeft = new std::vector<int>{0};
    enemy->animIdleRight = new std::vector<int>{0};
    enemy->animMoveLeft = new std::vector<int>{0};
    enemy->animMoveRight = new std::vector<int>{0};
    enemy->animAttackLeft = new std::vector<int>{0};
    enemy->animAttackRight = new std::vector<int>{0};
    enemy->animIndices = enemy->animIdleRight;
    enemy->animIndex = 0;
    enemy->animCols = 1;
    enemy->animRows = 1;
    enemy->acceleration.y = -9.81f;
    enemy->position.x = 8.5f;
    enemy->position.y = 0.5f;
    enemy->direction = RIGHT;
    enemy->aiState = JUMPING;
    enemy->SetMoveState(IDLE);
    state.entities.push_back(enemy);

    enemy = new Entity(ENEMY, houndTextureID);
    enemy->animIdleLeft = new std::vector<int>{0};
    enemy->animIdleRight = new std::vector<int>{0};
    enemy->animMoveLeft = new std::vector<int>{0};
    enemy->animMoveRight = new std::vector<int>{0};
    enemy->animAttackLeft = new std::vector<int>{0};
    enemy->animAttackRight = new std::vector<int>{0};
    enemy->animIndices = enemy->animIdleRight;
    enemy->animIndex = 0;
    enemy->animCols = 1;
    enemy->animRows = 1;
    enemy->acceleration.y = -9.81f;
    enemy->position.x = 11.5f;
    enemy->position.y = 1.5f;
    enemy->direction = RIGHT;
    enemy->aiState = JUMPING;
    enemy->SetMoveState(IDLE);
    state.entities.push_back(enemy);
}

void ProcessInput()
{

    SDL_Event event;
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
            case SDLK_LEFT:
                break;

            case SDLK_RIGHT:
                break;
            case SDLK_UP:
                if (state.player->collidedBottom && state.player->moveState != ATTACK)
                    state.player->SetMoveState(JUMP);
                break;
            case SDLK_SPACE:
                if (state.player->collidedBottom && state.player->moveState != ATTACK)
                    state.player->SetMoveState(ATTACK);
                break;
            }
            break; // SDL_KEYDOWN
        }
    }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    // Disable other controls during attack or jump
    if (state.player->moveState == ATTACK || !(state.player->collidedBottom))
        return;

    if (keys[SDL_SCANCODE_LEFT])
    {
        if (state.player->direction != LEFT)
        {
            state.player->direction = LEFT;
        }
        if (state.player->moveState != RUN) state.player->SetMoveState(RUN);
    }
    else if (keys[SDL_SCANCODE_RIGHT])
    {
        if (state.player->direction != RIGHT)
        {
            state.player->direction = RIGHT;
        }
        if (state.player->moveState != RUN) state.player->SetMoveState(RUN);

    }
    else
    {
        if (state.player->moveState != IDLE)
            state.player->SetMoveState(IDLE);
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
        for (Entity *&entity_ptr : state.entities)
        {
            if (entity_ptr->alive)
                entity_ptr->Update(FIXED_TIMESTEP, state.entities, state.map);
        }
        deltaTime -= FIXED_TIMESTEP;
    }
    accumulator = deltaTime;

    // side scrolling
    viewMatrix = glm::mat4(1.0f);

    if (state.player->position.x > 5)
    {
        viewMatrix = glm::translate(viewMatrix,
                                    glm::vec3(-state.player->position.x, 3.75, 0));
    }
    else
    {
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 3.75, 0));
    }

    // End game when player dies
    if (!(state.player->alive))
    {
        state.gameMode = GAME_LOSE;
    }
}

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    program.SetViewMatrix(viewMatrix);
    for (Entity *&entity : state.entities)
    {
        if (entity->alive)
            entity->Render(&program);
    }
    state.map->Render(&program);

    if (state.gameMode == GAME_LOSE)
        Util::DrawText(&program, fontTextureID, "You Lose", 1.2f, 0.05f, glm::vec3(1.0f, -0.8f, 0.0f));

    else if (state.gameMode == GAME_WIN)
        Util::DrawText(&program, fontTextureID, "You Win", 1.2f, 0.05f, glm::vec3(1.0f, -0.8f, 0.0f));

    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown()
{
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    Initialize();

    while (gameIsRunning)
    {
        ProcessInput();
        if (state.gameMode == GAME_PLAY)
            Update();
        Render();
    }

    Shutdown();
    return 0;
}
