#include <precompiled_header.h>

#include "fiber.h"
#include "windows.h"


namespace kth
{
	
	static void fiber_starter(void* args)
	{
		std::function<void()>* pfunc = static_cast<std::function<void()>*>(args);
		(*pfunc)();
		delete pfunc;
	}
	Fiber create_fiber_impl(std::function<void()> func)
	{
		Fiber fiber;
		// We need a pointer that will live until the fiber is switched to
		std::function<void()>* pfunc = new std::function<void()>();
		*pfunc = func;
		fiber.address = CreateFiber(0, (LPFIBER_START_ROUTINE)fiber_starter, pfunc);
		return fiber;
	}

	Fiber convert_thread_to_fiber()
	{
		Fiber fiber;
		fiber.address = ConvertThreadToFiber(nullptr);
		return fiber;
	}
	void switch_to_fiber(Fiber& fiber)
	{
		SwitchToFiber(fiber.address);
	}

	void delete_fiber(Fiber& fiber)
	{
		DeleteFiber(fiber.address);
	}
	Fiber get_current_fiber()
	{
		Fiber fiber;
		fiber.address = GetCurrentFiber();
		return fiber;
	}
}

