#ifndef GAME_HPP
#define GAME_HPP

#include <SDL2/SDL.h>

#include "Triangle.hpp"

#include <vector>
#include <memory>

class Game
{
private:
	bool initialized_;
	bool running_;
	int game_ticks_;
	
	SDL_Window* window_;
	SDL_Renderer* renderer_;

	std::vector<std::unique_ptr<Triangle>> triangles_;

public:
	Game();

	~Game();

	bool Initialize();

	void Finalize();

	void Run();

	void HandleEvents();
	
	void Tick();
	
	void Render();

	bool InitializeTriangles();
};

#endif