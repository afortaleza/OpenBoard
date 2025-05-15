#include <QCoreApplication>
#include <QString>
#include <QFileInfo>
#include "UBPenController.h"
#include "xb.h"
#include "UBCalibrationWindow.h"
#include "../core/UBApplication.h"
#include "../gui/UBMainWindow.h"
#include "UBVirtualScreen.h"
#include <QMessageBox>
#include "../board/UBBoardController.h"

UBCalibrationWindow* penCalibrationWindow = nullptr;
UBVirtualScreen* virtualScreen = nullptr;

UBPenController::UBPenController() {
    virtualScreen = new UBVirtualScreen();
    virtualDesktopRect = QRect(0, 0, 0, 0);

    // Get the screen the main window is on
    primaryScreen = UBApplication::mainWindow->screen();

    qInfo() << QString("Primary Monitor Geometry: x=%1, y=%2, width=%3, height=%4")
        .arg(primaryScreen->geometry().x())
        .arg(primaryScreen->geometry().y())
        .arg(static_cast<int>(primaryScreen->devicePixelRatio() * primaryScreen->geometry().width()))
        .arg(static_cast<int>(primaryScreen->devicePixelRatio() * primaryScreen->geometry().height()));

    // Calculate unscaled (native) resolution
    int pWidth = primaryScreen->geometry().width() * primaryScreen->devicePixelRatio();
    int pHeight = primaryScreen->geometry().height() * primaryScreen->devicePixelRatio();

    virtualScreen->setPrimaryScreenDimensions(pWidth, pHeight);

    // Get the list of available screens
    QList<QScreen*> screens = QGuiApplication::screens();
    virtualScreen->hasSecondaryScreen = screens.length() == 2;

    // Look for a screen that is not the main window's screen
    if (virtualScreen->hasSecondaryScreen)
    {
        for (QScreen* screen : screens) {
            if (screen != primaryScreen) {
                secondaryScreen = screen;

                qInfo() << QString("Secondary Monitor Geometry: x=%1, y=%2, width=%3, height=%4")
                               .arg(secondaryScreen->geometry().x())
                               .arg(secondaryScreen->geometry().y())
                               .arg(static_cast<int>(secondaryScreen->devicePixelRatio() * secondaryScreen->geometry().width()))
                               .arg(static_cast<int>(secondaryScreen->devicePixelRatio() * secondaryScreen->geometry().height()));

                // Calculate unscaled (native) resolution
                int sWidth = secondaryScreen->geometry().width() * secondaryScreen->devicePixelRatio();
                int sHeight = secondaryScreen->geometry().height() * secondaryScreen->devicePixelRatio();

                virtualScreen->setSecondaryScreenDimensions(sWidth, sHeight);
                break;
            }
        }
    }
    else {
        qInfo() << "[VSCREEN] - Number of screens is different from two: " << screens.length();
    }
}

UBPenController::~UBPenController() {

    if (penCalibrationWindow) delete penCalibrationWindow;
    penCalibrationWindow = nullptr;

    if (virtualScreen) delete virtualScreen;
    virtualScreen = nullptr;

    if (hPenSDK) {
        pAFUnInit();
        // FreeLibrary(hPenSDK); // Crashing when exiting
        hPenSDK = nullptr;
    }
}

void UBPenController::loadPenSDK() {
    QString dllPath = QCoreApplication::applicationDirPath() + "/xbc.dll";
    if (!QFileInfo::exists(dllPath)) {
        qWarning() << "[PEN] Error: xbc.dll not found in application directory";
    }

    hPenSDK = LoadLibrary(L"xbc.dll");
    if (!hPenSDK) {
        qWarning() << "[PEN] Failed to load xbc.dll. Error: %1";
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
        qWarning() << "[PEN] Failed to load one or more SDK functions";
        FreeLibrary(hPenSDK);
        hPenSDK = nullptr;
    }
    else {
        if (pAFInit() != 0) {
            qWarning() << "[PEN] Failed to initialize SDK";
            FreeLibrary(hPenSDK);
            hPenSDK = nullptr;
        }
        else {
            pAFSetBleEventListener(bleEventCallback);
            pAFSetPenEventListener(penEventCallback);
            setPaperSizes();
            qInfo() << "[PEN] SDK successfully initialized";
        }
    }
}

void UBPenController::connect()
{
    pAFScanStop();

    try {
        int ret = pAFScanStart();
        if (ret != -1) {
            scanningCanceled = false;
            qInfo() << "[PEN] Scanning and connecting";
            emit scanningAndConnecting();
        }
        else {
            qWarning() << "[PEN] Unable to start scanning";
            cancelScanning();
        }
    } catch (...) {
        QMessageBox::critical(nullptr, "Bluetooth desativado", "O bluetooth está desativado.");
    }
}

int UBPenController::getBatteryLevel()
{
    uint16_t batteryInfo = pAFGetBatteryInfo();
    qInfo() << "[Pen] Battery Level: " << batteryInfo;
    if (batteryInfo == 32676)
        return -1;
    else
        return batteryInfo * 10;
}

void showCalibrationWindow()
{
    if (penCalibrationWindow == nullptr)
        penCalibrationWindow = new UBCalibrationWindow();

    penCalibrationWindow->show();
}

void hideCalibrationWindow()
{
    if (penCalibrationWindow) delete penCalibrationWindow;
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

void UBPenController::cancelScanning()
{
    scanningCanceled = true;
    pAFScanStop();
    emit stopScanning();
}

bool UBPenController::isVirtualDesktopEnabled()
{
    return virtualDesktop != nullptr;
}

bool UBPenController::isVirtualDesktopSelected()
{
    return virtualDesktop->isSelected();
}

bool UBPenController::isInsideVirtualDesktop(int x, int y)
{
    return this->virtualDesktopRect.contains(x, y);
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
            qInfo() << "[PEN] Found device: " + deviceName + ". Connecting...";
            UBApplication::penController->pAFConnect(deviceName.toStdWString().c_str());
        }
        else {
            qWarning() << "[PEN] Found device but it has no name. Can't connect to a nameless device";
            UBApplication::penController->cancelScanning();
        }
        break;
    }
    case BLE_EVENT_STATUS: {
        AFBLEDeviceStatus* deviceStatus = (AFBLEDeviceStatus*)data;

        switch (deviceStatus->status) {
            case PEN_CONNECTION_SUCCESS:
                UBApplication::penController->connectionStatus = Connected;
                UBApplication::boardController->checkPenBatteryStatus();
                emit UBApplication::penController->connected();
                qInfo() << "[PEN] Connected!";

                if (UBApplication::penController->calibrationStatus == NotCalibrated)
                {
                    UBApplication::penController->calibrationStatus = CalibratingP1;

                    // Show calibration window on GUI thread
                    QMetaObject::invokeMethod(QApplication::instance(), []() {
                        showCalibrationWindow();
                    }, Qt::QueuedConnection);
                }
                break;
            case PEN_CONNECTION_FAILURE:
                qInfo() << "[PEN] Connection failed";
                UBApplication::penController->cancelScanning();
                break;
            case PEN_DISCONNECTED:
                UBApplication::penController->connectionStatus = NotConnected;
                qInfo() << "[PEN] Disconnected from device, restarting scanning.";
                UBApplication::penController->connect();
                break;
            case PEN_CONNECTION_TRY:
                qInfo() << "[PEN] Trying to connect";
                break;
            case PEN_CONNECTING:
                qInfo() << "[PEN] Connecting...";
                break;
            case PEN_DISCONNECTING:
                qInfo() << "[PEN] Disconnecting...";
                break;
            case PEN_CONNECTION_UNKNOWN:
                qInfo() << "[PEN] Connection Unknown";
                UBApplication::penController->connectionStatus = NotConnected;
                UBApplication::penController->cancelScanning();
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

        if (UBApplication::penController->calibrationStatus == Calibrated) {
            // Set pen status
            if (dot->type == 1) {
                UBApplication::penController->penTipStatus = (UBApplication::penController->penTipStatus == PenUp) ? PenDown : PenMove;
            }
            else if (dot->type == 2) {
                UBApplication::penController->penTipStatus = PenUp;
            }

            auto [x, y] = virtualScreen->getPrimaryScreenDot(dot->x, dot->y);
            // Converts dot to mouse action
            if (!UBApplication::penController->isVirtualDesktopEnabled()) {
                virtualScreen->dotToMouse(x, y, dot->x, dot->y);
            }
            else {
                if (UBApplication::penController->isVirtualDesktopSelected())
                {
                    if (UBApplication::penController->isInsideVirtualDesktop(x, y))
                    {
                        auto [sX, sY] = virtualScreen->getSecondaryScreenDot(UBApplication::penController->virtualDesktopRect, x, y);
                        int secondX = UBApplication::penController->secondaryScreen->geometry().x() + sX;
                        int secondY = UBApplication::penController->secondaryScreen->geometry().y() + sY;
                        virtualScreen->dotToMouse(secondX, secondY, dot->x, dot->y);
                    }
                    else
                    {
                        virtualScreen->dotToMouse(x, y, dot->x, dot->y);
                    }
                }
                else
                {
                    virtualScreen->dotToMouse(x, y, dot->x, dot->y);
                }
            }
        }
        else {
            // Only considers pen up when calibrating
            if (dot->type == 2)
            {
                switch (UBApplication::penController->calibrationStatus) {
                    case CalibratingP1:
                        virtualScreen->calibrationSetFirstPoint(static_cast<int>(dot->x), static_cast<int>(dot->y));
                        UBApplication::penController->calibrationStatus = CalibratingP2;

                        QMetaObject::invokeMethod(QApplication::instance(), []() {
                            // Update UI
                            penCalibrationWindow->update();
                        }, Qt::QueuedConnection);
                        break;
                    case CalibratingP2:
                        if (dot->type == 2) {
                            virtualScreen->calibrationSetSecondPoint(static_cast<int>(dot->x), static_cast<int>(dot->y));
                            UBApplication::penController->calibrationStatus = Calibrated;

                            QMetaObject::invokeMethod(QApplication::instance(), []() {
                                hideCalibrationWindow();

                                if (UBApplication::penController->calibrationStatus == Calibrated)
                                    UBApplication::showMessage("Caneta calibrada");
                            }, Qt::QueuedConnection);
                        }
                        break;
                    default:
                        qWarning() << "[PEN] - Ignoring calibration dot";
                        break;
                }
            }
        }
    }

    return true;
}
