#include <entity/entity.h>

uint32 BaseComponent::_index_counter = 0;

Entity EntityManager::create_empty_entity()
{
	uint32 entity = allocate_entity();
	_entity_masks[entity].reset();
	return Entity(entity, _entity_versions[entity], this);
}

void* EntityManager::add_component(Entity::Id entity, const std::string& component_name)
{
	ASSERT(valid(entity));
	auto& entity_mask = _entity_masks[entity.id];
	auto itfind = _component_name_lookup.find(component_name);
	if (itfind == _component_name_lookup.end()) return nullptr;
	int component_index = itfind->second;

	if (entity_mask[component_index]) return _component_pools[component_index]->get_element(entity.id);

	BasePool* pool = _component_pools[component_index];
	pool->reserve(entity.id);
	void* component = pool->get_element(entity.id);

	pool->default_constuct(entity.id);
	entity_mask.set(component_index, true);
	return component;
}

void EntityManager::remove_component(Entity::Id entity, const std::string& component_name)
{
	ASSERT(valid(entity));
	auto& entity_mask = _entity_masks[entity.id];
	auto itfind = _component_name_lookup.find(component_name);
	if (itfind == _component_name_lookup.end()) return;
	int component_index = itfind->second;
	if (!entity_mask[component_index]) return;

	BasePool* pool = _component_pools[component_index];
	pool->destroy(entity.id);

	entity_mask.set(component_index, false);
}

bool EntityManager::has_component(Entity::Id entity, const std::string& component_name) const
{
	ASSERT(valid(entity));
	auto& entity_mask = _entity_masks[entity.id];
	auto itfind = _component_name_lookup.find(component_name);
	if (itfind == _component_name_lookup.end()) return false;
	return true;
}

uint32 EntityManager::allocate_entity()
{
	if(_free_slots.size())
	{
		uint32 index = _free_slots.back();
		_free_slots.pop_back();
		return index;
	}

	_entity_masks.resize(_free_index+1);
	_entity_versions.resize(_free_index+1);
	
	_entity_versions[_free_index] = 0;

	return _free_index++;
}
