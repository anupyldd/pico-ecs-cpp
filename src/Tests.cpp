
#include <iostream>
#include <string>

void TestTitle(const std::string& title)
{
	std::cout << ">>> " << title << " -------------\n";
}

#define PICO_ECS_IMPLEMENTATION

// wrapper configuration
#define PICO_ECS_CPP_ERROR_USE_CALLBACK
#define PICO_ECS_CPP_SHORTHAND_MACROS

#include "PicoEcsCpp.h"

using namespace pico_ecs_cpp;

PICO_ECS_CPP_SYSTEM_FUNCTION(ComponentPrintSystem)
{
	EcsInstance* instance = static_cast<EcsInstance*>(udata);
	if (instance)
	{
		for (int i = 0; i < entity_count; ++i)
		{

		}
	}
	return 1;
};


int main()
{
	PicoEcsCppErrorHandler = [](StatusCode code, const std::string& msg)
		{
			if (code != StatusCode::Success) std::cerr << '[' << GetStatusMessage(code) << "] " << msg << '\n';
		};

	EcsInstance ecs1;
	EcsInstance ecs2;

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

	std::cout << "Starting tests.\nError output is expected as long as it's not an assert failure.\n\n";

	TestTitle("Instance initialization");
	assert(ecs1.Init(-1) == StatusCode::InitFail);
	assert(ecs1.Init(100) == StatusCode::Success);
	assert(ecs2.Init(200) == StatusCode::Success);

	TestTitle("Component registration");
	assert(ecs1.ComponentRegister<Transform>(TransformConstructor) == StatusCode::Success);
	assert(ecs1.ComponentRegister<Transform>(TransformConstructor) == StatusCode::CompExists);
	assert(ecs1.ComponentRegister<Velocity>(VelocityConstructor) == StatusCode::Success);
	assert(ecs2.ComponentRegister<Name>(NameConstructor) == StatusCode::Success);

	TestTitle("System registration");
	assert(ecs1.SystemRegister("ComponentPrintSystem", ComponentPrintSystem) == StatusCode::Success);
	assert(ecs1.SystemRegister("ComponentPrintSystem", ComponentPrintSystem) == StatusCode::SysExists);

	TestTitle("Instance destruction");
	assert(ecs1.Destroy() == StatusCode::Success);
	assert(ecs2.Destroy() == StatusCode::Success);
}