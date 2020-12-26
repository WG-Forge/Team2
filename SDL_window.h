#pragma once
#include "SDL.h"
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
	double targetMaxX = 0.0;
	double targetMinX = 0.0;
	double targetMaxY = 0.0;
	double targetMinY = 0.0;
	bool hasTarget = false;
public:
	SdlWindow(const std::string& name, size_t width = 800, size_t height = 600);
	TextureManager CreateTextureManager();
	void DrawLine(int x0, int y0, int x1, int y1);
	void DrawTexture(int xMiddle, int yMiddle, int h, int w, SDL_Texture* texture, int absoluteOffsetY = 0, bool toMirror = false);
	void FillRectangle(int xMiddle, int yMiddle, int h, int w, int absoluteOffsetY = 0);
	void DrawRectangle(int xMiddle, int yMiddle, int h, int w, int absoluteOffsetY);
	void SetDrawColor(unsigned char r, unsigned char g, unsigned char b);
	void Clear();
	void Update();
	void Close();
	bool HasCloseRequest();
	~SdlWindow();
private:
	void UpdateTarget(int minX, int minY, int maxX, int maxY);
	int TranslateX(int x);
	int TranslateY(int y);
};

