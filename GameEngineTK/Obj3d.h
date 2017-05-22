/// <summary>
/// �R�c�I�u�W�F�N�g�̃N���X
/// </summary>
#pragma once

#include <memory>
#include <windows.h>
#include <wrl/client.h>
#include <Effects.h>
#include <CommonStates.h>
#include <SimpleMath.h>
#include <Model.h>

#include "Camera.h"

class Obj3d
{
	// �ÓI�����o
public:
	// �ÓI�����o�̏�����
	static void InitializeStatic(Camera* pCamera,
		Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dContext);

private:
	// �J����
	static Camera* m_pCamera;
	// �f�o�C�X
	static Microsoft::WRL::ComPtr<ID3D11Device>            m_d3dDevice;
	// �R���e�L�X�g
	static Microsoft::WRL::ComPtr<ID3D11DeviceContext>     m_d3dContext;
	// �ėp�X�e�[�g
	static std::unique_ptr<DirectX::CommonStates> m_states;
	// �G�t�F�N�g�t�@�N�g��
	static std::unique_ptr<DirectX::EffectFactory> m_factory;

public:
	// �R���X�g���N�^
	Obj3d();

	// ���f���̓ǂݍ���
	void LoadModel(const wchar_t* fileName);

	// setter
	// �X�P�[�����O�p

	// ��]�p�p

	// ���s�ړ��p

	// getter
	// �X�P�[�����O�p

	// ��]�p�p

	// ���s�ړ��p

	// ���[���h�s����擾

private:
	// ���f���̃��j�[�N�|�C���^
	
	// �X�P�[�����O

	// ��]�p

	// ���s�ړ�

	// ���[���h�s��

	// �e�ƂȂ�R�c�I�u�W�F�N�g�̃N���X�̃|�C���^
	Obj3d* m_pObjParent;
};