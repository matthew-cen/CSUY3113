#include "Level1.h"
#define LEVEL1_WIDTH 22
#define LEVEL1_HEIGHT 8

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

void Level1::Initialize()
{
    state.nextScene = -1;
    state.entities.clear();
    // Load Map
    GLuint mapTilesTextureID = Util::LoadTexture("assets/Tileset.png"); // 6x8 Tile Set
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTilesTextureID, 1.0f, 8, 6);

    // Load Sprites
    GLuint playerTextureID = Util::LoadTexture("assets/adventurer.png"); // 6x8 Tile Set
    state.player = createPlayer(playerTextureID, 1.0f, -1.0f);
    state.entities.push_back(state.player);

    const GLuint houndTextureID = Util::LoadTexture("assets/enemies/hell-hound.png"); // 6x8 Tile Set
    Entity *enemy = createHound(houndTextureID, 6.6f, -0.5f, JUMPER);
    state.entities.push_back(enemy);
    enemy = createHound(houndTextureID, 9.7f, 0.5f, PATROLLER);
    state.entities.push_back(enemy);
    enemy = createHound(houndTextureID, 15.2f, 2.5f, COWARD);
    state.entities.push_back(enemy);

    bgTextureID = Util::LoadTexture("assets/BG1.png"); // background
}
void Level1::Update(float deltaTime)
{
    Scene::Update(deltaTime);
    if (state.aliveEnemyNum == 0) state.nextScene = 2;
}
