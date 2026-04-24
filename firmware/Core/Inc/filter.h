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
#define FSK_FILTER_Fs 2500  	// [Hz]
#define FSK_FILTER_F0 440		// [Hz]
#define FSK_FILTER_F1 880		// [Hz]
#define FSK_FILTER_A 0.2f

#define FSK_FILTER_IDLE_STEP_SZ 10
#define FSK_FILTER_SCHWELLENWERT_HIGH 100
#define FSK_FILTER_SCHWELLENWERT_LOW 90


#define FSK_FILTER_BUF_SZ 250 // T_Bit * Fs

typedef struct {
	float adc_buf[FSK_FILTER_BUF_SZ];
	uint32_t adc_ptr;
	float calc_buf[FSK_FILTER_BUF_SZ];
	uint32_t calc_ptr;

	float s0_sin[FSK_FILTER_BUF_SZ];
	float s0_cos[FSK_FILTER_BUF_SZ];
	float s1_sin[FSK_FILTER_BUF_SZ];
	float s1_cos[FSK_FILTER_BUF_SZ];

	float I_0;
	float Q_0;
	float I_1;
	float Q_1;

	float y0;
	float y1;

	uint32_t skip_Ts_idle;
	uint32_t skip_Ts_idle_CNT;

	uint32_t threshold_high;
	uint32_t threshold_low;

	uint32_t T_bit_Counter;

	uint8_t signal_detected;

	uint8_t bit_cnt;
	uint8_t byte;

} FSK_Filter;

void FSK_Filter_init(FSK_Filter*);
void FSK_Filter_addVal(FSK_Filter*, float);
void FSK_Filter_conv(FSK_Filter*);
void FSK_Filter_update(FSK_Filter*, float);
uint8_t FSK_Filter_isByteFinished(FSK_Filter*);



#endif /* INC_FILTER_H_ */
