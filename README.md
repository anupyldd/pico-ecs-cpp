# C++ wrapper for [pico_ecs](https://github.com/empyreanx/pico_headers/blob/main/pico_ecs.h)

This header-only library wraps `pico_ecs` ECS instance into an `EcsInstance` object and implements all its functionality through methods.

Each `EcsInstance` holds its own set of component and system IDs, stored in maps. Components are associated with their `std::type_index`, while systems are associated with user-provided `std::string` names.

## Error handling

You can configure the error-handling mechanism by defining an appropriate macro *before* including the header. The available options are:

- **`PICO_ECS_CPP_ERRORS_USE_EXCEPTIONS`**  
  Use C++ exceptions (`std::runtime_error`) for error handling.

- **`PICO_ECS_CPP_ERROR_USE_STD_ERR`**  
  Output error messages to `std::cerr`. 

- **`PICO_ECS_CPP_ERROR_USE_CALLBACK`**  
  Use a user-defined callback function (`PicoEcsCppErrorHandler`) to handle errors. By default, the callback logs the error to `std::cerr`, but you can provide your own implementation for custom error handling.

- **No macro defined**  
  Disables the internal error-handling mechanism. In this mode, no error logging or handling is performed by the library.

Regardless of the selected error-handling option, __most methods return status codes__, so you can always rely on them for error handling if you choose to disable other mechanisms.
