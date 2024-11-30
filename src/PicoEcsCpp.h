#pragma once

#include "pico_ecs.h"

#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <algorithm>
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
		CompRegFail
	};

	inline std::string GetStatusMessage(StatusCode code)
	{
		switch (code)
		{
		case StatusCode::Success: return "Success";

		case StatusCode::InitFail: return "Initialization Failure";

		case StatusCode::CompExists: return "Component Already Registered";

		case StatusCode::CompRegFail: return "Component Registration Failed";

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

#if defined(PICO_ECS_CPP_COMPONENT_MACROS)

#define PICO_ECS_CPP_COMPONENT_CONSTRUCTOR(CompName)														\
	inline void CompName##Constructor (ecs_t* ecs, ecs_id_t entity_id, void* ptr, void* args)	\
	{																							\
		CompName* comp = static_cast<CompName*>(ptr);											\
		CompName* init = static_cast<CompName*>(args);											\
		if(init) (*comp) = (*init);																\
	}

#endif

namespace pico_ecs_cpp
{
	// aliases --------------------------------------------------------------

	using Ecs						= ecs_t;
	using ReturnCode				= ecs_ret_t;
	
	using EcsId						= ecs_id_t;
	using EntityId					= ecs_id_t;
	using ComponentId				= ecs_id_t;
	using SystemId					= ecs_id_t;

	using ComponentCtor				= ecs_constructor_fn;
	using ComponentDtor				= ecs_destructor_fn;
	
	using SystemFunc				= ecs_system_fn;
	using SystemAddCallback			= ecs_added_fn;
	using SystemRemoveCallback		= ecs_removed_fn;

	// components -------------------------------------------------------------

	template<typename T>
	struct Component
	{
		T comp;
		ComponentCtor ctor = nullptr;
		ComponentDtor dtor = nullptr;
	};

	// systems -----------------------------------------------------------------
	

	
	// ecs instance -------------------------------------------------------------

	class EcsInstance
	{
	public:
		EcsInstance() = default;
		~EcsInstance();

		// initializes an ECS instance
		EcsInstance(size_t entityCount);

		// initializes an ECS instance
		StatusCode Init(size_t entityCount);

		// destroys an ECS instance
		StatusCode Destroy();

		// removes all entities from the ECS, preserving systems and components
		StatusCode Reset();

	public:

		// register a single component with optional constructor and destructor
		template<typename CompType>
		StatusCode RegisterComponent(ComponentCtor ctor = nullptr, ComponentDtor dtor = nullptr);

	private:
		Ecs* instance = nullptr;

		std::unordered_map<std::type_index, ComponentId> components;
	};

	// definitions -----------------------------------------------

	inline EcsInstance::EcsInstance(size_t entityCount)
	{
		instance = ecs_new(entityCount, nullptr);
		if (!instance) PICO_ECS_CPP_ERROR(StatusCode::InitFail, "Failed to initialize ECS instance");
	}

	inline EcsInstance::~EcsInstance()
	{
		if(ecs_is_not_null(instance)) Destroy();
	}

	inline StatusCode EcsInstance::Init(size_t entityCount)
	{
		instance = ecs_new(entityCount, nullptr);

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

	template<typename CompType>
	inline StatusCode EcsInstance::RegisterComponent(ComponentCtor ctor, ComponentDtor dtor)
	{
		for (const auto& comp : components)
		{
			if (comp.first == typeid(CompType))
			{
				PICO_ECS_CPP_ERROR(StatusCode::CompExists, "Component already exists in this instance");
			}
		}

		return StatusCode();
	}
}