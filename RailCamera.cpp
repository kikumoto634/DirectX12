#include "RailCamera.h"
#include "WinApp.h"

using namespace DirectX;

void RailCamera::Initialize(Vector3 pos, Vector3 rot)
{
	eye = {0,0,0};
	target = {0,0,0};
	up = {1,1,1};
	
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void RailCamera::Update(Vector3 pos, Vector3 rot)
{
	eye = Vector3Transform(pos, MatricChange(matView));
	//���[���h�O��vector
	Vector3 forward(0,0,1);
	//���[���J������]���f
	forward = {forward.x*cosf(rot.z)-forward.y*sinf(rot.z), forward.x*sinf(rot.z)+forward.y*cosf(rot.z), forward.z};
	//���_����O���ɓK���ȋ����i�񂾈ʒu�������_
	target = eye + forward;
	//���[���h����x�N�g��
	Vector3 lup(0,1,0);
	//���[���J�����̉�]�𔽉f
	up = {lup.x*cosf(rot.y)+lup.z*sinf(rot.y), lup.y, -lup.x*sinf(rot.y)+lup.z*cosf(rot.y)};
	//�r���[�v���W�F�N�V�������X�V
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void RailCamera::UpdateViewMatrix()
{
	XMFLOAT3 keye{eye.x, eye.y, eye.z};
	XMFLOAT3 ktarget{target.x, target.y, target.z};
	XMFLOAT3 kup{up.x, up.y, up.z};
	matView = XMMatrixLookAtLH(XMLoadFloat3(&keye), XMLoadFloat3(&ktarget), XMLoadFloat3(&kup));
}

void RailCamera::UpdateProjectionMatrix()
{
	matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),	//�㉺��p45��
		(float)WinApp::window_width / WinApp::window_height,			//aspect��(��ʉ���/��ʏc��)
		nearZ, farZ				//�O�[�A���[
	);
}

Matrix4 RailCamera::MatricChange(DirectX::XMMATRIX mat)
{
	Matrix4 result = {
		mat.r[0].m128_f32[0], mat.r[0].m128_f32[1], mat.r[0].m128_f32[2], mat.r[0].m128_f32[3],
		mat.r[1].m128_f32[0], mat.r[1].m128_f32[1], mat.r[1].m128_f32[2], mat.r[1].m128_f32[3],
		mat.r[2].m128_f32[0], mat.r[2].m128_f32[1], mat.r[2].m128_f32[2], mat.r[2].m128_f32[3],
		mat.r[3].m128_f32[0], mat.r[3].m128_f32[1], mat.r[3].m128_f32[2], mat.r[3].m128_f32[3],
	};

	return result;
}
