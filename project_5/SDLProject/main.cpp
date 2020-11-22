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
#include "Scene.h"
#include "Level1.h"


#include <vector>

// Configuration
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.016666f

#define LEVEL1_WIDTH 22
#define LEVEL1_HEIGHT 8



Scene* currentScene;
Level1* level1;

void SwitchToScene(Scene* scene) {
    currentScene = scene;
    currentScene->Initialize();
}

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
    level1 = new Level1();
    SwitchToScene(level1);
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
                if (currentScene->state.player->collidedBottom && currentScene->state.player->moveState != ATTACK)
                    currentScene->state.player->SetMoveState(JUMP);
                break;
            case SDLK_SPACE:
                if (currentScene->state.player->collidedBottom && currentScene->state.player->moveState != ATTACK)
                    currentScene->state.player->SetMoveState(ATTACK);
                break;
            }
            break; // SDL_KEYDOWN
        }
    }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    // Disable other controls during attack or jump
    if (currentScene->state.player->moveState == ATTACK || !(currentScene->state.player->collidedBottom))
        return;

    if (keys[SDL_SCANCODE_LEFT])
    {
        if (currentScene->state.player->direction != LEFT)
        {
            currentScene->state.player->direction = LEFT;
        }
        if (currentScene->state.player->moveState != RUN)
            currentScene->state.player->SetMoveState(RUN);
    }
    else if (keys[SDL_SCANCODE_RIGHT])
    {
        if (currentScene->state.player->direction != RIGHT)
        {
            currentScene->state.player->direction = RIGHT;
        }
        if (currentScene->state.player->moveState != RUN)
            currentScene->state.player->SetMoveState(RUN);
    }
    else
    {
        if (currentScene->state.player->moveState != IDLE)
            currentScene->state.player->SetMoveState(IDLE);
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
        deltaTime -= FIXED_TIMESTEP;
    }
    accumulator = deltaTime;



    // side scrolling
    viewMatrix = glm::mat4(1.0f);

    if (currentScene->state.player->position.x > 5)
    {
        viewMatrix = glm::translate(viewMatrix,
                                    glm::vec3(-(currentScene->state.player->position.x), 3.75, 0));
    }
    else
    {
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 3.75, 0));
    }

    // End game when player dies
    if (!(currentScene->state.player->alive))
    {   
        if (--(currentScene->state.lives) == 0) currentScene->state.gameMode = GAME_LOSE;
    }
}

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    program.SetViewMatrix(viewMatrix);
    currentScene->Render(&program);
    if (currentScene->state.gameMode == GAME_LOSE)
        Util::DrawText(&program, fontTextureID, "You Lose", 1.0f, -0.5f, glm::vec3(currentScene->state.player->position.x, -0.8f, 0.0f));

    else if (currentScene->state.gameMode == GAME_WIN)
        Util::DrawText(&program, fontTextureID, "You Win", 1.0f, -0.5f, glm::vec3(currentScene->state.player->position.x, -0.8f, 0.0f));

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
        if (currentScene->state.gameMode == GAME_PLAY)
            Update();
        Render();
    }

    Shutdown();
    return 0;
}
