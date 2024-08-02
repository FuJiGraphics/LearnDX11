#ifndef _CORE_H_
#define _CORE_H_
#pragma once

#include "Core.h"

// Includes
#include <iostream>
#include <memory>

// pre-processing directives
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Pointer
namespace fz {
	template <typename T>
	using Shared = std::shared_ptr<T>;

	template <typename T, typename ...Args>
	static Shared<T> CreateShared(const Args&& ...args)
	{
		return (std::make_shared<T>(std::forward<Args>(args)...));
	}
}

#endif
