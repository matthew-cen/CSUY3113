#pragma once
#define GL_SILENCE_DEPRECATION
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Util.h"
#include "Entity.h"
#include "Map.h"


struct GameState
{
    std::vector<Entity*> entities;
    unsigned int aliveEnemyNum;
    Entity* player;
    Map* map;
    int nextScene;
};
class Scene {
public:
	GameState state;
    GLuint fontTextureID;
    GLuint bgTextureID;
	virtual void Initialize() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render(ShaderProgram* program);
    void renderBG(ShaderProgram* program);
    Entity* createPlayer(const GLuint const textureID, float const pos_x, float const pos_y);
    Entity* createHound(const GLuint const textureID, float const pos_x, float const pos_y, enum AIType type);
};