#include "Menu.h"

void Menu::Initialize()
{
	state.nextScene = -1;
	fontTextureID = Util::LoadTexture("assets/font1.png");
	bgTextureID = Util::LoadTexture("assets/BG3.png"); // background
}
void Menu::Update(float deltaTime)
{

}
void Menu::Render(ShaderProgram* program)
{
	Scene::renderBG(program);
	Util::DrawText(program, fontTextureID, "Wooded Adventures", 1.0f, -0.5f, glm::vec3(-4.0f, 2.0f, 0.0f));
	Util::DrawText(program, fontTextureID, "Press [Enter] to begin", 0.7f, -0.4f, glm::vec3(-4.0f, 1.0f, 0.0f));

}