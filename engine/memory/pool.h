#pragma once
#include <vector>

class BasePool
{
public:
	BasePool(std::uint32_t element_size, std::uint32_t elements_per_block = 8192) : _capacity(0), _size(0), _elements_per_block(elements_per_block), _element_size(element_size){}
	virtual ~BasePool();
	virtual void* operator[](const int index);
	
	void* reserve(const int index);
private:
	std::uint32_t _capacity;
	std::uint32_t _size;
	std::uint32_t _elements_per_block;
	std::uint32_t _element_size;
	void grow(const int index);

	std::vector<char*> _memory_blocks;
};
template<typename type>
class Pool : BasePool
{
public:
	Pool(std::uint32_t elements_per_block = 8192) : BasePool(sizeof(type), elements_per_block){}
	void destroy(const int index);
	type* operator[](const int index) override
	{
		return static_cast<type*>(operator[](index));
	}
};


inline BasePool::~BasePool() 
{
	for (auto block : _memory_blocks)
		delete[] block;
	_memory_blocks.clear();
	
}

inline void* BasePool::operator[](const int index)
{
	int block_index = index / _elements_per_block;
	int index_in_block = index%_elements_per_block;
	return _memory_blocks[block_index] + index_in_block*_element_size;
}

template <typename type>
void Pool<type>::destroy(const int index) 
{
	operator[](index)->~type();
}


inline void* BasePool::reserve(const int index) 
{
	if(index>_size) 
	{
		if (index > _capacity) grow(index);
		_size = index;
	}
	return operator[](index);
}

inline void BasePool::grow(const int index) 
{
	while(_capacity<index) 
	{
		_memory_blocks.push_back(new char[_elements_per_block*_element_size]);
		_capacity += _elements_per_block;
	}
}
