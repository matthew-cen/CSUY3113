#include "Scene.h"

Entity* Scene::createPlayer(const GLuint const textureID, float const pos_x, float const pos_y) {
    Entity* entity = new Entity(PLAYER, textureID);
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
Entity* Scene::createHound(const GLuint const textureID, float const pos_x, float const pos_y, enum AIType type) {
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