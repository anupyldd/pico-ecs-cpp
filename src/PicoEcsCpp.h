#pragma once

#include "pico_ecs.h"

#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <algorithm>
#include <functional>
#include <string>

// error handling -----------------------------------------------------

namespace pico_ecs_cpp
{
	enum class StatusCode
	{
		Success,
		UnknownError,

		InitFail,

		CompExists,
		CompRegFail,

		SysExists,
		SysRegFail,
		SysUpdateFail
	};

	inline std::string GetStatusMessage(StatusCode code)
	{
		switch (code)
		{
		case StatusCode::Success: return "Success";

		case StatusCode::InitFail: return "Initialization Failure";

		case StatusCode::CompExists: return "Component Already Registered";

		case StatusCode::CompRegFail: return "Component Registration Failed";

		case StatusCode::SysExists: return "System Already Registered";

		case StatusCode::SysRegFail: return "System Registration Failed";

		case StatusCode::SysUpdateFail: return "System Update Failure";

		case StatusCode::UnknownError:
		default: return "Unknown Error";
		}
	}

	template<typename ... Args>
	std::string FormatString(const std::string& format, Args ... args)
	{
		int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; 
		if (size_s > 0)
		{
			auto size = static_cast<size_t>(size_s);
			std::unique_ptr<char[]> buf(new char[size]);
			std::snprintf(buf.get(), size, format.c_str(), args ...);
			return std::string(buf.get(), buf.get() + size - 1); 
		}
	}

	// aliases --------------------------------------------------------------

	using Ecs						= ecs_t;
	using ReturnCode				= ecs_ret_t;
	using EcsDt						= ecs_dt_t;
	
	using EcsId						= ecs_id_t;
	using EntityId					= ecs_id_t;
	using ComponentId				= ecs_id_t;
	using SystemId					= ecs_id_t;

	using ComponentCtor				= ecs_constructor_fn;
	using ComponentDtor				= ecs_destructor_fn;
	
	using SystemFunc				= ecs_system_fn;
	using SystemAddedCb				= ecs_added_fn;
	using SystemRemovedCb			= ecs_removed_fn;
}

#if defined(PICO_ECS_CPP_ERRORS_USE_EXCEPTIONS)

	#include <exception>
	#include <sstream>
	#define	PICO_ECS_CPP_ERROR(code, msg)				\
			do											\
			{											\
				std::stringstream sstr;					\
				sstr << '[' << code << "]: " << msg;	\
				throw std::runtime_error(sstr.str());	\
			}											\
			while(0)

#elif defined(PICO_ECS_CPP_ERROR_USE_STD_ERR)
	
	#include <iostream>
	#define	PICO_ECS_CPP_ERROR(code, msg)												\
			do																			\
			{																			\
				std::cerr << "[PICO_ECS_CPP][" << code << "] " << msg << '\n';			\
			}																			\
			while(0)

#elif defined(PICO_ECS_CPP_ERROR_USE_CALLBACK)

	#include <functional>
	#include <iostream>
	#include <string>
	std::function<void(pico_ecs_cpp::StatusCode, const std::string&)> PicoEcsCppErrorHandler =
		[](pico_ecs_cpp::StatusCode code, const std::string& msg)
		{ std::cerr << "[PICO_ECS_CPP][" << pico_ecs_cpp::GetStatusMessage(code) << "] " << msg << '\n'; };
																							
	#define PICO_ECS_CPP_ERROR(code, msg)														\
			do																					\
			{																					\
				PicoEcsCppErrorHandler(code, msg);												\
			}																					\
			while(0)

#else

	#define	PICO_ECS_CPP_ERROR(code, msg)

#endif 

// component creation -----------------------------------------------------

#if defined(PICO_ECS_CPP_SHORTHAND_MACROS)

/*
does not include function body.
adds "Constructor" to the name
*/
#define PICO_ECS_CPP_COMPONENT_CONSTRUCTOR(CtorName)							\
	pico_ecs_cpp::ComponentCtor CtorName##Constructor = [](ecs_t* ecs, ecs_id_t entity_id, void* ptr, void* args)					\

/*
does not include function body.
adds "Destructor" to the name
*/
#define PICO_ECS_CPP_COMPONENT_DESTRUCTOR(DtorName)								\
	pico_ecs_cpp::ComponentDtor DtorName##Destructor = [](ecs_t* ecs, ecs_id_t entity_id, void* ptr)

/* 
creates a constructor that accepts an object of type CompName and
copies its contents into the component.
adds "Constructor" to the name
*/
#define PICO_ECS_CPP_COMPONENT_CONSTRUCTOR_COPY(CtorName)										\
	pico_ecs_cpp::ComponentCtor CtorName##Constructor = [](ecs_t* ecs, ecs_id_t entity_id, void* ptr, void* args)					\
	{																							\
		CtorName* comp = static_cast<CtorName*>(ptr);											\
		CtorName* init = static_cast<CtorName*>(args);											\
		if(init) (*comp) = (*init);																\
	}

// does not include function body
#define PICO_ECS_CPP_SYSTEM_FUNCTION(FuncName)									\
	auto FuncName = [](ecs_t* ecs, ecs_id_t* entities, int entity_count, ecs_dt_t dt, void* udata)

#endif

namespace pico_ecs_cpp
{
	
	
	// ecs instance -------------------------------------------------------------

	class EcsInstance
	{
	public:
		EcsInstance() = default;
		~EcsInstance();

		// initializes an ECS instance
		EcsInstance(int entityCount);

		// initializes an ECS instance
		StatusCode Init(int entityCount);

		// destroys an ECS instance
		StatusCode Destroy();

		// removes all entities from the ECS, preserving systems and components
		StatusCode Reset();

		// updates all systems, should be called once per frame
		StatusCode Update(EcsDt dt = 0.0f);

	public:

		EntityId EntityCreate();

	public:

		// register a single component with optional constructor and destructor
		template<typename CompType>
		StatusCode ComponentRegister(ComponentCtor ctor = nullptr, ComponentDtor dtor = nullptr);

		template<typename CompType>
		CompType* GetComponent(EntityId id);

	public:

		template<typename T>
		StatusCode SystemRegister(
			const std::string& name,
			SystemFunc func, 
			SystemAddedCb add = nullptr, 
			SystemRemovedCb rem = nullptr,
			T* udata = nullptr);

	private:
		Ecs* instance = nullptr;

		std::unordered_map<std::type_index, ComponentId> components;
		std::unordered_map<std::string, SystemId> systems;
	};

	// definitions -----------------------------------------------

	inline EcsInstance::EcsInstance(int entityCount)
	{
		Init(entityCount);
	}

	inline EcsInstance::~EcsInstance()
	{
		if(ecs_is_not_null(instance)) Destroy();
	}

	inline StatusCode EcsInstance::Init(int entityCount)
	{
		if (!(entityCount > 0))
		{
			PICO_ECS_CPP_ERROR(StatusCode::InitFail, "Invalid entity count");
			return StatusCode::InitFail;
		}

		instance = ecs_new(static_cast<size_t>(entityCount), nullptr);

		if (instance)
		{
			return StatusCode::Success;
		}
		else
		{
			PICO_ECS_CPP_ERROR(StatusCode::InitFail, "Failed to initialize ECS instance");
			return StatusCode::InitFail;
		}
	}

	inline StatusCode EcsInstance::Destroy()
	{
		ecs_free(instance);
		instance = nullptr;
		return StatusCode::Success;
	}

	inline StatusCode EcsInstance::Reset()
	{
		ecs_reset(instance);
		return StatusCode::Success;
	}

	inline StatusCode EcsInstance::Update(EcsDt dt)
	{
		if (ecs_update_systems(instance, dt))
			return StatusCode::Success;
		else 
			return StatusCode::SysUpdateFail;
	}

	template<typename CompType>
	inline StatusCode EcsInstance::ComponentRegister(ComponentCtor ctor, ComponentDtor dtor)
	{
		for (const auto& comp : components)
		{
			if (comp.first == typeid(CompType))
			{
				PICO_ECS_CPP_ERROR(StatusCode::CompExists, 
					FormatString("Component %s is already registered", typeid(CompType).name()));
				return StatusCode::CompExists;
			}
		}

		components[typeid(CompType)] = ecs_register_component(instance, sizeof(CompType), ctor, dtor);
		return StatusCode::Success;
	}
	
	template<typename CompType>
	inline CompType* EcsInstance::GetComponent(EntityId id)
	{
		return static_cast<CompType*>(ecs_get(instance, id, typeid(CompType)));	
	}

	template<typename T>
	inline StatusCode EcsInstance::SystemRegister(const std::string& name, SystemFunc func, SystemAddedCb add, SystemRemovedCb rem, T* udata)
	{
		for (const auto& sys : components)
		{
			if (sys.first == name)
			{
				PICO_ECS_CPP_ERROR(StatusCode::SysExists,
					FormatString("System %s is already registered", name));
				return StatusCode::SysExists;
			}
		}

		return StatusCode();
	}
}