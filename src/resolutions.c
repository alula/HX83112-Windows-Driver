/*++
    Copyright (c) Microsoft Corporation. All Rights Reserved.
    Copyright (c) Bingxing Wang. All Rights Reserved.
    Copyright (c) LumiaWoA authors. All Rights Reserved.

    Module Name: 

        resolutions.c

    Abstract:

        This module retrieves platform-specific configuration
        parameters from the registry, and translates touch
        controller pixel units to display pixel units.

    Environment:

        Kernel Mode

    Revision History:

--*/

#include <wdm.h>
#include <controller.h>
#include <resolutions.h>
#include <resolutions.tmh>

//
// Registry values explaining the relationship of the touch
// controller coordinates to the physical LCD, as well as
// any differences between the physical LCD dimensons and
// viewable LCD area, are required. If not provided for whatever
// reason, we will assume everything is 480x800 and perfectly
// aligned.
//

TOUCH_SCREEN_PROPERTIES gDefaultProperties =
{
    0x0, // TouchSwapAxes
    0x0, // TouchInvertXAxis
    0x0, // TouchInvertYAxis
    TOUCH_DEFAULT_RESOLUTION_X, // TouchPhysicalWidth
    TOUCH_DEFAULT_RESOLUTION_Y, // TouchPhysicalHeight
    0x0, // TouchPhysicalButtonHeight
    0x0, // TouchPillarBoxWidthLeft
    0x0, // TouchPillarBoxWidthRight
    0x0, // TouchLetterBoxHeightTop
    0x0, // TouchLetterBoxHeightBottom
    TOUCH_DEFAULT_RESOLUTION_X, // DisplayPhysicalWidth
    TOUCH_DEFAULT_RESOLUTION_Y, // DisplayPhysicalHeight
    TOUCH_DEFAULT_RESOLUTION_X, // DisplayViewableWidth
    TOUCH_DEFAULT_RESOLUTION_Y, // DisplayViewableHeight
    0x0, // DisplayPillarBoxWidthLeft
    0x0, // DisplayPillarBoxWidthRight
    0x0, // DisplayLetterBoxHeightTop
    0x0, // DisplayLetterBoxHeightBottom
    0x1ac2, // DisplayHeight10um
    0x3840, // DisplayWidth10um
    0x1 // TouchHardwareLacksContinuousReporting
};


RTL_QUERY_REGISTRY_TABLE gResParamsRegTable[] =
{
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"TouchSwapAxes",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, TouchSwapAxes)),
        REG_DWORD,
        &gDefaultProperties.TouchSwapAxes,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"TouchInvertXAxis",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, TouchInvertXAxis)),
        REG_DWORD,
        &gDefaultProperties.TouchInvertXAxis,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"TouchInvertYAxis",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, TouchInvertYAxis)),
        REG_DWORD,
        &gDefaultProperties.TouchInvertYAxis,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"TouchPhysicalWidth",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, TouchPhysicalWidth)),
        REG_DWORD,
        &gDefaultProperties.TouchPhysicalWidth,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"TouchPhysicalHeight",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, TouchPhysicalHeight)),
        REG_DWORD,
        &gDefaultProperties.TouchPhysicalHeight,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"TouchPhysicalButtonHeight",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, TouchPhysicalButtonHeight)),
        REG_DWORD,
        &gDefaultProperties.TouchPhysicalButtonHeight,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"TouchPillarBoxWidthLeft",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, TouchPillarBoxWidthLeft)),
        REG_DWORD,
        &gDefaultProperties.TouchPillarBoxWidthLeft,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"TouchPillarBoxWidthRight",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, TouchPillarBoxWidthRight)),
        REG_DWORD,
        &gDefaultProperties.TouchPillarBoxWidthRight,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"TouchLetterBoxHeightTop",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, TouchLetterBoxHeightTop)),
        REG_DWORD,
        &gDefaultProperties.TouchLetterBoxHeightTop,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"TouchLetterBoxHeightBottom",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, TouchLetterBoxHeightBottom)),
        REG_DWORD,
        &gDefaultProperties.TouchLetterBoxHeightBottom,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DisplayPhysicalWidth",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, DisplayPhysicalWidth)),
        REG_DWORD,
        &gDefaultProperties.DisplayPhysicalWidth,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DisplayPhysicalHeight",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, DisplayPhysicalHeight)),
        REG_DWORD,
        &gDefaultProperties.DisplayPhysicalHeight,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DisplayViewableWidth",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, DisplayViewableWidth)),
        REG_DWORD,
        &gDefaultProperties.DisplayViewableWidth,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DisplayViewableHeight",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, DisplayViewableHeight)),
        REG_DWORD,
        &gDefaultProperties.DisplayViewableHeight,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DisplayPillarBoxWidthLeft",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, DisplayPillarBoxWidthLeft)),
        REG_DWORD,
        &gDefaultProperties.DisplayPillarBoxWidthLeft,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DisplayPillarBoxWidthRight",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, DisplayPillarBoxWidthRight)),
        REG_DWORD,
        &gDefaultProperties.DisplayPillarBoxWidthRight,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DisplayLetterBoxHeightTop",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, DisplayLetterBoxHeightTop)),
        REG_DWORD,
        &gDefaultProperties.DisplayLetterBoxHeightTop,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DisplayLetterBoxHeightBottom",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, DisplayLetterBoxHeightBottom)),
        REG_DWORD,
        &gDefaultProperties.DisplayLetterBoxHeightBottom,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DisplayHeight10um",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, DisplayHeight10um)),
        REG_DWORD,
        &gDefaultProperties.DisplayHeight10um,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DisplayWidth10um",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, DisplayWidth10um)),
        REG_DWORD,
        &gDefaultProperties.DisplayWidth10um,
        sizeof(ULONG)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"TouchHardwareLacksContinuousReporting",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_PROPERTIES, TouchHardwareLacksContinuousReporting)),
        REG_DWORD,
        &gDefaultProperties.TouchHardwareLacksContinuousReporting,
        sizeof(ULONG)
    },
    //
    // List Terminator - set to NULL to indicate end of table
    //
    {
        NULL, 0,
        NULL,
        0,
        REG_DWORD,
        NULL,
        0
    }
};

static const ULONG gcbRegistryTable = sizeof(gResParamsRegTable);
static const ULONG gcRegistryTable = 
    sizeof(gResParamsRegTable) / sizeof(gResParamsRegTable[0]);


VOID
TchTranslateToDisplayCoordinates(
    IN PUSHORT PX,
    IN PUSHORT PY,
    IN PTOUCH_SCREEN_PROPERTIES Props
    )
/*++
 
  Routine Description:

    This routine performs translations on touch coordinates
    to ensure points reported to the OS match pixels on the
    display.

  Arguments:

    X - pointer to the pre-processed X coordinate
    Y - pointer the pre-processed Y coordinate
    Props - pointer to screen information

  Return Value:

    None. The X/Y values will be modified by this function.

--*/
{
    return;

    ULONG X;
    ULONG Y;

    //
    // Avoid overflow
    //
    X = (ULONG) *PX;
    Y = (ULONG) *PY;

    //
    // Swap the axes reported by the touch controller if requested
    //
    if (Props->TouchSwapAxes)
    {
        ULONG temp = Y;
        Y = X;
        X = temp;
    }

    //
    // Invert the coordinates as requested
    //
    if (Props->TouchInvertXAxis)
    {
        if (X >= Props->TouchPhysicalWidth)
        {
            X = Props->TouchPhysicalWidth - 1u;
        }

        X = Props->TouchPhysicalWidth - X - 1u;
    }
    if (Props->TouchInvertYAxis)
    {
        if (Y >= Props->TouchPhysicalHeight)
        {
            Y = Props->TouchPhysicalHeight - 1u;
        }

        Y = Props->TouchPhysicalHeight - Y - 1u;
    }

    //
    // Handle touch clipping boundaries so touch matches
    // the physical display
    //
    if (X <= Props->TouchPillarBoxWidthLeft)
    {
        X = 0;
    }
    else
    {
        X -= Props->TouchPillarBoxWidthLeft;
    }

    if (X >= Props->TouchPhysicalWidth - Props->TouchPillarBoxWidthRight)
    {
        X = Props->TouchPhysicalWidth;
    }
    else
    {
        X += Props->TouchPillarBoxWidthRight;
    }

    if (Y <= Props->TouchLetterBoxHeightTop)
    {
        Y = 0;
    }
    else
    {
        Y -= Props->TouchLetterBoxHeightTop;
    }

    if (Y >= Props->TouchPhysicalHeight - Props->TouchLetterBoxHeightBottom)
    {
        Y = Props->TouchPhysicalHeight;
    }
    else
    {
        Y += Props->TouchLetterBoxHeightBottom;
    }

    //
    // Scale the raw touch pixel units into physical display pixels,
    // leaving off the capacitive button region.
    //
    X = X * Props->DisplayPhysicalWidth / Props->TouchPhysicalWidth;
    Y = Y * Props->DisplayPhysicalHeight / 
        (Props->TouchPhysicalHeight - Props->TouchPhysicalButtonHeight);

    //
    // If the display is additionally being letterboxed or pillarboxed, make
    // further adjustments to the touch coordinates.
    //
    if (X <= Props->DisplayPillarBoxWidthLeft)
    {
        X = 0;
    }
    else
    {
        X -= Props->DisplayPillarBoxWidthLeft;
    }

    if (X >= Props->DisplayPhysicalWidth - Props->DisplayPillarBoxWidthRight)
    {
        X = Props->DisplayPhysicalWidth;
    }
    else
    {
        X += Props->DisplayPillarBoxWidthRight;
    }

    if (Y <= Props->DisplayLetterBoxHeightTop)
    {
        Y = 0;
    }
    else
    {
        Y -= Props->DisplayLetterBoxHeightTop;
    }

    if (Y >= Props->DisplayPhysicalHeight - Props->DisplayLetterBoxHeightBottom)
    {
        Y = Props->DisplayPhysicalHeight;
    }
    else
    {
        Y += Props->DisplayLetterBoxHeightBottom;
    }

    *PX = (USHORT) X;
    *PY = (USHORT) Y;
}

VOID
TchGetScreenProperties(
    IN PTOUCH_SCREEN_PROPERTIES Props
    )
/*++
 
  Routine Description:

    This routine retrieves coordinate translation settings
    from the registry.

  Arguments:

    Props - receives the Props

  Return Value:

    None. On failure, defaults are returned.

--*/
{
    ULONG i;
    PRTL_QUERY_REGISTRY_TABLE regTable;
    NTSTATUS status;

    regTable = NULL;

    //
    // Table passed to RtlQueryRegistryValues must be allocated 
    // from NonPagedPoolNx
    //
    regTable = ExAllocatePoolWithTag(
        NonPagedPoolNx,
        gcbRegistryTable,
        TOUCH_POOL_TAG);

    if (regTable == NULL)
    {
        return;
    }

    RtlCopyMemory(
        regTable,
        gResParamsRegTable,
        gcbRegistryTable);

    //
    // Update offset values with base pointer
    // 
    for (i=0; i < gcRegistryTable-1; i++)
    {
        regTable[i].EntryContext = (PVOID) (
            ((SIZE_T) regTable[i].EntryContext) +
            ((ULONG_PTR) Props));
    }

    //
    // Start with default values
    //
    RtlCopyMemory(
        Props,
        &gDefaultProperties,
        sizeof(TOUCH_SCREEN_PROPERTIES));

    //
    // Populate device context with registry overrides (or defaults)
    //
    status = RtlQueryRegistryValues(
        RTL_REGISTRY_ABSOLUTE,
        TOUCH_SCREEN_PROPERTIES_REG_KEY,
        regTable,
        NULL,
        NULL);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_WARNING,
            TRACE_REGISTRY,
            "Error retrieving registry configuration - 0x%08lX",
            status);
    }

    //
    // Sanity check values provided from the registry
    //

    if (Props->TouchPillarBoxWidthLeft + 
        Props->TouchPillarBoxWidthRight >=
        Props->TouchPhysicalWidth)
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_REGISTRY,
            "Invalid pillar box widths provided (%d,%d for %d)",
            Props->TouchPillarBoxWidthLeft,
            Props->TouchPillarBoxWidthRight,
            Props->TouchPhysicalWidth);

        Props->TouchPillarBoxWidthLeft = 
            gDefaultProperties.TouchPillarBoxWidthLeft;
        Props->TouchPillarBoxWidthRight = 
            gDefaultProperties.TouchPillarBoxWidthRight;

    }

    if (Props->TouchLetterBoxHeightTop + 
        Props->TouchLetterBoxHeightBottom >=
        Props->TouchPhysicalHeight)
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_REGISTRY,
            "Invalid letter box heights provided (%d,%d for %d)",
            Props->TouchLetterBoxHeightTop,
            Props->TouchLetterBoxHeightBottom,
            Props->TouchPhysicalHeight);

        Props->TouchLetterBoxHeightTop = 
            gDefaultProperties.TouchLetterBoxHeightTop;
        Props->TouchLetterBoxHeightBottom = 
            gDefaultProperties.TouchLetterBoxHeightBottom;
    }

    if (regTable != NULL)
    {
        ExFreePoolWithTag(regTable, TOUCH_POOL_TAG);
    }
}