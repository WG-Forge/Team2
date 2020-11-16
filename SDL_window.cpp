#include "SDL_window.h"
#include <stdexcept>
#include <iostream>

SdlWindow::SdlWindow(const std::string& name, size_t width, size_t height) {
	this->width = width;
	this->height = height;
	has_target = false;
	window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
	if (!window) {
		throw std::runtime_error{ SDL_GetError() };
	}
	renderer = SDL_CreateRenderer(window, 0, 0);
	if (!renderer) {
		SDL_DestroyWindow(window);
		throw std::runtime_error{ SDL_GetError() };
	}
}

void SdlWindow::DrawLine(int x0, int y0, int x1, int y1) {
	SDL_RenderDrawLine(renderer, offset_x + x0 * scale, offset_y + y0 * scale, offset_x + x1 * scale, offset_y + y1 * scale);
	updateTarget(x0, y0, x1, y1);
}

void SdlWindow::DrawRectangle(int x0, int y0, int x1, int y1) {
	SDL_Rect rect;
	rect.x = offset_x + x0 * scale;
	rect.y = offset_y + y0 * scale;
	rect.h = (y1 - y0) * scale;
	rect.w = (x1 - x0) * scale;
	if (std::min(rect.h, rect.w) > 1)
	{
		SDL_RenderDrawRect(renderer, &rect);
	}
	else
	{
		SDL_RenderDrawPoint(renderer, rect.x, rect.y);
	}
	updateTarget(x0, y0, x1, y1);
}

void SdlWindow::SetDrawColor(unsigned char r, unsigned char g, unsigned char b) {
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

void SdlWindow::Clear() {
	SDL_RenderClear(renderer);
}

void SdlWindow::Update() {
	SDL_RenderPresent(renderer);
	if (has_target)
	{
		scale = std::min(width / ((target_max_x - target_min_x)), height / ((target_max_y - target_min_y)));
		offset_x = -target_min_x * scale;
		offset_x += (width - (target_max_x * scale + offset_x)) / 2;
		offset_y = -target_min_y * scale;
		offset_y += ((height - (target_max_y * scale + offset_y)) / 2);
	}
	has_target = false;
}

bool SdlWindow::HasCloseRequest() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			return true;
		case SDL_KEYDOWN:
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_ESCAPE:
				return true;
			}
		}
	}
	return false;
}

SdlWindow::~SdlWindow() {
	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}
	if (window) {
		SDL_DestroyWindow(window);
	}
}

void SdlWindow::updateTarget(int minX, int minY, int maxX, int maxY)
{
	if (!has_target)
	{
		target_min_x = minX;
		target_max_x = maxX;
		target_min_y = minY;
		target_max_y = maxY;
	}
	has_target = true;
	if (target_min_x > minX)
	{
		target_min_x = minX;
	}
	if (target_max_x < maxX)
	{
		target_max_x = maxX;
	}
	if (target_min_y > minY)
	{
		target_min_y = minY;
	}
	if (target_max_y < maxY)
	{
		target_max_y = maxY;
	}
}
