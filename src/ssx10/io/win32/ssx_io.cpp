#include "ssx_io.h"

#include <winioctl.h>
#include <setupapi.h>

namespace ssx {
	namespace io {
		namespace win32 {
			// {CC1ED238-B6E4-4010-99AC-BA13239651C8}
			static const GUID SSX_DEVICE_GUID = { 0xcc1ed238, 0xb6e4, 0x4010,{ 0x99, 0xac, 0xba, 0x13, 0x23, 0x96, 0x51, 0xc8 } };

			static std::vector<std::string> search_devices(const GUID *guid,int start_index,int end_index)//枚举设备的接口的具体实现代码
			{
				std::vector<std::string> path_list;
				//QString DevicePath = "";
				HDEVINFO info = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
				if (info != INVALID_HANDLE_VALUE)
				{
					SP_INTERFACE_DEVICE_DATA ifdata;
					ifdata.cbSize = sizeof(ifdata);
					DWORD devindex;
					for (devindex = start_index;devindex < end_index;  ++devindex)
					{
						if (!SetupDiEnumDeviceInterfaces(info, NULL, guid, devindex, &ifdata)) {
							continue;
						}
						DWORD needed;
						SetupDiGetDeviceInterfaceDetail(info, &ifdata, NULL, 0, &needed, NULL);
						PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(needed);
						detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
						SP_DEVINFO_DATA did;
						memset(&did, 0, sizeof(SP_DEVINFO_DATA));
						did.cbSize = sizeof(SP_DEVINFO_DATA);
						SetupDiGetDeviceInterfaceDetail(info, &ifdata, detail, needed, NULL, &did);
						path_list.push_back(std::string(detail->DevicePath));
						//QString path = QString::fromWCharArray(detail->DevicePath);
						free((PVOID)detail);
						/*
						if ((path.indexOf(QString("dev_%1").arg(pid, 4, 16, QChar('0'))) >= 0) && (path.indexOf(QString("ven_%1").arg(vid, 4, 16, QChar('0'))) >= 0))
						{
							DevicePath = path;
							break;
						}
						*/
					}
					SetupDiDestroyDeviceInfoList(info);
				}

				return path_list;
			}
			static int DeviceExecute(HANDLE hDevice, unsigned char *buf, unsigned int size)
			{
				DWORD dwRead;
				OVERLAPPED overlap;
				memset(&overlap, 0, sizeof(overlap));
				overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
				if (WriteFile(hDevice, buf, (DWORD)size, &dwRead, &overlap)
					|| GetOverlappedResult(hDevice, &overlap, &dwRead, TRUE))
				{
					CloseHandle(overlap.hEvent);
					return dwRead;
				}

				CloseHandle(overlap.hEvent);
				return -1;
			}

			static int DeviceExecuteV2(HANDLE hDevice, unsigned char *buf, unsigned int size,unsigned int timeout_ms)
			{
				DWORD dwRead;
				OVERLAPPED overlap;
				memset(&overlap, 0, sizeof(overlap));
				overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
				HandleWrapper evtHandle(overlap.hEvent);
				if (WriteFile(hDevice, buf, (DWORD)size, &dwRead, &overlap)) {
					return dwRead;
				}
				int waitStat = WaitForSingleObject(overlap.hEvent, timeout_ms);
				if (WAIT_OBJECT_0 != waitStat) {
					return -1;
				}
				if (GetOverlappedResult(hDevice, &overlap, &dwRead, TRUE)) {
					return dwRead;
				}
				else {
					return -1;
				}
			}
			//////////////////////////////////////////////////////////////////////////

			HandleWrapper::HandleWrapper(HANDLE handle)
				:fd(handle)
			{
			}
			HandleWrapper::~HandleWrapper()
			{
				if (isValid()) {
					close_device(fd);
					fd = INVALID_HANDLE_VALUE;
				}
			}

			bool HandleWrapper::isInvalidHandle()
			{
				return fd == INVALID_HANDLE_VALUE;
			}

			bool HandleWrapper::isNull()
			{
				return fd == NULL;
			}

			bool HandleWrapper::isValid()
			{
				return (!isNull()) && (!isInvalidHandle());
			}

			HANDLE HandleWrapper::getHandle()
			{
				return fd;
			}

			std::vector<std::string> search_ssx_devices(int start_index, int end_index) {
				return search_devices(&SSX_DEVICE_GUID, start_index, end_index);
			}

			HANDLE open_device(const std::string& path) {

				return CreateFile(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL,
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
			}
			bool close_device(HANDLE fd) {
				return ::CloseHandle(fd);
			}
			int exchange_data(HANDLE fd, void* buffer, size_t buffer_size) {
				return DeviceExecute(fd,(unsigned char*) buffer,buffer_size);
			}
			int exchange_data(HANDLE fd, const void* buffer, size_t buffer_size, unsigned int timeout_ms) {
				return DeviceExecuteV2(fd, (unsigned char*)buffer, buffer_size, timeout_ms);
			}
		}
	}
}