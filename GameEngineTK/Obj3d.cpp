#include "Obj3d.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// �ÓI�����o�֐��̎���
// �J����
Camera* Obj3d::m_pCamera;
// �ėp�X�e�[�g
std::unique_ptr<DirectX::CommonStates> Obj3d::m_states;
// �f�o�C�X
Microsoft::WRL::ComPtr<ID3D11Device>            Obj3d::m_d3dDevice;
// �R���e�L�X�g
Microsoft::WRL::ComPtr<ID3D11DeviceContext>     Obj3d::m_d3dContext;
// �G�t�F�N�g�t�@�N�g��
std::unique_ptr<DirectX::EffectFactory> Obj3d::m_factory;


void Obj3d::InitializeStatic(Camera * pCamera, Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dContext)
{
	m_pCamera = pCamera;
	m_d3dDevice = d3dDevice;
	m_d3dContext = d3dContext;

	// �X�e�[�g�̐ݒ�
	m_states = std::make_unique<CommonStates>(m_d3dDevice.Get());

	// �G�t�F�N�g�t�@�N�g������
	m_factory = std::make_unique<EffectFactory>(m_d3dDevice.Get());
	// �e�N�X�`���̓ǂݍ��݃t�H���_���w��
	m_factory->SetDirectory(L"Resources");
}

Obj3d::Obj3d()
{
	// �ϐ��̏�����
}

void Obj3d::LoadModel(const wchar_t * fileName)
{
}
