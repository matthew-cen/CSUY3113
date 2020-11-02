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
enum EntityMoveState {RUN, JUMP, IDLE, ATTACK};
enum EntityDirection {LEFT, RIGHT};
enum AIState {
    PATROLLING, // patrol flat platform
    ALERTED, // enemy senses player 
    PASSIVE,
    ATTACKING // enemy attacks player
    };
enum AIType {
    PATROLLER, // patrols platforn, aggressive
    JUMPER, // jumps in place, attacks only when player close
    COWARD // runs from player, attacks only when player close
    };


class Entity {
public:
    EntityType entityType;
    EntityMoveState moveState;
    EntityDirection direction;
    AIState aiState;
    AIType aiType;

    bool active = true;
    bool alive = true;
    bool jump = false;

    float jumpPower = 5.0f;
    float attackRange = 0.6f;
    float detectionRange = 1.5f;

    glm::vec3 position = glm::vec3(0);
    glm::vec3 velocity = glm::vec3(0);
    glm::vec3 acceleration = glm::vec3(0);
    GLuint textureID;
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    // TODO: Create class/map for animations
    std::vector<int> *animMoveRight = NULL;
    std::vector<int> *animMoveLeft = NULL;
    std::vector<int> *animMoveUp = NULL;
    std::vector<int> *animMoveDown = NULL;
    std::vector<int> *animIdleLeft = NULL;
    std::vector<int> *animIdleRight = NULL;
    std::vector<int> *animAttackLeft = NULL;
    std::vector<int> *animAttackRight= NULL;

    std::vector<int> *animIndices = NULL;

    float moveSpeed = 1.0f;
    unsigned int animIndex = 0;
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
    bool collidedRightFallSensor; 
    bool collidedLeftFallSensor; 



    EntityType lastCollision;
    
    Entity(enum EntityType entityType, GLuint textureID);
    
    void Update(float deltaTime, std::vector<Entity*> &entities, Map *map);
    void Render(ShaderProgram *program);
    bool CheckCollision(std::vector<Entity*> &entities);

    void CheckCollisionsX(Map *map);
    void CheckCollisionsY(Map *map);
    void SetMoveState(enum EntityMoveState newState);
    void DrawSpriteFromTextureAtlas(ShaderProgram *program, GLuint textureID, int index);

    void AI(Entity *player);
};
