#include "Map.h"
#define NONSOLID_TILE 0

Map::Map(int width, int height, unsigned int *levelData, GLuint textureID, float tile_size, int texMapColNum, int texMapRowNum)
{
    this->width = width;
    this->height = height;
    this->levelData = levelData;
    this->textureID = textureID;
    this->tile_size = tile_size;
    this->texMapColNum = texMapColNum;
    this->texMapRowNum = texMapRowNum;
    this->Build();
}

void Map::Build()
{
    for (int y = 0; y < this->height; y++)
    {
        for (int x = 0; x < this->width; x++)
        {
            int tile = levelData[y * width + x];
            if (tile == 48)
                continue;
            float u = (float)(tile % texMapColNum) / (float)texMapColNum;
            float v = (float)(tile / texMapColNum) / (float)texMapRowNum;
            float tileWidth = 1.0f / (float)texMapColNum;
            float tileHeight = 1.0f / (float)texMapRowNum;
            float xoffset = -(tile_size / 2); // From center of tile
            float yoffset = (tile_size / 2);  // From center of tile
            vertices.insert(vertices.end(), {xoffset + (tile_size * x), yoffset + -tile_size * y,
                                             xoffset + (tile_size * x), yoffset + (-tile_size * y) - tile_size,
                                             xoffset + (tile_size * x) + tile_size, yoffset + (-tile_size * y) - tile_size,
                                             xoffset + (tile_size * x), yoffset + -tile_size * y,
                                             xoffset + (tile_size * x) + tile_size, yoffset + (-tile_size * y) - tile_size,
                                             xoffset + (tile_size * x) + tile_size, yoffset + -tile_size * y});

            texCoords.insert(texCoords.end(), {u, v,
                                               u, v + (tileHeight),
                                               u + tileWidth, v + (tileHeight),
                                               u, v,
                                               u + tileWidth, v + (tileHeight),
                                               u + tileWidth, v});
        }
    }
    left_bound = 0 - (tile_size / 2);
    right_bound = (tile_size * width) - (tile_size / 2);
    top_bound = 0 + (tile_size / 2);
    bottom_bound = -(tile_size * height) + (tile_size / 2);
}

void Map::Render(ShaderProgram *program)
{
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    program->SetModelMatrix(modelMatrix);
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)vertices.size() / 2);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

bool Map::IsSolid(glm::vec3 position, float *penetration_x, float *penetration_y)
{
    *penetration_x = 0;
    *penetration_y = 0;
    if (position.x < left_bound || position.x > right_bound)
        return false;
    if (position.y > top_bound || position.y < bottom_bound)
        return false;
    int tile_x = floor((position.x + (tile_size / 2)) / tile_size);
    int tile_y = -(ceil(position.y - (tile_size / 2))) / tile_size; // Our array counts up as Y goes down.
    if (tile_x < 0 || tile_x >= width)
        return false;
    if (tile_y < 0 || tile_y >= height)
        return false;
    int tile = levelData[tile_y * width + tile_x];
    if (tile == NONSOLID_TILE)
        return false;
    float tile_center_x = (tile_x * tile_size);
    float tile_center_y = -(tile_y * tile_size);
    *penetration_x = (tile_size / 2) - fabs(position.x - tile_center_x);
    *penetration_y = (tile_size / 2) - fabs(position.y - tile_center_y);
    return true;
}