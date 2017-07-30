#include "stdafx.h"
#include "Gui.h"
#include "Util.h"

namespace OGL
{
	ProfileType s_profileType = CPU_PROFILE;

	void renderGui()
	{
		ImGui::End();
		ImGui::Render();
	}

	void initGui()
	{
		ImGui_ImplGlfwGL3_NewFrame();
		//Prepare next frame
		ImGui::SetNextWindowPos(ImVec2(0, 0), 0);
		ImGui::SetNextWindowSize(ImVec2(640.f / 2.5f, 480.f / 2.f), 0);
		static bool no_titlebar = true;
		static bool no_border = true;
		static bool no_resize = true;
		static bool no_move = true;
		static bool no_scrollbar = false;
		static bool no_collapse = true;
		static bool no_menu = true;
		static f32 bg_alpha = 0.25f; // <0: default
									  // Demonstrate the various window flags. Typically you would just use the default.
		ImGuiWindowFlags window_flags = 0;
		if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
		if (!no_border)   window_flags |= ImGuiWindowFlags_ShowBorders;
		if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
		if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
		if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
		if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
		if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;

		bool showImgui = true;
		if (!ImGui::Begin("ImGui Demo", &showImgui, ImVec2(550, 30), bg_alpha, window_flags))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		ImGui::PushItemWidth(-140); // Right align, keep 140 pixels for labels
	}

	void drawSlider(const char* label, f32* value, f32 min, f32 max)
	{
		ImGui::SliderFloat(label, value, min, max);
	}

	void drawSlider(const char* label, s32* value, s32 min, s32 max)
	{
		ImGui::SliderInt(label, value, min, max);
	}

	void drawLabel(const char* label, const char* value)
	{
		ImGui::LabelText(label, value);
	}
}