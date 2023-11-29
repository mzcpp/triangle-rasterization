#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include "SDL2/SDL.h"

#include <array>

class Triangle
{
private:
	SDL_Renderer* renderer_;
	std::array<SDL_Point, 3> vertices_;
	SDL_Rect bbox_;
	SDL_Color color_;
	Uint32 pixel_color_;
	std::array<SDL_Color, 3> vertices_colors_;
	Uint32* pixels_;
	SDL_Texture* texture_;
	SDL_FPoint center_;

public:
	Triangle(SDL_Renderer* renderer, SDL_Point v1, SDL_Point v2, SDL_Point v3, SDL_Color color);

	~Triangle();

	void CreateTriangleBarycentric();

	bool IsTopLeft(int vx, int vy);
	
	float EdgeCross(SDL_Point v1, SDL_Point v2, SDL_Point p);

	void CreateTexture();
	
	void UpdateBBox();
	
	void UpdateColor();

	void Rotate(const SDL_FPoint& pivot, float degrees);

	SDL_FPoint RotatePoint(const SDL_FPoint& point, const SDL_FPoint& pivot, float degrees);

	void Tick();

	void Render();
};

#endif
