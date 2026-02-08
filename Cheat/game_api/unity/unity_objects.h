#pragma once
#include "unity_types.h"
#include "../il2cpp/il2cpp_types.h"

namespace Unity
{

	struct Object : Il2CppObject {};

	struct Transform;
	struct Component : Object {
		Transform* GetTransform();
	};

	struct Transform : Component {
		Vector3 GetPosition();
		void SetPosition(const Vector3&);
		Vector3 GetForward();
		Vector3 GetRight();
	};

	struct GameObject : Object {
		Transform* GetTransform();
	};

	struct Behaviour : Component {};

	struct Rigidbody : Component {
		void SetVelocity(Vector3 value);
		void SetDetectCollisions(bool value);
		void SetCollisionDetectionMode(int32_t value);
	};

	struct Camera : Behaviour {
		//static Camera* get_current();
		static Camera* GetMain();

		Vector3 WorldToScreenPoint(Vector3 position);
		Vector3 WorldToViewportPoint(Vector3 position);
		void set_fieldOfView(float value);
	};

	// utils, needs to be refactored
	struct Time {
		static float GetDeltaTime();
	};

}
