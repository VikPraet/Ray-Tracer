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

	for (int px{}; px < m_Width; ++px)
	{
		float x = (2.f * ((static_cast<float>(px) + 0.5f) / static_cast<float>(m_Width)) - 1.f) * aspectRatio * camera.fovValue;

		for (int py{}; py < m_Height; ++py)
		{
			float y = (1.f - 2.f * ((static_cast<float>(py) + 0.5f) / static_cast<float>(m_Height))) * camera.fovValue;
			Vector3 rayDirection{ x, y, 1 };

			rayDirection.Normalize();
			rayDirection = camera.CalculateCameraToWorld().TransformVector(rayDirection);
			Ray hitray{ camera.origin, rayDirection };

			ColorRGB finalColor{};

			HitRecord closestHit{};
			pScene->GetClosestHit(hitray, closestHit);

			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();

				switch (m_CurrentLightingMode)
				{
				case dae::Renderer::LightingMode::ObservedArea:
				{
					CalculateObservedArea(pScene, closestHit, finalColor);
				}
				break;

				case dae::Renderer::LightingMode::Radiance:
				{
					CalculateRadiance(pScene, closestHit, finalColor);
				}
				break;

				case dae::Renderer::LightingMode::BDRF:
				{

				}
				break;

				case dae::Renderer::LightingMode::Combined:
				{

				}
				break;

				default:
					break;
				}
				CalculateShadows(pScene, closestHit, finalColor);
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

void dae::Renderer::CalculateObservedArea(const Scene* pScene, const HitRecord& closestHit, ColorRGB& finalColor) const
{
	finalColor = { 1.f,1.f ,1.f };

	float averageObserveAreaMeasure{ };
	for (int i{}; i < pScene->GetLights().size(); ++i)
	{
		Vector3 LightVector{ LightUtils::GetDirectionToLight(pScene->GetLights()[i], closestHit.origin) };
		Ray lightRayDirection{ closestHit.origin + closestHit.normal * 0.001f, LightVector.Normalized() };
		lightRayDirection.max = LightVector.Magnitude();

		float temp{ Vector3::Dot(closestHit.normal, lightRayDirection.direction.Normalized()) };
		if (temp > 0)
		{
			averageObserveAreaMeasure += temp;
		}
	}

	averageObserveAreaMeasure /= pScene->GetLights().size();
	finalColor *= averageObserveAreaMeasure;
}

void dae::Renderer::CalculateRadiance(const Scene* pScene, const HitRecord& closestHit, ColorRGB& finalColor) const
{
	ColorRGB averageRadiance{ 0.f, 0.f, 0.f };

	for (int i{}; i < pScene->GetLights().size(); ++i)
	{
		Vector3 LightVector{ LightUtils::GetDirectionToLight(pScene->GetLights()[i], closestHit.origin) };
		Ray lightRayDirection{ closestHit.origin + closestHit.normal * 0.001f, LightVector.Normalized() };
		lightRayDirection.max = LightVector.Magnitude();

		averageRadiance += LightUtils::GetRadiance(pScene->GetLights()[i], closestHit.origin);
	}

	averageRadiance /= pScene->GetLights().size();
	finalColor *= averageRadiance;
}

void dae::Renderer::CalculateShadows(const Scene* pScene, const HitRecord& closestHit, ColorRGB& finalColor) const
{
	for (int i{}; i < pScene->GetLights().size(); ++i)
	{
		Vector3 LightVector{ LightUtils::GetDirectionToLight(pScene->GetLights()[i], closestHit.origin) };
		Ray lightRayDirection{ closestHit.origin + closestHit.normal * 0.001f, LightVector.Normalized() };
		lightRayDirection.max = LightVector.Magnitude();

		if (pScene->DoesHit(lightRayDirection) && m_ShadowsEnabled)
		{
			finalColor *= 0.5f;
		}
	}
}
