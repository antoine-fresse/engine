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

	Fiber create_fiber_impl(std::function<void()> func);

	template<class Fn, class ... Args>
	Fiber create_fiber(Fn&& func, Args&& ... args)
	{
		return create_fiber_impl(std::bind(func, std::forward<Args>(args)...));
	}

	Fiber convert_thread_to_fiber();
	void switch_to_fiber(Fiber& fiber);
	void delete_fiber(Fiber& fiber);
	Fiber get_current_fiber();
	

}