#ifndef SSX_IO_WIN32_SSX_IO_H
#define SSX_IO_WIN32_SSX_IO_H

#include <string>
#include <vector>

#include <Windows.h>

namespace ssx {
	namespace io {
		namespace win32 {
			class HandleWrapper {
			private:
				HANDLE fd;
			public:
				HandleWrapper(HANDLE handle);
				~HandleWrapper();
				bool isInvalidHandle();
				bool isNull();
				bool isValid();
				HANDLE getHandle();
			};

			std::vector<std::string> search_ssx_devices(int start_index, int end_index);
			HANDLE open_device(const std::string& path);
			bool close_device(HANDLE fd);
			int exchange_data(HANDLE fd, void* buffer, size_t buffer_size);
			int exchange_data(HANDLE fd, void* buffer, size_t buffer_size,unsigned int timeout_ms);
		}
	}
}
#endif