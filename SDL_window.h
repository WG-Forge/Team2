#pragma once
#include "SDL.h"
#include <string>
#include "TextureManager.h"

class SdlWindow { // window class containing all methods for drawing
private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	int width;
	int height;
	int offsetX = 0;
	int offsetY = 0;
	double scaleX = 1.0;
	double scaleY = 1.0;
	double scale = 1.0;
	double targetMaxX;
	double targetMinX;
	double targetMaxY;
	double targetMinY;
	bool hasTarget;
public:
	SdlWindow(const std::string& name, size_t width = 800, size_t height = 600);
	TextureManager CreateTextureManager();
	void DrawLine(int x0, int y0, int x1, int y1);
	void DrawRectangle(int x0, int y0, int x1, int y1);
	void DrawTexture(int xMiddle, int yMiddle, int h, int w, SDL_Texture* texture);
	void SetDrawColor(unsigned char r, unsigned char g, unsigned char b);
	void SetScale(int width, int height);
	void Clear();
	void Update();
	bool HasCloseRequest();
	~SdlWindow();
private:
	void UpdateTarget(int minX, int minY, int maxX, int maxY);
};

