#include <thread/thread.h>

#include <windows.h>
namespace std
{
	namespace this_thread
	{
		void set_affinity(uint32 core)
		{
			SetThreadAffinityMask(GetCurrentThread(), 1<<core);
		}
	}
}