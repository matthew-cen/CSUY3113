#include "Entity.h"
#include <vector>

Entity::Entity(enum EntityType entityType) : entityType(entityType)
{
    position = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    modelMatrix = glm::mat4(1.0f);
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

void Entity::Update(float deltaTime, Entity *objects, int objectCount, Map *map)
{
    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;
    velocity += acceleration * deltaTime;
    position.y += velocity.y * deltaTime; // Move on Y
    CheckCollisionsY(map);
    CheckCollisionsY(objects, objectCount); // Fix if needed
    position.x += velocity.x * deltaTime;   // Move on X
    CheckCollisionsX(map);
    CheckCollisionsX(objects, objectCount); // Fix if needed
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
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }
    else
    {
        float vertices[] = {
            -0.5f * size * tile_repeat_cols,
            -0.5f * size * tile_repeat_rows,
            0.5f * size * tile_repeat_cols,
            -0.5f * size * tile_repeat_rows,
            0.5f * size * tile_repeat_cols,
            0.5f * size * tile_repeat_rows,
            -0.5f * size * tile_repeat_cols,
            -0.5f * size * tile_repeat_rows,
            0.5f * size * tile_repeat_cols,
            0.5f * size * tile_repeat_rows,
            -0.5f * size * tile_repeat_cols,
            0.5f * size * tile_repeat_rows};
        float texCoords[] = {
            0.0f * tile_repeat_cols,
            1.0f * tile_repeat_rows,
            1.0f * tile_repeat_cols,
            1.0f * tile_repeat_rows,
            1.0f * tile_repeat_cols,
            0.0f * tile_repeat_rows,
            0.0f * tile_repeat_cols,
            1.0f * tile_repeat_rows,
            1.0f * tile_repeat_cols,
            0.0f * tile_repeat_rows,
            0.0f * tile_repeat_cols,
            0.0f * tile_repeat_rows};

        glBindTexture(GL_TEXTURE_2D, textureID);

        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);

        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }
}
