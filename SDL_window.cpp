#include "SDL_window.h"
#include <stdexcept>
#include <iostream>

SdlWindow::SdlWindow(const std::string& name, size_t width, size_t height) {
	this->width = width;
	this->height = height;
	hasTarget = false;
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

TextureManager SdlWindow::CreateTextureManager() {
	return TextureManager{ renderer };
}

void SdlWindow::DrawLine(int x0, int y0, int x1, int y1) {
	SDL_RenderDrawLine(renderer, offsetX + x0 * scale, offsetY + y0 * scale, offsetX + x1 * scale, offsetY + y1 * scale);
	UpdateTarget(x0, y0, x1, y1);
}

void SdlWindow::DrawRectangle(int x0, int y0, int x1, int y1) {
	SDL_Rect rect;
	rect.x = offsetX + x0 * scale;
	rect.y = offsetY + y0 * scale;
	rect.h = (y1 - y0) * scale;
	rect.w = (x1 - x0) * scale;
	if (std::min(rect.h, rect.w) > 1) {
		SDL_RenderDrawRect(renderer, &rect);
	}
	else {
		SDL_RenderDrawPoint(renderer, rect.x, rect.y);
	}
	UpdateTarget(x0, y0, x1, y1);
}

void SdlWindow::DrawTexture(int xMiddle, int yMiddle, int h, int w, SDL_Texture* texture) {
	SDL_Rect target;
	target.h = h;
	target.w = w;
	target.x = xMiddle * scaleX;
	target.y = yMiddle * scaleY;
	SDL_RenderCopy(renderer, texture, NULL, &target);
}

void SdlWindow::SetDrawColor(unsigned char r, unsigned char g, unsigned char b) {
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

void SdlWindow::SetScale(int width, int height) {
	scaleX = (this->width * 1.0) / width;
	scaleY = (this->height * 1.0) / height;
}

void SdlWindow::Clear() {
	SDL_RenderClear(renderer);
}

void SdlWindow::Update() {
	SDL_RenderPresent(renderer);
	if (hasTarget) {
		scale = std::min(width / ((targetMaxX - targetMinX)), height / ((targetMaxY - targetMinY)));
		offsetX = -targetMinX * scale;
		offsetX += (width - (targetMaxX * scale + offsetX)) / 2;
		offsetY = -targetMinY * scale;
		offsetY += ((height - (targetMaxY * scale + offsetY)) / 2);
	}
	hasTarget = false;
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

void SdlWindow::UpdateTarget(int minX, int minY, int maxX, int maxY)
{
	if (!hasTarget) {
		targetMinX = minX;
		targetMaxX = maxX;
		targetMinY = minY;
		targetMaxY = maxY;
	}
	hasTarget = true;
	if (targetMinX > minX) {
		targetMinX = minX;
	}
	if (targetMaxX < maxX) {
		targetMaxX = maxX;
	}
	if (targetMinY > minY) {
		targetMinY = minY;
	}
	if (targetMaxY < maxY) {
		targetMaxY = maxY;
	}
}
