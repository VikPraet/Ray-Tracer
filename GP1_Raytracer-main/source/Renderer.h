#pragma once

#include <cstdint>
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;
struct ColorRGB;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;

		void CycleLigntingMode();
		void ToggleShadows();

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};

		enum class LightingMode
		{
			ObservedArea,
			Radiance,
			BDRF,
			Combined
		};

		LightingMode m_CurrentLightingMode{ LightingMode::ObservedArea };
		bool m_ShadowsEnabled{ true };

		void CalculateObservedArea(const Scene* pScene, const HitRecord& closestHit, ColorRGB& finalColor) const;
		void CalculateRadiance(const Scene* pScene, const HitRecord& closestHit, ColorRGB& finalColor) const;
		void CalculateShadows(const Scene* pScene, const HitRecord& closestHit, ColorRGB& finalColor) const;
	};
}
