#pragma once
#include "entt/entt.hpp"
#include "Core.h"
#include "Utils/EditorCamera.h"

namespace MoonEngine
{
	class Entity;
	class Framebuffer;
	
	class Scene
	{
	private:
		static Scene* m_ActiveScene;
		friend class Entity;
		friend class HierarchyView;
		friend class Serializer;
		entt::registry m_Registry;
	public:
		Scene();
		~Scene();
		void OnPlay();
		void OnReset();
		void UpdateRuntime();
		void UpdateEditor(const EditorCamera* camera);
		void ResizeViewport(float width, float height);
		Entity CreateEntity();
	};
}