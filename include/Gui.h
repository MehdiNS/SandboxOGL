#pragma once

namespace OGL
{
		// Mainly helper functions here
		template <typename T>
		void drawLabel(const char* label, T value)
		{
			string temp = std::to_string(value);
			ImGui::LabelText(label, temp.c_str());
		}

		void initGui();
		void drawLabel(const char* label, const char* value);
		void drawSlider(const char* label, f32* value, f32 min, f32 max);
		void drawSlider(const char* label, s32* value, s32 min, s32 max);
		void renderGui();

		enum
		{
			CPU_PROFILE = 0,
			GPU_PROFILE
		};
		using ProfileType = int;
		extern ProfileType s_profileType;
}