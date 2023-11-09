#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region SlabTest TriangeMesh
		inline bool Slabtest_TrianglMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));
			return tmax > 0 && tmax >= tmin;
		}
#pragma endregion
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1

			//float a{ Vector3::Dot(ray.direction, ray.direction) };
			float b{ Vector3::Dot((ray.direction), (ray.origin - sphere.origin)) };
			float c{ Vector3::Dot((ray.origin - sphere.origin), ((ray.origin - sphere.origin))) - (sphere.radius * sphere.radius) };

			float d{ (b * b) - (c) };
			if (d <= 0) return false;

			float t{ (-b - sqrt(d)) };
			if(t < ray.min) t = (-b + sqrt(d));
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

			float Dot{ Vector3::Dot(triangle.normal, ray.direction) };

			if (Dot < FLT_EPSILON && Dot > -FLT_EPSILON) return false;

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

			// vertex 1
			e = triangle.v1 - triangle.v0;
			p = point - triangle.v0;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0) return false;

			// vertex 2
			e = triangle.v2 - triangle.v1;
			p = point - triangle.v1;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0) return false;

			// vertex 3
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
			if (!Slabtest_TrianglMesh(mesh, ray)) return false;

			bool hasHitSomething{ false };
			HitRecord temp{};
			float distance = FLT_MAX;

			for (int i{}; i < mesh.indices.size(); i += 3)
			{
				const Vector3& v0{ mesh.transformedPositions[mesh.indices[i]]     };
				const Vector3& v1{ mesh.transformedPositions[mesh.indices[i + 1]] };
				const Vector3& v2{ mesh.transformedPositions[mesh.indices[i + 2]] };

				const Vector3 edge1{ v1 - v0 };
				const Vector3 edge2{ v2 - v0 };

				const Vector3 h{ Vector3::Cross(ray.direction, edge2) };
				const float a{ Vector3::Dot(edge1, h) };

				if (a < -FLT_EPSILON && mesh.cullMode == TriangleCullMode::BackFaceCulling) continue;
				if (a > FLT_EPSILON && mesh.cullMode == TriangleCullMode::FrontFaceCulling) continue;

				const float f{ 1.0f / a };
				const Vector3 s{ ray.origin - v0 };
				const float u{ f * Vector3::Dot(s, h) };
				if (u < 0.0 || u > 1.0) continue;

				const Vector3 q{ Vector3::Cross(s, edge1) };
				const float v{ f * Vector3::Dot(ray.direction, q) };
				if (v < 0.0 || u + v > 1.0) continue;

				const float t{ f * Vector3::Dot(edge2, q) };
				if (t > ray.max || t < ray.min) continue;

				if (t < distance)
				{
					hasHitSomething = true;
					distance = t;
					if (!ignoreHitRecord)
					{
						hitRecord.t = t;
						hitRecord.origin = ray.origin + ray.direction * hitRecord.t;
						hitRecord.didHit = true;
						hitRecord.materialIndex = mesh.materialIndex;
						hitRecord.normal = Vector3::Cross(edge1, edge2).Normalized();
					}
				}
			}
			return hasHitSomething;
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