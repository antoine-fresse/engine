#pragma once

#include <functional>

namespace kth
{
	struct Fiber
	{
		Fiber() : address(nullptr){}
		Fiber(void* address) : address(address) {}
		void* address;
	};

	Fiber create_fiber(std::function<void(void)> func);
	Fiber convert_thread_to_fiber();
	void switch_to_fiber(Fiber& fiber);
	void delete_fiber(Fiber& fiber);
	Fiber get_current_fiber();
	

}