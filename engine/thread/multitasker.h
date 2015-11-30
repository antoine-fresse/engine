#pragma once
#include <utils.h>
#include <atomic>
#include <condition_variable>
#include <thread/fiber.h>
#include <vector>
#include <concurrentqueue/blockingconcurrentqueue.h>

/*
Fiber types : 

fiber pool : worker loop, pop a task, run it and decrease its counter
fiber_switching_fibers : loop : enqueue origin fiber (from tls) back in fiber pool then switch to target fiber (from tls) (use when restoring a waiting fiber in worker loop)
waiting_counter_fiber : loop : enqueue origin fiber (from tls) in waiting fiber map then switch to target fiber (from tls, one of the worker from pool)

*/
namespace kth
{

	
	typedef std::atomic<int> AtomicCounter;

	struct Task
	{
		Task(){}
		Task(std::function<void(void)> func, std::shared_ptr<AtomicCounter> counter) : function(func), counter(counter) {}

		std::function<void(void)> function;
		std::shared_ptr<AtomicCounter> counter;
	};

	struct WaitingTask
	{
		WaitingTask() : counter(nullptr), target(0){}
		WaitingTask(Fiber fiber, std::shared_ptr<AtomicCounter> counter, int target) : fiber(fiber), counter(counter), target(target) {}

		Fiber fiber;
		std::shared_ptr<AtomicCounter> counter;
		int target;
	};

	class Multitasker
	{
	public:
		Multitasker(uint32 worker_threads, uint32 fiber_pool_size);
		
		void init_worker(uint32 worker_id);
		void fiber_switching_fiber_routine();
		void waiting_counter_fiber_routine();
		void process_tasks();

		template<class Fn, class... Args>
		std::shared_ptr<AtomicCounter> enqueue(Fn&& f, Args&& ... args);

		template<int N>
		std::shared_ptr<AtomicCounter> enqueue(std::function<void(void)>(&tasks)[N]);
		std::shared_ptr<AtomicCounter> enqueue(std::function<void(void)>* tasks, int n);

		template<class Fn, class... Args>
		std::function<void(void)> make_task(Fn&& f, Args&& ... args)
		{
			return std::bind(f, std::forward<Args>(args)...);
		}

		void wait_for(std::shared_ptr<AtomicCounter>& counter, int value);

		void stop() { _stop = true; }

	protected:
		static thread_local uint32 thread_id;
		std::mutex _mutex;
		std::vector<Fiber> _fiber_switching_fibers;
		static thread_local Fiber _fiber_switching_fiber_origin;
		static thread_local Fiber _fiber_switching_fiber_destination;

		std::vector<Fiber> _waiting_counter_fibers;
		static thread_local Fiber _waiting_counter_fiber_origin;
		static thread_local Fiber _waiting_counter_fiber_destination;
		static thread_local std::shared_ptr<AtomicCounter> _waiting_counter_fiber_counter;
		static thread_local int _waiting_counter_fiber_counter_target;

		moodycamel::BlockingConcurrentQueue<Fiber> _fiber_pool;
		std::vector<WaitingTask> _waiting_tasks;
		moodycamel::ConcurrentQueue<Task> _task_queue;

		std::atomic<bool> _stop;

	};

	template <class Fn, class ... Args>
	std::shared_ptr<AtomicCounter> Multitasker::enqueue(Fn&& f, Args&& ... args)
	{ 
		std::shared_ptr<AtomicCounter> counter = std::make_shared<AtomicCounter>(1);
		_task_queue.enqueue(Task(std::bind(f, std::forward<Args>(args)...), counter));
		return counter;
	}

	template<int N>
	std::shared_ptr<AtomicCounter> Multitasker::enqueue(std::function<void(void)> (&tasks)[N])
	{
		std::shared_ptr<AtomicCounter> counter = std::make_shared<AtomicCounter>(N);

		for (int i = 0; i < N; ++i)
		{
			_task_queue.enqueue(Task(tasks[i], counter));
		}
		return counter;
	}

	inline std::shared_ptr<AtomicCounter> Multitasker::enqueue(std::function<void(void)>* tasks, int n)
	{
		std::shared_ptr<AtomicCounter> counter = std::make_shared<AtomicCounter>(n);

		for (int i = 0; i < n; ++i)
		{
			_task_queue.enqueue(Task(tasks[i], counter));
		}
		return counter;
	}

}
