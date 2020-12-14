#pragma once
#include "Entity.h"
#include <vector>
#include <random>


std::default_random_engine generator;
std::uniform_real_distribution <float> distribution(0.0, 1.0);

Entity::Entity(enum EntityType entityType, GLuint textureID) : entityType(entityType), textureID(textureID)
{
}

void Entity::CheckCollisionsY(Map *map)
{
    // Probes for tiles
    glm::vec3 top = glm::vec3(position.x, position.y + (height / 2), position.z);
    glm::vec3 top_left = glm::vec3(position.x - (width / 2), position.y + (height / 2), position.z);
    glm::vec3 top_right = glm::vec3(position.x + (width / 2), position.y + (height / 2), position.z);
    glm::vec3 bottom = glm::vec3(position.x, position.y - (height / 2), position.z);
    glm::vec3 bottom_left = glm::vec3(position.x - (width / 2), position.y - (height / 2), position.z);
    glm::vec3 bottom_right = glm::vec3(position.x + (width / 2), position.y - (height / 2), position.z);
    float penetration_x = 0;
    float penetration_y = 0;
    if (map->IsSolid(top, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
    }
    else if (map->IsSolid(top_left, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
    }
    else if (map->IsSolid(top_right, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
    }
    if (map->IsSolid(bottom, &penetration_x, &penetration_y) && velocity.y < 0)
    {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
    }
    else if (map->IsSolid(bottom_left, &penetration_x, &penetration_y) && velocity.y < 0)
    {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
    }
    else if (map->IsSolid(bottom_right, &penetration_x, &penetration_y) && velocity.y < 0)
    {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
    }

    if (map->IsSolid(bottom_left, &penetration_x, &penetration_y))
    {
        collidedLeftFallSensor = true;
    }
    if (map->IsSolid(bottom_right, &penetration_x, &penetration_y))
    {
        collidedRightFallSensor = true;
    }
}

void Entity::CheckCollisionsX(Map *map)
{
    // Probes for tiles
    glm::vec3 left = glm::vec3(position.x - (width / 2), position.y, position.z);
    glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);
    float penetration_x = 0;
    float penetration_y = 0;
    if (map->IsSolid(left, &penetration_x, &penetration_y) && velocity.x < 0)
    {
        position.x += penetration_x;
        velocity.x = 0;
        collidedLeft = true;
    }
    if (map->IsSolid(right, &penetration_x, &penetration_y) && velocity.x > 0)
    {
        position.x -= penetration_x;
        velocity.x = 0;
        collidedRight = true;
    }
}

bool Entity::CheckCollision(std::vector<Entity *> &entities)
{
    for (Entity *&entity_ptr : entities)
    {
        if (entity_ptr->entityType == ENEMY)
        {
            float dist = glm::distance(position, entity_ptr->position);
            // Kill enemy if within player's attack range during attack
            if (entityType == PLAYER && dist < attackRange && moveState == ATTACK && animIndex >= 2 && animIndex < 4)
            {
                entity_ptr->alive = false;
            }
        }
    }
    return true;
}

void Entity::Update(float deltaTime, std::vector<Entity *> &entities, Map *map)
{

    if (entityType == ENEMY)
        // pass pointer to player entity
        AI(entities[0]);

    if (animIndices != NULL)
    {

            // accumulate deltaTime
        animTime += deltaTime;

        // change animation frame if enough time accumulated
        if (animTime >= 0.15f)
        {
            animTime = 0.0f;
            animIndex++;
            if (animIndex >= animIndices->size())
            {
                animIndex = 0;
                if (moveState == ATTACK)
                    SetMoveState(IDLE);
            }
        }
        //else
        //{
        //    animIndex = 0;
        //}
    }



    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;
    collidedRightFallSensor = false;
    collidedLeftFallSensor = false;

    //

    velocity += acceleration * deltaTime;
    position.y += velocity.y * deltaTime; // Move on Y
    CheckCollisionsY(map);
    position.x += velocity.x * deltaTime; // Move on X
    CheckCollisionsX(map);


    if (entityType == PLAYER)
        CheckCollision(entities);



    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

void scaleCoords(float *floatArray, int array_size, float scale)
{
    for (int i = 0; i < array_size; i++)
    {
        floatArray[i] = floatArray[i] * scale;
    }
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram *program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;

    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;

    float texCoords[] = {u, v + height, u + width, v + height, u + width, v,
                         u, v + height, u + width, v, u, v};

    float vertices[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f};

    scaleCoords(vertices, 12, size);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram *program)
{
    program->SetModelMatrix(modelMatrix);

    if (animIndices != NULL)
    {
        DrawSpriteFromTextureAtlas(program, textureID, (*animIndices)[animIndex]);
        return;
    }
}
void Entity::SetXDirection(enum EntityDirection newXDir) {
    if (newXDir != move_dir.x) {
        animIndex = 0;
        move_dir.x = newXDir;
        if (newXDir != NONE) face_dir = newXDir;
    }
}
void Entity::SetYDirection(enum EntityDirection newYDir) {
    if (newYDir != move_dir.y) {
        move_dir.y = newYDir;
    }
}


void Entity::SetMoveState(enum EntityMoveState newMoveState)
{
    switch (newMoveState)
    {
    case IDLE:
        velocity.x = 0.0f;
        velocity.y = 0.0f;
        if (moveState != IDLE) animIndex = 0;

        if (face_dir == LEFT)
            animIndices = animIdleLeft;
        else if (face_dir == RIGHT)
            animIndices = animIdleRight;

        break;
    case RUN:
        velocity.x = 1.0f;
        velocity.y = 1.0f;

        if (face_dir == LEFT) animIndices = animMoveLeft;
        else if (face_dir == RIGHT) animIndices = animMoveRight;
        
        if (move_dir.x == LEFT) velocity.x *= -1.0f;
        else if (move_dir.x == NONE) velocity.x = 0.0f;
        
        if (move_dir.y == UP)
            velocity.y *= 1.0f;
        else if (move_dir.y == DOWN)
            velocity.y *= -1.0f;
        else if (move_dir.y == NONE)
            velocity.y = 0.0f;

        break;

    case ATTACK:
        velocity.x = 0.0f, velocity.y = 0.0f;
        if (face_dir == LEFT)
            animIndices = animAttackLeft;
        else if (face_dir == RIGHT)
            animIndices = animAttackRight;
        if (moveState != ATTACK) animIndex = 0;

    default:
        break;
    }

    if (glm::length(velocity) > 1.0f) {
        glm::normalize(velocity);
    }
    velocity *= moveSpeed;
    moveState = newMoveState;
}
float x_chance;
float y_chance;
void Entity::AI(Entity *player)
{
    float dist = glm::distance(position, player->position);
    bool directionChanged = false;

    // Get direction of player
    EntityDirection playerDirectionX = (player->position.x < position.x) ? LEFT : RIGHT;
    EntityDirection playerDirectionY = (player->position.y < position.y) ? DOWN : UP;
    switch (aiState)
    {
    case ALERTED:
        if (dist >= detectionRange)
        {
            aiState = PASSIVE;
            return;
        }

        if (aiType == PATROLLER)
        {
            // Mix_PlayChannel(-1, alert, 0);
            // move towards player
            if (move_dir.x != playerDirectionX) {
                face_dir = playerDirectionX;
                move_dir.x = playerDirectionX;
                directionChanged = true;
            }
            if (move_dir.y != playerDirectionY) {
                move_dir.y = playerDirectionY;
                directionChanged = true;
            }
            if (directionChanged)
                SetMoveState(RUN);
            if (dist <= attackRange)
                aiState = ATTACKING;
        }


        break;
    case ATTACKING:
        if (aiType == PATROLLER)
        {
            if (dist >= attackRange)
            {
                aiState = ALERTED;
                return;
            }
            if (moveState != ATTACK)
            SetMoveState(ATTACK);
            
            if (animIndex > 4) Mix_PlayChannel(-1, sword_swing2, 0);
            if (animIndex > 5)
                player->alive = false;
        }
        break;
    case PASSIVE:
        if (dist < detectionRange)
            aiState = ALERTED;
        if (aiType == PATROLLER)
        {
            bool direction_changed = false;
            
            // randomly change direction
            x_chance = distribution(generator);
            if (x_chance < 0.01f) {
                if (move_dir.x == LEFT) {
                    move_dir.x = RIGHT;
                    face_dir = RIGHT;
                    direction_changed = true;
                }
                else if (move_dir.x == RIGHT) {
                    move_dir.x = LEFT;
                    face_dir = LEFT;
                    direction_changed = true;
                }
            }
                y_chance = distribution(generator);
            if (y_chance < 0.01f) {
                if (move_dir.y == UP)
                    move_dir.y = DOWN;
                else if (move_dir.y == DOWN)
                    move_dir.y = UP;
            
            }
            if (direction_changed) SetMoveState(RUN);

        }
        break;
    default:
        break;
    }
}