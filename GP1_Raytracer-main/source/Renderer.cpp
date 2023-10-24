//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

#include <execution>

#define PARALLEL_EXECUTION

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	std::vector<dae::Material*> materials = pScene->GetMaterials();
	std::vector<dae::Light> lights = pScene->GetLights();

	float aspectRatio{ float(m_Width) / float(m_Height) };

	Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

#if defined(PARALLEL_EXECUTION)
	// parallel logic
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	std::vector<uint32_t> pixelIndices{};

	pixelIndices.reserve(amountOfPixels);
	for (uint32_t idx{}; idx < amountOfPixels; ++idx) pixelIndices.emplace_back(idx);
	{
		std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i) {
			RenderPixel(pScene, i, camera.fovValue, aspectRatio, cameraToWorld, camera.origin, camera, materials, lights);
			});
	}

#else
	// Synchronous logic (no trheading)
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	for (uint32_t pixelIndex{}; pixelIndex < amountOfPixels; ++pixelIndex)
	{
		RenderPixel(pScene, pixelIndex, camera.fovValue, aspectRatio, cameraToWorld, camera.origin);
	}

#endif
	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 camerOrigin, Camera& camera, std::vector<dae::Material*>& materials, std::vector<dae::Light>& lights) const
{
	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

	float rx{ px + 0.5f }, ry{ py + 0.5f };
	float cx{ (2 * (rx / float(m_Width)) - 1) * aspectRatio * fov };
	float cy{ (1 - (2 * (ry / float(m_Height)))) * fov };


	Ray viewRay{ camera.origin };
	Vector3 rayDirection{cx, cy, 1 };

	viewRay.direction = cameraToWorld.TransformVector(rayDirection.Normalized());
	Vector3 v{ viewRay.direction * -1 };

	HitRecord closestHit{};
	pScene->GetClosestHit(viewRay, closestHit);

	ColorRGB finalColor{};
	const Vector3 hitPlusOffset{ closestHit.origin + closestHit.normal * 0.001f };

	if (closestHit.didHit)
	{
		for (int i{}; i < pScene->GetLights().size(); ++i)
		{
			Vector3 toHitVector{ hitPlusOffset - lights[i].origin };
			Vector3 l{ toHitVector.Normalized() };

			Ray toLightRay{ lights[i].origin, l, 0.0f, toHitVector.Magnitude() };

			// skip light calculation when light does not hit pixel
			if (m_ShadowsEnabled && pScene->DoesHit(toLightRay)) continue;

			float cosineLaw{ std::max(0.f, Vector3::Dot(closestHit.normal, -toLightRay.direction)) };

			switch (m_CurrentLightingMode)
			{
			case dae::Renderer::LightingMode::ObservedArea:
			{
				finalColor += ColorRGB{ 1.f, 1.f, 1.f } *cosineLaw;
			}
			break;

			case dae::Renderer::LightingMode::Radiance:
			{
				finalColor += LightUtils::GetRadiance(pScene->GetLights()[i], closestHit.origin);
			}
			break;

			case dae::Renderer::LightingMode::BDRF:
			{
				finalColor += materials[closestHit.materialIndex]->Shade(closestHit, -l, v);
			}
			break;

			case dae::Renderer::LightingMode::Combined:
			{
				finalColor += LightUtils::GetRadiance(pScene->GetLights()[i], closestHit.origin) * materials[closestHit.materialIndex]->Shade(closestHit, -l, v) * cosineLaw;
			}
			break;
			}
		}
	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::CycleLigntingMode()
{
	m_CurrentLightingMode = static_cast<LightingMode>(int(m_CurrentLightingMode) + 1);
	if (int(m_CurrentLightingMode) > 3)
		m_CurrentLightingMode = LightingMode::ObservedArea;
}

void dae::Renderer::ToggleShadows()
{
	m_ShadowsEnabled = !m_ShadowsEnabled;
}
