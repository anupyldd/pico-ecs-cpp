#pragma once

#include "pico_ecs.h"

namespace pico_ecs_cpp
{
	enum class PicoEcsCppErrorCode
	{
		Success = 0,
		UnknownError = -1
	};
}

#if defined(PICO_ECS_CPP_ERRORS_USE_EXCEPTIONS)

	#include <exception>
	#include <sstream>
	#define	PICO_ECS_ERROR(code, msg)					\
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
				std::cerr << "[PICO_ECS_CPP_ERROR][" << code << "] " << msg << '\n';	\
			}																			\
			while(0)

#elif defined(PICO_ECS_CPP_ERROR_USE_CALLBACK)

	#include <functional>
	#include <string>
	std::function<void(int, const std::string&)> PicoEcsCppErrorHandler =					\
		[](int code, const std::string& msg)												\
		{ std::cerr << "[PICO_ECS_CPP_ERROR][" << code << "] " << msg << '\n'; }			\
	#define PICO_ECS_CPP_ERROR(code, msg)													\
			do																				\
			{																				\
				PicoEcsCppErrorHandler(code, msg);											\
			}																				\
			while(0)

#else

	#define	PICO_ECS_CPP_ERROR(code, msg)

#endif 

namespace pico_ecs_cpp
{


	class Ecs
	{
	public:

	private:

	};
}