#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1

			//float a{ Vector3::Dot(ray.direction, ray.direction) };
			float b{ Vector3::Dot((2.f * ray.direction), (ray.origin - sphere.origin)) };
			float c{ Vector3::Dot((ray.origin - sphere.origin), ((ray.origin - sphere.origin))) - (sphere.radius * sphere.radius) };

			float d = (b * b) - (4.f * c);
			if (d <= 0) return false;

			float t = (-b - sqrt(d)) / 2;
			if (t < ray.min || t > ray.max) return false;

			hitRecord.t = t;
			hitRecord.origin = ray.origin + ray.direction * hitRecord.t;
			hitRecord.normal = (hitRecord.origin - sphere.origin) / sphere.radius;
			hitRecord.didHit = true;
			hitRecord.materialIndex = sphere.materialIndex;
			return true;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			
			float t{Vector3::Dot((plane.origin - ray.origin), plane.normal) / (Vector3::Dot(ray.direction, plane.normal))};

			if (t < ray.max && t > ray.min)
			{
				hitRecord.t = t;
				hitRecord.origin = ray.origin + ray.direction * hitRecord.t;
				hitRecord.normal = plane.normal;
				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
				return true;
			}
			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			Vector3 e{};
			Vector3 p{};

			Vector3 a{ triangle.v1 - triangle.v0 };
			Vector3 b{ triangle.v2 - triangle.v0 };
			Vector3 n{ Vector3::Cross(a, b) };

			if (dae::AreEqual(Vector3::Dot(n, ray.direction), 0)) return false;

			float t{ Vector3::Dot((triangle.v0 - ray.origin), n) / (Vector3::Dot(ray.direction, n)) };
			if (t < ray.min || t > ray.max) return false;

			Vector3 point{ ray.origin + ray.direction * t };

			// vertiex 1
			e = triangle.v1 - triangle.v0;
			p = point - triangle.v0;
			if (Vector3::Dot(Vector3::Cross(e, p), n) < 0) return false;

			// vertiex 2
			e = triangle.v2 - triangle.v1;
			p = point - triangle.v1;
			if (Vector3::Dot(Vector3::Cross(e, p), n) < 0) return false;

			// vertiex 3
			e = triangle.v0 - triangle.v2;
			p = point - triangle.v2;
			if (Vector3::Dot(Vector3::Cross(e, p), n) < 0) return false;

			hitRecord.t = t;
			hitRecord.origin = ray.origin + ray.direction * hitRecord.t;
			hitRecord.normal = n;
			hitRecord.didHit = true;
			hitRecord.materialIndex = triangle.materialIndex;

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			return { light.origin - origin };
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			
			if (light.type == LightType::Point)
			{
				float distanceSqr{ Vector3{light.origin - target }.SqrMagnitude() };
				float radiance{ light.intensity / distanceSqr };

				return{ light.color * radiance };
			}
			else
			{
				return{ light.color * light.intensity };
			}

			return{};
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}