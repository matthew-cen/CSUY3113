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
#include "Map.h"

enum EntityType {PLAYER, PLATFORM, ENEMY};

class Entity {
public:
    EntityType entityType;
    bool active = true;
    bool needUpdate = true;
    bool jump = false;
    float jumpPower = 5.0f;
    glm::vec3 position = glm::vec3(0);
    glm::vec3 velocity = glm::vec3(0);
    glm::vec3 acceleration = glm::vec3(0);
    GLuint textureID;
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    
    std::vector<int> *animRight = NULL;
    std::vector<int> *animLeft = NULL;
    std::vector<int> *animUp = NULL;
    std::vector<int> *animDown = NULL;
    std::vector<int> *animStationary = NULL;

    std::vector<int> *animIndices = NULL;
    int animIndex = 0;
    float animTime = 0;
    int animCols = 0;
    int animRows = 0;

    float size = 1.0f;
    float height = 1.0f;
    float width = 1.0f;

    // Collision Markers
    bool collidedTop;
    bool collidedBottom;
    bool collidedLeft;
    bool collidedRight;


    EntityType lastCollision;
    
    Entity(enum EntityType entityType, GLuint textureID);
    
    void Update(float deltaTime, std::vector<Entity*> &entities, Map *map);
    void Render(ShaderProgram *program);
    bool CheckCollision(Entity *object);

    void CheckCollisionsX(Map *map);
    void CheckCollisionsY(Map *map);

    void DrawSpriteFromTextureAtlas(ShaderProgram *program, GLuint textureID, int index);
};
