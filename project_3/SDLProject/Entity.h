#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include <vector>

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

enum EntityType {PLAYER, PLATFORM, TARGET};

// Either "sprite" or "tile" type. Tile supports texture wrapping but cannot use texture atlas or animation and cannot move
class Entity {
public:
    EntityType entityType;
    bool active = true;
    bool needUpdate = true;
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    GLuint textureID;
    
    glm::mat4 modelMatrix;
    
    int *animRight = NULL;
    int *animLeft = NULL;
    int *animUp = NULL;
    int *animDown = NULL;

    int *animIndices = NULL;
    int animFrames = 0;
    int animIndex = 0;
    float animTime = 0;
    int animCols = 0;
    int animRows = 0;

    int tile_repeat_rows = NULL;
    int tile_repeat_cols = NULL;
    float size = 1.0f;
    float height = 1.0f;
    float width = 1.0f;

    EntityType lastCollision;
    
    Entity(enum EntityType entityType);
    
    void Update(float deltaTime, std::vector<Entity*> &entities);
    void Render(ShaderProgram *program);
    bool CheckCollision(Entity *object);

    void CheckCollisionsX(std::vector<Entity*> &objects);
    void CheckCollisionsY(std::vector<Entity*> &objects);

    void DrawSpriteFromTextureAtlas(ShaderProgram *program, GLuint textureID, int index);
};
