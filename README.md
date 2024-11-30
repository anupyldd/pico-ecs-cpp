# C++ wrapper for [pico_ecs](https://github.com/empyreanx/pico_headers/blob/main/pico_ecs.h)

This header-only library wraps `pico_ecs` ECS instance into an `EcsInstance` object and implements all its functionality through methods.

## Error handling

You can configure the error-handling mechanism by defining an appropriate macro *before* including the header. The available options are:

- __PICO_ECS_CPP_ERRORS_USE_EXCEPTIONS__
Use C++ exceptions (`std::runtime_error`) for error handling. This is the default choice for most C++ applications.

- __PICO_ECS_CPP_ERROR_USE_STD_ERR__
Output error messages to `std::cerr`. This is suitable for applications where logging errors to the console is sufficient.

- __PICO_ECS_CPP_ERROR_USE_CALLBACK__
Use a user-defined callback function (`PicoEcsCppErrorHandler`) to handle errors. By default, the callback logs the error to `std::cerr`, but you can provide your own implementation for custom error handling.

- __No macro defined__
Disables the internal error-handling mechanism. In this mode, no error logging or handling is performed by the library.

Regardless of the selected error-handling option, __most methods return status codes__, so you can always rely on them for error handling if you choose to disable other mechanisms.
