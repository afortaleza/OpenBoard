#include <QCoreApplication>
#include <QString>
#include <QFileInfo>
#include <QObject> // For the helper class
#include "UBPenController.h"
#include "xb.h"
#include "UBCalibrationWindow.h"
#include "../core/UBApplication.h"
#include "UBVirtualScreen.h"

// Helper class to handle GUI operations on the main thread
class UBGuiHelper : public QObject {
    Q_OBJECT
public:
    UBGuiHelper(QObject* parent = nullptr) : QObject(parent) {}
    static UBGuiHelper* instance();

public slots:
    void showCalibrationWindow() {
        if (UBPenController::penCalibrationWindow == nullptr)
            UBPenController::penCalibrationWindow = new UBCalibrationWindow();
        UBPenController::penCalibrationWindow->show();
    }

signals:
    void signalShowCalibrationWindow();

private:
    static UBGuiHelper* guiInstance;
};

UBGuiHelper* UBGuiHelper::guiInstance = nullptr;

UBGuiHelper* UBGuiHelper::instance() {
    if (guiInstance == nullptr) {
        guiInstance = new UBGuiHelper();
        // Connect signal to slot in the helper itself
        QObject::connect(guiInstance, &UBGuiHelper::signalShowCalibrationWindow,
                         guiInstance, &UBGuiHelper::showCalibrationWindow);
    }
    return guiInstance;
}

// Initialize static members of UBPenController
UBPenController* UBPenController::instance = nullptr;
UBCalibrationWindow* UBPenController::penCalibrationWindow = nullptr;

UBPenController::UBPenController() {
    if (loadPenSDK()) {
        pAFScanStart();
    }
}

UBPenController* UBPenController::getInstance() {
    if (instance == nullptr) {
        instance = new UBPenController();
        // Ensure the GUI helper is initialized
        UBGuiHelper::instance();
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
    // Note: penCalibrationWindow is deleted in hideCalibrationWindow()
}

bool UBPenController::loadPenSDK() {
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

void UBPenController::showCalibrationWindow() {
    // Emit the signal instead of creating the widget directly
    emit UBGuiHelper::instance()->signalShowCalibrationWindow();
}

void UBPenController::hideCalibrationWindow() {
    delete penCalibrationWindow;
    penCalibrationWindow = nullptr;
}

QString UBPenController::safeCharToQString(const char *str, size_t length) {
    if (!str || length == 0) {
        return QString();
    }
    return QString::fromUtf8(str, static_cast<int>(length));
}

bool __stdcall UBPenController::bleEventCallback(BLE_EVENT_TYPE evtType, uint8_t* data, int len) {
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
            if (UBVirtualScreen::getInstance().calibrationStatus == NOT_CALIBRATED) {
                UBVirtualScreen::getInstance().calibrationStatus = CALIBRATING_P1;
                UBPenController::showCalibrationWindow(); // Call the static method
            }
            qInfo() << "Connected!";
            break;
        case PEN_CONNECTION_FAILURE:
            qInfo() << "Connection failed";
            break;
        case PEN_DISCONNECTED:
            UBPenController::hideCalibrationWindow();
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

bool __stdcall UBPenController::penEventCallback(PEN_EVENT_TYPE evtType, uint8_t* data, int len) {
    if (evtType == PEN_EVENT_TYPE::PEN_EVENT_Dot) {
        AFEDot* dot = (AFEDot*)data;

        if (UBVirtualScreen::getInstance().calibrationStatus == CALIBRATED) {
            if (dot->type == 1) {
                UBPenController::getInstance()->penStatus = (UBPenController::getInstance()->penStatus == PenUp) ? PenDown : PenMove;
            } else if (dot->type == 2) {
                UBPenController::getInstance()->penStatus = PenUp;
            }
            UBVirtualScreen::getInstance().dotToMouse(static_cast<int>(dot->x), static_cast<int>(dot->y));
        } else {
            switch (UBVirtualScreen::getInstance().calibrationStatus) {
            case NOT_CALIBRATED:
                break;
            case CALIBRATING_P1:
                if (dot->type == 2) {
                    UBVirtualScreen::getInstance().calibrationSetFirstPoint(static_cast<int>(dot->x), static_cast<int>(dot->y));
                    UBPenController::penCalibrationWindow->update();
                }
                break;
            case CALIBRATING_P2:
                if (dot->type == 2) {
                    UBVirtualScreen::getInstance().calibrationSetSecondPoint(static_cast<int>(dot->x), static_cast<int>(dot->y));
                    UBPenController::hideCalibrationWindow();
                    UBPenController::getInstance()->pAFScanStop();
                }
                break;
            default:
                break;
            }
        }
    }
    return true;
}
