#pragma once
#include <unordered_map>
#include <SDL.h>

class TextureManager {
private:
	SDL_Renderer* renderer;
	std::unordered_map<std::string, SDL_Texture*> textures;
public:
	TextureManager(SDL_Renderer* renderer);
	TextureManager(const TextureManager& other) = delete;
	TextureManager(TextureManager&& other);
	SDL_Texture* operator[](const std::string& key);
	~TextureManager();
};

