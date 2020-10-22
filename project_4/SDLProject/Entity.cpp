#include "Entity.h"
#include <vector>

Entity::Entity(enum EntityType entityType): entityType(entityType)
{
    position = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    modelMatrix = glm::mat4(1.0f);
}

bool Entity::CheckCollision(Entity *other)
{
    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);
    if (xdist < 0 && ydist < 0)
    {
        lastCollision = other->entityType;
        return true;
    }
    return false;
}

void Entity::CheckCollisionsY(std::vector<Entity*>& objects)
{
    for (int i = 0; i < objects.size(); i++)
    {
        Entity *object = objects[i];
        if (object->entityType == PLATFORM || object->entityType == ENEMY) {

            if (CheckCollision(object))
            {
                float ydist = fabs(position.y - object->position.y);
                float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
                if (velocity.y > 0) {
                    position.y -= penetrationY;
                    velocity.y = 0;
                }
                else if (velocity.y < 0) {
                    position.y += penetrationY;
                    velocity.y = 0;
                }

            }
        }
    }
}

void Entity::CheckCollisionsX(std::vector<Entity*>&  objects)
{
    for (int i = 0; i < objects.size(); i++)
    {
        Entity *object = objects[i];
        if (object->entityType == PLATFORM) {

            if (CheckCollision(object))
            {
                float xdist = fabs(position.x - object->position.x);
                float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
                if (velocity.x > 0) {
                    position.x -= penetrationX;
                    velocity.x = 0;
                }
                else if (velocity.x < 0) {
                    position.x += penetrationX;
                    velocity.x = 0;
                }
            }
        }
    }
}



void Entity::Update(float deltaTime, std::vector<Entity*> & entities)
{
    if (animIndices != NULL) {
        if (glm::length(acceleration) != 0) {
            // accumulate deltaTime
            animTime += deltaTime;

            // change animation frame if enough time accumulated
            if (animTime >= 0.25f)
            {
                animTime = 0.0f;
                animIndex++;
                if (animIndex >= animFrames)
                {
                    animIndex = 0;
                }
            }
        } else {
            animIndex = 0;
        }
    }
    if (entityType == PLAYER) {
        velocity += acceleration * deltaTime;

        position.y +=  velocity.y * deltaTime;
        CheckCollisionsY(entities);

        position.x +=  velocity.x * deltaTime;
        CheckCollisionsX(entities);
    }
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

void scaleCoords(float* floatArray, int array_size, float scale) {
    for (int i = 0; i < array_size; i++) {
        floatArray[i] = floatArray[i] * scale;
    }
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram *program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;
    
    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;
    
    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v};

    
    float vertices[]  = { -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f , 0.5f};
    
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


void Entity::Render(ShaderProgram *program) {
    program->SetModelMatrix(modelMatrix);
    
    if (animIndices != NULL) {
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }
    else {
        float vertices[]  = { 
            -0.5f * size * tile_repeat_cols, 
            -0.5f * size * tile_repeat_rows,
            0.5f  * size * tile_repeat_cols, 
            -0.5f * size * tile_repeat_rows, 
            0.5f * size * tile_repeat_cols, 
            0.5f * size * tile_repeat_rows, 
            -0.5f * size * tile_repeat_cols, 
            -0.5f * size * tile_repeat_rows, 
            0.5f * size * tile_repeat_cols, 
            0.5f * size * tile_repeat_rows, 
            -0.5f * size * tile_repeat_cols, 
            0.5f * size * tile_repeat_rows };
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
            0.0f * tile_repeat_rows };
    
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
