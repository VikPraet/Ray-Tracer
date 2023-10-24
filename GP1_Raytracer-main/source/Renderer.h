#pragma once

#include <cstdint>
#include "DataTypes.h"
#include "Material.h"
#include "Camera.h"

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
		void RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 camerOrigin, Camera& camera, std::vector<dae::Material*>& materials, std::vector<dae::Light>& lights) const;
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

		LightingMode m_CurrentLightingMode{ LightingMode::Combined };
		bool m_ShadowsEnabled{ true };
	};
}
