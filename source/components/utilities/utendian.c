/******************************************************************************
 *
 * Module Name: utendian -- byte swapping support for other-endianness
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Copyright (c) 2020, Al Stone <ahs3@redhat.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, you may choose to be licensed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 *****************************************************************************/

#include "acpi.h"
#include "accommon.h"

#define _COMPONENT          ACPI_COMPILER
        ACPI_MODULE_NAME    ("utendian")


#if (defined ACPI_ASL_COMPILER || defined ACPI_EXEC_APP || defined ACPI_HELP_APP || ACPI_NAMES_APP)
/*
 * Endianness support functions.
 *
 * Ultimately, everything in ACPI tables or AML must be in little-endian
 * format.  However, we sometimes find it necessary to run on a big-endian
 * machine and create or read those little-endian values.  This is a small
 * libary of functions to make that easier, and more visible.
 *
 */

#ifdef ACPI_BIG_ENDIAN

static void
__SwapBytes (
    void                    *SrcPtr,
    UINT32                  SrcCount,
    void                    *DstPtr,
    UINT32                  DstCount)
{
    UINT8 *Src = (UINT8 *)SrcPtr;
    UINT8 *Dst = (UINT8 *)DstPtr;
    UINT64 Value;
    UINT8 *Ptr = (UINT8 *)&Value;

    if (SrcCount == DstCount)
    {
        Value = 0;
	memcpy(Ptr, Src, SrcCount);
        switch (DstCount)
        {
            case 2:  /* 16-bit quantity */
	        {
		    Dst[0] = Ptr[1];
		    Dst[1] = Ptr[0];
	        }
	        break;

            case 4:  /* 32-bit quantity */
	        {
		    Dst[0] = Ptr[3];
		    Dst[1] = Ptr[2];
		    Dst[2] = Ptr[1];
		    Dst[3] = Ptr[0];
	        }
	        break;

	    case 6:  /* 6-byte entity: odd case for things like OEM ID */
	        {
		    Dst[0] = Ptr[5];
		    Dst[1] = Ptr[4];
		    Dst[2] = Ptr[3];
		    Dst[3] = Ptr[2];
		    Dst[4] = Ptr[1];
		    Dst[5] = Ptr[0];
	        }
	        break;

            case 8:  /* 64-bit quantity */
	        {
		    Dst[0] = Ptr[7];
		    Dst[1] = Ptr[6];
		    Dst[2] = Ptr[5];
		    Dst[3] = Ptr[4];
		    Dst[4] = Ptr[3];
		    Dst[5] = Ptr[2];
		    Dst[6] = Ptr[1];
		    Dst[7] = Ptr[0];
	        }
	        break;

	    default:
	        /* should probably complain or something */
		return;
        }
	return;
    }

    /* src and dst are different lengths */
    {
	int ii;

        switch (SrcCount)
        {
	    case 1:  /* 8-bit quantity */
                Value = (UINT64)(*(UINT8 *)Src);
		break;

            case 2:  /* 16-bit quantity */
                Value = (UINT64)(*(UINT16 *)Src);
	        break;

            case 4:  /* 32-bit quantity */
                Value = (UINT64)(*(UINT32 *)Src);
	        break;

            case 8:  /* 64-bit quantity */
                Value = (UINT64)(*(UINT64 *)Src);
	        break;

	    default:
	        /* should probably complain or something */
	        return;
        }

	for (ii = 0; ii < DstCount; ii++)
	{
	    Dst[ii] = Ptr[8 - ii];
	}
    }
    return;
}

#endif

/*******************************************************************************
 *
 * FUNCTION:    AcpiUtConvertHostIntToLE
 *
 * PARAMETERS:  SrcPtr              - pointer to integer to convert to
 *                                    little-endian
 *              SrcCount            - number of bytes in the source integer
 *		DstPtr              - where to place the converted value
 *              DstCount            - number of bytes in the destination integer
 *
 * RETURN:      None. Output data is returned via DstPtr
 *
 * DESCRIPTION: Convert an unsigned integer host-native value to little-endian
 *              form; on a little-endian host, this reduces to a null function.
 *
 *              NB: NEVER EVER use this as a memcpy(); use it only when you
 *              need to ensure a value is little-endian.
 *
 ******************************************************************************/

#ifdef ACPI_BIG_ENDIAN
void
AcpiUtConvertHostIntToLE (
    void                    *SrcPtr,
    UINT32                  SrcCount,
    void                    *DstPtr,
    UINT32                  DstCount)
{
    __SwapBytes(SrcPtr, SrcCount, DstPtr, DstCount);
}

#else

void
AcpiUtConvertHostIntToLE (
    void                    *SrcPtr,
    UINT32                  SrcCount,
    void                    *DstPtr,
    UINT32                  DstCount)
{ }

#endif

/*******************************************************************************
 *
 * FUNCTION:    AcpiUtConvertLEToHostInt
 *
 * PARAMETERS:  SrcPtr              - pointer to integer to convert to
 *                                    little-endian
 *              SrcCount            - number of bytes in the source integer
 *		DstPtr              - where to place the converted value
 *              DstCount            - number of bytes in the destination integer
 *
 * RETURN:      None. Output data is returned via DstPtr
 *
 * DESCRIPTION: Convert an unsigned integer little-endian value to host-native
 *              form; on a littleig-endian host, this is a null function.
 *
 *              NB: NEVER EVER use this as a memcpy(); use it only when you
 *              need to ensure a value is little-endian.
 *
 ******************************************************************************/

#ifdef ACPI_BIG_ENDIAN

void
AcpiUtConvertLEToHostInt (
    void                    *SrcPtr,
    UINT32                  SrcCount,
    void                    *DstPtr,
    UINT32                  DstCount)
{
    __SwapBytes(SrcPtr, SrcCount, DstPtr, DstCount);
}

#else

void
AcpiUtConvertLEToHostInt (
    void                    *SrcPtr,
    UINT32                  SrcCount,
    void                    *DstPtr,
    UINT32                  DstCount)
{ }

#endif

#endif
