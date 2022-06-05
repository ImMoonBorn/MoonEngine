#include "mpch.h"
#include "Scene.h"
#include "Core/Window.h"
#include "Entity.h"
#include "Components.h"
#include "Renderer/Renderer.h"
#include "Renderer/Texture.h"
#include "Renderer/Frambuffer.h"
#include "glad/glad.h"
#include "Systems/ParticleSystem.h"

namespace MoonEngine
{
	Scene* Scene::m_ActiveScene;
	Ref<ParticleSystem> particleSystem;

	Scene::Scene()
	{
		m_ActiveScene = this;
		particleSystem = CreateRef<ParticleSystem>();
		DebugSys("Scene Created");
	}

	void Scene::OnPlay()
	{}

	void Scene::OnReset()
	{}

	void Scene::UpdateEditor(const EditorCamera* camera)
	{
		if (camera)
		{
			Renderer::Begin(camera->GetViewProjection());

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
			for (auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteComponent>(entity);
				Renderer::DrawQuad(transform.Position, transform.Rotation, transform.Size, sprite.Color, sprite.Texture);
			}

			Renderer::End();
		}
	}

	void Scene::UpdateRuntime()
	{
		m_Registry.view<Script>().each([=](auto entity, auto& nsc)
		{
			if (!nsc.Instance)
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity(entity);
				nsc.Instance->Awake();
			}
			nsc.Instance->Update();
		});

		Camera* sceneCamera = nullptr;
		glm::vec3 cameraPosition;

		auto cameras = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : cameras)
		{
			auto [transform, camera] = cameras.get<TransformComponent, CameraComponent>(entity);
			if (camera.isMain)
			{
				sceneCamera = &camera.Camera;
				cameraPosition = transform.Position;
				break;
			}
		}

		if (sceneCamera)
		{
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(cameraPosition.x, cameraPosition.y, 0.0f));
			glm::mat4 m_View = glm::inverse(transform);
			glm::mat4 viewProjection = sceneCamera->GetProjection() * m_View;

			Renderer::Begin(viewProjection);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
			for (auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteComponent>(entity);
				Renderer::DrawQuad(transform.Position, transform.Rotation, transform.Size, sprite.Color, sprite.Texture);
			}

			auto particleGroup = m_Registry.group<ParticleComponent>(entt::get<TransformComponent>);
			for (auto entity : particleGroup)
			{
				auto [particle, transform] = particleGroup.get<ParticleComponent, TransformComponent>(entity);
				for (int i = 0; i < particle.count; i++)
					particleSystem->Emit(particle, transform.Position);
			}

			particleSystem->Update();

			Renderer::End();
		}
	}

	void Scene::ResizeViewport(float width, float height)
	{
		auto camera = m_Registry.view<CameraComponent>();
		for (auto entity : camera)
		{
			CameraComponent& component = camera.get<CameraComponent>(entity);
			component.Camera.Resize(width, height, component.Distance);
		}
	}

	Entity Scene::CreateEntity()
	{
		Entity e{ m_Registry.create() };
		e.AddComponent<UUIDComponent>();
		e.AddComponent<IdentityComponent>();
		e.GetComponent<IdentityComponent>().Name = "Entity";
		e.AddComponent<TransformComponent>();
		e.AddComponent<SpriteComponent>();
		return e;
	}

	Scene::~Scene()
	{
		DebugSys("Scene Destroyed");
	}
}
