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

			if (!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.origin = ray.origin + ray.direction * hitRecord.t;
				hitRecord.normal = (hitRecord.origin - sphere.origin) / sphere.radius;
				hitRecord.didHit = true;
				hitRecord.materialIndex = sphere.materialIndex;
			}
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
				if (!ignoreHitRecord)
				{
					hitRecord.t = t;
					hitRecord.origin = ray.origin + ray.direction * hitRecord.t;
					hitRecord.normal = plane.normal;
					hitRecord.didHit = true;
					hitRecord.materialIndex = plane.materialIndex;
				}
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

			if (dae::AreEqual(Vector3::Dot(triangle.normal, ray.direction), 0)) return false;

			switch (triangle.cullMode)
			{
			case TriangleCullMode::FrontFaceCulling:
				if (Vector3::Dot(triangle.normal, ray.direction) < 0) return false;
				break;

			case TriangleCullMode::BackFaceCulling:
				if (Vector3::Dot(triangle.normal, ray.direction) > 0) return false;
				break;
			}


			float t{ Vector3::Dot((triangle.v0 - ray.origin), triangle.normal) / (Vector3::Dot(ray.direction, triangle.normal)) };
			if (t < ray.min || t > ray.max) return false;

			Vector3 point{ ray.origin + ray.direction * t };

			// vertiex 1
			e = triangle.v1 - triangle.v0;
			p = point - triangle.v0;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0) return false;

			// vertiex 2
			e = triangle.v2 - triangle.v1;
			p = point - triangle.v1;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0) return false;

			// vertiex 3
			e = triangle.v0 - triangle.v2;
			p = point - triangle.v2;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0) return false;

			if (!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.origin = ray.origin + ray.direction * hitRecord.t;
				hitRecord.normal = triangle.normal;
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
			}
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
			////todo W5
			//for (int i{}; i < mesh.indices.size(); i += 3)
			//{
			//	int idx0 = mesh.indices[i];
			//	int idx1 = mesh.indices[i + 1];
			//	int idx2 = mesh.indices[i + 2];

			//	Triangle triangle{};
			//	triangle.v0 = mesh.transformedPositions[idx0];
			//	triangle.v1 = mesh.transformedPositions[idx1];
			//	triangle.v2 = mesh.transformedPositions[idx2];

			//	triangle.cullMode = mesh.cullMode;
			//	triangle.materialIndex = mesh.materialIndex;
			//	triangle.normal = mesh.transformedNormals[i / 3];

			//	if (HitTest_Triangle(triangle, ray, hitRecord, ignoreHitRecord))
			//	{
			//		return true;
			//	}
			//}
			//return false;

			bool hitAnything = false;
			float closestHitDistance = FLT_MAX;

			for (size_t i = 0; i < mesh.indices.size(); i += 3)
			{
				const int i0 = mesh.indices[i];
				const int i1 = mesh.indices[i + 1];
				const int i2 = mesh.indices[i + 2];

				const Vector3& v0 = mesh.transformedPositions[i0];
				const Vector3& v1 = mesh.transformedPositions[i1];
				const Vector3& v2 = mesh.transformedPositions[i2];

				const Vector3 edge1 = v1 - v0;
				const Vector3 edge2 = v2 - v0;

				const Vector3 h = Vector3::Cross(ray.direction, edge2);
				const float a = Vector3::Dot(edge1, h);

				// Handle culling
				if (mesh.cullMode == TriangleCullMode::BackFaceCulling)
				{
					if (a < -0)
						continue;
				}
				else if (mesh.cullMode == TriangleCullMode::FrontFaceCulling)
				{
					if (a > -0)
						continue;
				}


				// Check if inside of triangle
				const float f = 1.0f / a;
				const Vector3 s = ray.origin - v0;
				const float u = f * Vector3::Dot(s, h);

				// Check for u inside triangle
				if (u < 0.0f || u > 1.0f)
					continue;

				const Vector3 q = Vector3::Cross(s, edge1);
				const float v = f * Vector3::Dot(ray.direction, q);

				// Check for v inside triangle
				if (v < 0.0f || u + v > 1.0f)
					continue;


				// Get distance
				const float distance = f * Vector3::Dot(edge2, q);

				// If hit is in ray bounds
				if (distance > ray.max || distance < ray.min)
					continue;

				// Check if this is the closes hit
				if (distance < closestHitDistance)
				{
					closestHitDistance = distance;
					hitAnything = true;

					if (!ignoreHitRecord)
					{
						hitRecord.t = distance;
						hitRecord.origin = ray.origin + ray.direction * distance;
						hitRecord.normal = Vector3::Cross(edge1, edge2).Normalized();
						hitRecord.didHit = true;
						hitRecord.materialIndex = mesh.materialIndex;
					}
				}
			}

			return hitAnything;
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