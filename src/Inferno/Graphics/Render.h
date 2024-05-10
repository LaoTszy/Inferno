#pragma once

#include "DeviceResources.h"
#include "ShaderLibrary.h"
#include "Heap.h"
#include "Camera.h"
#include "PostProcess.h"
#include "MaterialLibrary.h"
#include "LevelMesh.h"
#include "BitmapCache.h"
#include "Render.Canvas.h"

class CommandListManager;
class ContextManager;

namespace Inferno::Render {
    const DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Smart pointers in a namespace makes no sense as they will never trigger
    inline Ptr<DeviceResources> Adapter;
    inline Ptr<ShaderResources> Shaders;
    inline Ptr<EffectResources> Effects;
    inline Ptr<Inferno::PostFx::Bloom> Bloom;
    inline Ptr<Inferno::PostFx::ScanlineCS> Scanline;
    inline Ptr<DirectX::PrimitiveBatch<ObjectVertex>> g_SpriteBatch;
    inline Ptr<Canvas2D> Canvas, BriefingCanvas;

    inline bool DebugEmissive = false;
    //inline Ptr<TextureCache> NewTextureCache;

    inline D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSampler() {
        return Settings::Graphics.HighRes ? Heaps->States.AnisotropicWrap() : Heaps->States.PointWrap();
    }

    inline D3D12_GPU_DESCRIPTOR_HANDLE GetClampedTextureSampler() {
        return Settings::Graphics.HighRes ? Heaps->States.AnisotropicClamp() : Heaps->States.PointClamp();
    }

    void DrawVClip(ID3D12GraphicsCommandList* cmd, const VClip& vclip, const Vector3& position, float radius, const Color& color, float elapsed, bool additive = false, float rotation = 0, const Vector3* up = nullptr);

    void Initialize(HWND hwnd, int width, int height);
    void Resize(int width, int height);
    void Shutdown();
    void Present(float alpha);

    //void ReloadShaders();
    void ReloadTextures();

    void LoadModelDynamic(ModelID);
    void LoadTextureDynamic(TexID);
    void LoadTextureDynamic(LevelTexID);
    void LoadTextureDynamic(VClipID);
    void LoadLevel(Inferno::Level&);

    inline ID3D12Device* Device;

    // Camera needs to be swappable (pointer).
    // Editor could have multiple cameras (switch between first person and edit)
    // Also need to switch camera for drawing sub-views, like guided missiles or rear view.
    //void SetCamera(Inferno::Camera&);

    inline Inferno::Camera Camera;
    inline Matrix ViewProjection;

    inline uint16 DrawCalls = 0;
    inline uint16 PolygonCount = 0;
    inline float FrameTime = 0; // Time of this frame in seconds
    inline double ElapsedTime = 0; // Game time elapsed in seconds. Stops updating when paused or animations are disabled.

    enum class RenderCommandType {
        LevelMesh, Object
    };

    struct RenderCommand {
        float Depth; // Scene depth for sorting
        RenderCommandType Type;
        union Data {
            struct Object* Object;
            struct Inferno::LevelMesh* LevelMesh;
        } Data;

        RenderCommand(Object* obj, float depth)
            : Depth(depth), Type(RenderCommandType::Object) {
            Data.Object = obj;
        }

        RenderCommand(LevelMesh* mesh, float depth)
            : Depth(depth), Type(RenderCommandType::LevelMesh) {
            Data.LevelMesh = mesh;
        }
    };

    void DrawOpaque(RenderCommand);
    void DrawTransparent(RenderCommand);

    struct StaticTextureDef {
        Texture2D Font;
        Texture2D ImguiFont;
        Texture2D White, Black, Normal, Missing;
    };

    inline Ptr<StaticTextureDef> StaticTextures;
    extern bool LevelChanged;
}
