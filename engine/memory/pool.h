#pragma once
#include <vector>

namespace kth
{
	

	class BasePool
	{
	public:
		BasePool(uint32_t element_size, uint32_t elements_per_block = 8192) : _capacity(0), _size(0), _elements_per_block(elements_per_block), _element_size(element_size){}
		virtual ~BasePool();
		void* get_element(const uint32 index);
		virtual void destroy(const uint32 index) = 0;
		virtual void default_constuct(const uint32 index) = 0;
		void reserve(const uint32 index);

		uint32_t size_per_element() const
		{
			return _element_size;
		}

	private:
		uint32_t _capacity;
		uint32_t _size;
		uint32_t _elements_per_block;
		uint32_t _element_size;
		void grow(const uint32 index);
	
		std::vector<char*> _memory_blocks;
	};
	template<typename type>
	class Pool : public BasePool
	{
	public:
		Pool(uint32_t elements_per_block = 8192) : BasePool(sizeof(type), elements_per_block){}
		void destroy(const uint32 index) override;
		void default_constuct(const uint32 index) override;

		template <class ... ArgsType>
		void construct(const uint32 index, ArgsType&& ... args);

		type* get(const uint32 index)
		{
			return static_cast<type*>(get_element(index));
		}
	};


	inline BasePool::~BasePool() 
	{
		for (auto block : _memory_blocks)
			delete[] block;
		_memory_blocks.clear();
	
	}

	inline void* BasePool::get_element(const uint32 index)
	{
		int block_index = index / _elements_per_block;
		int index_in_block = index%_elements_per_block;
		return _memory_blocks[block_index] + index_in_block*_element_size;
	}

	template <typename type>
	void Pool<type>::destroy(const uint32 index)
	{
		get(index)->~type();
	}

	template <typename type>
	void Pool<type>::default_constuct(const uint32 index)
	{
		::new(get(index)) type();
	}

	template <typename type>
	template <class ... ArgsType>
	void Pool<type>::construct(const uint32 index, ArgsType&& ... args)
	{
		::new(get(index)) type(std::forward<ArgsType>(args)...);
	}

	inline void BasePool::reserve(const uint32 index) 
	{
		if(index>=_size) 
		{
			if (index >= _capacity) grow(index);
			_size = index;
		}
	}

	inline void BasePool::grow(const uint32 index)
	{
		while(_capacity<=index) 
		{
			_memory_blocks.push_back(new char[_elements_per_block*_element_size]);
			_capacity += _elements_per_block;
		}
	}

}