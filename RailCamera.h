#pragma once
#include <DirectXMath.h>

#include "Vector3.h"
#include "Matrix4.h"


/// <summary>
/// ���[���J����
/// </summary>
class RailCamera
{
public:
	void Initialize(Vector3 pos, Vector3 rot);
	void Update(Vector3 pos, Vector3 rot);

private:
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();

	Matrix4 MatricChange(DirectX::XMMATRIX mat);

private:
	//�������e
	DirectX::XMMATRIX matProjection;	//�v���W�F�N�V�����s��
	//�r���[�ϊ��s��
	DirectX::XMMATRIX matView;		//�r���[�s��
	Vector3 eye;			//���_���W
	Vector3 target;		//�����_���W
	Vector3 up;			//������x�N�g��

	float nearZ;
	float farZ;

	DirectX::XMMATRIX matViewProjection;
};

