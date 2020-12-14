#include "Level1.h"
#include <random>
#define LEVEL1_WIDTH 22
#define LEVEL1_HEIGHT 16
#define SPRITESHEET_COLS 8
#define SPRITESHEET_ROWS 133
#define ENEMY_COUNT 45



unsigned int level1_data[] =
    {
        210, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 88, 89, 89, 89, 89, 89,
        208,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,104,105,105,105,105,176,
        208,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,880,881,882,883,176,
        208,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 56,  0,  0,  0,  0,888,889,890,891,176,
        208,  0,  0,  0,  0,  0,  40,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,896,897,898,899,176,
        208,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,176,
        208,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,152,153,154,176,
        208,  0,  0,  0,  8,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,160,161,162,176,
        208,  0,  0,  0, 24, 25,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,168,169,170,176,
        208,  0,  0,  0, 16, 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,176,
        208,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,176,
        208,  0,  0,  0,  0,  0,  0,  0,  0,  0, 64,  0,  0,  0,  0,  0,  0,  0,874,  0,  0,176,
        208,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,176,
        208,  0, 46, 47,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,176,
        208,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,176,
        218, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216, 216,176};

void Level1::Initialize()
{
    state.nextScene = -1;
    state.entities.clear();
    // Load Map
    GLuint mapTilesTextureID = Util::LoadTexture("assets/[Base]BaseChip_pipo.png"); // Map Spritesheet
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTilesTextureID, 1.0f, SPRITESHEET_COLS, SPRITESHEET_ROWS);

    // Load Sprites
    GLuint playerTextureID = Util::LoadTexture("assets/adventurer.png"); // 6x8 Tile Set
    state.player = createPlayer(playerTextureID, 1.0f, -1.0f);
    state.entities.push_back(state.player);

    std::default_random_engine generator;
    std::uniform_real_distribution < float> distribution(-1.0, 1.0);
    Entity* enemy;
    for (int i = 0; i < ENEMY_COUNT; i++) {
        float x_pos = distribution(generator);
        float y_pos = distribution(generator);
        enemy = createGoblin(10.0f + 1.0f * x_pos, -7.0f + 1.0f * y_pos, PATROLLER);
        enemy->move_dir.x = LEFT;
        enemy->move_dir.y = DOWN;

        enemy->SetMoveState(RUN);

        state.entities.push_back(enemy);
    }



    bgTextureID = Util::LoadTexture("assets/BG1.png"); // background
}
void Level1::Update(float deltaTime)
{
    Scene::Update(deltaTime);
    // if (state.aliveEnemyNum == 0) state.nextScene = 2;
}
