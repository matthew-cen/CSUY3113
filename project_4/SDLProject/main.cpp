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

#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Configuration
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.016666f
#define GRAVITY_ACCEL -0.5f
#define PLAYER_SPEED 1.0f

#define PLAYER_SIZE 0.75f
#define TILE_SIZE 0.75f

#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5

enum GameMode
{
    GAME_PLAY,
    GAME_FAIL,
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
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2};

SDL_Window *displayWindow;
bool gameIsRunning = true;
GLuint fontTextureID;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint LoadTexture(const char *filePath)
{
    int w, h, n;
    unsigned char *image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL)
    {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}

void Initialize()
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Lunar Lander", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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
    fontTextureID = LoadTexture("assets/font1.png");

    Entity *entity;

    // Initialize Player

    entity = new Entity(PLAYER);
    state.player = entity;

    state.entities.push_back(entity);
    entity->position = glm::vec3(0.0f, 3.75f, 0.0f);
    entity->acceleration = glm::vec3(0.0f, GRAVITY_ACCEL, 0.0f);
    entity->textureID = LoadTexture("assets/SF01.png");
    entity->animIndices = new int[1]{0};
    entity->animFrames = 1;
    entity->animIndex = 0;
    entity->animTime = 0;
    entity->animCols = 1;
    entity->animRows = 1;
    entity->size = PLAYER_SIZE;
    entity->width = PLAYER_SIZE;
    entity->height = PLAYER_SIZE;

    GLuint moonTextureID = LoadTexture("assets/moon_tile.png");

    // Initialize Map (Static Map Border Tiles)
    entity = new Entity(PLATFORM);
    state.entities.push_back(entity); // Left Border
    entity->textureID = moonTextureID;
    entity->tile_repeat_cols = 1;
    entity->tile_repeat_rows = 10;
    entity->size = TILE_SIZE;
    entity->width = TILE_SIZE;
    entity->height = TILE_SIZE * 10;
    entity->position.x = -5.0f + TILE_SIZE / 2.0f;

    entity = new Entity(PLATFORM);
    state.entities.push_back(entity); // Right Border
    entity->textureID = moonTextureID;
    entity->tile_repeat_cols = 1;
    entity->tile_repeat_rows = 10;
    entity->size = TILE_SIZE;
    entity->width = TILE_SIZE;
    entity->height = TILE_SIZE * 10;
    entity->position.x = 5.0f - TILE_SIZE / 2.0f;

    entity = new Entity(PLATFORM);
    state.entities.push_back(entity); // Bottom Border Left
    entity->textureID = moonTextureID;
    entity->tile_repeat_cols = 5;
    entity->tile_repeat_rows = 1;
    entity->size = TILE_SIZE;
    entity->width = TILE_SIZE * 5;
    entity->height = TILE_SIZE;
    entity->position.x = -2.5f;
    entity->position.y = -3.75f + TILE_SIZE / 2.0f;

    entity = new Entity(PLATFORM);
    state.entities.push_back(entity); // Bottom Border Right
    entity->textureID = moonTextureID;
    entity->tile_repeat_cols = 5;
    entity->tile_repeat_rows = 1;
    entity->size = TILE_SIZE;
    entity->width = TILE_SIZE * 5;
    entity->height = TILE_SIZE;
    entity->position.x = 2.5f;
    entity->position.y = -3.75f + TILE_SIZE / 2.0f;

    entity = new Entity(TARGET);
    state.entities.push_back(entity); // Landing Pad
    entity->textureID = LoadTexture("assets/target_tile.png");
    entity->tile_repeat_cols = 2;
    entity->tile_repeat_rows = 1;
    entity->size = TILE_SIZE;
    entity->height = TILE_SIZE;
    entity->width = TILE_SIZE * 2;

    entity->position.y = -3.75f + TILE_SIZE / 2.0f;

    entity = new Entity(PLATFORM);
    state.entities.push_back(entity); // Obstacle 1
    entity->textureID = moonTextureID;
    entity->tile_repeat_cols = 4;
    entity->tile_repeat_rows = 1;
    entity->size = TILE_SIZE;
    entity->height = TILE_SIZE;
    entity->width = TILE_SIZE * 4;
    entity->position.y = 1.5f;
    entity->position.x = -1.75f;

    entity = new Entity(PLATFORM);
    state.entities.push_back(entity); // Obstacle 2
    entity->textureID = moonTextureID;
    entity->tile_repeat_cols = 3;
    entity->tile_repeat_rows = 1;
    entity->size = TILE_SIZE;
    entity->height = TILE_SIZE;
    entity->width = TILE_SIZE * 3;
    entity->position.y = -1.5f;
    entity->position.x = 2.0f;
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

            case SDLK_SPACE:
                // Some sort of action
                break;
            }
            break; // SDL_KEYDOWN
        }
    }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    state.player->acceleration.x = 0.0f;

    if (keys[SDL_SCANCODE_LEFT])
    {
        state.player->acceleration.x = -1.0f * PLAYER_SPEED;
    }
    else if (keys[SDL_SCANCODE_RIGHT])
    {
        state.player->acceleration.x = 1.0f * PLAYER_SPEED;
    }
}

void DrawText(ShaderProgram *program, GLuint fontTextureID, std::string text,
              float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;
    std::vector<float> vertices;
    std::vector<float> texCoords;
    for (int i = 0; i < text.size(); i++)
    {
        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;

        vertices.insert(vertices.end(), {
                                            offset + (-0.5f * size),
                                            0.5f * size,
                                            offset + (-0.5f * size),
                                            -0.5f * size,
                                            offset + (0.5f * size),
                                            0.5f * size,
                                            offset + (0.5f * size),
                                            -0.5f * size,
                                            offset + (0.5f * size),
                                            0.5f * size,
                                            offset + (-0.5f * size),
                                            -0.5f * size,
                                        });
        texCoords.insert(texCoords.end(), {
                                              u,
                                              v,
                                              u,
                                              v + height,
                                              u + width,
                                              v,
                                              u + width,
                                              v + height,
                                              u + width,
                                              v,
                                              u,
                                              v + height,
                                          });
    } // end of for loop
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
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
        for (auto &entity : state.entities)
        {
            if (entity->needUpdate)
            {
                entity->Update(FIXED_TIMESTEP, state.entities);
                if (entity->entityType == PLATFORM || entity->entityType == TARGET)
                    entity->needUpdate = false;
                else if (entity->entityType == PLAYER)
                {
                    switch (entity->lastCollision)
                    {
                    case PLATFORM:
                        state.gameMode = GAME_FAIL;
                        break;
                    case TARGET:
                        state.gameMode = GAME_WIN;
                        break;
                    }
                }
            }
        }
        deltaTime -= FIXED_TIMESTEP;
    }
    accumulator = deltaTime;
}

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto &entity : state.entities)
    {
        if (entity->active)
        {
            entity->Render(&program);
        }
    }
    if (state.gameMode == GAME_FAIL)
    {
        DrawText(&program, fontTextureID, "Mission Failed", 1, -0.5f, glm::vec3(-4.25f, 3, 0));
    }
    else if (state.gameMode == GAME_WIN)
    {
        DrawText(&program, fontTextureID, "Mission Success", 1, -0.5f, glm::vec3(-4.25f, 3, 0));
    }

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
