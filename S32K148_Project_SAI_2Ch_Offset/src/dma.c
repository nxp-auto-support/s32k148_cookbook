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
#include "device_registers.h"	/* include peripheral declarations */
#include "dma.h"

uint8_t TCD0_Source[] = {"Hello World"};	/*< TCD 0 source (11 byte string) 	*/
uint8_t volatile TCD0_Dest = 0;             /*< TCD 0 destination (1 byte) 	*/

uint32_t buffers[2][8] = {
						 {0x1234, 0x1234, 0x1234, 0x1234, 0x1234, 0x1234, 0x1234, 0x1234},
						 {0x4321, 0x4321, 0x4321, 0x4321, 0x4321, 0x4321, 0x4321, 0x4321}
};


/*!
* @brief DMA Initialization. Select SAI0 Tx channel source.
*/
void DMA_init (void)
{
	/* Turn DMAMUX clock on */
	PCC -> PCCn[PCC_DMAMUX_INDEX] |= PCC_PCCn_CGC_MASK;

	/* Enable DMA channel */
	DMAMUX -> CHCFG[0] = DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(EDMA_REQ_SAI0_TX);

}


/*!
 * @brief Configure the TCD of the DMA to allow sending each character (byte) of the
 *        source string to one single memory location defined as the destination address.
 */
void DMA_TCD_init(void)
{
	/* TCD0: Transfers string to a single memory location */

	DMA -> CR |= DMA_CR_EMLM_MASK;

	/* Source Address. */
	DMA -> TCD[0].SADDR = DMA_TCD_SADDR_SADDR((uint32_t volatile) &buffers[0][0]);

	DMA -> TCD[0].SOFF  = DMA_TCD_SOFF_SOFF(4*8);   				/* Source address add 32 bytes after each transfer */

	DMA -> TCD[0].ATTR  = DMA_TCD_ATTR_SMOD(0)  					/* Source modulo feature not used */
                        | DMA_TCD_ATTR_SSIZE(2)  					/* Source read 2^2 = 4 byte per transfer */
                        | DMA_TCD_ATTR_DMOD(0)   					/* Destination modulo feature not used */
                        | DMA_TCD_ATTR_DSIZE(2);  					/* Destination write 2^2 = 4 byte per transfer */

	/* 8 bytes per transfer request (4 bytes for each channel).
	 * The minor loop offset is applied to the SADDR (return to 4*8 * (2 transfers)) */
	DMA -> TCD[0].NBYTES.MLOFFYES = DMA_TCD_NBYTES_MLOFFYES_NBYTES(8)
		  	  	  	  	  	  	  | DMA_TCD_NBYTES_MLOFFYES_SMLOE_MASK
								  | DMA_TCD_NBYTES_MLOFFYES_MLOFF(4-(4*8*2));

	DMA -> TCD[0].SLAST = DMA_TCD_SLAST_SLAST(-(4*8*3-4)); 			/* Source address change after major loop */

	/* Destination Address. */
	DMA -> TCD[0].DADDR = DMA_TCD_DADDR_DADDR((uint32_t volatile) &(SAI0->TDR[0]));

	DMA -> TCD[0].DOFF = DMA_TCD_DOFF_DOFF(0);     					/* No destination address offset after transfer */

	DMA -> TCD[0].CITER.ELINKNO = DMA_TCD_CITER_ELINKNO_CITER(8)  	/* 8 minor loop iterations */
                                | DMA_TCD_CITER_ELINKNO_ELINK(0);   /* No minor loop channel linking */

	DMA -> TCD[0].DLASTSGA = DMA_TCD_DLASTSGA_DLASTSGA(0); 			/* No destination change after major loop */

	DMA -> TCD[0].CSR = DMA_TCD_CSR_START(0)         				/* Clear START status flag. Channel is not explicitly started */
                      | DMA_TCD_CSR_INTMAJOR(0)      				/* No IRQ after major loop */
                      | DMA_TCD_CSR_INTHALF(0)      				/* No IRQ after 1/2 major loop */
                      | DMA_TCD_CSR_DREQ(0)          				/* Disable channel after major loop */
                      | DMA_TCD_CSR_ESG(0)           				/* Disable Scatter Gather */
                      | DMA_TCD_CSR_MAJORELINK(0)    				/* No major loop channel linking */
                      | DMA_TCD_CSR_ACTIVE(0)        				/* Clear ACTIVE status flag */
                      | DMA_TCD_CSR_DONE(0)          				/* Clear DONE status flag */
                      | DMA_TCD_CSR_MAJORLINKCH(0)   				/* No channel-to-channel linking */
                      | DMA_TCD_CSR_BWC(0);          				/* No eDMA stalls after read/write */

	DMA -> TCD[0].BITER.ELINKNO = DMA_TCD_BITER_ELINKNO_BITER(8)    /* Initial iteration count */
                                | DMA_TCD_BITER_ELINKNO_ELINK(0);   /* No minor loop channel linking */
}


