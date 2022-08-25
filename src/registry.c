/*++
    Copyright (c) Microsoft Corporation. All Rights Reserved.
    Copyright (c) Bingxing Wang. All Rights Reserved.
    Copyright (c) LumiaWoA authors. All Rights Reserved.

    Module Name:

        registry.c

    Abstract:

        This module retrieves platform-specific controller
        configuration from the registry, or assigns default
        values if no registry configuration is present.

    Environment:

        Kernel mode

    Revision History:

--*/

#include <hx83112/hxinternal.h>
#include <registry.tmh>
#include <internal.h>

#define TOUCH_REG_KEY                    L"\\Registry\\Machine\\SYSTEM\\TOUCH"

NTSTATUS
RtlReadRegistryValue(
    PCWSTR registry_path,
    PCWSTR value_name,
    ULONG type,
    PVOID data,
    ULONG length
)
{
    UNICODE_STRING valname;
    UNICODE_STRING keyname;
    OBJECT_ATTRIBUTES attribs;
    PKEY_VALUE_PARTIAL_INFORMATION pinfo;
    HANDLE handle;
    NTSTATUS rc;
    ULONG len, reslen;

    RtlInitUnicodeString(&keyname, registry_path);
    RtlInitUnicodeString(&valname, value_name);

    InitializeObjectAttributes(
        &attribs, 
        &keyname, 
        OBJ_CASE_INSENSITIVE,
        NULL, 
        NULL
    );

    rc = ZwOpenKey(
        &handle, 
        KEY_QUERY_VALUE, 
        &attribs
    );

    if (!NT_SUCCESS(rc))
    {
        return 0;
    }

    len = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + length;

    pinfo = ExAllocatePoolWithTag(
        NonPagedPool, 
        len, 
        TOUCH_POOL_TAG
    );

    if (pinfo == NULL)
    {
        goto exit;
    }

    rc = ZwQueryValueKey(
        handle, 
        &valname, 
        KeyValuePartialInformation,
        pinfo, 
        len, 
        &reslen
    );

    if ((NT_SUCCESS(rc) || rc == STATUS_BUFFER_OVERFLOW) && 
        reslen >= (sizeof(KEY_VALUE_PARTIAL_INFORMATION) - 1) &&
        (!type || pinfo->Type == type))
    {
        reslen = pinfo->DataLength;
        memcpy(data, pinfo->Data, min(length, reslen));
    }
    else
    {
        reslen = 0;
    }

    if (pinfo != NULL)
    {
        ExFreePoolWithTag(pinfo, TOUCH_POOL_TAG);
    }

exit:
    ZwClose(handle);
    return rc;
}

NTSTATUS
TchRegistryGetControllerSettings(
    IN VOID* ControllerContext,
    IN WDFDEVICE FxDevice
)
/*++

  Routine Description:

    This routine retrieves controller wide settings
    from the registry.

  Arguments:

    FxDevice - a handle to the framework device object
    Settings - A pointer to the chip settings structure

  Return Value:

    NTSTATUS indicating success or failure

--*/
{
    HIMAX_CONTROLLER_CONTEXT* controller;
    NTSTATUS status;

    UNREFERENCED_PARAMETER(FxDevice);
    UNREFERENCED_PARAMETER(ControllerContext);

    status = STATUS_SUCCESS;

    return status;
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz = siz, truncation occurred.
 * 
 * From: https://stackoverflow.com/questions/1855956/how-do-you-concatenate-two-wchar-t-together
 */
size_t wstrlcat(wchar_t* dst, const wchar_t* src, size_t siz)
{
    wchar_t* d = dst;
    const wchar_t* s = src;
    size_t n = siz;
    size_t dlen;

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != L'\0') {
        d++;
    }

    dlen = d - dst;
    n = siz - dlen;

    if (n == 0) {
        return(dlen + wcslen(s));
    }

    while (*s != L'\0')
    {
        if (n != 1)
        {
            *d++ = *s;
            n--;
        }
        s++;
    }

    *d = '\0';
    return(dlen + (s - src));        /* count does not include NUL */
}
