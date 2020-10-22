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
        1, 1, 1, 1, 0, 0, 1, 1, 1, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5};

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
    fontTextureID = LoadTexture("assets/font1.png");

    // Load Map
    GLuint mapTilesTextureID = LoadTexture("assets/Tileset.png"); // 6x8 Tile Set
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTilesTextureID, 1.0f, 8,  6);

    // Load Sprites
    GLuint playerTextureID = LoadTexture("assets/adventurer-Sheet.png"); // 6x8 Tile Set
    state.player = new Entity(PLAYER, playerTextureID);
    state.player->animFrames = 1;
    state.player->animIndices = new int[1] {0};
    state.player->animIndex = 0;
    state.player->animCols = 7;
    state.player->animRows = 11;
    state.player->acceleration.y = -9.81f;


    state.entities.push_back(state.player);
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

    // state.player->acceleration.x = 0.0f;

    // if (keys[SDL_SCANCODE_LEFT])
    // {
    //     state.player->acceleration.x = -1.0f * PLAYER_SPEED;
    // }
    // else if (keys[SDL_SCANCODE_RIGHT])
    // {
    //     state.player->acceleration.x = 1.0f * PLAYER_SPEED;
    // }
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
        for (Entity* &entity_ptr : state.entities)
        {
            entity_ptr->Update(FIXED_TIMESTEP, state.entities, state.map);
        }
        deltaTime -= FIXED_TIMESTEP;
    }
    accumulator = deltaTime;
}

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    for (Entity* &entity : state.entities)
    {
        entity->Render(&program);
    }
    state.map->Render(&program);
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
