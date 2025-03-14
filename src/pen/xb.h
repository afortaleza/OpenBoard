#pragma once
#include <string.h>
#include <iostream>
#include <vector>
enum PEN_EVENT_TYPE :int
{
    PEN_EVENT_Undefine,
    PEN_EVENT_Dot,
    PEN_EVENT_GetVersion,
	PEN_EVENT_GetBattery,
	PEN_EVENT_GetStorageSize,
	PEN_EVENT_ClearStorage,
	PEN_EVENT_GetDotsCount,
	PEN_EVENT_GetDots,
	PEN_EVENT_CmdTimeout
};

enum BLE_EVENT_TYPE :int
{
	BLE_EVENT_STATUS,
	BLE_EVENT_FIND_DEVICE
};
enum BLE_STATUS_TYPE :int
{
	PEN_CONNECTION_TRY,
	PEN_CONNECTION_FAILURE,
	PEN_CONNECTING,
	PEN_DISCONNECTING,
	PEN_CONNECTION_SUCCESS,
	PEN_DISCONNECTED, 
	PEN_CONNECTION_UNKNOWN
};
enum CONNECT_RTN_CODE :int
{
	RTN_CODE_OK = 0,
	CONNECT_RTN_CODE_DEVICE_NOT_FOUND,
	CONNECT_RTN_CODE_ALREADY_CONNECT
};
#pragma pack(push, 1)
typedef struct {
	uint32_t x;
	uint32_t y;
	uint32_t page;
	uint8_t type;
	unsigned short book_no;
	uint32_t book_width;
	uint32_t book_height;
}AFEDot;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	bool bSuccess;
}AFEClearStorage;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	uint16_t val;
}AFEGetBattery;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	int readcnt;
	int readedcnt;
	int totalCnt;
	int dotCnt;
	AFEDot* list;
}AFEGetDots;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	uint32_t count;
}AFEGetDotsCount;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	uint32_t size;
}AFEGetStorageSize;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	uint16_t length;
	uint8_t* version;
}AFEGetVersion;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	uint8_t* mac;
	size_t namelen;
	const  char* name;
}AFBLEFindDevice;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct  {
	uint32_t pageFrom;
	uint32_t pageTo;
	uint32_t width;
	uint32_t height;
	uint16_t bookNum;
}AFAPaperSize;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	uint32_t offset;
	uint32_t count;
}AFAGetDots;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct  {

	uint8_t* mac;
	size_t namelen;
	const  char* name;
	BLE_STATUS_TYPE status;
}AFBLEDeviceStatus;
#pragma pack(pop)

typedef bool(* AFIPenEvent)(PEN_EVENT_TYPE evtType, uint8_t* data, int len);
typedef bool(* AFIBleEvent)(BLE_EVENT_TYPE evtType, uint8_t* data, int len);

extern "C" {
	__declspec(dllexport) int AFInit();
	__declspec(dllexport) int AFConnect(const wchar_t* name);
	__declspec(dllexport) int AFDisConnect();
	__declspec(dllexport) int AFScanStart();
	__declspec(dllexport) int AFScanStop();
	__declspec(dllexport) int AFSetPenEventListener(AFIPenEvent iPenEvent);
	__declspec(dllexport) int AFSetBleEventListener(AFIBleEvent iBleEvent);
	__declspec(dllexport) int AFGetFWInfo();
	__declspec(dllexport) int AFSetPaperSizes(AFAPaperSize* v, size_t size); //int AFSetPaperSizes(std::vector<AFAPaperSize>& v);
	__declspec(dllexport) int AFGetDots(AFAGetDots& v);
	__declspec(dllexport) int AFUnInit();
	__declspec(dllexport) int AFGetBatteryInfo();
	__declspec(dllexport) int AFGetDotsCount();
	__declspec(dllexport) int AFGetStorageSize();
	__declspec(dllexport) int AFClearDots();
}
