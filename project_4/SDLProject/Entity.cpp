#include "Entity.h"
#include <vector>

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

bool Entity::CheckCollision(std::vector<Entity*> &entities)
{
    for (Entity *&entity_ptr : entities)
    {
        if (entity_ptr->entityType == ENEMY) {
            float dist = glm::distance(position, entity_ptr->position);
            // Enemy attack range shorter than player's
            if (dist < 0.9f && moveState == ATTACK && animIndex >= 2 && animIndex < 4) {
                entity_ptr->alive = false;
            }  

            if (dist < 0.6f && entity_ptr->moveState == ATTACK) {
                alive = false;
            }  
            // Player Attacks
            // if (state == ATTACK) {}

            // Enemy Attacks player
            // if (entity_ptr->state == ATTACK) {

            // }
        }
    }
    return true;
}

void Entity::Update(float deltaTime, std::vector<Entity *> &entities, Map *map)
{

    if (entityType == ENEMY)
        AI();

    if (animIndices != NULL)
    {
        if (glm::length(acceleration) != 0)
        {
            // accumulate deltaTime
            animTime += deltaTime;

            // change animation frame if enough time accumulated
            if (animTime >= 0.25f)
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
        }
        else
        {
            animIndex = 0;
        }
    }

    if (jump)
    {
        jump = false;
        // SetState(RUN);
        velocity.y += jumpPower;
    }

    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;
    velocity += acceleration * deltaTime;
    position.y += velocity.y * deltaTime; // Move on Y
    CheckCollisionsY(map);
    // CheckCollisionsY(entities); // Fix if needed
    position.x += velocity.x * deltaTime; // Move on X
    CheckCollisionsX(map);
    // CheckCollisionsX(entities); // Fix if needed

    if (collidedBottom && moveState == JUMPING)
        SetMoveState(IDLE);

    if (entityType == PLAYER) CheckCollision(entities);

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

void Entity::SetMoveState(enum EntityMoveState newMoveState)
{
    switch (newMoveState)
    {
    case IDLE:
        velocity.x = 0.0f;
        animIndex = 0;
        if (direction == LEFT)
            animIndices = animIdleLeft;
        else if (direction == RIGHT)
            animIndices = animIdleRight;
        break;
    case RUN:
        velocity.x = 1.0f * moveSpeed;
        if (direction == LEFT)
        {
            velocity.x *= -1.0f;
            animIndices = animMoveLeft;
        }
        else if (direction == RIGHT)
            animIndices = animMoveRight;

        animIndex = 0;

        break;
    case JUMP:
        jump = true;
        break;
    case ATTACK:
        velocity.x = 0.0f;
        if (direction == LEFT)
            animIndices = animAttackLeft;
        else if (direction == RIGHT)
            animIndices = animAttackRight;
        animIndex = 0;
    default:
        break;
    }
    moveState = newMoveState;
}

void Entity::AI()
{
    switch (aiState)
    {
    case PATROLLING:
        /* code */
        break;
    case ATTACKING:
        break;
    case JUMPING:
        if (collidedBottom)
            SetMoveState(JUMP);
        break;
    default:
        break;
    }
}