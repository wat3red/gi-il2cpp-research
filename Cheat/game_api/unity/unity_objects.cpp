#include "unity_objects.h"

using namespace Unity;

Vector3 Transform::GetPosition()
{
	Vector3 ret{};
	Il2Cpp::Method::Call<void>("UnityEngine", "Transform", "get_position", 0, &ret, this);
	return ret;
}

void Transform::SetPosition(const Vector3& value)
{
	Il2Cpp::Method::Call<void>("UnityEngine", "Transform", "set_position", 1, this, value);
}

Transform* Component::GetTransform()
{
	return Il2Cpp::Method::Call<Transform*>("UnityEngine", "Component", "get_transform", 0, this);
}

Transform* GameObject::GetTransform()
{
	return Il2Cpp::Method::Call<Transform*>("UnityEngine", "GameObject", "get_transform", 0, this);
}

Camera* Camera::GetMain()
{
	return Il2Cpp::Method::Call<Camera*>("UnityEngine", "Camera", "get_main", 0);
}

Vector3 Camera::WorldToViewportPoint(Vector3 position)
{
	Vector3 ret{};
	Il2Cpp::Method::Call<Vector3>("UnityEngine", "Camera", "WorldToViewportPoint", 1, &ret, this, position);
	return ret;

	//return Il2Cpp::Method::Call<Vector3>("UnityEngine", "Camera", "WorldToViewportPoint", this, position);
}

void Camera::set_fieldOfView(float value)
{
	Il2Cpp::Method::Call<void>("UnityEngine", "Camera", "set_fieldOfView", 1, this, value);
}
