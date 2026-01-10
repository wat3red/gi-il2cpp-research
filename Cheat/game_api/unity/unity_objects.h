#pragma once
#include "unity_types.h"
#include "../il2cpp/il2cpp_types.h"

namespace Unity {

	struct Object : Il2CppObject {};

	struct Transform;
	struct Component : Object {
		Transform* GetTransform();
	};

	struct Transform : Component {
		Vector3 GetPosition();
		void SetPosition(const Vector3&);
	};

	struct GameObject : Object {
		Transform* GetTransform();
	};

	struct Behaviour : Component {
	};

	struct Camera : Behaviour {
		//static Camera* get_current();
		static Camera* GetMain();

		Vector3 WorldToScreenPoint(Vector3 position);
		Vector3 WorldToViewportPoint(Vector3 position);
		void set_fieldOfView(float value);
	};

}
