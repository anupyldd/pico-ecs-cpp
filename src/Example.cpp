
#define PICO_ECS_IMPLEMENTATION
#define PICO_ECS_CPP_ERROR_USE_CALLBACK

#include "PicoEcsCpp.h"

int main()
{
	using namespace pico_ecs_cpp;

	// setting a custom error handler
	PicoEcsCppErrorHandler = [](StatusCode code, const std::string& msg)
		{
			switch (code)
			{
			case StatusCode::InitFail:
				std::cerr << "Some init error: " << msg << '\n';
				break;
			case StatusCode::UnknownError:
				std::cerr << "Not good: " << msg << '\n';
				break;
			default:
				break;
			}
		};

	// creating an ecs instance
	EcsInstance ecs;
	ecs.Init(200);

	// not strictly required, all resources are freed on destruction anyway
	ecs.Destroy();

	PICO_ECS_CPP_ERROR(StatusCode::InitFail, FormatString("Success %i", 10));
}