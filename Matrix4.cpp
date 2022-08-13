#include "Matrix4.h"
#include <cmath>

Matrix4::Matrix4():
	m{
		0, 0, 0, 0,
		0, 0, 0, 0, 
		0, 0, 0, 0,
		0, 0, 0, 0
	}
{
}

Matrix4::Matrix4(
	float m00, float m01, float m02, float m03, 
	float m10, float m11, float m12, float m13, 
	float m20, float m21, float m22, float m23, 
	float m30, float m31, float m32, float m33
	):
	m{
	m00, m01, m02, m03,
	m10, m11, m12, m13,
	m20, m21, m22, m23,
	m30, m31, m32, m33
	}
{
}

Matrix4 &Matrix4::operator*=(const Matrix4 &m2)
{
	Matrix4 result;

	for(int i = 0; i < 4; i++){//’i—Ž
		for(int j = 0; j < 4; j++){//s
			for(int k = 0; k < 4; k++){//—ñ
				
					result.m[i][j] += this->m[i][k]*m2.m[k][j];
			}
		}
	}
	*this = result;

	return *this;
}

const Matrix4 &MatrixIdentity()
{
	static Matrix4 result{
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f, 
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	return result;
}

const Matrix4 &MatrixScaling(float x, float y, float z)
{
	static Matrix4 result{
		x  , 0.f, 0.f, 0.f,
		0.f,   y, 0.f, 0.f, 
		0.f, 0.f,   z, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	return result;
}

const Matrix4 &MatrixRotationX(float angle)
{
	float sin = std::sin(angle);
	float cos = std::cos(angle);

	static Matrix4 result
	{
		1.0f,	0.0f,	0.0f,	0.0f,
		0.0f,	cos,	sin,	0.0f,
		0.0f,	-sin,	cos,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};

	return result;
}

const Matrix4 &MatrixRotationY(float angle)
{
	float sin = std::sin(angle);
	float cos = std::cos(angle);

	static Matrix4 result
	{
		cos,	0.0f,	-sin,	0.0f,
		0.0f,	1.0f,	0.0f,	0.0f,
		sin,	0.0f,	cos,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};

	return result;
}

const Matrix4 &MatrixRotationZ(float angle)
{
	float sin = std::sin(angle);
	float cos = std::cos(angle);

	static Matrix4 result
	{
		cos,	sin,	0.0f,	0.0f,
		-sin,	cos,	0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};

	return result;
}

const Matrix4 &MatrixTranslation(float x, float y, float z)
{
	static Matrix4 result
	{
		1.0f,	0.0f,	0.0f,	0.0f,
		0.0f,	1.0f,	0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		x,		y,		z,		1.0f
	};

	return result;
}

const Matrix4 &MatrixLookAtLH(const Vector3& eye, const Vector3& target, const Vector3& up)
{
	//ZŽ²
	Vector3 vectorZ = target - eye;
	vectorZ = vectorZ.normalize();
	//XŽ²
	Vector3 vectorX = up.cross(vectorZ);
	vectorX = vectorX.normalize();
	//YŽ²
	Vector3 vectorY = vectorZ.cross(vectorX);

	static Matrix4 cameraViewMatrix
	{
		vectorX.x, vectorY.x, vectorZ.x,-eye.dot(vectorX),
		vectorX.y, vectorY.y, vectorZ.y,-eye.dot(vectorY),
		vectorX.z, vectorY.z, vectorZ.z,-eye.dot(vectorZ),
			  0.f,		 0.f,		0.f, 1.f
	};

	//static Matrix4 cameraViewMatrix
	//{
	//	vectorX.x, vectorX.y, vectorX.z, 0.f,
	//	vectorY.x, vectorY.y, vectorY.z, 0.f,
	//	vectorZ.x, vectorZ.y, vectorZ.z, 0.f,
	//	eye.dot(vectorX), eye.dot(vectorY), eye.dot(vectorZ), 1.f
	//};

	return cameraViewMatrix;
}

const Matrix4 &MatrixProjectionMatrix(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
{
	float scaleY = 1 / tan(FovAngleY / 2);
	float scaleX = 1 / tan(FovAngleY / 2) / AspectRatio;
	float scaleZ = 1 / (FarZ - NearZ) * FarZ;
	float TransZ = -NearZ / (FarZ - NearZ) * FarZ;

	static Matrix4 camaraProjection
	{
		scaleX,    0.f,    0.f, 0.f,
		   0.f, scaleY,    0.f, 0.f,
		   0.f,    0.f, scaleZ, 1.f,
		   0.f,    0.f, TransZ, 0.f
	};

	return camaraProjection;
}

const Vector3 &Vector3Transform(const Vector3 &v, const Matrix4 &m)
{
	float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];

	static Vector3 result
	{
		(v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0] + m.m[3][0])/w,
		(v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1] + m.m[3][1])/w,
		(v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2] + m.m[3][2])/w
	};

	return result;
}


const Matrix4 &operator*(const Matrix4& m1, const Matrix4& m2)
{
	Matrix4 temp(m1);

	return temp *= m2;
}

const Vector3 &operator*(const Vector3 &v, const Matrix4 &m)
{
	return Vector3Transform(v, m);
}
