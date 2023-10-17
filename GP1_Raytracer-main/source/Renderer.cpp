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
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float aspectRatio{ float(m_Width) / float(m_Height) };

	Matrix camMatrix{ camera.CalculateCameraToWorld() };

	for (int px{}; px < m_Width; ++px)
	{
		float x = (2.f * ((static_cast<float>(px) + 0.5f) / static_cast<float>(m_Width)) - 1.f) * aspectRatio * camera.fovValue;

		for (int py{}; py < m_Height; ++py)
		{
			float y = (1.f - 2.f * ((static_cast<float>(py) + 0.5f) / static_cast<float>(m_Height))) * camera.fovValue;
			Vector3 rayDirection{ x, y, 1 };
			rayDirection = camMatrix.TransformVector(rayDirection);

			rayDirection.Normalize();
			Ray hitray{ camera.origin, rayDirection };

			Vector3 v{ hitray.direction.Normalized() * -1.0f };

			HitRecord closestHit{};
			pScene->GetClosestHit(hitray, closestHit);

			ColorRGB finalColor{};

			if (closestHit.didHit)
			{
				for (int i{}; i < pScene->GetLights().size(); ++i)
				{
					Vector3 LightVector{ LightUtils::GetDirectionToLight(pScene->GetLights()[i], closestHit.origin) };
					Vector3 l = LightVector.Normalized();

					Ray lightRayDirection{ closestHit.origin + closestHit.normal * 0.001f, l };
					lightRayDirection.max = LightVector.Magnitude();

					// skip light calculation when light does not hit pixel
					if (pScene->DoesHit(lightRayDirection) && m_ShadowsEnabled) continue;

					float cosineLaw{ std::max(0.f, Vector3::Dot(closestHit.normal, lightRayDirection.direction.Normalized())) };

					switch (m_CurrentLightingMode)
					{
					case dae::Renderer::LightingMode::ObservedArea:
					{
						finalColor += ColorRGB{1.f, 1.f, 1.f} * cosineLaw;
					}
					break;

					case dae::Renderer::LightingMode::Radiance:
					{
						finalColor += LightUtils::GetRadiance(pScene->GetLights()[i], closestHit.origin);
					}
					break;

					case dae::Renderer::LightingMode::BDRF:
					{
						finalColor += materials[closestHit.materialIndex]->Shade(closestHit, l, v);
					}
					break;

					case dae::Renderer::LightingMode::Combined:
					{
						finalColor += LightUtils::GetRadiance(pScene->GetLights()[i], closestHit.origin) * materials[closestHit.materialIndex]->Shade(closestHit, l, v) * cosineLaw;
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
	}
	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
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
