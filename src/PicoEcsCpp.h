#pragma once

#include "pico_ecs.h"

#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <algorithm>
#include <functional>
#include <string>
#include <memory>

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
        CompNotReg,
        CompGetFail,

        SysExists,
        SysRegFail,
        SysNotReg,
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

        case StatusCode::CompNotReg: return "Component Not Registered";

        case StatusCode::CompGetFail: return "Component Get Failure";

        case StatusCode::SysExists: return "System Already Registered";

        case StatusCode::SysRegFail: return "System Registration Failed";

        case StatusCode::SysNotReg: return "System Not Registered";

        case StatusCode::SysUpdateFail: return "System Update Failure";

        case StatusCode::UnknownError:
        default: return "Unknown Error";
        }
    }

    template<typename ... Args>
    inline std::string FormatString(const std::string& format, Args ... args)
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

    using Ecs                       = ecs_t;
    using ReturnCode                = ecs_ret_t;
    using EcsDt                     = ecs_dt_t;
    
    using EcsId                     = ecs_id_t;
    using EntityId                  = ecs_id_t;
    using ComponentId               = ecs_id_t;
    using SystemId                  = ecs_id_t;

    using ComponentCtor             = ecs_constructor_fn;
    using ComponentDtor             = ecs_destructor_fn;
    
    using SystemFunc                = ecs_system_fn;
    using SystemAddedCb             = ecs_added_fn;
    using SystemRemovedCb           = ecs_removed_fn;
}

#if defined(PICO_ECS_CPP_ERROR_USE_EXCEPTIONS)

    #include <exception>
    #include <sstream>
    #define	PICO_ECS_CPP_ERROR(code, msg)               \
            do                                          \
            {                                           \
                std::stringstream sstr;                 \
                sstr << '[' << code << "]: " << msg;    \
                throw std::runtime_error(sstr.str());   \
            }                                           \
            while(0)

#elif defined(PICO_ECS_CPP_ERROR_USE_STD_ERR)
    
    #include <iostream>
    #define	PICO_ECS_CPP_ERROR(code, msg)                                               \
            do                                                                          \
            {                                                                           \
                std::cerr << "[PICO_ECS_CPP][" << code << "] " << msg << '\n';          \
            }                                                                           \
            while(0)

#elif defined(PICO_ECS_CPP_ERROR_USE_CALLBACK)

    #include <functional>
    #include <iostream>
    #include <string>
    std::function<void(pico_ecs_cpp::StatusCode, const std::string&)> PicoEcsCppErrorHandler =
        [](pico_ecs_cpp::StatusCode code, const std::string& msg)
        { std::cerr << "[PICO_ECS_CPP][" << pico_ecs_cpp::GetStatusMessage(code) << "] " << msg << '\n'; };
                                                                                            
    #define PICO_ECS_CPP_ERROR(code, msg)                                                       \
            do                                                                                  \
            {                                                                                   \
                PicoEcsCppErrorHandler(code, msg);                                              \
            }                                                                                   \
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
#define PICO_ECS_CPP_COMPONENT_CONSTRUCTOR_COPY(CtorName)										                    \
    pico_ecs_cpp::ComponentCtor CtorName##Constructor = [](ecs_t* ecs, ecs_id_t entity_id, void* ptr, void* args)   \
    {																							                    \
        CtorName* comp = static_cast<CtorName*>(ptr);											                    \
        CtorName* init = static_cast<CtorName*>(args);											                    \
        if(init) (*comp) = (*init);																                    \
    }

// does not include function body
#define PICO_ECS_CPP_SYSTEM_FUNCTION(FuncName)									\
    ecs_ret_t FuncName(ecs_t* ecs, ecs_id_t* entities, int entity_count, ecs_dt_t dt, void* udata)

#endif

namespace pico_ecs_cpp
{
    // ecs instance -------------------------------------------------------------

    class EcsInstance
    {
    public:
        EcsInstance() = default;
        ~EcsInstance();

        // initializes an ecs instance
        EcsInstance(int entityCount);

        // initializes an ecs instance
        StatusCode Init(int entityCount);

        // destroys an ecs instance
        StatusCode Destroy();

        // removes all entities from the ecs, preserving systems and components
        StatusCode Reset();

        // updates all systems, should be called once per frame
        StatusCode Update(EcsDt dt = 0.0f);

        // returns pointer to the base ecs instance
        Ecs* GetInstance() const;

    public:

        // creates a new entity, returns its id
        EntityId EntityCreate();

        // checks if entity is currently active
        bool EntityIsReady(EntityId id) const;

        // destroys entity
        StatusCode EntityDestroy(EntityId id);

        // checks if entity has specified component
        template<typename CompType>
        bool EntityHasComponent(EntityId id);

        // gets a pointer to the instance of specified component held by the entity
        template<typename CompType>
        CompType* EntityGetComponent(EntityId id);

        // adds a component to the entity, returns pointer to added component
        template<typename CompType>
        CompType* EntityAddComponent(EntityId id, void* args = nullptr);

        // removes specified component from the entity
        template<typename CompType>
        StatusCode EntityRemoveComponent(EntityId id);

        /*
        * queues an entity for destruction at the end of system execution
        * queued entities are destroyed after the curent iteration
        */
        StatusCode EntityQueueDestroy(EntityId id);

        /*
        * queues a component for removable
        * queued entity/component pairs that will be deleted after the current system returns
        */
        template<typename CompType>
        StatusCode EntityQueueRemoveComponent(EntityId id);

    public:

        // registers a single component with optional constructor and destructor
        template<typename CompType>
        StatusCode ComponentRegister(ComponentCtor ctor = nullptr, ComponentDtor dtor = nullptr);

    public:

        // registers a system with optional added/removed callbacks
        StatusCode SystemRegister(
            const std::string& name,
            SystemFunc func, 
            SystemAddedCb add = nullptr, 
            SystemRemovedCb rem = nullptr);

        // determines which components are available to the specified system
        template<typename CompType>
        StatusCode SystemRequire(const std::string& sysName);

        // excludes entities that have specified component from the system
        template<typename CompType>
        StatusCode SystemExclude(const std::string& sysName);

        // enables a system
        StatusCode SystemEnable(const std::string& sysName);

        // disables a system
        StatusCode SystemDisable(const std::string& sysName);

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
        if(instance) Destroy();
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

    inline Ecs* EcsInstance::GetInstance() const
    {
        return instance;
    }

    template<typename CompType>
    inline StatusCode EcsInstance::ComponentRegister(ComponentCtor ctor, ComponentDtor dtor)
    {
        if (components.find(typeid(CompType)) != components.end())
        {
            PICO_ECS_CPP_ERROR(StatusCode::CompExists, 
                FormatString("Component [%s] is already registered", typeid(CompType).name()));
            return StatusCode::CompExists;
        }

        components[typeid(CompType)] = ecs_register_component(instance, sizeof(CompType), ctor, dtor);
        return StatusCode::Success;
    }

    inline StatusCode EcsInstance::SystemRegister(const std::string& name, SystemFunc func, SystemAddedCb add, SystemRemovedCb rem)
    {
        if (systems.find(name) != systems.end())
        {
            PICO_ECS_CPP_ERROR(StatusCode::SysExists,
                FormatString("System [%s] is already registered", name));
            return StatusCode::SysExists;
        }

        systems[name] = ecs_register_system(instance, func, add, rem, this);

        return StatusCode::Success;
    }

    template<typename CompType>
    inline StatusCode EcsInstance::SystemRequire(const std::string& sysName)
    {
        if (systems.find(sysName) == systems.end())
        {
            PICO_ECS_CPP_ERROR(StatusCode::SysNotReg,
                FormatString("Name [%s] is not associated with any registered system", sysName));
            return StatusCode::SysNotReg;
        }
        if (components.find(typeid(CompType)) == components.end())
        {
            PICO_ECS_CPP_ERROR(StatusCode::CompNotReg,
                FormatString("Component of type [%s] is not registered", typeid(CompType).name()));
            return StatusCode::CompNotReg;
        }

        ecs_require_component(instance, systems.at(sysName), components.at(typeid(CompType)));
        return StatusCode::Success;
    }

    template<typename CompType>
    inline StatusCode EcsInstance::SystemExclude(const std::string& sysName)
    {
        if (systems.find(sysName) == systems.end())
        {
            PICO_ECS_CPP_ERROR(StatusCode::SysNotReg,
                FormatString("Name [%s] is not associated with any registered system", sysName));
            return StatusCode::SysNotReg;
        }
        if (components.find(typeid(CompType)) == components.end())
        {
            PICO_ECS_CPP_ERROR(StatusCode::CompNotReg,
                FormatString("Component of type [%s] is not registered", typeid(CompType).name()));
            return StatusCode::CompNotReg;
        }

        ecs_exclude_component(instance, systems.at(sysName), components.at(typeid(CompType)));
        return StatusCode::Success;
    }

    inline StatusCode EcsInstance::SystemEnable(const std::string& sysName)
    {
        if (systems.find(sysName) == systems.end())
        {
            PICO_ECS_CPP_ERROR(StatusCode::SysNotReg,
                FormatString("Name [%s] is not associated with any registered system", sysName));
            return StatusCode::SysNotReg;
        }

        ecs_enable_system(instance, systems.at(sysName));
        return StatusCode::Success;
    }

    inline StatusCode EcsInstance::SystemDisable(const std::string& sysName)
    {
        if (systems.find(sysName) == systems.end())
        {
            PICO_ECS_CPP_ERROR(StatusCode::SysNotReg,
                FormatString("Name [%s] is not associated with any registered system", sysName));
            return StatusCode::SysNotReg;
        }

        ecs_disable_system(instance, systems.at(sysName));
        return StatusCode::Success;
    }

    inline EntityId EcsInstance::EntityCreate()
    {
        return ecs_create(instance);
    }

    inline bool EcsInstance::EntityIsReady(EntityId id) const
    {
        return ecs_is_ready(instance, id);
    }

    inline StatusCode EcsInstance::EntityDestroy(EntityId id)
    {
        ecs_destroy(instance, id);
        return StatusCode::Success;
    }

    template<typename CompType>
    inline bool EcsInstance::EntityHasComponent(EntityId id)
    {
        return ecs_has(instance, id, components.at(typeid(CompType)));
    }

    template<typename CompType>
    inline CompType* EcsInstance::EntityGetComponent(EntityId id)
    {
        CompType* compPtr = static_cast<CompType*>(ecs_get(instance, id, components.at(typeid(CompType))));
        if (!compPtr)
        {
            PICO_ECS_CPP_ERROR(StatusCode::CompGetFail,
                FormatString("Failed to get component of type [%s] from entity [%i]", typeid(CompType).name(), id));
            return nullptr;
        }
        return compPtr;
    }

    template<typename CompType>
    inline CompType* EcsInstance::EntityAddComponent(EntityId id, void* args)
    {
        if (components.find(typeid(CompType)) == components.end())
        {
            PICO_ECS_CPP_ERROR(StatusCode::CompNotReg,
                FormatString("Component of type [%s] is not registered", typeid(CompType).name()));
            return nullptr;
        }

        return static_cast<CompType*>(
            ecs_add(instance, id, components.at(typeid(CompType)), std::move(args)));
    }

    template<typename CompType>
    inline StatusCode EcsInstance::EntityRemoveComponent(EntityId id)
    {
        if (components.find(typeid(CompType)) == components.end())
        {
            PICO_ECS_CPP_ERROR(StatusCode::CompNotReg,
                FormatString("Component of type [%s] is not registered", typeid(CompType).name()));
            return StatusCode::CompNotReg;
        }

        ecs_remove(instance, id, components.at(typeid(CompType)));

        return StatusCode::Success;
    }

    inline StatusCode EcsInstance::EntityQueueDestroy(EntityId id)
    {
        ecs_queue_destroy(instance, id);
        return StatusCode::Success;
    }

    template<typename CompType>
    inline StatusCode EcsInstance::EntityQueueRemoveComponent(EntityId id)
    {
        if (components.find(typeid(CompType)) == components.end())
        {
            PICO_ECS_CPP_ERROR(StatusCode::CompNotReg,
                FormatString("Component of type [%s] is not registered", typeid(CompType).name()));
            return StatusCode::CompNotReg;
        }

        ecs_queue_remove(instance, id, components.at(typeid(CompType)));
        
        return StatusCode::Success;
    }
}