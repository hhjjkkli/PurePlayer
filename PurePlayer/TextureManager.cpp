#include "TextureManager.h"
#include "ManagerPlayer.h"

SDL_Texture* TextureManager::LoadTexture(const char *filepath) {
	SDL_Surface* tempSurface = SDL_LoadBMP(filepath);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(ManagerPlayer::renderer, tempSurface);
	SDL_FreeSurface(tempSurface);

	return tex;
}

void TextureManager::Draw(SDL_Texture * tex, SDL_Rect *src, SDL_Rect *dest, SDL_RendererFlip flip)
{
	SDL_RenderCopyEx(ManagerPlayer::renderer, tex, src, dest, NULL, NULL, flip);
}
