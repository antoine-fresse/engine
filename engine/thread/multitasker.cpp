#include <thread/multitasker.h>
#include <thread/thread.h>

namespace kth
{

	thread_local uint32 Multitasker::thread_id;
	thread_local Fiber Multitasker::_fiber_switching_fiber_origin;
	thread_local Fiber Multitasker::_fiber_switching_fiber_destination;
	thread_local Fiber Multitasker::_waiting_counter_fiber_origin;
	thread_local Fiber Multitasker::_waiting_counter_fiber_destination;
	thread_local std::shared_ptr<AtomicCounter> Multitasker::_waiting_counter_fiber_counter;
	thread_local int Multitasker::_waiting_counter_fiber_counter_target;
	

	Multitasker::Multitasker(uint32 worker_threads, uint32 fiber_pool_size)
	{
		_stop = false;
		for (uint32 i = 0; i < fiber_pool_size; ++i)
		{
			_fiber_pool.enqueue(create_fiber(std::bind(&Multitasker::process_tasks, this)));
		}
		
		std::this_thread::set_affinity(0);
		convert_thread_to_fiber();
		thread_id = 0;
		_fiber_switching_fibers.resize(worker_threads);
		_waiting_counter_fibers.resize(worker_threads);

		_fiber_switching_fibers[thread_id] = create_fiber(std::bind(&Multitasker::fiber_switching_fiber_routine, this));
		_waiting_counter_fibers[thread_id] = create_fiber(std::bind(&Multitasker::waiting_counter_fiber_routine, this));

		for (uint32 i = 1; i < worker_threads; ++i)
		{
			std::thread(std::bind(&Multitasker::init_worker, this, i));
		}
	}

	void Multitasker::init_worker(uint32 worker_id)
	{
		thread_id = worker_id;
		convert_thread_to_fiber();

		std::this_thread::set_affinity(worker_id);

		_fiber_switching_fibers[thread_id] = create_fiber(std::bind(&Multitasker::fiber_switching_fiber_routine, this));
		_waiting_counter_fibers[thread_id] = create_fiber(std::bind(&Multitasker::waiting_counter_fiber_routine, this));

		process_tasks();
	}

	void Multitasker::fiber_switching_fiber_routine()
	{
		for (;;)
		{
			_fiber_pool.enqueue(_fiber_switching_fiber_origin);
			switch_to_fiber(_fiber_switching_fiber_destination);
		}
	}

	void Multitasker::waiting_counter_fiber_routine()
	{
		for (;;)
		{
			
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_waiting_tasks.emplace_back(_waiting_counter_fiber_origin, _waiting_counter_fiber_counter, _waiting_counter_fiber_counter_target);
			}
			switch_to_fiber(_waiting_counter_fiber_destination);
		}

	}

	void Multitasker::process_tasks()
	{
		uint32 id = thread_id;
		while (!_stop.load())
		{
			std::unique_lock<std::mutex> lock(_mutex);
			if(_waiting_tasks.size())
			{
				for (auto it = _waiting_tasks.begin(); it != _waiting_tasks.end(); ++it)
				{
					if(it->counter->load() == it->target)
					{
						_fiber_switching_fiber_destination = it->fiber;
						_fiber_switching_fiber_origin = get_current_fiber();

						if (it != --_waiting_tasks.end())
							*it = std::move(_waiting_tasks.back());
						_waiting_tasks.pop_back();

						lock.release();
						switch_to_fiber(_fiber_switching_fibers[id]);
						lock.lock();
						break;
					}
				}
			}
			lock.release();

			Task task;
			if(_task_queue.try_dequeue(task))
			{
				task.function();
				task.counter->fetch_sub(1);
			}
			else
			{
				std::this_thread::yield();
			}

			
		}


	}

	void Multitasker::wait_for(std::shared_ptr<AtomicCounter>& counter, int value)
	{
		if (counter->load() == value) return;

		auto id = thread_id;

		Fiber fiber;
		_fiber_pool.wait_dequeue(fiber);
		_waiting_counter_fiber_destination = fiber;
		_waiting_counter_fiber_origin = get_current_fiber();
		_waiting_counter_fiber_counter = counter;
		_waiting_counter_fiber_counter_target = value;

		switch_to_fiber(_waiting_counter_fibers[id]);
	}
}
