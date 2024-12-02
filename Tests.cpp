
#define PICO_ECS_IMPLEMENTATION

#define PICO_ECS_CPP_ERROR_USE_CALLBACK
#define PICO_ECS_CPP_SHORTHAND_MACROS

#include "src/PicoEcsCpp.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

void Test(const std::string& title)
{
	std::cout << "\n>>> " << title << " -------------\n";
}

void Instance(int i)
{
	std::cout << "> EcsInstance #" << i << '\n';
}

using namespace pico_ecs_cpp;

// components -----------------------------------------

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

struct UnregisteredComp { };

// systems ------------------------------------------------

// requires all components, excludes none
PICO_ECS_CPP_SYSTEM_FUNCTION(ComponentPrintSystem)
{
	EcsInstance* instance = static_cast<EcsInstance*>(udata);
	if (instance)
	{
		for (int i = 0; i < entity_count; ++i)
		{
			Transform* tr = instance->EntityGetComponent<Transform>(entities[i]);
			Velocity* vel = instance->EntityGetComponent<Velocity>(entities[i]);
			Name* name = instance->EntityGetComponent<Name>(entities[i]);

			float	trX = tr->x, 
					trY = tr->y,
					velX = vel->x,
					velY = vel->y;
			std::string nm = name->name;
			
			std::cout << FormatString("- Entity %i:\nTransform: %f, %f\nVelocity: %f, %f\nName: %s\n",
				entities[i], trX, trY, velX, velY, nm.c_str());
		}
		return 0;
	}
	return 1;
};

// requires velocity and transform, excludes name
PICO_ECS_CPP_SYSTEM_FUNCTION(MoveSystem)
{
	EcsInstance* instance = static_cast<EcsInstance*>(udata);
	if (instance)
	{
		for (int i = 0; i < entity_count; ++i)
		{
			Transform* tr = instance->EntityGetComponent<Transform>(entities[i]);
			Velocity* vel = instance->EntityGetComponent<Velocity>(entities[i]);

			std::cout << "- Entity " << entities[i] << ": ";
			std::cout << "was: " << tr->x << " - " << tr->y;

			tr->x += vel->x;
			tr->y += vel->y;

			std::cout << " | now: " << tr->x << " - " << tr->y << '\n';
		}
		return 0;
	}
	return 1;
}

PICO_ECS_CPP_SYSTEM_FUNCTION(UnregisteredSystem)
{
	return 0;
}

// tests --------------------------------------

int main()
{
	PicoEcsCppErrorHandler = [](StatusCode code, const std::string& msg)
		{
			if (code != StatusCode::Success) 
				std::cerr << '[' << GetStatusMessage(code) << "] " << msg << '\n';
		};

	EcsInstance ecs1;
	EcsInstance ecs2;

	std::cout << "Starting tests.\nError output is expected as long as it's not an assert failure.\n\n";

	/*
	* should output 1 error, when trying to initialize with negative number
	* other inits should be silent
	*/
	Test("Instance initialization");
	Instance(1);
	assert(ecs1.Init(-1) == StatusCode::InitFail);
	assert(ecs1.Init(100) == StatusCode::Success);
	
	Instance(2);
	assert(ecs2.Init(200) == StatusCode::Success);

	/*
	* should output 1 error when trying to register Transform for the 2nd time
	* other registrations should be silent
	*/
	Test("Component registration");
	Instance(1);
	assert(ecs1.ComponentRegister<Transform>(TransformConstructor) == StatusCode::Success);
	assert(ecs1.ComponentRegister<Transform>(TransformConstructor) == StatusCode::CompExists);
	assert(ecs1.ComponentRegister<Velocity>(VelocityConstructor) == StatusCode::Success);
	assert(ecs1.ComponentRegister<Name>(NameConstructor) == StatusCode::Success);
	
	Instance(2);
	assert(ecs2.ComponentRegister<Name>(NameConstructor) == StatusCode::Success);
	assert(ecs2.ComponentRegister<Transform>(TransformConstructor) == StatusCode::Success);
	assert(ecs2.ComponentRegister<Velocity>(VelocityConstructor) == StatusCode::Success);

	/*
	* should output 1 error when trying to register ComponentPrintSystem for the 2nd time
	* other registrations should be silent
	*/
	Test("System registration");
	Instance(1);
	assert(ecs1.SystemRegister(ComponentPrintSystemName, ComponentPrintSystem) == StatusCode::Success);
	assert(ecs1.SystemRegister(ComponentPrintSystemName, ComponentPrintSystem) == StatusCode::SysExists);

	Instance(2);
	assert(ecs2.SystemRegister(MoveSystemName, MoveSystem) == StatusCode::Success);

	/*
	* should output 4 errors: when trying to require/exclude unregistered component
	* and when trying to register/exclude using unregistered system
	*/
	Test("System component require/exclude");
	Instance(1);
	assert(ecs1.SystemRequire<Transform>(ComponentPrintSystemName) == StatusCode::Success);
	assert(ecs1.SystemRequire<Velocity>(ComponentPrintSystemName) == StatusCode::Success);
	assert(ecs1.SystemRequire<Name>(ComponentPrintSystemName) == StatusCode::Success);

	assert(ecs1.SystemRequire<UnregisteredComp>(ComponentPrintSystemName) == StatusCode::CompNotReg);
	assert(ecs1.SystemRequire<Velocity>(UnregisteredSystemName) == StatusCode::SysNotReg);
	
	Instance(2);
	assert(ecs2.SystemRequire<Transform>(MoveSystemName) == StatusCode::Success);
	assert(ecs2.SystemRequire<Velocity>(MoveSystemName) == StatusCode::Success);

	assert(ecs2.SystemExclude<Name>(MoveSystemName) == StatusCode::Success);

	assert(ecs2.SystemExclude<Name>(UnregisteredSystemName) == StatusCode::SysNotReg);
	assert(ecs2.SystemExclude<UnregisteredComp>(MoveSystemName) == StatusCode::CompNotReg);

	/*
	* should print 2 errors when trying to enable/disable unregistered system
	*/
	Test("System enable/disable");
	Instance(1);
	assert(ecs1.SystemDisable(ComponentPrintSystemName) == StatusCode::Success);
	assert(ecs1.SystemEnable(ComponentPrintSystemName) == StatusCode::Success);

	assert(ecs1.SystemEnable(UnregisteredSystemName) == StatusCode::SysNotReg);

	Instance(2);
	assert(ecs2.SystemDisable(MoveSystemName) == StatusCode::Success);
	assert(ecs2.SystemEnable(MoveSystemName) == StatusCode::Success);
			  
	assert(ecs2.SystemDisable(UnregisteredSystemName) == StatusCode::SysNotReg);

	/*
	* should be silent
	*/
	Test("Entity creation/destruction");
	Instance(1);
	EntityId e1 = ecs1.EntityCreate();
	EntityId e2 = ecs1.EntityCreate();
	EntityId e3 = ecs1.EntityCreate();
	assert(ecs1.EntityIsReady(e1) && ecs1.EntityIsReady(e2) && ecs1.EntityIsReady(e3));

	ecs1.EntityDestroy(e1);
	assert(!ecs1.EntityIsReady(e1));

	e1 = ecs1.EntityCreate();
	assert(ecs1.EntityIsReady(e1));

	Instance(2);
	std::vector<EntityId> entities;
	for (size_t i = 0; i < 10; ++i)
	{
		entities.push_back(ecs2.EntityCreate());
	}

	/*
	* should be silent
	*/
	Test("Entity add/get/remove component");
	Instance(1);
	Transform tr1{ 1.1f, 1.1f };
	Velocity vel1{ 1.1f, 1.1f };
	Name nm1{ "e1 name" };
	assert(ecs1.EntityAddComponent<Transform>(e1, &tr1));
	assert(ecs1.EntityAddComponent<Velocity>(e1, &vel1));
	assert(ecs1.EntityAddComponent<Name>(e1, &nm1));

	assert(ecs1.EntityGetComponent<Transform>(e1));
	assert(ecs1.EntityGetComponent<Velocity>(e1));
	assert(ecs1.EntityGetComponent<Name>(e1));

	Transform tr2{ 2.2f, 2.2f };
	Velocity vel2{ 2.2f, 2.2f };
	Name nm2{ "e2 name" };
	assert(ecs1.EntityAddComponent<Transform>(e2, &tr2));
	assert(ecs1.EntityAddComponent<Velocity>(e2, &vel2));
	assert(ecs1.EntityAddComponent<Name>(e2, &nm2));

	Transform tr3{ 3.3f, 3.3f };
	Velocity vel3{ 3.3f, 3.3f };
	Name nm3{ "e3 name" };
	assert(ecs1.EntityAddComponent<Transform>(e3, &tr3));
	assert(ecs1.EntityAddComponent<Velocity>(e3, &vel3));
	assert(ecs1.EntityAddComponent<Name>(e3, &nm3));
	
	assert(ecs1.EntityRemoveComponent<Transform>(e3) == StatusCode::Success);

	Instance(2);
	for (size_t i = 0; i < entities.size(); ++i)
	{
		if (i % 2 == 0)
		{
			Name nm{ "some name" };
			assert(ecs2.EntityAddComponent<Name>(entities[i], &nm));
		}
		Transform tr{ 0.0f, 0.0f };
		Velocity vel{ (float)i, (float)i };
		assert(ecs2.EntityAddComponent<Transform>(entities[i], &tr));
		assert(ecs2.EntityAddComponent<Velocity>(entities[i], &vel));
	}

	/*
	* should print what systems are outputting
	* entity components for the first
	* transform changes for the second
	*/
	Test("System update");
	Instance(1);
	ecs1.Update();
	ecs1.Update();

	Instance(2);
	ecs2.Update();

	/*
	* should be silent
	*/
	Test("Instance destruction");
	Instance(1);
	assert(ecs1.Destroy() == StatusCode::Success);
	Instance(2);
	assert(ecs2.Destroy() == StatusCode::Success);
}