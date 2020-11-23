#include "Scene.h"

void Scene::Update(float deltaTime) {
    state.aliveEnemyNum = 0;
    for (Entity*& entity_ptr : state.entities)
    {
        if (entity_ptr->alive)
        {
            entity_ptr->Update(deltaTime, state.entities, state.map);
            if (entity_ptr->entityType == ENEMY && entity_ptr->alive)
                state.aliveEnemyNum += 1;
        }
    }
}

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
    entity->animIdleLeft = new std::vector<int>{ 0,1,2,3,4,5,4,3,2,1 };
    entity->animIdleRight = new std::vector<int>{ 6,7,8,9,10,11,10,9,8,7};
    entity->animMoveLeft = new std::vector<int>{ 24,25,26,27,28,29,30,31,32,33,34,35 };
    entity->animMoveRight = new std::vector<int>{ 36,37,38,39,40,41,42,43,44,45,46,47 };
    entity->animAttackLeft = new std::vector<int>{ 12,13,14,15,16,17 };
    entity->animAttackRight = new std::vector<int>{ 18,19,20,21,22,23};
    entity->animIndices = entity->animIdleLeft;
    entity->animIndex = 0;
    entity->animCols = 12;
    entity->animRows = 4;
    entity->acceleration.y = -9.81f;
    entity->position.x = pos_x;
    entity->position.y = pos_y;
    entity->direction = LEFT;
    entity->aiType = type;
    entity->aiState = PASSIVE;
    entity->SetMoveState(IDLE);
    return entity;
}

void Scene::renderBG(ShaderProgram* program) {
    float vertices[] = { -5.0f, -5.0f, 5.0f, -5.0f, 5.0f, 5.0f, -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, 5.0f };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    program->SetModelMatrix(modelMatrix);
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, bgTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Scene::Render(ShaderProgram* program)
{
    state.map->Render(program);
    for (Entity*& entity : state.entities)
    {
        if (entity->alive)
            entity->Render(program);
    }
}