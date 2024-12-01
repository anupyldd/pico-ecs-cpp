﻿
#define PICO_ECS_IMPLEMENTATION

#define PICO_ECS_CPP_ERROR_USE_CALLBACK
#define PICO_ECS_CPP_SHORTHAND_MACROS

#include "PicoEcsCpp.h"

#include <iostream>
#include <string>

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

			std::cout << FormatString("Entity %i:\nTransform: %f, %f\nVelocity: %f, %f\nName: %s",
				tr->x, tr->y, vel->x, vel->y, name->name);
		}
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

			tr->x += vel->x;
			tr->y += vel->y;
		}
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
	for (size_t i = 0; i < 10; ++i)
	{
		ecs2.EntityCreate();
	}

	/*
	* should print values of all entities and 
	*/
	Test("System update");
	Instance(1);
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