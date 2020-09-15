#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <cmath>


SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, starMatrix, planet1Matrix, planet2Matrix, cometMatrix, projectionMatrix;

float star_rotate = 0.0f;
float planet_scale = 0.3f;

float planet_1_azimuth = 0.0f;
float planet_1_rotate = 0.0f;
float planet_1_radius = 4.0f;

float planet_2_azimuth = 43.0f;
float planet_2_rotate = 0.0f;
float planet_2_radius = 7.0;

float comet_ellipse_a = 6.0f;
float comet_ellipse_b = 12.0f;
float comet_azimuth = 23.0f;
float comet_rotate = 0.0f;
float comet_scale = 0.4f;
float comet_radius = 9.0f;


GLuint starTextureID, planet1TextureID, planet2TextureID, cometTextureID;


GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL) {
        std::cout << "Unable to load image: " << filePath << std::endl;
        assert(false); // halts
    }

    GLuint textureID;
    glGenTextures(1, &textureID); // get new texture ID
    glBindTexture(GL_TEXTURE_2D, textureID); // bind id to global target
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);

    return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Star System", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    starTextureID = LoadTexture("sprites/star_red_giant01.png");
    planet1TextureID = LoadTexture("sprites/planet41.png");
    planet2TextureID = LoadTexture("sprites/planet38.png");
    cometTextureID = LoadTexture("sprites/comet.png");



    viewMatrix = glm::mat4(1.0f);
    starMatrix = glm::mat4(1.0f);
    planet1Matrix = glm::mat4(1.0f);

    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Blending (allow image transparency)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        }
    }
}

float lastTicks = 0.0f; // last time frame rendered

float EllipseRadiusFromAngle(float a, float b, float angle) {
    float radius = (a * b) / ((std::sqrtf(std::powf(a, 2.0f) * std::powf(std::sinf(angle), 2.0f)) + std::powf(b, 2.0f) * std::powf(std::cosf(angle), 2.0f))); // polar ellipse equation
    return radius;
}

void Update() { 
    float ticks = (float)SDL_GetTicks() / 1000.0f; // get ticks since epoch in seconds
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;


    star_rotate += 1.0f * deltaTime;

    starMatrix = glm::mat4(1.0f);
    starMatrix = glm::rotate(starMatrix, star_rotate, glm::vec3(0.0f, 0.0f, 1.0f));

    planet_1_azimuth += 0.6f * deltaTime;
    planet_1_rotate += 1.0f * deltaTime;

    planet1Matrix = glm::mat4(1.0f);
    planet1Matrix = glm::scale(planet1Matrix, glm::vec3(planet_scale, planet_scale, 1.0f));
    planet1Matrix = glm::rotate(planet1Matrix, planet_1_azimuth, glm::vec3(0.0f, 0.0f, 1.0f));
    planet1Matrix = glm::translate(planet1Matrix, glm::vec3(planet_1_radius, 0.0f, 1.0f));
    planet1Matrix = glm::rotate(planet1Matrix, planet_1_rotate, glm::vec3(0.0f, 0.0f, 1.0f));

    planet_2_azimuth += 0.4f * deltaTime;
    planet_2_rotate += 0.6f * deltaTime;

    planet2Matrix = glm::mat4(1.0f);
    planet2Matrix = glm::scale(planet2Matrix, glm::vec3(planet_scale, planet_scale, 1.0f));
    planet2Matrix = glm::rotate(planet2Matrix, planet_2_azimuth, glm::vec3(0.0f, 0.0f, 1.0f));
    planet2Matrix = glm::translate(planet2Matrix, glm::vec3(planet_2_radius, 0.0f, 1.0f));
    planet2Matrix = glm::rotate(planet2Matrix, planet_2_rotate, glm::vec3(0.0f, 0.0f, 1.0f));

    comet_azimuth += 0.4f * deltaTime;
    comet_rotate += 0.6f * deltaTime;
    comet_radius = EllipseRadiusFromAngle(comet_ellipse_a, comet_ellipse_b, comet_azimuth);

    cometMatrix = glm::mat4(1.0f);
    cometMatrix = glm::scale(cometMatrix, glm::vec3(comet_scale, comet_scale, 1.0f));
    cometMatrix = glm::rotate(cometMatrix, comet_azimuth, glm::vec3(0.0f, 0.0f, 1.0f));
    cometMatrix = glm::translate(cometMatrix, glm::vec3(comet_radius, 0.0f, 1.0f));
    cometMatrix = glm::rotate(cometMatrix, comet_rotate, glm::vec3(0.0f, 0.0f, 1.0f));

}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);


    float vertices[] = { -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    // Position Coordinates
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    // Texture Coordinates
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);

    program.SetModelMatrix(starMatrix);
    glBindTexture(GL_TEXTURE_2D, starTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    program.SetModelMatrix(planet1Matrix);
    glBindTexture(GL_TEXTURE_2D, planet1TextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    program.SetModelMatrix(planet2Matrix);
    glBindTexture(GL_TEXTURE_2D, planet2TextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    program.SetModelMatrix(cometMatrix);
    glBindTexture(GL_TEXTURE_2D, cometTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}
