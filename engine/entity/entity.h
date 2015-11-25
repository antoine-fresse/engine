#pragma once

#include <utils.h>
#include <memory/pool.h>
#include <bitset>

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
		Id(uint32_t id, uint32_t version) : id(id), version(version){}
		union
		{
			struct {
				uint32_t id;
				uint32_t version;
			};
			int64_t full_id;
		};
		
	};

	Entity(uint32_t id, uint32_t version, EntityManager* manager) : _id(id, version), _entity_manager(manager) {}


	template<typename type>
	bool has_component() const;

	template<typename type>
	bool add_component();

	template<typename type>
	bool remove_component();

	void destroy();

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
	EntityManager(){}

	template<typename type>
	void register_component(const char* component_name)
	{
		_component_name[Component<type>::index()] = component_name;
	}

	Entity create_entity();

	bool valid(Entity::Id entity)
	{
		return entity.id < _entity_versions.size() && _entity_versions[entity.id] == entity.version;
	}

	void destroy(uint32_t id)
	{
		
	}

	std::bitset<MAX_COMPONENTS> mask(Entity::Id entity);

private:
	BasePool* _component_pools[MAX_COMPONENTS];
	std::string _component_name[MAX_COMPONENTS];

	std::vector<std::bitset<MAX_COMPONENTS>> _entity_masks;
	std::vector<uint32_t> _entity_versions;
};



class BaseComponent
{
public:

	
protected:

	static uint32_t _index_counter;
};

template<typename type>
class Component : public BaseComponent
{
public:
	static uint32_t index()
	{
		static uint32_t index = _index_counter++;
		ASSERT(index < MAX_COMPONENTS);
		return index;
	}
};

inline bool Entity::valid() const
{
	return _entity_manager && _entity_manager->valid(_id);
}

inline std::bitset<MAX_COMPONENTS> Entity::mask() const
{
	return _entity_manager->mask(_id);
}

inline std::bitset<MAX_COMPONENTS> EntityManager::mask(Entity::Id entity)
{
	return _entity_masks[entity.id];
}

template <typename type>
bool Entity::has_component() const
{
	int index = Component<type>::index();
	return _entity_manager && mask()[index];
}
