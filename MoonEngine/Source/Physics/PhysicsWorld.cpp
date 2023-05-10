#include "mpch.h"
#include "Physics/PhysicsWorld.h"

#include "Engine/Entity.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>

namespace MoonEngine
{
	struct RegisterGroup
	{
		Entity e;
		const TransformComponent& t;
		PhysicsBodyComponent& pb;

		RegisterGroup(Entity e, const TransformComponent& t, PhysicsBodyComponent& pb) :e(e), t(t), pb(pb) {}
	};

	struct UnregisterGroup
	{
		Entity e;
		PhysicsBodyComponent& pb;

		UnregisterGroup(Entity e, PhysicsBodyComponent& pb) :e(e), pb(pb) {}
	};

	std::vector<RegisterGroup> m_AddRegistry;
	std::vector<PhysicsBodyComponent> m_RemoveRegistry;

	float PhysicsWorld::Gravity = -9.8f;

	static b2BodyType ConvertBodyType(PhysicsBodyComponent::BodyType type)
	{
		switch (type)
		{
			case MoonEngine::PhysicsBodyComponent::BodyType::Static: return b2_staticBody;
			case MoonEngine::PhysicsBodyComponent::BodyType::Dynamic: return b2_dynamicBody;
			case MoonEngine::PhysicsBodyComponent::BodyType::Kinematic: return b2_kinematicBody;
		}

		ME_ASSERT(false, "Invalid body type!");
		return b2_staticBody;
	}

	void PhysicsWorld::BeginWorld()
	{
		m_PhysicsWorld = new b2World({ 0.0f, Gravity });
	}

	void PhysicsWorld::RegisterPhysicsBody(Entity entity, const TransformComponent& transform, PhysicsBodyComponent& pb, bool toRegistry)
	{
		if (toRegistry)
		{
			RegisterGroup rg(entity, transform, pb);
			m_AddRegistry.push_back(rg);
			return;
		}

		b2BodyDef bodyDef;
		bodyDef.type = ConvertBodyType(pb.Type);
		bodyDef.position.Set(transform.Position.x + pb.Offset.x, transform.Position.y + pb.Offset.y);
		bodyDef.angle = transform.Rotation.z;

		b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
		body->SetFixedRotation(pb.FreezeRotation);

		pb.RuntimeBody = body;

		b2PolygonShape boxShape;
		boxShape.SetAsBox(transform.Scale.x * pb.Size.x, transform.Scale.y * pb.Size.y);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &boxShape;
		fixtureDef.density = pb.Density;
		fixtureDef.friction = pb.Friction;
		fixtureDef.restitution = pb.Restitution;
		fixtureDef.restitutionThreshold = pb.RestitutionThreshold;
		body->CreateFixture(&fixtureDef);
		body->ResetMassData();
	}

	void PhysicsWorld::UnregisterPhysicsBody(PhysicsBodyComponent& pb, bool toRegistry)
	{
		if (toRegistry)
		{
			m_RemoveRegistry.push_back(pb);
			return;
		}

		b2Body* node = (b2Body*)pb.RuntimeBody;
		m_PhysicsWorld->DestroyBody(node);
		pb.RuntimeBody = nullptr;
	}

	void PhysicsWorld::UpdatePhysicsBodies(Entity e, TransformComponent& transform, const PhysicsBodyComponent& physicsBody)
	{
		auto* body = (b2Body*)physicsBody.RuntimeBody;

		//Update Body Properties
		body->SetType(ConvertBodyType(physicsBody.Type));

		auto* fixture = body->GetFixtureList();

		b2PolygonShape* boxShape = (b2PolygonShape*)fixture->GetShape();
		boxShape->SetAsBox(transform.Scale.x * physicsBody.Size.x, transform.Scale.y * physicsBody.Size.y);

		fixture->SetDensity(physicsBody.Density);
		fixture->SetFriction(physicsBody.Friction);
		fixture->SetRestitution(physicsBody.Restitution);
		fixture->SetRestitutionThreshold(physicsBody.RestitutionThreshold);
		body->ResetMassData();

		const auto& position = body->GetPosition();
		transform.Position.x = position.x - physicsBody.Offset.x;
		transform.Position.y = position.y - physicsBody.Offset.y;
		transform.Rotation.z = body->GetAngle();
	}

	void PhysicsWorld::StepWorld(float dt)
	{
		if (m_AddRegistry.size() > 0)
		{
			for (int i = 0; i < m_AddRegistry.size(); i++)
			{
				const RegisterGroup& rg = m_AddRegistry[i];
				RegisterPhysicsBody(rg.e, rg.t, rg.pb);
			}
			m_AddRegistry.clear();
		}

		if (m_RemoveRegistry.size() > 0)
		{
			for (int i = 0; i < m_RemoveRegistry.size(); i++)
				UnregisterPhysicsBody(m_RemoveRegistry[i]);
			m_RemoveRegistry.clear();
		}

		m_PhysicsWorld->Step(dt, m_VelocityIterations, m_PositionIterations);
	}

	void PhysicsWorld::EndWorld()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}
}