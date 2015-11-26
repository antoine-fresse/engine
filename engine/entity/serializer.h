#pragma once
#include "entity.h"
#include <cereal/types/vector.hpp>
#include <cereal/archives/json.hpp>



struct EntityInfo
{
	std::vector<std::string> components;
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(CEREAL_NVP(components));
	}
};

template<class Archive>
void save(Archive & archive, Entity const & m)
{
	auto mask = m.mask();
	EntityInfo info;
	for (int i = 0; i < MAX_COMPONENTS; ++i)
	{
		if (mask[i])
		{
			info.components.push_back(m.manager()->get_component_name(i));
		}
	}

	archive(info);
}

template<class Archive>
void load(Archive & archive, Entity & m)
{
	EntityInfo entity_info;
	archive(CEREAL_NVP(entity_info));
}
