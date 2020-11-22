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

#define LEVEL1_WIDTH 22
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
    unsigned int lives = 3;
};

GameState state;

unsigned int level1_data[] =
    {
        05, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 05,
        05, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 05,
        05, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 05,
        05, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 05,
        05, 15, 15, 15, 15, 15, 15, 15, 00, 01, 01, 01, 01, 15, 15, 15, 15, 15, 15, 15, 15, 05,
        05, 15, 15, 15, 15, 00, 01, 01, 05, 05, 05, 05, 05, 15, 01, 01, 01, 01, 01, 15, 15, 05,
        05, 01, 01, 01, 01, 05, 05, 05, 05, 05, 05, 05, 05, 15, 15, 15, 15, 15, 15, 15, 15, 05,
        05, 05, 05, 05, 05, 05, 05, 05, 05, 05, 05, 05, 05, 15, 15, 15, 15, 15, 15, 15, 15, 05};

SDL_Window *displayWindow;
bool gameIsRunning = true;
GLuint fontTextureID;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

Entity * createPlayer(const GLuint const textureID, float const pos_x, float const pos_y) {
    Entity * entity = new Entity(PLAYER, textureID);
    entity->animIdleLeft = new std::vector<int>{ 18, 19, 20 };
    entity->animIdleRight = new std::vector<int>{ 0, 1, 2 };
    entity->animMoveLeft = new std::vector<int>{ 25, 26, 27, 28, 29, 30 };
    entity->animMoveRight = new std::vector<int>{ 7, 8, 9, 10, 11, 12 };
    entity->animAttackLeft = new std::vector<int>{ 31, 32, 33, 34, 35 };
    entity->animAttackRight = new std::vector<int>{ 13, 14, 15, 16, 17 };
    entity->animIndex = 0;
    entity->animCols = 18;
    entity->animRows = 2;
    entity->acceleration.y = -9.81f;
    entity->position.x = pos_x;
    entity->position.y = pos_y;
    entity->direction = RIGHT;
    entity->attackRange = 0.9f;
    entity->moveSpeed = 2.0f;
    entity->SetMoveState(IDLE);
    entity->width = 0.5f; // reduce collision box size
    return entity;
}
Entity* createHound(const GLuint const textureID, float const pos_x, float const pos_y, enum AIType type) {
    Entity* entity = new Entity(ENEMY, textureID);
    entity->animIdleLeft = new std::vector<int>{ 0,1,2,3,4,5 };
    entity->animIdleRight = new std::vector<int>{ 0,1,2,3,4,5 };
    entity->animMoveLeft = new std::vector<int>{ 0,1,2,3,4,5 };
    entity->animMoveRight = new std::vector<int>{ 0,1,2,3,4,5 };
    entity->animAttackLeft = new std::vector<int>{ 0,1,2,3,4,5 };
    entity->animAttackRight = new std::vector<int>{ 0,1,2,3,4,5 };
    entity->animIndices = entity->animIdleRight;
    entity->animIndex = 0;
    entity->animCols = 6;
    entity->animRows = 1;
    entity->acceleration.y = -9.81f;
    entity->position.x = pos_x;
    entity->position.y = pos_y;
    entity->direction = LEFT;
    entity->aiType = type;
    entity->aiState = PASSIVE;
    entity->SetMoveState(IDLE);
    return entity;
}

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
    state.player = createPlayer(playerTextureID, 1.0f, -1.0f);
    state.entities.push_back(state.player);

    const GLuint houndTextureID = Util::LoadTexture("assets/enemies/hell-hound-idle.png"); // 6x8 Tile Set
    Entity *enemy = createHound(houndTextureID, 6.6f, -0.5f, JUMPER);
    state.entities.push_back(enemy);
    enemy = createHound(houndTextureID, 9.7f, 0.5f, PATROLLER);
    state.entities.push_back(enemy);
    enemy = createHound(houndTextureID, 15.2f, 2.5f, COWARD);
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
        if (state.player->moveState != RUN)
            state.player->SetMoveState(RUN);
    }
    else if (keys[SDL_SCANCODE_RIGHT])
    {
        if (state.player->direction != RIGHT)
        {
            state.player->direction = RIGHT;
        }
        if (state.player->moveState != RUN)
            state.player->SetMoveState(RUN);
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
    unsigned int aliveEnemyCount;
    while (deltaTime >= FIXED_TIMESTEP)
    {
        aliveEnemyCount = 0;

        for (Entity *&entity_ptr : state.entities)
        {
            if (entity_ptr->alive)
            {
                if (entity_ptr->entityType == ENEMY)
                    aliveEnemyCount += 1;
                entity_ptr->Update(FIXED_TIMESTEP, state.entities, state.map);
            }
        }
        deltaTime -= FIXED_TIMESTEP;
    }
    accumulator = deltaTime;

    if (aliveEnemyCount == 0)
    {
        state.gameMode = GAME_WIN;
    }

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
        if (--state.lives == 0) state.gameMode = GAME_LOSE;
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
        Util::DrawText(&program, fontTextureID, "You Lose", 1.0f, -0.5f, glm::vec3(state.player->position.x, -0.8f, 0.0f));

    else if (state.gameMode == GAME_WIN)
        Util::DrawText(&program, fontTextureID, "You Win", 1.0f, -0.5f, glm::vec3(state.player->position.x, -0.8f, 0.0f));

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
