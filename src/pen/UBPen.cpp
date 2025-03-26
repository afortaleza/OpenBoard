#include <QCoreApplication>
#include <QString>
#include <QFileInfo>
#include "UBPen.h"
#include "xb.h"
#include "UBPenCalibration.h"
#include "../core/UBApplication.h"

// Initialize the static instance pointer
UBPen* UBPen::instance = nullptr;

UBPen::UBPen() {
    if (loadPenSDK()) {
        pAFScanStart();
        // showCalibrationScreen();
    }
}

UBPen* UBPen::getInstance() {
    if (instance == nullptr) {
        instance = new UBPen();
    }
    return instance;
}

UBPen::~UBPen() {
    UBPen::getInstance()->pAFUnInit();
    if (hPenSDK) {
        FreeLibrary(hPenSDK);
        hPenSDK = nullptr;
    }
    instance = nullptr;  // Reset instance pointer when destroyed
}

bool UBPen::loadPenSDK()
{
    QString dllPath = QCoreApplication::applicationDirPath() + "/xbc.dll";
    if (!QFileInfo::exists(dllPath)) {
        qWarning() << "Error: xbc.dll not found in application directory";
        return false;
    }

    hPenSDK = LoadLibrary(L"xbc.dll");
    if (!hPenSDK) {
        qWarning() << "Failed to load xbc.dll. Error: %1";
        return false;
    }

    pAFInit = (PFN_AFInit)GetProcAddress(hPenSDK, "AFInit");
    pAFConnect = (PFN_AFConnect)GetProcAddress(hPenSDK, "AFConnect");
    pAFDisConnect = (PFN_AFDisConnect)GetProcAddress(hPenSDK, "AFDisConnect");
    pAFScanStart = (PFN_AFScanStart)GetProcAddress(hPenSDK, "AFScanStart");
    pAFScanStop = (PFN_AFScanStop)GetProcAddress(hPenSDK, "AFScanStop");
    pAFSetPenEventListener = (PFN_AFSetPenEventListener)GetProcAddress(hPenSDK, "AFSetPenEventListener");
    pAFSetBleEventListener = (PFN_AFSetBleEventListener)GetProcAddress(hPenSDK, "AFSetBleEventListener");
    pAFGetFWInfo = (PFN_AFGetFWInfo)GetProcAddress(hPenSDK, "AFGetFWInfo");
    pAFGetStorageSize = (PFN_AFGetStorageSize)GetProcAddress(hPenSDK, "AFGetStorageSize");
    pAFGetBatteryInfo = (PFN_AFGetBatteryInfo)GetProcAddress(hPenSDK, "AFGetBatteryInfo");
    pAFUnInit = (PFN_AFUnInit)GetProcAddress(hPenSDK, "AFUnInit");
    pAFGetDotsCount = (PFN_AFGetDotsCount)GetProcAddress(hPenSDK, "AFGetDotsCount");
    pAFClearDots = (PFN_AFClearDots)GetProcAddress(hPenSDK, "AFClearDots");

    if (!pAFInit || !pAFConnect || !pAFDisConnect || !pAFScanStart ||
        !pAFScanStop || !pAFSetPenEventListener || !pAFSetBleEventListener ||
        !pAFGetFWInfo || !pAFGetStorageSize || !pAFGetBatteryInfo ||
        !pAFUnInit || !pAFGetDotsCount || !pAFClearDots) {
        qWarning() << "Failed to load one or more SDK functions";
        FreeLibrary(hPenSDK);
        hPenSDK = nullptr;
        return false;
    }

    if (pAFInit() != 0) {
        qWarning() << "Failed to initialize SDK";
        FreeLibrary(hPenSDK);
        hPenSDK = nullptr;
        return false;
    }

    pAFSetBleEventListener(bleEventCallback);
    pAFSetPenEventListener(penEventCallback);
    qInfo() << "SDK successfully initialized";
    UBApplication::showMessage("SDK successfully initialized");
    return true;
}

void UBPen::showCalibrationScreen()
{
    UBPenCalibration* penCalibration = new UBPenCalibration();
    penCalibration->show();
}

QString UBPen::safeCharToQString(const char *str, size_t length)
{
    if (!str || length == 0) {
        return QString();
    }
    return QString::fromUtf8(str, static_cast<int>(length));
}

bool __cdecl UBPen::bleEventCallback(BLE_EVENT_TYPE evtType, uint8_t* data, int len)
{
    switch (evtType) {
    case BLE_EVENT_FIND_DEVICE: {
        AFBLEFindDevice* device = (AFBLEFindDevice*)data;
        if (device->name && device->namelen > 0) {
            QString deviceName = safeCharToQString(device->name, device->namelen);
            qInfo() << "Found device: " + deviceName + ". Connecting";
            UBPen::getInstance()->pAFConnect(deviceName.toStdWString().c_str());
        }
        break;
    }
    case BLE_EVENT_STATUS: {
        AFBLEDeviceStatus* status = (AFBLEDeviceStatus*)data;
        switch (status->status) {
        case PEN_CONNECTION_SUCCESS:
            qInfo() << "Connected!";
            break;
        case PEN_CONNECTION_FAILURE:
            qInfo() << "Connection failed";
            break;
        case PEN_DISCONNECTED:
            qInfo() << "Disconnected from device";
            break;
        case PEN_CONNECTION_TRY:
        case PEN_CONNECTING:
        case PEN_DISCONNECTING:
        case PEN_CONNECTION_UNKNOWN:
            break;
        }
        break;
    }
    }
    return true;
}

bool __cdecl UBPen::penEventCallback(PEN_EVENT_TYPE evtType, uint8_t* data, int len)
{
    switch (evtType) {
    case PEN_EVENT_GetVersion: {
        AFEGetVersion* version = (AFEGetVersion*)data;
        if (version->version && version->length > 0) {
            QString versionStr = QString::fromUtf8(reinterpret_cast<char*>(version->version), version->length);
            qInfo() << "Firmware version: " + versionStr;
        }
        break;
    }
    case PEN_EVENT_GetStorageSize: {
        AFEGetStorageSize* storage = (AFEGetStorageSize*)data;
        qInfo() << QString("Storage size: %1 bytes").arg(storage->size);
        break;
    }
    case PEN_EVENT_GetBattery: {
        AFEGetBattery* battery = (AFEGetBattery*)data;
        if (battery->val == 32676) {
            qInfo() << "Battery: Charging";
        }
        else {
            qInfo() << QString("Battery level: %1/10").arg(battery->val);
        }
        break;
    }
    case PEN_EVENT_GetDotsCount: {
        AFEGetDotsCount* dotsCount = (AFEGetDotsCount*)data;
        qInfo() << QString("Total dots count: %1").arg(dotsCount->count);
        break;
    }
    case PEN_EVENT_ClearStorage: {
        AFEClearStorage* clearResult = (AFEClearStorage*)data;
        qInfo() << "Clear storage " + QString(clearResult->bSuccess ? "successful" : "failed");
        break;
    }
    case PEN_EVENT_Dot: {
        AFEDot* dot = (AFEDot*)data;
        qInfo() << QString("Dot received: x=%1, y=%2, page=%3 tyoe=%4").arg(dot->x).arg(dot->y).arg(dot->page).arg(dot->type);
        break;
    }
    case PEN_EVENT_CmdTimeout: {
        qInfo() << "Command timeout occurred";
        break;
    }
    case PEN_EVENT_Undefine:
    case PEN_EVENT_GetDots:
        break;
    }
    return true;

}
