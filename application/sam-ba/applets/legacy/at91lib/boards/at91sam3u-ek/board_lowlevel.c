/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

//------------------------------------------------------------------------------
/// \unit
///
/// !Purpose
///
/// Provides the low-level initialization function that gets called on chip
/// startup.
///
/// !Usage
///
/// LowLevelInit() is called in #board_cstartup_xxx.c#.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "board.h"
#include "board_memories.h"
#include "board_lowlevel.h"
#include <pio/pio.h>
#include <pmc/pmc.h>

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// After POR, at91sam3u device is running on 4MHz internal RC
/// At the end of the LowLevelInit procedure MCK = 48MHz PLLA = 96 CPU=48MHz
/// Performs the low-level initialization of the chip. This includes EFC, master
/// clock, IRQ & watchdog configuration.
//------------------------------------------------------------------------------
void LowLevelInit(unsigned int clockConfigEnable)
{
    pllConfiguration pll;
    mckrConfiguration mckr;

    /* Enable NRST reset
     ************************************/
    AT91C_BASE_RSTC->RSTC_RMR |= (0xA5 << 24) | AT91C_RSTC_URSTEN;

#if defined(SAM_BA)
    { asm volatile ("cpsid i"); }
#endif
    
    /* Set 2 WS for Embedded Flash Access
     ************************************/
    AT91C_BASE_EFC0->EFC_FMR = (0x6 <<  8);
#if defined(AT91C_BASE_EFC1)    
    AT91C_BASE_EFC1->EFC_FMR = (0x6 <<  8);
#endif

    /* Watchdog initialization
     *************************/
    AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;


    if (clockConfigEnable)
    {
        //  enable Main oscillator 
        PMC_EnableMainOsc();
        // Then, cofigure PLLA and switch clock
        // MCK = 12MHz * 8 / 1 / 2 = 48MHz 
        pll.mul = 0x08;  pll.div = 0x01; pll.usbdiv = 0; pll.pllout = 0;
        mckr.prescaler = AT91C_PMC_PRES_CLK_2; mckr.mdiv = 0; mckr.plldiv2 = 0;
        PMC_ConfigureMckWithPlla(&pll, &mckr);
    }
    /* Enable clock for UART
     ************************/
    AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_DBGU);

    /* Optimize CPU setting for speed,  for engineering samples only */
    SetDefaultMaster(1);

}

//------------------------------------------------------------------------------
/// Enable or disable default master access
/// \param enalbe 1 enable defaultMaster settings, 0 disable it.
//------------------------------------------------------------------------------
void SetDefaultMaster(unsigned char enable)
{
    AT91PS_HMATRIX2 pMatrix = AT91C_BASE_MATRIX;

    // Set default master
    if (enable == 1) {

        // Set default master: SRAM0 -> Cortex-M3 System
        pMatrix->HMATRIX2_SCFG0 |= AT91C_MATRIX_FIXED_DEFMSTR_SCFG0_ARMS |
                                   AT91C_MATRIX_DEFMSTR_TYPE_FIXED_DEFMSTR;
        
        // Set default master: SRAM1 -> Cortex-M3 System
        pMatrix->HMATRIX2_SCFG1 |= AT91C_MATRIX_FIXED_DEFMSTR_SCFG1_ARMS |
                                   AT91C_MATRIX_DEFMSTR_TYPE_FIXED_DEFMSTR;

        // Set default master: Internal flash0 -> Cortex-M3 Instruction/Data
        pMatrix->HMATRIX2_SCFG3 |= AT91C_MATRIX_FIXED_DEFMSTR_SCFG3_ARMC |
                                   AT91C_MATRIX_DEFMSTR_TYPE_FIXED_DEFMSTR;
    } else {

        // Clear default master: SRAM0 -> Cortex-M3 System
        pMatrix->HMATRIX2_SCFG0 &= (~AT91C_MATRIX_DEFMSTR_TYPE);
        
        // Clear default master: SRAM1 -> Cortex-M3 System
        pMatrix->HMATRIX2_SCFG1 &= (~AT91C_MATRIX_DEFMSTR_TYPE);

        // Clear default master: Internal flash0 -> Cortex-M3 Instruction/Data
        pMatrix->HMATRIX2_SCFG3 &= (~AT91C_MATRIX_DEFMSTR_TYPE);
    }
}

//------------------------------------------------------------------------------
/// Set flash wait state
/// \param ws    Value of flash wait state
//------------------------------------------------------------------------------
void SetFlashWaitState(unsigned char ws)
{
    // Set Wait State for Embedded Flash Access
    AT91C_BASE_EFC0->EFC_FMR = ((ws << 8) & AT91C_EFC_FWS);
    
#if defined(AT91C_BASE_EFC1)
    AT91C_BASE_EFC1->EFC_FMR = ((ws << 8) & AT91C_EFC_FWS);
#endif    
}

