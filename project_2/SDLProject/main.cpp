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
#include <cmath>
#include <random>


SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, projectionMatrix, player1Matrix, player2Matrix, ballMatrix;


// CONFIGURATION
float half_ball_size = 0.1f;
float half_paddle_width = 0.1f;
float half_paddle_height = 0.7f;


void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex.glsl", "shaders/fragment.glsl");

    viewMatrix = glm::mat4(1.0f);

    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

glm::vec3 player1_movement;
glm::vec3 player2_movement;


void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        }
    }
    // Reset player movement direction since last loop
    player1_movement = glm::vec3(0, 0, 0);
    player2_movement = glm::vec3(0, 0, 0);

    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    // if else-if used to prevent up and down from being pressed at the same time
    if (keys[SDL_SCANCODE_W]) {
        player1_movement.y = 1.0f;
    }
    else if (keys[SDL_SCANCODE_S]) {
        player1_movement.y = -1.0f;
    }
    if (keys[SDL_SCANCODE_UP]) {
        player2_movement.y = 1.0f;
    }
    else if (keys[SDL_SCANCODE_DOWN]) {
        player2_movement.y = -1.0f;
    }

    // normalize vector (convert to unit vector)
    // currently unnecessary as only up/down movement
    if (glm::length(player1_movement) > 1.0f) {
        glm::normalize(player1_movement);
    }

    if (glm::length(player2_movement) > 1.0f) {
        glm::normalize(player2_movement);
    }
}



float lastTicks = 0.0f; // last time frame rendered

glm::vec3 player1_position = glm::vec3(-5.0f + half_paddle_width, 0.0f, 1.0f);
glm::vec3 player2_position = glm::vec3(5.0f - half_paddle_width, 0.0f, 1.0f);
glm::vec3 ball_position = glm::vec3(0.0f, 0.0f, 1.0f);

float player1_speed = 3.0f;
float player2_speed = 3.0f;
float ball_speed = 3.0f;
glm::vec3 ball_movement = glm::vec3(0, 0, 0);

bool game_started = false; // set on first run for initialization
bool game_ended = false;

// Randomness
std::random_device rd; 
std::mt19937 generator(rd()); // use OS/HW random source for seed
std::uniform_real_distribution<float> rand_angle(0.1f, 0.53f);
std::uniform_int_distribution<int> rand_direction(0, 1);


bool paddleBallCollided(glm::vec3& player_pos) {
    float p_ball_center_delta_x = fabs(ball_position.x - player_pos.x);
    float p_ball_center_delta_y = fabs(ball_position.y - player_pos.y);
    float p_ball_width_sum = half_paddle_width + half_ball_size;
    float p_ball_height_sum = half_paddle_height + half_ball_size;

    float p_ball_delta_x = p_ball_center_delta_x - p_ball_width_sum;
    float p_ball_delta_y = p_ball_center_delta_y - p_ball_height_sum;

    return ((p_ball_delta_x < 0.0f) && (p_ball_delta_y < 0.0f));
}

void paddleBorderCollisionHandler(glm::vec3& player_pos, glm::vec3& player_movement) {
    if ((player_pos.y + half_paddle_height) > 3.75f) {
        if (player_movement.y > 0.0f) {
            player_movement.y = 0.0f;
        }
    }
    else if ((player_pos.y - half_paddle_height) < -3.75f) {
        if (player_movement.y < 0.0f) {
            player_movement.y = 0.0f;
        }
    }
}

// Additional flags as high framerate causes issues with collision bounding checks
bool paddle_ball_collided_last_frame = false;
bool paddle_top_bottom_collided_last_frame = false;


void Update() {
    // COLLISION DETECTION 


    // Paddles Ball Collsion Check
    if (paddleBallCollided(player1_position) || paddleBallCollided(player2_position)) {
        if (!paddle_ball_collided_last_frame) {
            ball_movement.x *= -1.0f;
            paddle_ball_collided_last_frame = true;
        }
    }
    else {
        paddle_ball_collided_last_frame = false;
    }

    // Top/Bottom Border Collsion Check
    if (ball_position.y < (-3.75f + half_ball_size) || ball_position.y >(3.75f - half_ball_size)) {
        if (!paddle_top_bottom_collided_last_frame) {
           ball_movement.y *= -1.0f;
            paddle_top_bottom_collided_last_frame = true;
        }
    }
    else {
        paddle_top_bottom_collided_last_frame = false;
    }
    // Left/Right Border Collsion Check
    if (ball_position.x < (-5.0f + half_ball_size) || ball_position.x >(5.0f - half_ball_size)) {
        // gameIsRunning = false;
        game_ended = true;
    }

    // Paddles Border Collision Check
    paddleBorderCollisionHandler(player1_position, player1_movement);
    paddleBorderCollisionHandler(player2_position, player2_movement);

    float ticks = (float)SDL_GetTicks() / 1000.0f; // get ticks since epoch in seconds
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    player1_position += player1_movement * player1_speed * deltaTime;

    player1Matrix = glm::mat4(1.0f);
    player1Matrix = glm::translate(player1Matrix, player1_position);

    player2_position += player2_movement * player2_speed * deltaTime;


    player2Matrix = glm::mat4(1.0f);
    player2Matrix = glm::translate(player2Matrix, player2_position);

    // initialize random ball direction on start
    if (!game_started) {
        // Angle method chosen over random x,y + normalization for uniform distribution
        // see: https://towardsdatascience.com/the-best-way-to-pick-a-unit-vector-7bd0cc54f9b


        // choose random x direction of ball (left/right)
        int ball_x_dir = rand_direction(generator);

        // choose random y direction of ball (up/down)
        int ball_y_dir = rand_direction(generator);

        // choose random angle within a cone facing chosen direction
        // cone = 30 degrees (pi/6 radians)
        float angle = rand_angle(generator);

        // calculate unit vector for angle
        float ball_mov_x = cos(angle);
        float ball_mov_y = sin(angle);

        // Set randomly chosen directions
        ball_movement.x = ball_x_dir ? ball_mov_x : -ball_mov_x;
        ball_movement.y = ball_y_dir ? ball_mov_y : -ball_mov_y;
        game_started = true;
    }

    ball_position += ball_movement * ball_speed * deltaTime;

    ballMatrix = glm::mat4(1.0f);
    ballMatrix = glm::translate(ballMatrix, ball_position);



}



void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    float paddleVertices[] = { -half_paddle_width, -half_paddle_height, half_paddle_width, -half_paddle_height, half_paddle_width, half_paddle_height, -half_paddle_width, -half_paddle_height, half_paddle_width, half_paddle_height, -half_paddle_width, half_paddle_height };
    float ballVertices[] = { -half_ball_size, -half_ball_size, half_ball_size, -half_ball_size, half_ball_size, half_ball_size, -half_ball_size, -half_ball_size, half_ball_size, half_ball_size, -half_ball_size, half_ball_size };

    // Position Coordinates
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, paddleVertices);

    glEnableVertexAttribArray(program.positionAttribute);


    program.SetModelMatrix(player1Matrix);
    program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    program.SetModelMatrix(player2Matrix);
    program.SetColor(0.0f, 0.0f, 1.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);

    // TODO: maybe have ball change colors over time?
    program.SetModelMatrix(ballMatrix);
    program.SetColor(0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    glDisableVertexAttribArray(program.positionAttribute);



    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        if (!game_ended) Update();
        Render();
    }

    Shutdown();
    return 0;
}
