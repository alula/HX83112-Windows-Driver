/*++
      Copyright (c) Microsoft Corporation. All Rights Reserved.
      Sample code. Dealpoint ID #843729.

      Module Name:

            rmiinternal.c

      Abstract:

            Contains Synaptics initialization code

      Environment:

            Kernel mode

      Revision History:

--*/

#include <Cross Platform Shim\compat.h>
#include <spb.h>
#include <report.h>
#include <hx83112/hxinternal.h>
#include <ftinternal.tmh>

#define FOUR_BYTE_DATA_SZ     4
#define FOUR_BYTE_ADDR_SZ     4
#define FLASH_RW_MAX_LEN      256
#define FLASH_WRITE_BURST_SZ  8

NTSTATUS
HimaxBusWrite(
    IN SPB_CONTEXT* SpbContext,
    IN UINT8 Command,
    IN UINT8* Data,
    IN ULONG Length,
    IN UINT8 RetryCount
)
{
    NTSTATUS status = STATUS_SUCCESS;
    LARGE_INTEGER delay;

    for (UCHAR i = 0; i < RetryCount; i++) 
    {
        status = SpbWriteDataSynchronously(SpbContext, Command, Data, Length);

        if (NT_SUCCESS(status))
        {
            break;
        }

        delay.QuadPart = -20000;
        KeDelayExecutionThread(KernelMode, TRUE, &delay);
    }

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INTERRUPT,
            "Bus write error - 0x%08lX",
            status);
    }

exit:
    return status;
}

NTSTATUS
HimaxBusWriteCommand(
    IN SPB_CONTEXT* SpbContext,
    IN UINT8 Command,
    IN UINT8 RetryCount
)
{
    return HimaxBusWrite(SpbContext, Command, NULL, 0, RetryCount);
}

NTSTATUS
HimaxBusRead(
    IN SPB_CONTEXT* SpbContext,
    IN UINT8 Command,
    OUT UINT8* Data,
    IN ULONG Length,
    IN UINT8 RetryCount
)
{
    NTSTATUS status = STATUS_SUCCESS;
    LARGE_INTEGER delay;

    for (UCHAR i = 0; i < RetryCount; i++)
    {
        status = SpbReadDataSynchronously(SpbContext, Command, Data, Length);

        if (NT_SUCCESS(status))
        {
            break;
        } 

        delay.QuadPart = -20000;
        KeDelayExecutionThread(KernelMode, TRUE, &delay);
    }

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INTERRUPT,
            "Bus read error - 0x%08lX",
            status);
    }

exit:
    return status;
}


NTSTATUS
HimaxBusReadEventStack(
    IN SPB_CONTEXT* SpbContext,
    OUT UINT8* Data,
    IN ULONG Length)
{
    NTSTATUS status = STATUS_SUCCESS;
    UINT8 cmd;
    
    cmd = 0; // AHB_I2C Burst Read Off
    status = HimaxBusWrite(SpbContext, 0, &cmd, 1, HIMAX_I2C_RETRY_TIMES);
    if (!NT_SUCCESS(status)) return status;

    cmd = 0x30; // Event Stack
    status = HimaxBusRead(SpbContext, cmd, Data, Length, HIMAX_I2C_RETRY_TIMES);
    if (!NT_SUCCESS(status)) return status;

    cmd = 1; // AHB_I2C Burst Read On
    status = HimaxBusWrite(SpbContext, 0, &cmd, 1, HIMAX_I2C_RETRY_TIMES);
    if (!NT_SUCCESS(status)) return status;

    return status;
}

NTSTATUS
HimaxMCUBurstEnable(
    IN SPB_CONTEXT* SpbContext,
    IN UINT8 AutoAdd4Byte
)
{
    NTSTATUS status = STATUS_SUCCESS;
    UINT8 tmp[FOUR_BYTE_DATA_SZ];

    tmp[0] = 0x31; // ic_cmd_conti
    status = HimaxBusWrite(SpbContext, 0x13, tmp, 1, HIMAX_I2C_RETRY_TIMES);
    if (!NT_SUCCESS(status)) return status;

    tmp[0] = 0x10 | AutoAdd4Byte; // ic_cmd_incr4
    status = HimaxBusWrite(SpbContext, 0x0d, tmp, 1, HIMAX_I2C_RETRY_TIMES);
    if (!NT_SUCCESS(status)) return status;
    
    return status;
}

NTSTATUS
HimaxMCURegisterRead(
    IN SPB_CONTEXT* SpbContext,
    IN UINT32 ReadAddr,
    OUT UINT8* ReadData,
    IN ULONG ReadLength,
    IN UINT8 ConfigFlag
)
{
    NTSTATUS status = STATUS_SUCCESS;
    UINT8 tmp[FOUR_BYTE_DATA_SZ];
    int i = 0;
    int address = 0;

    if (ConfigFlag == 0) 
    {
        if (ReadLength > FLASH_RW_MAX_LEN)
        {
            Trace(
                TRACE_LEVEL_ERROR,
                TRACE_INTERRUPT,
                "MCU Register Read failed - flash chunk size is cannot be over %d",
                FLASH_RW_MAX_LEN);
            return STATUS_BUFFER_OVERFLOW;
        }

        HimaxMCUBurstEnable(SpbContext, (ReadLength > FOUR_BYTE_DATA_SZ) ? 1 : 0);

        tmp[0] = (UINT8)(ReadAddr & 0xff);
        tmp[1] = (UINT8)((ReadAddr >> 8) & 0xff);
        tmp[2] = (UINT8)((ReadAddr >> 16) & 0xff);
        tmp[3] = (UINT8)((ReadAddr >> 24) & 0xff);

        // ic_adr_ahb_addr_byte_0
        status = HimaxBusWrite(SpbContext, 0x00, tmp, FOUR_BYTE_DATA_SZ, HIMAX_I2C_RETRY_TIMES);
        if (!NT_SUCCESS(status)) return status;

        tmp[0] = 0; // ic_cmd_ahb_access_direction_read

        // ic_cmd_ahb_access_direction
        status = HimaxBusWrite(SpbContext, 0x0c, tmp, 1, HIMAX_I2C_RETRY_TIMES);
        if (!NT_SUCCESS(status)) return status;

        // ic_adr_ahb_rdata_byte_0
        status = HimaxBusRead(SpbContext, 0x08, ReadData, ReadLength, HIMAX_I2C_RETRY_TIMES);
        if (!NT_SUCCESS(status)) return status;

        if (ReadLength > FOUR_BYTE_DATA_SZ) 
        {
            HimaxMCUBurstEnable(SpbContext, 0);
        }
    }
    else if (ConfigFlag == 1) {
        status = HimaxBusRead(SpbContext, (UINT8)ReadAddr, ReadData, ReadLength, HIMAX_I2C_RETRY_TIMES);
    }

    return status;
}

NTSTATUS
HimaxMCUFlashWriteBurst(
    IN SPB_CONTEXT* SpbContext,
    IN UINT32 RegByte,
    OUT UINT8* WriteData)
{
    NTSTATUS status;
    UINT8 buffer[FLASH_WRITE_BURST_SZ];

    buffer[0] = (UINT8)(RegByte & 0xff);
    buffer[1] = (UINT8)((RegByte >> 8) & 0xff);
    buffer[2] = (UINT8)((RegByte >> 16) & 0xff);
    buffer[3] = (UINT8)((RegByte >> 24) & 0xff);
    buffer[4] = WriteData[0];
    buffer[5] = WriteData[1];
    buffer[6] = WriteData[2];
    buffer[7] = WriteData[3];

    // ic_adr_ahb_addr_byte_0
    status = HimaxBusWrite(SpbContext, 0, buffer, FLASH_WRITE_BURST_SZ, HIMAX_I2C_RETRY_TIMES);

    return status;
}

NTSTATUS
HimaxMCUFlashWriteBurstLength(
    IN SPB_CONTEXT* SpbContext,
    IN UINT32 RegByte,
    OUT UINT8* WriteData,
    IN ULONG Length)
{
    NTSTATUS status;
    UINT8* buffer;
    ULONG bufferLen = Length + 4;

    buffer = ExAllocatePoolWithTag(
        NonPagedPoolNx,
        bufferLen,
        TOUCH_POOL_TAG_F12);

    if (buffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    buffer[0] = (UINT8)(RegByte & 0xff);
    buffer[1] = (UINT8)((RegByte >> 8) & 0xff);
    buffer[2] = (UINT8)((RegByte >> 16) & 0xff);
    buffer[3] = (UINT8)((RegByte >> 24) & 0xff);

    RtlCopyMemory(&buffer[4], WriteData, Length);

    // ic_adr_ahb_addr_byte_0
    status = HimaxBusWrite(SpbContext, 0, buffer, bufferLen, HIMAX_I2C_RETRY_TIMES);
    
    ExFreePoolWithTag(
        buffer,
        TOUCH_POOL_TAG_F12
    );

    return status;
}

NTSTATUS
HimaxMCURegisterWrite(
    IN SPB_CONTEXT* SpbContext,
    IN UINT32 WriteAddr,
    OUT UINT8* WriteData,
    IN ULONG WriteLength,
    IN UINT8 ConfigFlag
)
{
    NTSTATUS status = STATUS_SUCCESS;
    UINT8 tmp[FOUR_BYTE_DATA_SZ];
    int i = 0;
    int address = 0;

    if (ConfigFlag == 0)
    {
        UINT32 EndAddr = WriteAddr + (UINT32)WriteLength;
        for (UINT32 i = WriteAddr; i < EndAddr; i++)
        {
            HimaxMCUBurstEnable(SpbContext, (WriteLength > FOUR_BYTE_DATA_SZ) ? 1 : 0);
            HimaxMCUFlashWriteBurstLength(SpbContext, WriteAddr, WriteData, WriteLength);
        }
    }
    else if (ConfigFlag == 1) {
        status = HimaxBusWrite(SpbContext, (UINT8)WriteAddr, WriteData, WriteLength, HIMAX_I2C_RETRY_TIMES);
    }

    return status;
}

NTSTATUS HimaxMCUInterfaceOn(
    IN SPB_CONTEXT* SpbContext
)
{
    NTSTATUS status = STATUS_SUCCESS;
    LARGE_INTEGER delay;
    UINT8 tmp[FOUR_BYTE_DATA_SZ];
    int cnt = 0;

    // Read a dummy register to wake up I2C.
    
    // ic_adr_ahb_rdata_byte_0
    status = HimaxBusRead(SpbContext, 0x08, tmp, FOUR_BYTE_DATA_SZ, HIMAX_I2C_RETRY_TIMES);
    if (!NT_SUCCESS(status)) return status;

    do {
        // ============================================
        // Enable continuous burst mode : 0x13 ==> 0x31
        // ============================================
        tmp[0] = 0x31; // ic_cmd_conti
        status = HimaxBusWrite(SpbContext, 0x13, tmp, 1, HIMAX_I2C_RETRY_TIMES);
        if (!NT_SUCCESS(status)) return status;

        // ============================================
        //   AHB address auto + 4		: 0x0D ==> 0x11
        //  Do not AHB address auto + 4 : 0x0D ==> 0x10
        // ============================================

        tmp[0] = 0x10; // ic_cmd_incr4
        status = HimaxBusWrite(SpbContext, 0x0d, tmp, 1, HIMAX_I2C_RETRY_TIMES);
        if (!NT_SUCCESS(status)) return status;

        UINT8 cmd_conti;
        UINT8 cmd_incr4;

        HimaxBusRead(SpbContext, 0x13, &cmd_conti, sizeof(cmd_conti), HIMAX_I2C_RETRY_TIMES);
        HimaxBusRead(SpbContext, 0x0d, &cmd_incr4, sizeof(cmd_incr4), HIMAX_I2C_RETRY_TIMES);

        if (cmd_conti == 0x31 && cmd_incr4 == 0x10)
        {
            break;
        }

        delay.QuadPart = -1000;
        KeDelayExecutionThread(KernelMode, TRUE, &delay);
    } while (++cnt < 10);

    if (cnt > 0)
    {
        Trace(
            TRACE_LEVEL_INFORMATION,
            TRACE_INTERRUPT,
            "Polling burst mode: %d times",
            cnt);
    }

    return status;
}

NTSTATUS HimaxMCUSystemReset(
    IN SPB_CONTEXT* SpbContext
)
{
    UINT8 tmp[FOUR_BYTE_DATA_SZ];
    RtlZeroMemory(tmp, sizeof(tmp));

    tmp[0] = 0x55; // fw_data_system_reset

    // addr_system_reset
    HimaxMCURegisterWrite(SpbContext, 0x90000018, tmp, FOUR_BYTE_DATA_SZ, 0);
}

NTSTATUS HimaxMCUSenseOn(
    IN SPB_CONTEXT* SpbContext,
    IN UINT8 FlashMode)
{
    NTSTATUS status = STATUS_SUCCESS;
    UINT8 tmp[FOUR_BYTE_DATA_SZ];
    int retry = 0;
    LARGE_INTEGER delay;

    memset(tmp, 0, sizeof(tmp));

    HimaxMCUInterfaceOn(SpbContext);
    HimaxMCURegisterWrite(SpbContext, 0x9000005c, tmp, FOUR_BYTE_DATA_SZ, 0);

    delay.QuadPart = -20000;
    KeDelayExecutionThread(KernelMode, TRUE, &delay);

    if (FlashMode == 0)
    {
        HimaxMCUSystemReset(SpbContext);
    }
    else {
        do {
            // fw_data_safe_mode_release_pw_active
            tmp[3] = 0x00; 
            tmp[2] = 0x00; 
            tmp[1] = 0x00; 
            tmp[0] = 0x53;

            // fw_addr_safe_mode_release_pw
            HimaxMCURegisterWrite(SpbContext, 0x90000098, tmp, FOUR_BYTE_DATA_SZ, 0);

            // fw_addr_flag_reset_event
            HimaxMCURegisterRead(SpbContext, 0x900000e4, tmp, FOUR_BYTE_DATA_SZ, 0);
        } while ((tmp[1] != 0x01 || tmp[0] != 0x00) && retry++ < 5);

        if (retry >= 5)
        {
            Trace(
                TRACE_LEVEL_ERROR,
                TRACE_INTERRUPT,
                "Safe mode release failed.");
            HimaxMCUSystemReset(SpbContext);
        }
        else {
            Trace(
                TRACE_LEVEL_INFORMATION,
                TRACE_INTERRUPT,
                "OK and Read status from IC = %x,%x",
                tmp[0], tmp[1]);

            tmp[0] = 0x00;

            // ic_adr_i2c_psw_lb
            status = HimaxBusWrite(SpbContext, 0x31, tmp, 1, HIMAX_I2C_RETRY_TIMES);
            if (!NT_SUCCESS(status)) return status;
            
            // ic_adr_i2c_psw_ub
            status = HimaxBusWrite(SpbContext, 0x32, tmp, 1, HIMAX_I2C_RETRY_TIMES);
            if (!NT_SUCCESS(status)) return status;

            // fw_data_safe_mode_release_pw_reset
            tmp[3] = 0x00;
            tmp[2] = 0x00;
            tmp[1] = 0x00;
            tmp[0] = 0x00;

            // fw_addr_safe_mode_release_pw
            HimaxMCURegisterWrite(SpbContext, 0x90000098, tmp, FOUR_BYTE_DATA_SZ, 0);
        }
    }
}

NTSTATUS HimaxMCUAssignSortingMode(
    IN SPB_CONTEXT* SpbContext,
    UINT8* Data
)
{
    Trace(
        TRACE_LEVEL_INFORMATION,
        TRACE_INTERRUPT,
        "Now tmp_data[3]=0x%02x,tmp_data[2]=0x%02x,tmp_data[1]=0x%02x,tmp_data[0]=0x%02x",
        Data[3], Data[2], Data[1], Data[0]);

    return HimaxMCUFlashWriteBurst(SpbContext, 0x10007f04, Data);
}

// todo read
#define HX_MAX_PT 10
const int raw_cnt_max = HX_MAX_PT / 4;
const int raw_cnt_rmd = HX_MAX_PT % 4;
const int g_hx_rawdata_size = 67;
const int touch_info_size = 56;
const int coord_data_size = 4 * HX_MAX_PT;
const int area_data_size = ((HX_MAX_PT / 4) + (HX_MAX_PT % 4 ? 1 : 0)) * 4;
const int coord_info_size = (4 * HX_MAX_PT) + (((HX_MAX_PT / 4) + (HX_MAX_PT % 4 ? 1 : 0)) * 4) + 4;
//const int coord_info_size = coord_data_size + area_data_size + 4;

NTSTATUS
HimaxBuildFunctionsTable(
      IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);

      return STATUS_SUCCESS;
}

NTSTATUS
HimaxChangePage(
      IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN int DesiredPage
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(DesiredPage);

      return STATUS_SUCCESS;
}

NTSTATUS
HimaxConfigureFunctions(
      IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext
)
{
      UINT8 tmp[FOUR_BYTE_DATA_SZ];
      RtlZeroMemory(tmp, sizeof(tmp));

      HimaxMCURegisterWrite(SpbContext, 0x800204b4, tmp, FOUR_BYTE_DATA_SZ, 0);
      HimaxMCUAssignSortingMode(SpbContext, tmp);
      HimaxMCUSenseOn(SpbContext, 0x00);

      ControllerContext->MaxFingers = HX_MAX_PT;

      return STATUS_SUCCESS;
}

NTSTATUS
HimaxGetObjectStatusFromControllerF12(
      IN VOID* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN DETECTED_OBJECTS* Data
)
/*++

Routine Description:

      This routine reads raw touch messages from hardware. If there is
      no touch data available (if a non-touch interrupt fired), the
      function will not return success and no touch data was transferred.

Arguments:

      ControllerContext - Touch controller context
      SpbContext - A pointer to the current i2c context
      Data - A pointer to any returned F11 touch data

Return Value:

      NTSTATUS, where only success indicates data was returned

--*/
{
      NTSTATUS status;
      HIMAX_CONTROLLER_CONTEXT* controller;

      HIMAX_EVENT_DATA controllerData;
      controller = (HIMAX_CONTROLLER_CONTEXT*)ControllerContext;

      status = HimaxBusReadEventStack(SpbContext, &controllerData, sizeof(controllerData));

      if (!NT_SUCCESS(status))
      {
            Trace(
                  TRACE_LEVEL_ERROR,
                  TRACE_INTERRUPT,
                  "Error reading finger status data - 0x%08lX",
                  status);

            goto exit;
      }

      RtlCopyMemory(controller->CoordBuf, &controllerData.data, touch_info_size);

      if (controllerData.state_info[0] != 0xff && controllerData.state_info[1] != 0xff)
      {
          RtlCopyMemory(controller->StateInfo, &controllerData.state_info, 2);
      }
      else {
          RtlZeroMemory(controller->StateInfo, 2);
      }

      int x = 0;
      int y = 0;
      int w = 0;
      int base = 0;
      int loop_i = 0;

      controller->OldFinger = controller->PreFingerMask;
      controller->PreFingerMask = 0;
      controller->FingerNum = controller->CoordBuf[coord_info_size - 4] & 0x0f;
      controller->FingerOn = 1;
      controller->AAPress = 1;

      for (loop_i = 0; loop_i < HX_MAX_PT; loop_i++)
      {
          base = loop_i * 4;
          x = (int)controller->CoordBuf[base] << 8 | (int)controller->CoordBuf[base + 1];
          y = (int)controller->CoordBuf[base + 2] << 8 | (int)controller->CoordBuf[base + 3];
          w = (int)controller->CoordBuf[(HX_MAX_PT * 4) + loop_i];

          if (x >= 0 && x <= 1080 && y >= 0 && y <= 2160)
          {
              Data->States[loop_i] = OBJECT_STATE_FINGER_PRESENT_WITH_ACCURATE_POS;

              Data->Positions[loop_i].X = x;
              Data->Positions[loop_i].Y = y;

              /*Trace(
                  TRACE_LEVEL_INFORMATION,
                  TRACE_INTERRUPT,
                  "finger[%d]: x=%d, y=%d, w=%d",
                  loop_i, x, y, w);*/
          }
      }

exit:
      return status;
}

NTSTATUS
TchServiceObjectInterrupts(
      IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN PREPORT_CONTEXT ReportContext
)
{
      NTSTATUS status = STATUS_SUCCESS;
      DETECTED_OBJECTS data;
      RtlZeroMemory(&data, sizeof(data));

      //
      // See if new touch data is available
      //
      status = HimaxGetObjectStatusFromControllerF12(
            ControllerContext,
            SpbContext,
            &data
      );

      if (!NT_SUCCESS(status))
      {
            Trace(
                  TRACE_LEVEL_VERBOSE,
                  TRACE_SAMPLES,
                  "No object data to report - 0x%08lX",
                  status);

            goto exit;
      }

      if (ControllerContext->ProcessReports)
      {
          status = ReportObjects(
              ReportContext,
              data);
      }

      if (!NT_SUCCESS(status))
      {
            Trace(
                  TRACE_LEVEL_VERBOSE,
                  TRACE_SAMPLES,
                  "Error while reporting objects - 0x%08lX",
                  status);

            goto exit;
      }

exit:
      return status;
}


NTSTATUS
HimaxServiceInterrupts(
      IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN PREPORT_CONTEXT ReportContext
)
{
      NTSTATUS status = STATUS_SUCCESS;

      TchServiceObjectInterrupts(ControllerContext, SpbContext, ReportContext);

      return status;
}

NTSTATUS
HimaxSetReportingFlagsF12(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR NewMode,
    OUT UCHAR* OldMode
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(NewMode);
      UNREFERENCED_PARAMETER(OldMode);

      if (NewMode == HX83112_F12_REPORTING_CONTINUOUS_MODE)
      {
          ControllerContext->ProcessReports = TRUE;
      }

      return STATUS_SUCCESS;
}

NTSTATUS
HimaxChangeChargerConnectedState(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR ChargerConnectedState
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(ChargerConnectedState);

      return STATUS_SUCCESS;
}

NTSTATUS
HimaxChangeSleepState(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR SleepState
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(SleepState);

      return STATUS_SUCCESS;
}

NTSTATUS
HimaxGetFirmwareVersion(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);

      return STATUS_SUCCESS;
}

NTSTATUS
HimaxCheckInterrupts(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN ULONG* InterruptStatus
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(InterruptStatus);

      return STATUS_SUCCESS;
}

NTSTATUS
HimaxConfigureInterruptEnable(
    IN HIMAX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);

      return STATUS_SUCCESS;
}