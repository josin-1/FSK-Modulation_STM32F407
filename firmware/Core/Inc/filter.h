/*
 * filter.h
 *
 *  Created on: Mar 27, 2026
 *      Author: alexi
 */

#ifndef INC_FILTER_H_
#define INC_FILTER_H_

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "arm_math.h"

#define FSK_FILTER_T_Bit 0.1f	// [s]
#define FSK_FILTER_Fs 10000  	// [Hz]
#define FSK_FILTER_F0 440		// [Hz]
#define FSK_FILTER_F1 880		// [Hz]
#define FSK_FILTER_A 0.2f

#define FSK_FILTER_IDLE_STEP_SZ 10
#define FSK_FILTER_SCHWELLENWERT_HIGH 350
#define FSK_FILTER_SCHWELLENWERT_LOW 300


#define FSK_FILTER_BUF_SZ 1000 // T_Bit * Fs

typedef struct {
	uint32_t adc_buf[FSK_FILTER_BUF_SZ];
	uint32_t adc_ptr;
	uint32_t calc_buf[FSK_FILTER_BUF_SZ];
	uint32_t calc_ptr;

	uint32_t s0_sin[FSK_FILTER_BUF_SZ];
	uint32_t s0_cos[FSK_FILTER_BUF_SZ];
	uint32_t s1_sin[FSK_FILTER_BUF_SZ];
	uint32_t s1_cos[FSK_FILTER_BUF_SZ];

	uint32_t I_0;
	uint32_t Q_0;
	uint32_t I_1;
	uint32_t Q_1;

	uint32_t dotBuf[FSK_FILTER_BUF_SZ];

	uint32_t y0;
	uint32_t y1;

	uint32_t skip_Ts_idle;
	uint32_t skip_Ts_idle_CNT;

	uint32_t threshold_high;
	uint32_t threshold_low;

	uint32_t T_bit_Counter;

	uint8_t signal_detected;

	uint8_t bit_cnt;
	uint8_t byte;

} FSK_Filter;

uint32_t FSK_Filter_DotP(uint32_t*, uint32_t*, uint32_t);
void FSK_Filter_init(FSK_Filter*);
void FSK_Filter_addVal(FSK_Filter*, uint32_t);
void FSK_Filter_conv(FSK_Filter*);
void FSK_Filter_update(FSK_Filter*, uint32_t);
uint8_t FSK_Filter_isByteFin(FSK_Filter*);



#endif /* INC_FILTER_H_ */
