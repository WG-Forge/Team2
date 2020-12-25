#include "SDL_window.h"
#include <stdexcept>

constexpr int BORDER_WIDTH = 100;

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
	SDL_RenderDrawLine(renderer, TranslateX(x0), TranslateY(y0), TranslateX(x1), TranslateY(y1));
	UpdateTarget(x0, y0, x1, y1);
}

void SdlWindow::DrawRectangle(int x0, int y0, int x1, int y1) {
	SDL_Rect rect;
	rect.x = TranslateX(x0);
	rect.y = TranslateY(y0);
	rect.h = (y1 - y0) * scaleY;
	rect.w = (x1 - x0) * scaleX;
	if (std::min(rect.h, rect.w) > 1) {
		SDL_RenderDrawRect(renderer, &rect);
	}
	else {
		SDL_RenderDrawPoint(renderer, rect.x, rect.y);
	}
	UpdateTarget(x0, y0, x1, y1);
}

void SdlWindow::DrawTexture(int xMiddle, int yMiddle, int h, int w, SDL_Texture* texture, int absoluteOffsetY, bool toMirror) {
	SDL_Rect target;
	target.h = h;
	target.w = w;
	target.x = TranslateX(xMiddle) - w / 2;
	target.y = absoluteOffsetY + TranslateY(yMiddle) - h / 2;
	if (toMirror) {
		SDL_RenderCopyEx(renderer, texture, NULL, &target, 0.0, NULL, SDL_FLIP_HORIZONTAL);
	}
	else {
		SDL_RenderCopyEx(renderer, texture, NULL, &target, 0.0, NULL, SDL_FLIP_NONE);
	}
}

void SdlWindow::FillRectangle(int xMiddle, int yMiddle, int h, int w, int absoluteOffsetY) {
	SDL_Rect target;
	target.h = h;
	target.w = w;
	target.x = TranslateX(xMiddle) - w / 2;
	target.y = absoluteOffsetY + TranslateY(yMiddle) - h / 2;
	SDL_RenderFillRect(renderer, &target);
}

void SdlWindow::DrawRectangle(int xMiddle, int yMiddle, int h, int w, int absoluteOffsetY) {
	SDL_Rect target;
	target.h = h;
	target.w = w;
	target.x = TranslateX(xMiddle) - w / 2;
	target.y = absoluteOffsetY + TranslateY(yMiddle) - h / 2;
	SDL_RenderDrawRect(renderer, &target);
}

void SdlWindow::SetDrawColor(unsigned char r, unsigned char g, unsigned char b) {
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

void SdlWindow::SetScale(int width, int height) {
	scaleX = (this->width * 1.0) / (width - 2 * BORDER_WIDTH);
	scaleY = (this->height * 1.0) / (height - 2 * BORDER_WIDTH);
}

void SdlWindow::Clear() {
	SDL_RenderClear(renderer);
}

void SdlWindow::Update() {
	SDL_RenderPresent(renderer);
	if (hasTarget) {
		scaleX = (width - 2 * BORDER_WIDTH) / (targetMaxX - targetMinX);
		scaleY = (height - 2 * BORDER_WIDTH) / (targetMaxY - targetMinY);
		offsetX = -targetMinX * scaleX;
		offsetX += ((width - 2 * BORDER_WIDTH) - (targetMaxX * scaleX + offsetX)) / 2;
		offsetY = -targetMinY * scaleY;
		offsetY += (((height - 2 * BORDER_WIDTH) - (targetMaxY * scaleY + offsetY)) / 2);
	}
	hasTarget = false;
}

void SdlWindow::Close() {
	if (renderer) {
		SDL_DestroyRenderer(renderer);
		renderer = nullptr;
	}
	if (window) {
		SDL_DestroyWindow(window);
		window = nullptr;
	}
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
	Close();
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

int SdlWindow::TranslateX(int x)
{
	return BORDER_WIDTH + x * scaleX + offsetX;
}

int SdlWindow::TranslateY(int y)
{
	return BORDER_WIDTH + y * scaleY + offsetY;
}
