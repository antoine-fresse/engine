#include "fiber.h"
#include "windows.h"


namespace kth
{
	
	static void fiber_starter(void* args)
	{
		(*static_cast<std::function<void(void)>*>(args))();
	}
	Fiber create_fiber(std::function<void(void)> func)
	{
		Fiber fiber;
		fiber.address = CreateFiber(0, (LPFIBER_START_ROUTINE)fiber_starter, &func);
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

