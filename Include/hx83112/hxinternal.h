/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Sample code. Dealpoint ID #843729.

	Module Name:

		hxinternal.h

	Abstract:

		Contains common types and defintions used internally
		by the multi touch screen driver.

	Environment:

		Kernel mode

	Revision History:

--*/

#pragma once

#include <wdm.h>
#include <wdf.h>
#include <controller.h>
#include <resolutions.h>
#include <Cross Platform Shim/bitops.h>
#include <Cross Platform Shim/hweight.h>
#include <report.h>

// Ignore warning C4152: nonstandard extension, function/data pointer conversion in expression
#pragma warning (disable : 4152)

// Ignore warning C4201: nonstandard extension used : nameless struct/union
#pragma warning (disable : 4201)

// Ignore warning C4201: nonstandard extension used : bit field types other than in
#pragma warning (disable : 4214)

// Ignore warning C4324: 'xxx' : structure was padded due to __declspec(align())
#pragma warning (disable : 4324)

typedef struct _HIMAX_EVENT_DATA
{
	BYTE data[53];
	BYTE state_info[3];
} HIMAX_EVENT_DATA, * PHIMAX_EVENT_DATA;

#define TOUCH_POOL_TAG_F12              (ULONG)'21oT'
#define HIMAX_MAX_DATA_SIZE				8191
#define HIMAX_I2C_RETRY_TIMES 10

#define HX83112_MILLISECONDS_TO_TENTH_MILLISECONDS(n) n/10
#define HX83112_SECONDS_TO_HALF_SECONDS(n) 2*n

//
// Driver structures
//

typedef struct _HX83112_CONFIGURATION
{
	UINT32 PepRemovesVoltageInD3;
} HX83112_CONFIGURATION;

typedef struct _HX83112_CONTROLLER_CONTEXT
{
	WDFDEVICE FxDevice;
	WDFWAITLOCK ControllerLock;

	//
	// Power state
	//
	DEVICE_POWER_STATE DevicePowerState;

	UCHAR Data1Offset;

	BYTE MaxFingers;

    int HidQueueCount;

	UINT8 FingerNum;
	UINT8 FingerOn;
	UINT8 CoordBuf[56];
	UINT8 StateInfo[2];
	UINT8 AAPress;

	UINT16 PreFingerMask;
	UINT16 OldFinger;
	BOOLEAN ProcessReports;
} HIMAX_CONTROLLER_CONTEXT;

NTSTATUS
HimaxBuildFunctionsTable(
	IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
	IN SPB_CONTEXT* SpbContext
);

NTSTATUS
HimaxChangePage(
	IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
	IN SPB_CONTEXT* SpbContext,
	IN int DesiredPage
);

NTSTATUS
HimaxConfigureFunctions(
	IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
	IN SPB_CONTEXT* SpbContext
);

NTSTATUS
HimaxServiceInterrupts(
	IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
	IN SPB_CONTEXT* SpbContext,
	IN PREPORT_CONTEXT ReportContext
);

#define HX83112_F01_DEVICE_CONTROL_SLEEP_MODE_OPERATING  0
#define HX83112_F01_DEVICE_CONTROL_SLEEP_MODE_SLEEPING   1

#pragma pack(push)
#pragma pack(1)
typedef enum _HX83112_F12_REPORTING_FLAGS
{
	HX83112_F12_REPORTING_CONTINUOUS_MODE = 0,
	HX83112_F12_REPORTING_REDUCED_MODE = 1,
	HX83112_F12_REPORTING_WAKEUP_GESTURE_MODE = 2,
} HX83112_F12_REPORTING_FLAGS;
#pragma pack(pop)

NTSTATUS
HimaxSetReportingFlagsF12(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR NewMode,
    OUT UCHAR* OldMode
);

NTSTATUS
HimaxChangeChargerConnectedState(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR ChargerConnectedState
);

NTSTATUS
HimaxChangeSleepState(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR SleepState
);

NTSTATUS
HimaxGetFirmwareVersion(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext
);

NTSTATUS
HimaxCheckInterrupts(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN ULONG* InterruptStatus
);

NTSTATUS
HimaxConfigureInterruptEnable(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext
);

