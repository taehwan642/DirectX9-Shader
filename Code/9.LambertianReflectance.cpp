#include "DXUT.h"
#include "Draw.h"
#include <string>

namespace LambertianReflectance
{
	/*조명 계산을 월드 좌표계에서*/

	LPD3DXEFFECT effect;
	D3DXHANDLE technique;
	D3DXHANDLE mWVP;
	D3DXHANDLE mWIT;
	D3DXHANDLE lightDir;

	LPD3DXMESH background;
	LPD3DXMESH ufo;

	LPD3DXBUFFER subsetBuffer;
	LPD3DXMATERIAL subset;
	LPDIRECT3DTEXTURE9 texture;
	DWORD numMaterial;
	std::wstring filepath = L"Source/";
	std::wstring filename = L"map.x";
	std::wstring filename2 = L"ufo.x";

	HRESULT Initialize(LPDIRECT3DDEVICE9 device)
	{
		WCHAR finalpath[MAX_PATH];

		::lstrcpy(finalpath, filepath.c_str());
		::lstrcat(finalpath, filename.c_str());

		if (FAILED(D3DXLoadMeshFromX(finalpath, D3DXMESH_MANAGED, device, nullptr, &subsetBuffer, nullptr, nullptr, &background)))
			return E_FAIL;

		subset = static_cast<LPD3DXMATERIAL>(subsetBuffer->GetBufferPointer());

		WCHAR _filename[MAX_PATH] = L"";
		::lstrcpy(finalpath, filepath.c_str());
		MultiByteToWideChar(CP_ACP, 0, subset[0].pTextureFilename, strlen(subset[0].pTextureFilename), _filename, MAX_PATH);
		::lstrcat(finalpath, _filename);

		if (FAILED(D3DXCreateTextureFromFile(device, finalpath, &texture)))
			return E_FAIL;

		::lstrcpy(finalpath, filepath.c_str());
		::lstrcat(finalpath, filename2.c_str());

		if (FAILED(D3DXLoadMeshFromX(finalpath, D3DXMESH_MANAGED, device, nullptr, &subsetBuffer, nullptr, &numMaterial, &ufo)))
			return E_FAIL;

		LPD3DXBUFFER error;
		if (FAILED(D3DXCreateEffectFromFile(
			device, // 디바이스
			L"Shader/LambertianReflectance.fx", // 이펙트 파일 명 포인터
			NULL, // 전처리기 포인터
			NULL, // 옵션 인터페이스 포인터
			0, // D3DXSHADER 식별 컴파일 옵션
			NULL, // 공유 인수로 사용하는 ID3DXEffectPool 오브젝트 포인터
			&effect, // 컴파일된 이펙트 파일이 저장될 버퍼
			&error))) // 컴파일 에러가 저장될 버퍼
		{
			MessageBox(NULL, (LPCTSTR)error->GetBufferPointer(), L"ERROR", MB_OK);
		}
		else
		{
			technique = effect->GetTechniqueByName("TShader");
			mWVP = effect->GetParameterByName(NULL, "mWVP");
			mWIT = effect->GetParameterByName(NULL, "mWIT");
			lightDir = effect->GetParameterByName(NULL, "vLightDir");
		}

		if (error != nullptr) error->Release();

		device->SetRenderState(D3DRS_LIGHTING, TRUE);

		return S_OK;
	}

	void SetMatrices(LPDIRECT3DDEVICE9 device)
	{
		D3DXMATRIX mWorld, mView, mProj;
		D3DXMatrixIdentity(&mWorld);
		device->SetTransform(D3DTS_WORLD, &mWorld);

		D3DXVECTOR3 eye{ 0.f,3.f,-8.f };
		D3DXVECTOR3 lookat{ 0.f,-0.f,0.f };
		D3DXVECTOR3 up{ 0.f,1.f,0.f };
		D3DXMatrixLookAtLH(&mView, &eye, &lookat, &up);
		device->SetTransform(D3DTS_VIEW, &mView);

		D3DXMatrixPerspectiveFovLH(&mProj,
			D3DX_PI / 4, // 시야각
			1280 / 720,  // 화면 비율
			1.f,		 // 전방 클립면
			100.f);		 // 후방 클립면

		device->SetTransform(D3DTS_PROJECTION, &mProj);
	}

	void DrawBackGround(LPDIRECT3DDEVICE9 device)
	{
		device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

		device->SetTexture(0, texture);
		background->DrawSubset(0);
	}

	void DrawUFO(LPDIRECT3DDEVICE9 device)
	{
		device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

		D3DXMATRIX mWorld, mView, mProj;

		D3DXVECTOR3 eye{ 0.f,3.f,-8.f };
		D3DXVECTOR3 lookat{ 0.f,-0.f,0.f };
		D3DXVECTOR3 up{ 0.f,1.f,0.f };
		D3DXMatrixLookAtLH(&mView, &eye, &lookat, &up);
		D3DXMatrixPerspectiveFovLH(&mProj,
			D3DX_PI / 4, // 시야각
			1280 / 720,  // 화면 비율
			1.f,		 // 전방 클립면
			100.f);		 // 후방 클립면


		D3DXMATRIX mTrans, mRot, m;
		D3DXMatrixTranslation(&m, 1.0f, 0.0f, 0.0f);
		static float delta = 0;
		delta += DXUTGetElapsedTime();
		D3DXMatrixRotationY(&mRot, delta);
		D3DXMatrixTranslation(&mTrans, 0.0f, 2.2f, -4.0f);
		mWorld = m * mRot * mTrans;

		D3DXMatrixInverse(&m, NULL, &mWorld);
		D3DXMatrixTranspose(&m, &m);
		effect->SetMatrix(mWIT, &m);

		m = mWorld * mView * mProj;
		effect->SetMatrix(mWVP, &m);

		D3DXVECTOR4 light_pos{ -0.577f, -0.577f, -0.577f, 0 };
		effect->SetVector(lightDir, &light_pos);
		
		device->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);

		effect->SetTechnique(technique);

		effect->Begin(
			NULL, // 셰이더 파일에서 최대 pass 수를 읽는다
			0); // 옵션
		effect->BeginPass(0);

		for (int i = 0; i < numMaterial; ++i)
		{
			D3DXVECTOR4 v;
			v.x = subset[i].MatD3D.Diffuse.r;
			v.y = subset[i].MatD3D.Diffuse.g;
			v.z = subset[i].MatD3D.Diffuse.b;
			v.w = subset[i].MatD3D.Diffuse.a;

			effect->SetVector((D3DXHANDLE)"k_d", &v);
			effect->SetVector((D3DXHANDLE)"k_a", &v);
			ufo->DrawSubset(i);
		}

		effect->EndPass();
		effect->End();
	}

	void Update(LPDIRECT3DDEVICE9 device)
	{
		SetMatrices(device);
		DrawBackGround(device);
		DrawUFO(device);
	}

	void Close()
	{
		effect->Release();
		texture->Release();
		background->Release();
		ufo->Release();
	}
}