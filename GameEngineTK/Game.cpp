//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() :
    m_window(0),
    m_outputWidth(800),
    m_outputHeight(600),
    m_featureLevel(D3D_FEATURE_LEVEL_9_1)
{
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_window = window;
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateDevice();

    CreateResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */

	// キーボードの生成
	keyboard = std::make_unique<Keyboard>();

	// カメラの生成
	m_Camera = std::make_unique<FollowCamera>(
		m_outputWidth, m_outputHeight);
	// カメラにキーボードをセット
	m_Camera->SetKeyboard(keyboard.get());

	// 3Dオブジェクトクラスの静的メンバを初期化
	Obj3d::InitializeStatic(
		m_Camera.get(),
		m_d3dDevice,
		m_d3dContext);

	m_batch = std::make_unique<PrimitiveBatch<VertexPositionNormal>>(m_d3dContext.Get());

	m_effect = std::make_unique<BasicEffect>(m_d3dDevice.Get());

	m_effect->SetProjection(XMMatrixOrthographicOffCenterRH(0,
		m_outputWidth, m_outputHeight, 0, 0, 1));
	m_effect->SetVertexColorEnabled(true);

	void const* shaderByteCode;
	size_t byteCodeLength;

	m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	m_d3dDevice->CreateInputLayout(VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		shaderByteCode, byteCodeLength,
		m_inputLayout.GetAddressOf());
	// デバッグカメラの生成
	m_debugCamera = std::make_unique<DebugCamera>(m_outputWidth, m_outputHeight);

	// エフェクトファクトリ生成
	m_factory = std::make_unique<EffectFactory>(m_d3dDevice.Get());
	// テクスチャの読み込みフォルダを指定
	m_factory->SetDirectory(L"Resources");
	// モデルの読み込み
	m_objSkydome.LoadModel(L"Resources/skydome.cmo");
	
	m_modelGround = Model::CreateFromCMO(
		m_d3dDevice.Get()
		, L"Resources/ground200m.cmo",
		*m_factory
	);
	m_modelBall = Model::CreateFromCMO(
		m_d3dDevice.Get()
		, L"Resources/ball.cmo",
		*m_factory
	);
	//m_modelHead = Model::CreateFromCMO(
	//	m_d3dDevice.Get()
	//	, L"Resources/head.cmo",
	//	*m_factory
	//);

	m_AngleBall = 0.0f;

	tank_angle = 0.0f;

	// 自機パーツのロード
	m_ObjPlayer.resize(PLAYER_PARTS_NUM);
	m_ObjPlayer[PLAYER_PARTS_TOWER].LoadModel(L"Resources/tower.cmo");
	m_ObjPlayer[PLAYER_PARTS_BASE].LoadModel(L"Resources/base.cmo");
	m_ObjPlayer[PLAYER_PARTS_ENGINE_R].LoadModel(L"Resources/engine.cmo");
	m_ObjPlayer[PLAYER_PARTS_ENGINE_L].LoadModel(L"Resources/engine.cmo");
	m_ObjPlayer[PLAYER_PARTS_FAN].LoadModel(L"Resources/fan.cmo");
	m_ObjPlayer[PLAYER_PARTS_SCORE].LoadModel(L"Resources/score.cmo");
	// 親子関係の構築（子供に親をセット）
	m_ObjPlayer[PLAYER_PARTS_BASE].SetObjParent(
		&m_ObjPlayer[PLAYER_PARTS_TOWER]);

	m_ObjPlayer[PLAYER_PARTS_SCORE].SetObjParent(
		&m_ObjPlayer[PLAYER_PARTS_BASE]);

	m_ObjPlayer[PLAYER_PARTS_ENGINE_R].SetObjParent(
		&m_ObjPlayer[PLAYER_PARTS_TOWER]);
	m_ObjPlayer[PLAYER_PARTS_ENGINE_L].SetObjParent(
		&m_ObjPlayer[PLAYER_PARTS_TOWER]);

	m_ObjPlayer[PLAYER_PARTS_FAN].SetObjParent(
		&m_ObjPlayer[PLAYER_PARTS_TOWER]);

	// 親からのオフセット（ローカルの座標ずれ）
	m_ObjPlayer[PLAYER_PARTS_TOWER].SetScale(
		Vector3(2, 2, 2));

	m_ObjPlayer[PLAYER_PARTS_BASE].SetTranslation(
		Vector3(0, 0.7f, 0));

	m_ObjPlayer[PLAYER_PARTS_SCORE].SetTranslation(
		Vector3(0, 1.0f, 0));
	m_ObjPlayer[PLAYER_PARTS_SCORE].SetScale(
		Vector3(2, 2, 2));

	m_ObjPlayer[PLAYER_PARTS_ENGINE_R].SetTranslation(
		Vector3(0.22f, 0.3f, 0.22f));
	m_ObjPlayer[PLAYER_PARTS_ENGINE_R].SetRotation(
		Vector3(0, XMConvertToRadians(45), 0));

	m_ObjPlayer[PLAYER_PARTS_ENGINE_L].SetTranslation(
		Vector3(-0.22f, 0.3f, 0.22f));
	m_ObjPlayer[PLAYER_PARTS_ENGINE_L].SetRotation(
		Vector3(0, XMConvertToRadians(-45), 0));

	m_ObjPlayer[PLAYER_PARTS_FAN].SetTranslation(
		Vector3(0, 0.3f, 1.0f));

	m_sinAngle = 0.0f;
}

// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;
	// ゲームの毎フレーム処理
	// デバッグカメラの更新
	m_debugCamera->Update();

	// 角度を加算
	m_AngleBall += 1.0f;
	
	// 球のワールド行列を計算
	for (int i = 0; i < 10; i++)
	{
		// スケーリング
		Matrix scalemat = Matrix::CreateScale(1.0f);
		// ロール
		Matrix rotmatZ = Matrix::CreateRotationZ(XMConvertToRadians(0.0f));
		// ピッチ（仰角）
		Matrix rotmatX = Matrix::CreateRotationX(XMConvertToRadians(0.0f));
		// ヨー（方位角）
		Matrix rotmatY = Matrix::CreateRotationY(XMConvertToRadians(360.0f/10.0f*i+ m_AngleBall));
		// 回転行列の合成
		Matrix rotmat = rotmatZ * rotmatX * rotmatY;
		// 平行移動
		Matrix transmat = Matrix::CreateTranslation(20, 0, 0);
		// ワールド行列の合成
		m_worldBall[i] = rotmat * transmat;
	}

	for (int i = 0; i < 10; i++)
	{
		// スケーリング
		Matrix scalemat = Matrix::CreateScale(1.0f);
		// ロール
		Matrix rotmatZ = Matrix::CreateRotationZ(XMConvertToRadians(0.0f));
		// ピッチ（仰角）
		Matrix rotmatX = Matrix::CreateRotationX(XMConvertToRadians(0.0f));
		// ヨー（方位角）
		Matrix rotmatY = Matrix::CreateRotationY(XMConvertToRadians(360.0f / 10.0f*i- m_AngleBall));
		// 回転行列の合成
		Matrix rotmat = rotmatZ * rotmatX * rotmatY;
		// 平行移動
		Matrix transmat = Matrix::CreateTranslation(40, 0, 0);
		// ワールド行列の合成
		m_worldBall[10+i] = rotmat * transmat;
	}

	// 自機パーツのギミック
	{
		// 自機の角度を変動
		Vector3 angle;

		angle = m_ObjPlayer[PLAYER_PARTS_SCORE].GetRotation();
		m_ObjPlayer[PLAYER_PARTS_SCORE].SetRotation(
			angle + Vector3(0.2f,0.1f,0));

		// サインの引数の角度がだんだん増える
		m_sinAngle += 0.1f;
		// ファンの角度がいったりきたりする(-1〜+1)
		float fan_angle = sinf(m_sinAngle) * 3.0f;
		/*m_ObjPlayer[PLAYER_PARTS_FAN].SetRotation(
			Vector3(0, 0, fan_angle));*/
		m_ObjPlayer[PLAYER_PARTS_FAN].SetTranslation(
			Vector3(sinf(m_sinAngle), 0, cosf(m_sinAngle)*3.0f));
	}

	// キーボードの状態を取得
	Keyboard::State g_key = keyboard->GetState();
	
	// Aキーを押している間
	if (g_key.A)
	{		
		// 自機の角度を変動
		float angle = m_ObjPlayer[0].GetRotation().y;
		m_ObjPlayer[0].SetRotation(
			Vector3(0,angle + 0.03f,0));
	}

	// Dキーを押している間
	if (g_key.D)
	{
		// 自機の角度を変動
		float angle = m_ObjPlayer[0].GetRotation().y;
		m_ObjPlayer[0].SetRotation(
			Vector3(0, angle - 0.03f, 0));
	}

	// Wキーを押している間
	if (g_key.W)
	{
		// 移動ベクトル
		Vector3 moveV(0, 0, -0.1f);
		// 今の角度に合わせて移動ベクトルを回転
		// 回転行列
		float angle = m_ObjPlayer[0].GetRotation().y;
		Matrix rotmat = Matrix::CreateRotationY(angle);
		moveV = Vector3::TransformNormal(moveV, rotmat);
		// 自機の座標を移動
		Vector3 pos = m_ObjPlayer[0].GetTranslation();
		pos += moveV;
		m_ObjPlayer[0].SetTranslation(pos);
	}

	// Sキーを押している間
	if (g_key.S)
	{
		// 移動ベクトル
		Vector3 moveV(0, 0, +0.1f);
		// 回転行列
		float angle = m_ObjPlayer[0].GetRotation().y;
		Matrix rotmat = Matrix::CreateRotationY(angle);
		moveV = Vector3::TransformNormal(moveV, rotmat);
		// 自機の座標を移動
		Vector3 pos = m_ObjPlayer[0].GetTranslation();
		pos += moveV;
		m_ObjPlayer[0].SetTranslation(pos);
	}

	//{// 自機のワールド行列を計算
	//	// 回転行列
	//	Matrix rotmat = Matrix::CreateRotationY(tank_angle);
	//	// 平行移動行列
	//	Matrix transmat = Matrix::CreateTranslation(tank_pos);
	//	// ワールド行列を合成
	//	tank_world = rotmat * transmat;

	//	// 自機のパーツ２を計算
	//	// 回転行列(パーツ１からの回転分）
	//	static float angle = 0.0f;
	//	angle += 0.1f;
	//	Matrix rotmat2 = Matrix::CreateRotationZ(XM_PIDIV2) * Matrix::CreateRotationY(angle);
	//	// 平行移動行列(パーツ１からの平行移動分）
	//	Matrix transmat2 = Matrix::CreateTranslation(Vector3(0,0.5f,0));
	//	// ワールド行列を合成
	//	tank2_world = rotmat2 * transmat2 * tank_world;
	//}

	{// 追従カメラ
		m_Camera->SetTargetPos(tank_pos);
		m_Camera->SetTargetAngle(tank_angle);

		m_Camera->Update();
		m_view = m_Camera->GetView();
		m_proj = m_Camera->GetProj();
	}

	m_objSkydome.Update();

	for (std::vector<Obj3d>::iterator it = m_ObjPlayer.begin();
		it != m_ObjPlayer.end();
		it++)
	{
		it->Update();
	}

}

// Draws the scene.
void Game::Render()
{
	// 頂点インデックス配列
	uint16_t indices[] =
	{
		0,1,2,
		2,1,3
	};

	// 頂点データ配列
	VertexPositionNormal vertices[] =
	{
		{ Vector3(-1,+1,0), Vector3(0,0,1) },
		{ Vector3(-1,-1,0), Vector3(0,0,1) },
		{ Vector3(+1,+1,0), Vector3(0,0,1) },
		{ Vector3(+1,-1,0), Vector3(0,0,1) }
	};

    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    // TODO: Add your rendering code here.
	// 描画はここに書く。
	m_states = std::make_unique<CommonStates>(m_d3dDevice.Get());
	m_d3dContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	m_d3dContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
	m_d3dContext->RSSetState(m_states->Wireframe());

	//m_view = Matrix::CreateLookAt(Vector3(0, 2.f, 2.f),
	//	Vector3(0,0,0), Vector3(0,1,0));
	// デバッグカメラからビュー行列を取得
	//m_view = m_debugCamera->GetCameraMatrix();
	//// カメラの位置（視点座標）
	//Vector3 eyepos(0,0,5.0f);
	//// どこのみるのか（注視点/参照点)
	//Vector3 refpos(0, 0, 0);
	//// 上方向ベクトル
	//Vector3 upvec(1, -1, 0);
	//upvec.Normalize();
	//// ビュー行列を生成
	//m_view = Matrix::CreateLookAt(eyepos, refpos, upvec);
	// 垂直方向視野角
	//float fovY = XMConvertToRadians(60.0f);
	//// 画面横幅と縦幅の比率
	//float aspect = (float)m_outputWidth / m_outputHeight;
	//// 手前の表示限界距離
	//float nearclip = 0.1f;
	//// 奥の表示限界距離
	//float farclip = 1000.0f;
	//// 射影行列を生成
	//m_proj = Matrix::CreatePerspectiveFieldOfView(
	//fovY, aspect, nearclip, farclip);

	m_effect->SetView(m_view);
	m_effect->SetProjection(m_proj);

	m_effect->Apply(m_d3dContext.Get());
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());

	// 天球を描画
	m_objSkydome.Draw();
	// 地面を描画
	m_modelGround->Draw(m_d3dContext.Get(),
		*m_states,
		Matrix::Identity,
		m_view,
		m_proj);
	// 球を描画
	for (int i = 0; i < 20; i++)
	{
		m_modelBall->Draw(m_d3dContext.Get(),
			*m_states,
			m_worldBall[i],
			m_view,
			m_proj);
	}

	//// パーツ１を描画
	//m_modelHead->Draw(m_d3dContext.Get(),
	//	*m_states,
	//	tank_world,
	//	m_view,
	//	m_proj);

	//// パーツ２を描画
	//m_modelHead->Draw(m_d3dContext.Get(),
	//	*m_states,
	//	tank2_world,
	//	m_view,
	//	m_proj);

	for (std::vector<Obj3d>::iterator it = m_ObjPlayer.begin();
		it != m_ObjPlayer.end();
		it++)
	{
		it->Draw();
	}

	m_batch->Begin();

	m_batch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, indices, 6, vertices, 4);

	m_batch->End();

    Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL featureLevels [] =
    {
        // TODO: Modify for supported Direct3D feature levels (see code below related to 11.1 fallback handling).
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    HRESULT hr = D3D11CreateDevice(
        nullptr,                                // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        m_d3dDevice.ReleaseAndGetAddressOf(),   // returns the Direct3D device created
        &m_featureLevel,                        // returns feature level of device created
        m_d3dContext.ReleaseAndGetAddressOf()   // returns the device immediate context
        );

    if (hr == E_INVALIDARG)
    {
        // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it.
        hr = D3D11CreateDevice(nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            &featureLevels[1],
            _countof(featureLevels) - 1,
            D3D11_SDK_VERSION,
            m_d3dDevice.ReleaseAndGetAddressOf(),
            &m_featureLevel,
            m_d3dContext.ReleaseAndGetAddressOf()
            );
    }

    DX::ThrowIfFailed(hr);

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(m_d3dDevice.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide [] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    // DirectX 11.1 if present
    if (SUCCEEDED(m_d3dDevice.As(&m_d3dDevice1)))
        (void)m_d3dContext.As(&m_d3dContext1);

    // TODO: Initialize device dependent objects here (independent of window size).
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory1> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        ComPtr<IDXGIFactory2> dxgiFactory2;
        if (SUCCEEDED(dxgiFactory.As(&dxgiFactory2)))
        {
            // DirectX 11.1 or later

            // Create a descriptor for the swap chain.
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
            swapChainDesc.Width = backBufferWidth;
            swapChainDesc.Height = backBufferHeight;
            swapChainDesc.Format = backBufferFormat;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = backBufferCount;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
            fsSwapChainDesc.Windowed = TRUE;

            // Create a SwapChain from a Win32 window.
            DX::ThrowIfFailed(dxgiFactory2->CreateSwapChainForHwnd(
                m_d3dDevice.Get(),
                m_window,
                &swapChainDesc,
                &fsSwapChainDesc,
                nullptr,
                m_swapChain1.ReleaseAndGetAddressOf()
                ));

            DX::ThrowIfFailed(m_swapChain1.As(&m_swapChain));
        }
        else
        {
            DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
            swapChainDesc.BufferCount = backBufferCount;
            swapChainDesc.BufferDesc.Width = backBufferWidth;
            swapChainDesc.BufferDesc.Height = backBufferHeight;
            swapChainDesc.BufferDesc.Format = backBufferFormat;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.OutputWindow = m_window;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Windowed = TRUE;

            DX::ThrowIfFailed(dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &swapChainDesc, m_swapChain.ReleaseAndGetAddressOf()));
        }

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain1.Reset();
    m_swapChain.Reset();
    m_d3dContext1.Reset();
    m_d3dContext.Reset();
    m_d3dDevice1.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}