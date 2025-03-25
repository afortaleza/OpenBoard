#include <QCoreApplication>
#include <QString>
#include <QFileInfo>
#include "UBPen.h"
#include "xb.h"
#include "UBPenCalibration.h"

UBPen::UBPen() {
    if (loadPenSDK()) {
        showCalibrationScreen();
    }
}

UBPen::~UBPen()
{
    this->pAFUnInit();
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
    return true;
}

void UBPen::showCalibrationScreen()
{
    UBPenCalibration* penCalibration = new UBPenCalibration();
    penCalibration->show();
}

bool __cdecl UBPen::bleEventCallback(BLE_EVENT_TYPE evtType, uint8_t* data, int len)
{
    return false;
}

bool __cdecl UBPen::penEventCallback(PEN_EVENT_TYPE evtType, uint8_t* data, int len)
{
    return false;
}
