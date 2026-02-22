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

Vector3 Unity::Transform::GetForward() {
	Vector3 ret{};
	Il2Cpp::Method::Call<void>("UnityEngine", "Transform", "get_forward", 0, &ret, this);
	return ret;
}

Vector3 Unity::Transform::GetRight() {
	Vector3 ret{};
	Il2Cpp::Method::Call<void>("UnityEngine", "Transform", "get_right", 0, &ret, this);
	return ret;
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
	return Il2Cpp::Method::Call<Vector3>("UnityEngine", "Camera", "WorldToViewportPoint", 1, this, position);
}

void Camera::set_fieldOfView(float value)
{
	Il2Cpp::Method::Call<void>("UnityEngine", "Camera", "set_fieldOfView", 1, this, value);
}

void Unity::Rigidbody::SetUseGravity(bool value)
{
	Il2Cpp::Method::Call<void>("UnityEngine", "Rigidbody", "set_useGravity", 1, this, value);
}

void Unity::Rigidbody::SetIsKinematic(bool value)
{
	Il2Cpp::Method::Call<void>("UnityEngine", "Rigidbody", "set_isKinematic", 1, this, value);
}

void Unity::Rigidbody::SetVelocity(Vector3 value) {
	Il2Cpp::Method::Call<void>("UnityEngine", "Rigidbody", "set_velocity", 1, this, value);
}

void Unity::Rigidbody::SetDetectCollisions(bool value) {
	Il2Cpp::Method::Call<void>("UnityEngine", "Rigidbody", "set_detectCollisions", 1, this, value);
}

void Unity::Rigidbody::SetCollisionDetectionMode(int32_t value) {
	Il2Cpp::Method::Call<void>("UnityEngine", "Rigidbody", "set_collisionDetectionMode", 1, this, value);
}

float Unity::Time::GetDeltaTime() {
	return Il2Cpp::Method::Call<float>("UnityEngine", "Time", "get_deltaTime", 0);
}

void Unity::Cursor::SetVisible(bool value) {
	Il2Cpp::Method::Call<void>("UnityEngine", "Cursor", "set_visible", 1, value);
}

bool Unity::Cursor::GetVisible() {
	return Il2Cpp::Method::Call<bool>("UnityEngine", "Cursor", "get_visible", 0);
}

void Unity::Cursor::SetLockState(int value) {
	Il2Cpp::Method::Call<void>("UnityEngine", "Cursor", "set_lockState", 1, value);
}

int Unity::Cursor::GetLockState() {
	return Il2Cpp::Method::Call<int>("UnityEngine", "Cursor", "get_lockState", 0);
}
