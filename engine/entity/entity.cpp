#include <entity/entity.h>

namespace kth
{
	uint32 BaseComponent::_index_counter = 0;

	Entity EntityManager::create_empty_entity()
	{
		uint32 entity = allocate_entity();
		_entity_masks[entity].reset();
		return Entity(entity, _entity_versions[entity], this);
	}

	
	Entity::Id EntityManager::next_entity(Entity::Id id)
	{
		for (uint32 i = id.id + 1; i < _entity_masks.size(); ++i)
		{
			if (!(_entity_versions[i] & 0x8000))
			{
				return Entity::Id(i, _entity_versions[i]);
			}
		}
		return Entity::Id(-1, -1);
	}

	Entity::Id EntityManager::next_entity(std::bitset<MAX_COMPONENTS> comp_mask, Entity::Id id)
	{
		for (uint32 i = id.id+1; i < _entity_masks.size(); ++i)
		{
			if ((_entity_masks[i] & comp_mask) == comp_mask)
			{
				return Entity::Id(i, _entity_versions[i]);
			}
		}

		return Entity::Id(-1, -1);
	}

	uint32 EntityManager::allocate_entity(bool force_at_end)
	{
		if(!force_at_end && _free_slots.size())
		{
			uint32 index = _free_slots.back();
			_free_slots.pop_back();
			// Unset last bit to indicate valid entity
			_entity_versions[index] = _entity_versions[index] & 0x7FFF;
			return index;
		}

		_entity_masks.resize(_free_index+1);
		_entity_versions.resize(_free_index+1);
	
		_entity_versions[_free_index] = 0;

		return _free_index++;
	}

}