#pragma once
#include <entity/entity.h>
#include <chrono>

namespace kth
{
	template<typename ... ComponentTypes>
	class System
	{
	public:
		System() = delete;

		explicit System(EntityManager& manager) : _manager(manager)
		{
			_mask = _manager.components_mask<ComponentTypes...>();
		}

		virtual void update(std::chrono::duration<double> dt) = 0;

		virtual ~System()
		{
		}

	protected:
		std::bitset<MAX_COMPONENTS> _mask;
		EntityManager& _manager;
	};

}