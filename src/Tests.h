#pragma once

#include "PicoEcsCpp.h"

#include <cassert>

inline void RunTests()
{
	using namespace pico_ecs_cpp;

	EcsInstance ecs1;
	EcsInstance ecs2;
	EcsInstance ecs3;

	struct Transform
	{
		float x, y;
	};
	PICO_ECS_CPP_COMPONENT_CONSTRUCTOR_COPY(Transform);

	struct Velocity
	{
		float x, y;
	};
	PICO_ECS_CPP_COMPONENT_CONSTRUCTOR_COPY(Velocity);

	struct Name
	{
		std::string name;
	};
	PICO_ECS_CPP_COMPONENT_CONSTRUCTOR_COPY(Name);

	assert(ecs1.Init(-1) == StatusCode::InitFail);
	assert(ecs1.Init(100) == StatusCode::Success);
	assert(ecs2.Init(200) == StatusCode::Success);

	assert(ecs1.ComponentRegister<Transform>(TransformConstructor) == StatusCode::Success);
	assert(ecs1.ComponentRegister<Velocity>(VelocityConstructor) == StatusCode::Success);
	assert(ecs2.ComponentRegister<Name>(NameConstructor) == StatusCode::Success);

	assert(ecs1.Destroy() == StatusCode::Success);
	assert(ecs2.Destroy() == StatusCode::Success);
}