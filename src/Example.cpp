#define PICO_ECS_IMPLEMENTATION

// wrapper configuratoin
#define PICO_ECS_CPP_ERROR_USE_CALLBACK
#define PICO_ECS_CPP_SHORTHAND_MACROS

#include "PicoEcsCpp.h"
#include "Tests.h"

int main()
{
	RunTests();

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
				std::cerr << "idk what's going on\n";
				break;
			}
		};


	// creating components 
	struct Transform
	{
		float x, y;
	};
	PICO_ECS_CPP_COMPONENT_CONSTRUCTOR_COPY(Transform);

	struct Name
	{
		std::string name;
	};
	PICO_ECS_CPP_COMPONENT_CONSTRUCTOR_COPY(Name);

	// creating two ecs instances
	EcsInstance ecs, ecs2;

	// initializing instances
	ecs.Init(200);
	ecs2.Init(100);

	ecs.ComponentRegister<Transform>(TransformConstructor);
	ecs.ComponentRegister<Name>(NameConstructor);

	ecs2.ComponentRegister<Name>(NameConstructor);

	// not strictly required, all resources are freed on destruction anyway
	ecs.Destroy();
	ecs2.Destroy();
}