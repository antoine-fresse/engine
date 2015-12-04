#pragma once
#include <imgui/imgui.h>
#include <chrono>

namespace sf{
	class RenderWindow;
}

IMGUI_API bool        ImGui_Impl_Init(sf::RenderWindow* window);
IMGUI_API void        ImGui_Impl_Shutdown();
IMGUI_API void		  ImGui_Impl_NewFrame(sf::RenderWindow* target, std::chrono::duration<double> dt);

IMGUI_API void        ImGui_Impl_InvalidateDeviceObjects();
IMGUI_API bool        ImGui_Impl_CreateDeviceObjects();

IMGUI_API void        ImGui_Impl_MouseButtonCallback(int button, bool action);
IMGUI_API void        ImGui_Impl_ScrollCallback(double xoffset, double yoffset);
IMGUI_API void        ImGui_Impl_KeyCallback(int key, int scancode, int action, int mods);
IMGUI_API void        ImGui_Impl_CharCallback(unsigned int c);