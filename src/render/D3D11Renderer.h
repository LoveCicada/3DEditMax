#pragma once
#include "core/StateSnapshot.h"
#include "render/FeedbackQueue.h"
#include "render/DebugDraw.h"
#include "render/MeshGpu.h"
#include "render/ShaderSet.h"
#include <d3d11.h>
#include <d3d11sdklayers.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <string>

class D3D11Renderer {
public:
  explicit D3D11Renderer(const std::wstring& shaderDir);
  bool initialize(HWND hwnd, int w, int h, bool wantDebug, FeedbackQueue* fb);
  void shutdown();
  bool resize(int w, int h);
  void render(const StateSnapshot& snap);
  bool reloadShaders(FeedbackQueue* fb);
  std::string adapterNameUtf8() const;
  bool dead() const { return m_dead; }
  bool initialized() const { return m_initialized; }
  bool viewsValid() const { return m_rtv && m_dsv; }
private:
  bool createSizeDependentResources();
  bool createLabStates(FeedbackQueue* fb);
  bool createEdgeBuffers(FeedbackQueue* fb);
  void bindLabStates(const StateSnapshot& snap);
  void bindMeshPipeline(const StateSnapshot& snap);
  void drawAxisCones(const StateSnapshot& snap,
                     DirectX::FXMMATRIX view,
                     DirectX::FXMMATRIX proj,
                     DirectX::FXMVECTOR origin,
                     float axisLen);
  void handlePresentResult(HRESULT hr);
  void copyVariantName(const char* name);
  void copyTriedName(const char* name);
  void attachInfoQueue();
  void pumpInfoQueue();
  void selectMultisampleLevel();
  std::wstring m_shaderDir;
  HWND m_hwnd;
  int m_w;
  int m_h;
  bool m_wantDebug;
  bool m_initialized;
  bool m_dead;
  FeedbackQueue* m_fb;
  Microsoft::WRL::ComPtr<IDXGIFactory1> m_factory;
  Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
  Microsoft::WRL::ComPtr<ID3D11Device> m_device;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
  Microsoft::WRL::ComPtr<IDXGISwapChain> m_swap;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depth;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_raster[2][3];
  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthOn;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthOff;
  Microsoft::WRL::ComPtr<ID3D11BlendState> m_blend;
  char m_shaderVariant[32];
  char m_triedVariant[32];
  Microsoft::WRL::ComPtr<ID3D11InfoQueue> m_infoQueue;
  float m_lastLoggedShadeX;
  bool m_loggedCb;
  UINT m_sampleCount;
  UINT m_sampleQuality;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_cb;
  MeshGpu m_cube;
  MeshGpu m_sphere;
  MeshGpu m_cyl;
  MeshGpu m_cone;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_cubeEdges;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_sphereEdges;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_cylEdges;
  UINT m_cubeEdgeVerts;
  UINT m_sphereEdgeVerts;
  UINT m_cylEdgeVerts;
  ShaderSet m_shaders;
  DebugDraw m_debug;
};
