#pragma once

namespace OGL
{
	class TextureManager;
	class ShaderManager;
	class FrameBuffer;
	struct TimerValues;
	struct SceneOptions;

	class PostEffectManager
	{
	public:
		PostEffectManager();
		~PostEffectManager();
		void init(TextureManager& tm, u32 finalTextureObj);
		void renderAll(ShaderManager& sm, TextureManager& tm, FrameBuffer& fbo, TimerValues& timerValues, SceneOptions& sceneOptions);

		u32 fxFbo;
		u32 fxDrawBuffers[1];
		static const u32 FX_TEXTURE_COUNT = 4;
		u32 fxTexturesId[FX_TEXTURE_COUNT];
	};

}