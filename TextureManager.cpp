#include "TextureManager.h"
#include <SDL_image.h>

TextureManager::TextureManager(SDL_Renderer* renderer) : renderer{ renderer } {
}

TextureManager::TextureManager(TextureManager&& other) : renderer{ other.renderer }, textures{ std::move(other.textures) } {
}

SDL_Texture* TextureManager::operator[](const std::string& key)
{
    if (textures.find(key) != textures.end()) {
        return textures[key];
    }
    SDL_Surface* surface = IMG_Load(key.c_str());
    SDL_Texture* result = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    textures[key] = result;
    return result;
}

TextureManager::~TextureManager() {
    for (auto& [key, value] : textures) {
        SDL_DestroyTexture(value);
    }
}
