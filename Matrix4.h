#pragma once
#include "Vector3.h"

class Matrix4
{
public:
	float m[4][4];

	Matrix4();
	Matrix4(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33
	);

	Matrix4& operator*=(const Matrix4& m2);
};

const Matrix4& MatrixIdentity();

const Matrix4& MatrixScaling(float x, float y, float z);

const Matrix4& MatrixRotationX(float angle);
const Matrix4& MatrixRotationY(float angle);
const Matrix4& MatrixRotationZ(float angle);

const Matrix4& MatrixTranslation(float x, float y, float z);

const Matrix4& MatrixLookAtLH(const Vector3& eye, const Vector3& target, const Vector3& up);

const Matrix4& MatrixProjectionMatrix(float FovAngleY, float AspectRatio, float NearZ, float FarZ);

const Vector3& Vector3Transform(const Vector3& v, const Matrix4& m);

const Matrix4& operator*(const Matrix4& m1, const Matrix4& m2);
const Vector3& operator*(const Vector3& v, const Matrix4& m);