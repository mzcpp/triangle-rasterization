#include "Constants.hpp"
#include "Triangle.hpp"

#include "SDL2/SDL.h"

#include <iostream>
#include <cmath>

Triangle::Triangle(SDL_Renderer* renderer, SDL_Point v1, SDL_Point v2, SDL_Point v3, SDL_Color color) : 
	renderer_(renderer), 
	color_(color), 
	pixel_color_(0), 
	pixels_(nullptr), 
	texture_(nullptr)
{
	vertices_[0] = v1;
	vertices_[1] = v2;
	vertices_[2] = v3;

	CreateTexture();
	CreateTriangleBarycentric();
}

Triangle::~Triangle()
{
	SDL_DestroyTexture(texture_);
	texture_ = nullptr;

	delete[] pixels_;
	pixels_ = nullptr;
}

void Triangle::CreateTriangleBarycentric()
{
	memset(pixels_, 0, bbox_.w * bbox_.h * sizeof(Uint32));

	int bias0 = IsTopLeft(vertices_[1].x - vertices_[0].x, vertices_[1].y - vertices_[0].y) ? 0 : -1;
	int bias1 = IsTopLeft(vertices_[2].x - vertices_[1].x, vertices_[2].y - vertices_[1].y) ? 0 : -1;
	int bias2 = IsTopLeft(vertices_[0].x - vertices_[2].x, vertices_[0].y - vertices_[2].y) ? 0 : -1;

	float area = EdgeCross(vertices_[2], vertices_[0], vertices_[1]);

	for (Uint32 y = 0; y < static_cast<Uint32>(bbox_.h); ++y)
	{
		for (Uint32 x = 0; x < static_cast<Uint32>(bbox_.w); ++x)
		{
			SDL_Point p = { static_cast<int>(x + bbox_.x), static_cast<int>(y + bbox_.y) };
			float w0 = EdgeCross(vertices_[0], vertices_[1], p) + bias0;
			float w1 = EdgeCross(vertices_[1], vertices_[2], p) + bias1;
			float w2 = EdgeCross(vertices_[2], vertices_[0], p) + bias2;

			if (w0 >= 0 && w1 >= 0 && w2 >= 0)
			{
				float alpha = w0 / area;
				float beta = w1 / area;
				float gamma = w2 / area;

				color_.r = alpha * vertices_colors_[0].r + beta * vertices_colors_[1].r + gamma * vertices_colors_[2].r;
				color_.g = alpha * vertices_colors_[0].g + beta * vertices_colors_[1].g + gamma * vertices_colors_[2].g;
				color_.b = alpha * vertices_colors_[0].b + beta * vertices_colors_[1].b + gamma * vertices_colors_[2].b;
				color_.a = 0xff;

				UpdateColor();

				pixels_[y * bbox_.w + x] = pixel_color_;
			}
			else
			{
				//pixels_[y * bbox_.w + x] = 0xff0000ff;
			}
		}
	}

	SDL_UpdateTexture(texture_, nullptr, pixels_, bbox_.w * sizeof(Uint32));
}

bool Triangle::IsTopLeft(int vx, int vy)
{
	return (vx > 0 && vy == 0) || vy < 0;
}

float Triangle::EdgeCross(SDL_Point v1, SDL_Point v2, SDL_Point p)
{
	SDL_Point v1_v2_vector = { v2.x - v1.x, v2.y - v1.y };
	SDL_Point v1_p_vector = { p.x - v1.x, p.y - v1.y };

	return static_cast<float>(v1_v2_vector.x * v1_p_vector.y - v1_v2_vector.y * v1_p_vector.x);
}

void Triangle::CreateTexture()
{
	if (texture_ != nullptr)
	{
		SDL_DestroyTexture(texture_);
		texture_ = nullptr;
	}
	if (pixels_ != nullptr)
	{
		delete[] pixels_;
		pixels_ = nullptr;
	}
	
	UpdateColor();
	UpdateBBox();
	
	center_ = { (bbox_.w / 2.0f) + bbox_.x, (bbox_.h / 2.0f) + bbox_.y };

	pixels_ = new Uint32[bbox_.w * bbox_.h];
	memset(pixels_, 0, bbox_.w * bbox_.h * sizeof(Uint32));

	texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, bbox_.w, bbox_.h);
	SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);	
}
	
void Triangle::UpdateBBox()
{
	int max_x = std::max(vertices_[0].x, std::max(vertices_[1].x, vertices_[2].x));
	int min_x = std::min(vertices_[0].x, std::min(vertices_[1].x, vertices_[2].x));
	int max_y = std::max(vertices_[0].y, std::max(vertices_[1].y, vertices_[2].y));
	int min_y = std::min(vertices_[0].y, std::min(vertices_[1].y, vertices_[2].y));

	bbox_.x = min_x;
	bbox_.y = min_y;
	bbox_.w = max_x - min_x;
	bbox_.h = max_y - min_y;
}

void Triangle::UpdateColor()
{
	vertices_colors_[0].r = 0xff;
	vertices_colors_[0].g = 0x00;
	vertices_colors_[0].b = 0x00;
	vertices_colors_[0].a = 0xff;

	vertices_colors_[1].r = 0x00;
	vertices_colors_[1].g = 0xff;
	vertices_colors_[1].b = 0x00;
	vertices_colors_[1].a = 0xff;
	
	vertices_colors_[2].r = 0x00;
	vertices_colors_[2].g = 0x00;
	vertices_colors_[2].b = 0xff;
	vertices_colors_[2].a = 0xff;

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	pixel_color_ = (color_.b << 24) + (color_.g << 16) + (color_.r << 8) + 255;
	#else
	pixel_color_ = (255 << 24) + (color_.r << 16) + (color_.g << 8) + color_.b;
	#endif
}

void Triangle::Rotate(const SDL_FPoint& pivot, float degrees)
{
	for (int i = 0; i < 3; ++i)
	{
		const SDL_FPoint original = { static_cast<float>(vertices_[i].x), static_cast<float>(vertices_[i].y) };
		const SDL_FPoint rotated = RotatePoint(original, pivot, degrees);
		vertices_[i].x = static_cast<int>(rotated.x);
		vertices_[i].y = static_cast<int>(rotated.y);
	}

	CreateTexture();
}

SDL_FPoint Triangle::RotatePoint(const SDL_FPoint& point, const SDL_FPoint& pivot, float degrees)
{
	SDL_FPoint result_point = point;

	const double sin_degrees = std::sin(degrees);
	const double cos_degrees = std::cos(degrees);

	const double new_x = (result_point.x - pivot.x) * cos_degrees - (result_point.y - pivot.y) * sin_degrees;
	const double new_y = (result_point.x - pivot.x) * sin_degrees + (result_point.y - pivot.y) * cos_degrees;

	result_point.x = new_x + pivot.x;
	result_point.y = new_y + pivot.y;

	return result_point;
}

void Triangle::Tick()
{
}

void Triangle::Render()
{
	SDL_RenderCopy(renderer_, texture_, nullptr, &bbox_);

	// SDL_RenderDrawLine(renderer_, vertices_[0].x, vertices_[0].y, vertices_[1].x, vertices_[1].y);
	// SDL_RenderDrawLine(renderer_, vertices_[1].x, vertices_[1].y, vertices_[2].x, vertices_[2].y);
	// SDL_RenderDrawLine(renderer_, vertices_[2].x, vertices_[2].y, vertices_[0].x, vertices_[0].y);

	// SDL_SetRenderDrawColor(renderer_, 0x00, 0xff, 0xff, 0xff);
	// SDL_RenderDrawPoint(renderer_, vertices_[0].x, vertices_[0].y);
	// SDL_RenderDrawPoint(renderer_, vertices_[1].x, vertices_[1].y);
	// SDL_RenderDrawPoint(renderer_, vertices_[2].x, vertices_[2].y);

	// SDL_SetRenderDrawColor(renderer_, 0xff, 0xff, 0xff, 0xff);
	// SDL_Rect bbox_copy = bbox_;
	// bbox_copy.x -= 1;
	// bbox_copy.y -= 1;
	// bbox_copy.w += 2;
	// bbox_copy.h += 2;
	// SDL_RenderDrawRect(renderer_, &bbox_copy);
	// SDL_RenderDrawRect(renderer_, &bbox_);
}