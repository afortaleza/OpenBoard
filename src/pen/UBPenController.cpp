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
    AFAPaperSize* pArrs;
    int count = 3;
    pArrs = new AFAPaperSize[count];

    AFAPaperSize a5p;
    a5p.pageFrom = 1;
    a5p.pageTo = 10000;
    a5p.width = 4960;
    a5p.height = 7040;
    a5p.bookNum = 1;
    a5p.flipmode = 0;
    pArrs[0] = a5p;

    AFAPaperSize sp;
    sp.pageFrom = 77649;
    sp.pageTo = 77650;
    sp.width = 25080 * 3;
    sp.height = 30096 * 2;
    sp.bookNum = 2;
    sp.flipmode = 0;
    pArrs[1] = sp;

    AFAPaperSize aboard;
    aboard.pageFrom = 65600;
    aboard.pageTo = 65601;
    aboard.width = 28913;
    aboard.height = 22772;
    aboard.bookNum = 1;
    aboard.flipmode = 1;
    pArrs[2] = aboard;

    try {
        int ret = this->pAFSetPaperSizes(pArrs, count);

        if (ret != 0) {
            // Handle error if needed
            qCritical() << "Failed to set paper sizes";
        }
    }
    catch (const std::exception& e) {
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
            emit getInstance()->connected();
            qInfo() << "Connected!";

            if (UBVirtualScreen::getInstance().calibrationStatus == NOT_CALIBRATED)
            {
                UBVirtualScreen::getInstance().calibrationStatus = CALIBRATING_P1;

                // Show calibration window on GUI thread
                QMetaObject::invokeMethod(QApplication::instance(), []() {
                    getInstance()->showCalibrationWindow();
                }, Qt::QueuedConnection);
            }
            break;
        case PEN_CONNECTION_FAILURE:
            qInfo() << "Connection failed";
            break;
        case PEN_DISCONNECTED:
            emit getInstance()->disconnected();
            qInfo() << "Disconnected from device";
            break;
        case PEN_CONNECTION_TRY:
            qInfo() << "Connection try";
            break;
        case PEN_CONNECTING:
            qInfo() << "Pen connecting";
            break;
        case PEN_DISCONNECTING:
            qInfo() << "Pen disconnecting";
            break;
        case PEN_CONNECTION_UNKNOWN:
            getInstance()->pAFDisConnect();
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

        qInfo() << "Dot at X: " << dot->x << ", Y: " << dot->y;

        if (UBVirtualScreen::getInstance().calibrationStatus == CALIBRATED) {

            // Set pen status
            if (dot->type == 1) {
                getInstance()->penTipStatus = (getInstance()->penTipStatus == PenUp) ? PenDown : PenMove;
            }
            else if (dot->type == 2) {
                getInstance()->penTipStatus = PenUp;
            }

            // Converts dot to mouse action
            UBVirtualScreen::getInstance().dotToMouse(static_cast<int>(dot->x), static_cast<int>(dot->y));
        }
        else {
            qInfo() << "[PEN] - Calibrating";
            switch (UBVirtualScreen::getInstance().calibrationStatus) {
            case NOT_CALIBRATED:
                qInfo() << "[PEN] - Not calibrated";
                break;
            case CALIBRATING_P1:
                qInfo() << "[PEN] - P1 - Calibrating";
                if (dot->type == 2) {
                    qInfo() << "[PEN] - P1 - At x: " << dot->x << ", y: " << dot->y;
                    UBVirtualScreen::getInstance().calibrationSetFirstPoint(static_cast<int>(dot->x), static_cast<int>(dot->y));

                    QMetaObject::invokeMethod(QApplication::instance(), []() {
                        qInfo() << "[PEN] - P1 - Calibrated, updating UI";
                        // Update UI
                        penCalibrationWindow->update();
                    }, Qt::QueuedConnection);
                }
                break;
            case CALIBRATING_P2:
                qInfo() << "[PEN] - P2 - Calibrating";
                if (dot->type == 2) {
                    qInfo() << "[PEN] - P2 - At x: " << dot->x << ", y: " << dot->y;
                    UBVirtualScreen::getInstance().calibrationSetSecondPoint(static_cast<int>(dot->x), static_cast<int>(dot->y));

                    QMetaObject::invokeMethod(QApplication::instance(), []() {
                        qInfo() << "[PEN] - P2 - Calibrated, updating UI";
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
