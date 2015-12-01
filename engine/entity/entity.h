#pragma once

#include <utils.h>
#include <memory/pool.h>
#include <bitset>
#include <map>
#include <functional>


#define MAX_COMPONENTS 64

#define RegisterComponent(Component, EM) (EM)->register_component<Component>(#Component)

namespace kth
{
	template<typename type>
	class Component;
	class EntityManager;

	class Entity
	{
	public:

		struct Id
		{
			Id() = default;
			Id(uint32 id, uint32 version) : id(id), version(version) {}
			union
			{
				struct {
					uint32 id;
					uint32 version;
				};
				int64_t full_id;
			};
		};

		Entity() : _id(0, 0), _entity_manager(nullptr){}
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

		template <typename type>
		type* get_component() const;

		void destroy() const;

		bool valid() const;


		Id id() const { return _id; }
		EntityManager* manager() const { return _entity_manager; }

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
			_component_name_lookup[component_name] = index<type>();
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

		template <typename type>
		type* get_component(Entity::Id entity) const;


		template<typename ... Types>
		bool has_components(Entity::Id entity) const
		{
			auto comp_mask = components_mask<Types>();
			if ((mask(entity) & comp_mask) == comp_mask)
				return true;
			return false;
		}

		void for_each(std::function<void(Entity entity)> func);

		template <typename T, typename ... Types>
		void for_each(typename std::identity<std::function<void(Entity, T*, Types*...)>>::type func);

		std::bitset<MAX_COMPONENTS> mask(Entity::Id entity) const;

		std::string get_component_name(uint32 index) const { return _component_name[index]; }

		template <typename type>
		uint32 index() const
		{
			return Component<std::remove_const<type>>::index();
		}

		Entity::Id next_entity(std::bitset<MAX_COMPONENTS> comp_mask, Entity::Id id);
		Entity::Id next_entity(Entity::Id id);

	protected:
		BasePool* _component_pools[MAX_COMPONENTS];
		std::string _component_name[MAX_COMPONENTS];
		std::map<std::string, uint32> _component_name_lookup;

		std::vector<std::bitset<MAX_COMPONENTS>> _entity_masks;
		std::vector<uint32> _entity_versions;

		uint32 _free_index;
		std::vector<uint32> _free_slots;

		uint32 allocate_entity(bool force_at_end = false);

		template<typename First>
		std::bitset<MAX_COMPONENTS> components_mask() const
		{
			auto cmask = std::bitset<MAX_COMPONENTS>();
			cmask[index<First>()] = true;
			return cmask;
		}

		template<typename First, typename Second, typename ... Rest>
		std::bitset<MAX_COMPONENTS> components_mask() const
		{
			auto cmask = components_mask<Second, Rest...>();
			cmask[index<First>()] = true;
			return cmask;
		}

	};


	class EntityIterator : public std::iterator<std::forward_iterator_tag, Entity, ptrdiff_t, Entity, Entity>
	{
	public:
		EntityIterator(EntityManager* manager, std::bitset<MAX_COMPONENTS> mask) : _manager(manager), _mask(mask) {};
		EntityIterator(EntityManager* manager, std::bitset<MAX_COMPONENTS> mask, Entity::Id id) : _id(id), _manager(manager), _mask(mask) {};
		
		Entity operator*() const
		{
			return Entity(_id.id, _id.version, _manager);
		}

		Entity operator->() const
		{
			return Entity(_id.id, _id.version, _manager);
		}

		EntityIterator& operator++()
		{
			next();
			return *this;
		};

		EntityIterator operator++(int)
		{
			EntityIterator it(*this);
			next();
			return it;
		};

		bool operator==(const EntityIterator& other) const {
			return _id.full_id == other._id.full_id && _manager == other._manager && _mask == other._mask;
		}

		bool operator!=(const EntityIterator& other) const {
			return _id.full_id != other._id.full_id || _manager != other._manager || _mask != other._mask;
		}

	private:
		void next()
		{
			_id = _manager->next_entity(_mask, _id);
		}

		Entity::Id _id;
		EntityManager* _manager;
		std::bitset<MAX_COMPONENTS> _mask;
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
		// Set last bit to indicate destroyed entity
		_entity_versions[entity.id] = (_entity_versions[entity.id] + 1) | 0x8000;
		_free_slots.push_back(entity.id);
	}



	inline std::bitset<MAX_COMPONENTS> EntityManager::mask(Entity::Id entity) const
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

	template <typename type>
	type* EntityManager::get_component(Entity::Id entity) const
	{
		ASSERT(valid(entity));
		auto& entity_mask = _entity_masks[entity.id];
		int component_index = index<type>();
		if (!entity_mask[component_index]) return nullptr;
		Pool<type>* pool = static_cast<Pool<type>*>(_component_pools[component_index]);
		return pool->get(entity.id);
	}


	inline void EntityManager::for_each(std::function<void(Entity entity)> func)
	{
		for (uint32 i = 0; i < _entity_masks.size(); ++i)
		{
			if(!(_entity_versions[i] & 0x8000))
			{
				auto ent = Entity(i, _entity_versions[i], this);
				func(ent);
			}
		}
	}

	template <typename T, typename ... Types>
	void EntityManager::for_each(typename std::identity<std::function<void(Entity, T*, Types*...)>>::type func)
	{
		auto comp_mask = components_mask<T, Types...>();


		EntityIterator it(this, comp_mask, {(uint32)-1, 0});
		++it;

		EntityIterator end(this, comp_mask, { (uint32)-1, (uint32)-1 });
		for (;it != end; ++it)
		{
			auto ent = *it;
			func(ent, get_component<T>(ent.id()), get_component<Types>(ent.id())...);
		}
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

	template <typename type>
	type* Entity::get_component() const
	{
		return _entity_manager->get_component<type>(_id);
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

}