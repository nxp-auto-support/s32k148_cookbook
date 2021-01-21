/*
 * Copyright (c) 2014 - 2016, Freescale Semiconductor, Inc.
 * Copyright (c) 2016 - 2018, NXP.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "SAI.h"
#include "device_registers.h"

#define BIT_SIZE            (8)   						/* Bit size per word */
#define CHANNEL_MASK        (0xF) 						/* All channels */
#define WATERMARK           (6)   						/* FIFO size is 8, then watermark is set to FIFO_SIZE/2 */


/*!
* @brief SAI Initialization.
*/
void SAI_init (void)
{
	PCC -> PCCn[PCC_SAI0_INDEX] |= PCC_PCCn_CGC_MASK;	/* Enable clock for SAI */

    /* Disable transmitter and receiver */
    SAI0 -> TCSR &= ~(SAI_TCSR_TE_MASK);
    SAI0 -> RCSR &= ~(SAI_RCSR_RE_MASK);

    /* Enable on debug mode and reset FIFO */
    SAI0 -> TCSR |= (SAI_TCSR_DBGE_MASK | SAI_TCSR_FR_MASK);
    SAI0 -> RCSR |= (SAI_RCSR_DBGE_MASK | SAI_RCSR_FR_MASK);

    /* Asynchronous mode */
    SAI0 -> TCR2 &= ~SAI_TCR2_SYNC_MASK;
    SAI0 -> RCR2 &= ~SAI_RCR2_SYNC_MASK;

    /* Clock polarity: Bit clock is active high with drive outputs on rising edge and sample inputs on falling edge.*/
    SAI0 -> TCR2 &= ~SAI_TCR2_BCP_MASK;
    SAI0 -> TCR2 |= SAI_TCR2_BCP(0);

    /* Frame Sync Early: Frame synchronization asserts one bit before the first bit of the frame */
    SAI0 -> TCR4 &= ~(SAI_TCR4_FSE_MASK);
    SAI0 -> TCR4 |= (SAI_TCR4_FSE(0));

    /* Frame Sync Polarity: Frame synchronization is active high. */
    SAI0 -> TCR4 &= ~(SAI_TCR4_FSP_MASK);
    SAI0 -> TCR4 |= (SAI_TCR4_FSP(0));

    /* Frame Sync Width: Configure the length of the frame sync in number of bit clocks.
     *                   The value written must be one less than the number of bit clocks */
    SAI0 -> TCR4 &= ~(SAI_TCR4_SYWD_MASK);
    SAI0 -> TCR4 |= (SAI_TCR4_SYWD(0));

    /* Data order: MSB is transmitted first. */
    SAI0 -> TCR4 &= ~(SAI_TCR4_MF_MASK);
    SAI0 -> TCR4 |= (SAI_TCR4_MF(1));

    /* Slot offset: Configure the bit index for the first bit transmitted for each word in the frame */
    SAI0 -> TCR5 &= ~(SAI_TCR5_FBT_MASK);
    SAI0 -> TCR5 |= SAI_TCR5_FBT(31);

    /* Slot size: Configure the number of bits in each word and for first word independently */
    SAI0 -> TCR5 &= ~(SAI_TCR5_W0W_MASK | SAI_TCR5_WNW_MASK);
    SAI0 -> TCR5 |= SAI_TCR5_WNW(31) | SAI_TCR5_W0W(31);

    /* Slot count: Configure the number of words in each frame.
     *             The value written must be one less than the number of words in the frame*/
    SAI0 -> TCR4 &= ~(SAI_TCR4_FRSZ_MASK);
    SAI0 -> TCR4 |= (SAI_TCR4_FRSZ(7));



    /* Generate FS and BCLK as SAI is configured as Master */

    /* Bit clock is generated internally in Master mode.*/
    SAI0 -> TCR2 |= SAI_TCR2_BCD_MASK;

    /* Frame sync is generated internally in Master mode. */
    SAI0 -> TCR4 |= SAI_TCR4_FSD_MASK;

    /* FIFO combine mode enabled on FIFO writes (by software) */
    SAI0 -> TCR4 |= SAI_TCR4_FCOMB(2);




    /* Sample */

    /* Sample Rate Frequency Formula
     *
     * 		Bit Clock Frequency (BCLK)
     * 		Master Clock SAI0 (MCLK_SAI) = Bus Clock (40 MHz)
     * 		Bits per channel = 32 bits
     * 		Number of channels = 8 channels
     *
     * 		BCLK = MCLK_SAI / ((DIV + 1) * 2)
     * 		BCLK = bits per channel * number of channels * sample rate frequency
     *
     * 		To get a desired Sample Rate frequency the divisor (DIV) must be calculate
     * 		as follows. Equate the two equations and solve for DIV
     *
     * 		DIV = [MCLK_SAI / (bits per channel * number of channels * sample rate frequency * 2)] - 1
     * 		DIV = [40 MHz / (512 * sample rate frequency)] - 1
     *
     * 		For a desired Sample Rate Frequency of 8 kHz.... DIV = 8.77 ~ 9
     */
    SAI0 -> TCR2 &= ~(SAI_TCR2_MSEL_MASK | SAI_TCR2_DIV_MASK);
    SAI0 -> TCR2 |= (SAI_TCR2_MSEL(0) | SAI_TCR2_DIV(9));

    /* Enable 8 slots/channels in the audio frame (bits[7:0] are set to zero) */
    SAI0 -> TMR = 0xFF00;

    /* Disable all SAI0 data lines */
    SAI0 -> TCR3 &= ~SAI_TCR3_TCE_MASK;

    /* Enable SAI transmitter */
    SAI0 -> TCSR |= (SAI_TCSR_TE_MASK | SAI_TCSR_FRDE_MASK);

}


/*!
* @brief SAI transfer by DMA.
*/
void SAI_transfer (void)
{
	/* Enable request */
	DMA -> ERQ |= (1 << 0);

	/* Enable SAI0_D0 and SAI0_D1 data line */
	SAI0 -> TCR3 |= SAI_TCR3_TCE(1 << 1) | SAI_TCR3_TCE(1 << 0);
}
