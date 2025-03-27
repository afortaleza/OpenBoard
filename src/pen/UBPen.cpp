#include <QCoreApplication>
#include <QString>
#include <QFileInfo>
#include "UBPen.h"
#include "xb.h"
#include "UBPenCalibration.h"
#include "../core/UBApplication.h"
#include "UBVirtualScreen.h"

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
    if (evtType == PEN_EVENT_TYPE::PEN_EVENT_Dot) {
        // Convert raw pointer to structure
        AFEDot* dot = (AFEDot*)data;

        if (VirtualScreen::getInstance().calibrationStatus == CalibrationStatus::CALIBRATED) {

            // Set pen status
            if (dot->type == 1) {
                getInstance()->penStatus = (getInstance()->penStatus == PenUp) ? PenDown : PenMove;
            }
            else if (dot->type == 2) {
                getInstance()->penStatus = PenUp;
            }

            // Converts dot to mouse action
            VirtualScreen::getInstance().dotToMouse(static_cast<int>(dot->x), static_cast<int>(dot->y));
        }
        else {
            switch (VirtualScreen::getInstance().calibrationStatus) {
            case CalibrationStatus::NOT_CALIBRATED:
                break;
            case CalibrationStatus::CALIBRATING_P1:
                if (dot->type == 2) {
                    VirtualScreen::getInstance().calibrationSetFirstPoint(static_cast<int>(dot->x), static_cast<int>(dot->y));
                    // Set calibration window state
                    // _calibrationForm->FirstTargetClicked = true;
                    // _calibrationForm->Invalidate();
                }
                break;
            case CalibrationStatus::CALIBRATING_P2:
                if (dot->type == 2) {
                    VirtualScreen::getInstance().calibrationSetSecondPoint(static_cast<int>(dot->x), static_cast<int>(dot->y));
                    // Set calibration window state
                    //_calibrationForm->Close();
                    //_calibrationForm = nullptr;
                    //this->TopMost = false;  // Note: this assumes a window class context
                    //this->WindowState = FormWindowState::Minimized;  // Need to adapt to C++ window handling
                    getInstance()->pAFScanStop();  // Assuming this is a global function
                }
                break;
            default:
                break;
            }
        }
    }

    return true;
}
