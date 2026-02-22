#pragma once
#include "../il2cpp/il2cpp.h"

#include <string>
#include <cmath>

namespace Unity {
	struct Vector3 {
		float x = 0, y = 0, z = 0;
		inline float Distance(Vector3 b) const {
			return sqrt(
				pow(x - b.x, 2) + pow(y - b.y, 2) + pow(z - b.z, 2)
			);
		}
		inline float Magnitude() const {
			return sqrt(
				pow(x, 2) + pow(y, 2) + pow(z, 2)
			);
		}
		Vector3 Normalize() const {
			const float lenghtInverse = 1 / Magnitude();
			return Vector3(
				x * lenghtInverse, y * lenghtInverse, z * lenghtInverse
			);
		}

		bool Zero() { return (x == 0 && y == 0 && z == 0); }
		Vector3 operator *(float k) { return { x * k, y * k, z * k }; }
		Vector3 operator -(Vector3 B) { return { x - B.x, y - B.y, z - B.z }; }
		Vector3 operator +(Vector3 B) { return { x + B.x, y + B.y, z + B.z }; }
		Vector3& operator +=(const Vector3& b) {
			x += b.x;
			y += b.y;
			z += b.z;
			return *this;
		}

		Vector3& operator -=(const Vector3& b) {
			x -= b.x;
			y -= b.y;
			z -= b.z;
			return *this;
		}

		Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
		Vector3() {}

		inline std::string ToString() {
			return std::to_string(this->x) + ", " + std::to_string(this->y) + ", " + std::to_string(this->z);
		}

		static inline Vector3 Null() {
			return { 0, 0, 0 };
		}
	};

	struct Vector2 {
		float x, y;
	};

	struct Bounds {
		Vector3 center;
		Vector3 extents;
	};

	struct Rect {
		float m_XMin; // 0x0
		float m_YMin; // 0x4
		float m_Width; // 0x8
		float m_Height; // 0xC
	};
}
