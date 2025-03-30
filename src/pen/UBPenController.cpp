#include <QCoreApplication>
#include <QString>
#include <QFileInfo>
#include "UBPenController.h"
#include "xb.h"
#include "UBCalibrationWindow.h"
#include "../core/UBApplication.h"
#include "UBVirtualScreen.h"
#include <QMessageBox>

// Initialize the static instance pointer
UBPenController* UBPenController::instance = nullptr;
UBCalibrationWindow* penCalibrationWindow = nullptr;

UBPenController::UBPenController() {
}

UBPenController* UBPenController::getInstance() {
    if (instance == nullptr) {
        instance = new UBPenController();
    }
    return instance;
}

UBPenController::~UBPenController() {
    UBPenController::getInstance()->pAFUnInit();

    if (hPenSDK) {
        FreeLibrary(hPenSDK);
        hPenSDK = nullptr;
    }

    instance = nullptr;
}

void UBPenController::loadPenSDK()
{
    QString dllPath = QCoreApplication::applicationDirPath() + "/xbc.dll";
    if (!QFileInfo::exists(dllPath)) {
        qWarning() << "Error: xbc.dll not found in application directory";
    }

    hPenSDK = LoadLibrary(L"xbc.dll");
    if (!hPenSDK) {
        qWarning() << "Failed to load xbc.dll. Error: %1";
    }

    pAFInit = (PFN_AFInit)GetProcAddress(hPenSDK, "AFInit");
    pAFConnect = (PFN_AFConnect)GetProcAddress(hPenSDK, "AFConnect");
    pAFDisConnect = (PFN_AFDisConnect)GetProcAddress(hPenSDK, "AFDisConnect");
    pAFScanStart = (PFN_AFScanStart)GetProcAddress(hPenSDK, "AFScanStart");
    pAFScanStop = (PFN_AFScanStop)GetProcAddress(hPenSDK, "AFScanStop");
    pAFSetPenEventListener = (PFN_AFSetPenEventListener)GetProcAddress(hPenSDK, "AFSetPenEventListener");
    pAFSetBleEventListener = (PFN_AFSetBleEventListener)GetProcAddress(hPenSDK, "AFSetBleEventListener");
    pAFGetFWInfo = (PFN_AFGetFWInfo)GetProcAddress(hPenSDK, "AFGetFWInfo");
    pAFSetPaperSizes = (PFN_AFSetPaperSizes)GetProcAddress(hPenSDK, "AFSetPaperSizes");
    pAFGetStorageSize = (PFN_AFGetStorageSize)GetProcAddress(hPenSDK, "AFGetStorageSize");
    pAFGetBatteryInfo = (PFN_AFGetBatteryInfo)GetProcAddress(hPenSDK, "AFGetBatteryInfo");
    pAFUnInit = (PFN_AFUnInit)GetProcAddress(hPenSDK, "AFUnInit");
    pAFGetDotsCount = (PFN_AFGetDotsCount)GetProcAddress(hPenSDK, "AFGetDotsCount");
    pAFClearDots = (PFN_AFClearDots)GetProcAddress(hPenSDK, "AFClearDots");

    if (!pAFInit || !pAFConnect || !pAFDisConnect || !pAFScanStart ||
        !pAFScanStop || !pAFSetPenEventListener || !pAFSetBleEventListener ||
        !pAFGetFWInfo || !pAFSetPaperSizes || !pAFGetStorageSize || !pAFGetBatteryInfo ||
        !pAFUnInit || !pAFGetDotsCount || !pAFClearDots) {
        qWarning() << "Failed to load one or more SDK functions";
        FreeLibrary(hPenSDK);
        hPenSDK = nullptr;
    }
    else {
        if (pAFInit() != 0) {
            qWarning() << "Failed to initialize SDK";
            FreeLibrary(hPenSDK);
            hPenSDK = nullptr;
        }
        else {
            pAFSetBleEventListener(bleEventCallback);
            pAFSetPenEventListener(penEventCallback);
            setPaperSizes();
            qInfo() << "SDK successfully initialized";
        }
    }
}

void UBPenController::connect()
{
    getInstance()->pAFScanStop();

    try {
        int ret = getInstance()->pAFScanStart();
        if (ret != -1) {
            emit scanningAndConnecting();
        }
    } catch (...) {
        QMessageBox::critical(nullptr, "Bluetooth desativado", "O bluetooth está desativado.");
    }
}

void UBPenController::showCalibrationWindow()
{
    if (penCalibrationWindow == nullptr)
        penCalibrationWindow = new UBCalibrationWindow();

    penCalibrationWindow->show();
}

void UBPenController::hideCalibrationWindow()
{
    delete penCalibrationWindow;
    penCalibrationWindow = nullptr;
}

void UBPenController::setPaperSizes()
{
    // Create paper size structures
    AFAPaperSize a5p = {1, 10000, 4960, 7040, 1};
    AFAPaperSize aboard = {65600, 65601, 28913, 22772, 1};
    AFAPaperSize sp = {77649, 77650, 25080 * 3, 30096 * 2, 2};

    // Create vector of paper sizes
    std::vector<AFAPaperSize> paperSizes;
    paperSizes.push_back(a5p);
    paperSizes.push_back(sp);
    paperSizes.push_back(aboard);

    // Set flipmode if necessary (equivalent to C# aboard.flipmode = 1)
    // Note: AFAPaperSize doesn't have a flipmode field in the provided xb.h
    // If this is needed, the struct would need to be extended

    try {
        // Call AFSetPaperSizes with the vector data
        int ret = getInstance()->pAFSetPaperSizes(
            paperSizes.data(),          // Pointer to the array of AFAPaperSize structs
            paperSizes.size()           // Number of elements in the array
        );

        if (ret != RTN_CODE_OK) {
            // Handle error if needed
            qCritical() << "Failed to set paper sizes";
        }
    }
    catch (const std::exception& e) {
        // Error handling equivalent to C# MessageBox
        qCritical() << "Error setting paper sizes: " << e.what();
    }
}

QString UBPenController::safeCharToQString(const char *str, size_t length)
{
    if (!str || length == 0) {
        return QString();
    }
    return QString::fromUtf8(str, static_cast<int>(length));
}

bool __cdecl UBPenController::bleEventCallback(BLE_EVENT_TYPE evtType, uint8_t* data, int len)
{
    switch (evtType) {
    case BLE_EVENT_FIND_DEVICE: {
        AFBLEFindDevice* device = (AFBLEFindDevice*)data;
        if (device->name && device->namelen > 0) {
            QString deviceName = safeCharToQString(device->name, device->namelen);
            qInfo() << "Found device: " + deviceName + ". Connecting";
            UBPenController::getInstance()->pAFConnect(deviceName.toStdWString().c_str());
        }
        break;
    }
    case BLE_EVENT_STATUS: {
        AFBLEDeviceStatus* status = (AFBLEDeviceStatus*)data;
        switch (status->status) {
        case PEN_CONNECTION_SUCCESS:
            if (UBVirtualScreen::getInstance().calibrationStatus == NOT_CALIBRATED)
            {
                UBVirtualScreen::getInstance().calibrationStatus = CALIBRATING_P1;

                QMetaObject::invokeMethod(QApplication::instance(), []() {
                    emit getInstance()->connected();
                    // getInstance()->showCalibrationWindow();
                }, Qt::QueuedConnection);
            }
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

bool __cdecl UBPenController::penEventCallback(PEN_EVENT_TYPE evtType, uint8_t* data, int len)
{
    if (evtType == PEN_EVENT_TYPE::PEN_EVENT_Dot) {
        // Convert raw pointer to structure
        AFEDot* dot = (AFEDot*)data;

        if (UBVirtualScreen::getInstance().calibrationStatus == CALIBRATED) {

            // Set pen status
            if (dot->type == 1) {
                getInstance()->penStatus = (getInstance()->penStatus == PenUp) ? PenDown : PenMove;
            }
            else if (dot->type == 2) {
                getInstance()->penStatus = PenUp;
            }

            // Converts dot to mouse action
            UBVirtualScreen::getInstance().dotToMouse(static_cast<int>(dot->x), static_cast<int>(dot->y));
        }
        else {
            switch (UBVirtualScreen::getInstance().calibrationStatus) {
            case NOT_CALIBRATED:
                break;
            case CALIBRATING_P1:
                if (dot->type == 2) {
                    UBVirtualScreen::getInstance().calibrationSetFirstPoint(static_cast<int>(dot->x), static_cast<int>(dot->y));
                    QMetaObject::invokeMethod(QApplication::instance(), []() {
                        // Update UI
                        penCalibrationWindow->update();
                    }, Qt::QueuedConnection);
                }
                break;
            case CALIBRATING_P2:
                if (dot->type == 2) {
                    UBVirtualScreen::getInstance().calibrationSetSecondPoint(static_cast<int>(dot->x), static_cast<int>(dot->y));

                    QMetaObject::invokeMethod(QApplication::instance(), []() {
                        // Hide calibration window
                        getInstance()->hideCalibrationWindow();
                        UBApplication::showMessage("Caneta calibrada");
                    }, Qt::QueuedConnection);
                }
                break;
            default:
                break;
            }
        }
    }

    return true;
}
