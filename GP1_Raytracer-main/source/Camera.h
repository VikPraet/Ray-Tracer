#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

#include <iostream>

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{45.f};
		float fovValue{tanf(fovAngle * TO_RADIANS / 2.f) };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			right = Vector3::Cross(Vector3::UnitY, forward);
			right.Normalize();
			
			up = Vector3::Cross(forward, right);
			up.Normalize();
			return{
				{ right.x	, right.y	, right.z	, 0},
				{ up.x		, up.y		, up.z		, 0},
				{ forward.x	, forward.y	, forward.z	, 0},
				{ origin.x	, origin.y	, origin.z	, 0}
			};
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			const float SPEED{7};
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward *  SPEED * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * SPEED * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * SPEED * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * SPEED * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_Q])
			{
				origin.y += up.y * SPEED * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_E])
			{
				origin.y -= up.y * SPEED * deltaTime;
			}


			const float FOV_INCREMENT	{ 20 };
			const int MAX_FOV			{ 179 };
			const int MIN_FOV			{ 10 };

			if (pKeyboardState[SDL_SCANCODE_DOWN] && fovAngle < MAX_FOV)
			{
				fovAngle += FOV_INCREMENT * deltaTime;
				fovValue = tanf(fovAngle * TO_RADIANS / 2.f);
			}
			if (pKeyboardState[SDL_SCANCODE_UP] && fovAngle > MIN_FOV)
			{
				fovAngle -= FOV_INCREMENT * deltaTime;
				fovValue = tanf(fovAngle * TO_RADIANS / 2.f);
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			const float SENSITIVITY{ 0.007f };
			const float MOVEMENT_SENSITIVITY{ 0.07f };

			if (mouseState & SDL_BUTTON_RMASK)
			{
				totalPitch += mouseX * SENSITIVITY;
				totalYaw -= mouseY * SENSITIVITY;
				Matrix final{ Matrix::CreateRotationX(totalYaw) * Matrix::CreateRotationY(totalPitch) };

				forward = final.TransformVector(Vector3::UnitZ);
				forward.Normalize();
			}

			if (mouseState & SDL_BUTTON_LMASK)
			{
				totalPitch += mouseX * SENSITIVITY;
				float movement = mouseY * MOVEMENT_SENSITIVITY;
				Matrix final{ Matrix::CreateRotationX(totalYaw) * Matrix::CreateRotationY(totalPitch) };

				origin.x += forward.x * -movement;
				origin.z += forward.z * -movement;

				forward = final.TransformVector(Vector3::UnitZ);
				forward.Normalize();
			}
		}
	};
}
