#pragma once

#include <utils.h>
#include <memory/pool.h>
#include <bitset>
#include <map>

#define MAX_COMPONENTS 64

#define RegisterComponent(Component, EM) (EM)->register_component<Component>(#Component)


template<typename type>
class Component;
class EntityManager;

class Entity
{
public:

	struct Id
	{
		Id() = default;
		Id(uint32 id, uint32 version) : id(id), version(version){}
		union
		{
			struct {
				uint32 id;
				uint32 version;
			};
			int64_t full_id;
		};
	};

	Entity(uint32 id, uint32 version, EntityManager* manager) : _id(id, version), _entity_manager(manager) {}


	template<typename type>
	bool has_component() const;
	bool has_component(const std::string& component_name) const;

	template<typename type, class ... ArgsTypes>
	type* add_component(ArgsTypes && ... args);
	void* add_component(const std::string& componenent_name) const;

	template<typename type>
	void remove_component();
	void remove_component(const std::string& component_name) const;

	void destroy() const;

	bool valid() const;
	
	Id id() const { return _id; }

	std::bitset<MAX_COMPONENTS> mask() const;

private:
	
	Id _id;
	EntityManager* _entity_manager;
};



class EntityManager
{
public:
	EntityManager() : _free_index(0){}

	template<typename type>
	void register_component(const char* component_name)
	{
		_component_name[index<type>()] = component_name;
		_component_name_lookup[component_name] = Component<type>::index();
		_component_pools[index<type>()] = new Pool<type>();
	}

	Entity create_empty_entity();

	bool valid(Entity::Id entity) const
	{
		return entity.id < _entity_versions.size() && _entity_versions[entity.id] == entity.version;
	}

	void destroy(Entity::Id entity);
	
	template <typename type, class ... Types>
	type* add_component(Entity::Id entity, Types&& ... args);
	void* add_component(Entity::Id entity, const std::string& component_name);

	template <typename type>
	void  remove_component(Entity::Id entity);
	void  remove_component(Entity::Id entity, const std::string& component_name);

	template <typename type>
	bool has_component(Entity::Id entity) const;
	bool has_component(Entity::Id entity, const std::string& component_name) const;

	std::bitset<MAX_COMPONENTS> mask(Entity::Id entity);

	template <typename type>
	uint32 index()
	{
		return Component<std::remove_const<type>>::index();
	}
	
protected:
	BasePool* _component_pools[MAX_COMPONENTS];
	std::string _component_name[MAX_COMPONENTS];
	std::map<std::string, uint32> _component_name_lookup;

	std::vector<std::bitset<MAX_COMPONENTS>> _entity_masks;
	std::vector<uint32> _entity_versions;

	uint32 _free_index;
	std::vector<uint32> _free_slots;

	uint32 allocate_entity();
};



class BaseComponent
{
public:

	
protected:

	static uint32 _index_counter;
};



template<typename type>
class Component : public BaseComponent
{
public:
	static uint32 index()
	{
		static uint32 index = _index_counter++;
		ASSERT(index < MAX_COMPONENTS);
		return index;
	}
};

inline bool Entity::valid() const
{
	return _entity_manager && _entity_manager->valid(_id);
}


inline void EntityManager::destroy(Entity::Id entity)
{
	ASSERT(valid(entity));
	auto& entity_mask = _entity_masks[entity.id];
	for (int c_index = 0; c_index < MAX_COMPONENTS; ++c_index)
	{
		if(entity_mask[c_index])
		{
			_component_pools[c_index]->destroy(entity.id);
		}
	}
	entity_mask.reset();
	++_entity_versions[entity.id];
	_free_slots.push_back(entity.id);
}



inline std::bitset<MAX_COMPONENTS> EntityManager::mask(Entity::Id entity)
{
	ASSERT(valid(entity));
	return _entity_masks[entity.id];
}


template <typename type, class ... Types>
type* EntityManager::add_component(Entity::Id entity, Types&& ... args)
{
	ASSERT(valid(entity));
	auto& entity_mask = _entity_masks[entity.id];
	int component_index = index<type>();
	if (entity_mask[component_index]) return static_cast<Pool<type>*>(_component_pools[component_index])->get(entity.id);
	
	Pool<type>* pool = static_cast<Pool<type>*>(_component_pools[component_index]);
	pool->reserve(entity.id);
	type* component = pool->get(entity.id);
	
	::new (component) type(std::forward<Types>(args) ...);
	entity_mask.set(component_index, true);

	return component;
}



template <typename type>
void EntityManager::remove_component(Entity::Id entity)
{
	ASSERT(valid(entity));
	auto& entity_mask = _entity_masks[entity.id];
	int component_index = index<type>();
	if (!entity_mask[component_index]) return;

	Pool<type>* pool = static_cast<Pool<type>*>(_component_pools[component_index]);
	pool->destroy(entity.id);

	entity_mask.set(component_index, false);
}

template <typename type>
bool EntityManager::has_component(Entity::Id entity) const
{
	ASSERT(valid(entity));
	auto& entity_mask = _entity_masks[entity.id];
	int component_index = index<type>();
	return entity_mask[component_index];
}

/*
*	Entity
*/

template<typename type>
inline bool Entity::has_component() const
{
	return _entity_manager->has_component<type>(_id);
};

template<typename type, class ... ArgsTypes>
inline type* Entity::add_component(ArgsTypes && ... args)
{
	return _entity_manager->add_component<type>(_id, std::forward<ArgsTypes>(args) ...);
}

inline bool Entity::has_component(const std::string& component_name) const
{
	return _entity_manager->has_component(_id, component_name);
}

inline void* Entity::add_component(const std::string& componenent_name) const
{
	return _entity_manager->add_component(_id, componenent_name);
}

template<typename type>
inline void Entity::remove_component()
{
	_entity_manager->remove_component<type>(_id);
}

inline void Entity::remove_component(const std::string& component_name) const
{
	_entity_manager->remove_component(_id, component_name);
}

inline void Entity::destroy() const
{
	_entity_manager->destroy(_id);
}

inline std::bitset<MAX_COMPONENTS> Entity::mask() const
{
	return _entity_manager->mask(_id);
}