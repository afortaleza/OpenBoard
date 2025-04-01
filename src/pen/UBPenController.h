#ifndef UBPENCONTROLLER_H
#define UBPENCONTROLLER_H

#include <Windows.h>
#include <QString>
#include "qobject.h"
#include "xb.h"

// PenTipStatus
enum PenTipStatus { PenUp, PenDown, PenMove };

class UBPenController: public QObject
{
    Q_OBJECT

    public:
        ~UBPenController();
        static UBPenController* getInstance();  // Method to get the singleton instance
        PenTipStatus penTipStatus = PenUp;
        void showMessageDialog();
        void hideMessageDialog();
        void loadPenSDK();
        void connect();

    signals:
        void sdkLoaded();
        void scanningAndConnecting();
        void connected();
        void disconnected();

    private:
        UBPenController();
        UBPenController(const UBPenController&) = delete;
        UBPenController& operator=(const UBPenController&) = delete;

        void showCalibrationWindow();
        void hideCalibrationWindow();
        void setPaperSizes();

        static UBPenController* instance;
        static QString safeCharToQString(const char* str, size_t length);

        static bool __stdcall bleEventCallback(BLE_EVENT_TYPE evtType, uint8_t* data, int len);
        static bool __stdcall penEventCallback(PEN_EVENT_TYPE evtType, uint8_t* data, int len);

        // SDK function pointers
        typedef int (__stdcall *PFN_AFInit)();
        typedef int (__stdcall *PFN_AFConnect)(const wchar_t*);
        typedef int (__stdcall *PFN_AFDisConnect)();
        typedef int (__stdcall *PFN_AFScanStart)();
        typedef int (__stdcall *PFN_AFScanStop)();
        typedef int (__stdcall *PFN_AFSetPenEventListener)(AFIPenEvent);
        typedef int (__stdcall *PFN_AFSetBleEventListener)(AFIBleEvent);
        typedef int (__stdcall *PFN_AFGetFWInfo)();
        typedef int (__stdcall *PFN_AFSetPaperSizes)(AFAPaperSize* v, size_t size);
        typedef int (__stdcall *PFN_AFGetStorageSize)();
        typedef int (__stdcall *PFN_AFGetBatteryInfo)();
        typedef int (__stdcall *PFN_AFUnInit)();
        typedef int (__stdcall *PFN_AFGetDotsCount)();
        typedef int (__stdcall *PFN_AFClearDots)();

        PFN_AFInit pAFInit = nullptr;
        PFN_AFConnect pAFConnect = nullptr;
        PFN_AFDisConnect pAFDisConnect = nullptr;
        PFN_AFScanStart pAFScanStart = nullptr;
        PFN_AFScanStop pAFScanStop = nullptr;
        PFN_AFSetPenEventListener pAFSetPenEventListener = nullptr;
        PFN_AFSetBleEventListener pAFSetBleEventListener = nullptr;
        PFN_AFGetFWInfo pAFGetFWInfo = nullptr;
        PFN_AFSetPaperSizes pAFSetPaperSizes = nullptr;
        PFN_AFGetStorageSize pAFGetStorageSize = nullptr;
        PFN_AFGetBatteryInfo pAFGetBatteryInfo = nullptr;
        PFN_AFUnInit pAFUnInit = nullptr;
        PFN_AFGetDotsCount pAFGetDotsCount = nullptr;
        PFN_AFClearDots pAFClearDots = nullptr;

        HMODULE hPenSDK = nullptr;
};

#endif // UBPENCONTROLLER_H
