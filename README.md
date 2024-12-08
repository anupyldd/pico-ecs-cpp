# C++ wrapper for [pico_ecs](https://github.com/empyreanx/pico_headers/blob/main/pico_ecs.h)

This header-only library wraps `pico_ecs` ECS instance into an `EcsInstance` object and implements all its functionality through methods.

Each `EcsInstance` holds its own set of component and system IDs, stored in maps. Components are associated with their `std::type_index`, while systems are associated with user-provided `std::string` names.

## Usage

To use this wrapper, simply copy the `PicoEcsCpp.h` header wherever you need and adjust the `#include` path inside it to correctly point to the `pico_ecs.h` in your project.

## Error handling

You can configure the error-handling mechanism by defining an appropriate macro *before* including the header. The available options are:

- **`PICO_ECS_CPP_ERROR_USE_EXCEPTIONS`**  
  Use C++ exceptions (`std::runtime_error`) for error handling.

- **`PICO_ECS_CPP_ERROR_USE_STD_ERR`**  
  Output error messages to `std::cerr`. 

- **`PICO_ECS_CPP_ERROR_USE_CALLBACK`**  
  Use a user-defined callback function (`PicoEcsCppErrorHandler`) to handle errors. By default, the callback logs the error to `std::cerr`, but you can provide your own implementation for custom error handling.

- **No macro defined**  
  Disables the internal error-handling mechanism. In this mode, no error logging or handling is performed by the library.

Regardless of the selected error-handling option, __most methods return status codes__, so you can always rely on them for error handling if you choose to disable other mechanisms.

## Additional configuration macros

- **`PICO_ECS_CPP_SHORTHAND_MACROS`**  
  Access the following macros:

    -  **`PICO_ECS_CPP_COMPONENT_CONSTRUCTOR`**
    Declare a constructor for specified component. Does not include the function body.

    - **`PICO_ECS_CPP_COMPONENT_DESTRUCTOR`**
    Declare a destructor for specified component. Does not include the function body.

    - **`PICO_ECS_CPP_COMPONENT_CONSTRUCTOR_COPY`**
    Define a constructor for the specified component that accepts an object of the component type and copies it into the component itself. Includes the function body.

    - **`PICO_ECS_CPP_SYSTEM_FUNCTION`**
    Declare a system function. Does not include the function body.

## Example

```cpp
// needed for pico_ecs
#define PICO_ECS_IMPLEMENTATION

// wrapper configuration
#define PICO_ECS_CPP_ERROR_USE_CALLBACK
#define PICO_ECS_CPP_SHORTHAND_MACROS

#include "PicoEcsCpp.h"

using namespace pico_ecs_cpp;

// creating a component
struct Transform
{
    int x = 0, y = 0;
};

// creating a system using a macro
const std::string componentPrintSystemName("ComponentPrintSystem");
PICO_ECS_CPP_SYSTEM_FUNCTION(ComponentPrintSystem)
{
    // getting the current ecs instance
    EcsInstance* instance = static_cast<EcsInstance*>(udata);
    
    // checking if the instance is valid
    if (instance)
    {
        // iterating over the entities with components
        // required by the system
        for (int i = 0; i < entity_count; ++i)
        {
            // getting the instance of the component
            Transform* tr = instance->EntityGetComponent<Transform>(entities[i]);

            std::cout << tr->x << " - " << tr->y << '\n';
        }
        return 0;
    }
    return 1;
}

int main()
{
    // setting a custom callback that will be called when errors occur
    PicoEcsCppErrorHandler = [](StatusCode code, const std::string& msg)
        {
            std::cerr << "This is a custom callback function\n";
            std::cerr << "Error Type: " << GetStatusMessage(code) << 
            "\nError Message: " << msg << '\n';
        };

    // creating an ecs instance
    EcsInstance ecs;

    // initializing the instance with entity count (entity pool size)
    ecs.Init(100);

    // registering the component
    ecs.ComponentRegister<Transform>();

    // registering the system
    ecs.SystemRegister(componentPrintSystemName, ComponentPrintSystem);

    // setting required component for the system
    ecs.SystemRequire<Transform>(componentPrintSystemName);

    // creating an entity
    EntityId e1 = ecs.EntityCreate();

    // adding a component to the entity
    ecs1.EntityAddComponent<Transform>(e1);

    // updating the system
    ecs.Update();
}
```

## License

This wrapper is licensed under CC0 / Public domain.
See the license for `pico_ecs` library [here](https://github.com/empyreanx/pico_headers/blob/fcdc26d0d5955ba57917628e1dd24970957176b9/pico_ecs.h#L1500).
