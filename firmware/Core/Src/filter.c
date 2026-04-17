/*
 * filter.c
 *
 *  Created on: Mar 27, 2026
 *      Author: alexi
 */

#include "filter.h"

float maxY0;
float maxY1;


void FSK_Filter_init(FSK_Filter *f) {
	for (uint32_t k = FSK_FILTER_BUF_SZ; k > 0; --k) {
		float t = k / (float) FSK_FILTER_Fs;
		f->s0[k - 1] = FSK_FILTER_A * sin(2 * M_PI * FSK_FILTER_F0 * t);
		f->s1[k - 1] = FSK_FILTER_A * sin(2 * M_PI * FSK_FILTER_F1 * t);
	}

	for (uint32_t i = 0; i < FSK_FILTER_BUF_SZ; ++i) {
		f->adc_buf[i] = 0;
		f->calc_buf[i] = 0;
	}
	f->adc_ptr = 0;
	f->calc_ptr = 0;

	f->y0 = 0;
	f->y1 = 0;

	f->skip_Ts_idle = FSK_FILTER_IDLE_STEP_SZ;
	f->skip_Ts_idle_CNT = 0;

	f->threshold_high = FSK_FILTER_SCHWELLENWERT_HIGH;
	f->threshold_low = FSK_FILTER_SCHWELLENWERT_LOW;

	f->T_bit_Counter = 0;

	f->signal_detected = 0;

	f->bit_cnt = 0;
	f->byte = 0;
}

void FSK_Filter_addVal(FSK_Filter *f, float new_val) {
	f->adc_buf[f->adc_ptr] = new_val;

	f->adc_ptr++;
	if (f->adc_ptr >= FSK_FILTER_BUF_SZ) {
		f->adc_ptr = 0;
	}
}

void FSK_Filter_conv(FSK_Filter *f) {
	// Circular ADC-Buffer has elements not in order in respect to time
	// f.e: [5 6 7 8 9 1 2 3 4]
	//                 ^
	//                 | adc_pointer
	// correct data in respect to time would be [1 2 3 ... 9]
	// with indices: [(adc_pointer) ... (BUF_SZ) (0) ... (BUF_SZ - 1)]
	//
	// To calculate convolution with matlabs conv() or filter(), we're copying adc data
	// to a dedicated calculation array where the data is in the right order
	f->calc_ptr = f->adc_ptr;
	for (uint32_t j = 0; j < FSK_FILTER_BUF_SZ; ++j) {
		f->calc_buf[j] = f->adc_buf[f->calc_ptr];
		f->calc_ptr = f->calc_ptr + 1;
		if (f->calc_ptr > FSK_FILTER_BUF_SZ) {
			f->calc_ptr = 1;
		}
	}


	// For both filters, if signal has not been detected then the
	// filter output needs to reach threshold_high, afterwards
	// (signal detected) it only needs to be higher then threshold
	// low (should be a bit smaller than high), because the at the
	// next T_bit conv() it could be a bit smaller than
	// threshold_high
	float32_t conv_buf[FSK_FILTER_BUF_SZ * 2];

	for (uint32_t i = 0; i < FSK_FILTER_BUF_SZ * 2; ++i) {
		conv_buf[i] = 0;
	}

	arm_conv_f32(f->s0, FSK_FILTER_BUF_SZ, f->calc_buf, FSK_FILTER_BUF_SZ,
			conv_buf);
	arm_rms_f32(conv_buf, FSK_FILTER_BUF_SZ * 2, &(f->y0));
	if ((f->signal_detected == 0 && f->y0 >= f->threshold_high)
			|| (f->signal_detected == 1 && f->y0 >= f->threshold_low)) {
		if (f->signal_detected == 0) {
			f->signal_detected = 1;
		}

		// Bitshift a 0 into the current bit position (kinda
		// unnecessary), and or it together with the complete byte
		f->byte |= (0b0 << (7 - f->bit_cnt));
		f->bit_cnt++;
	}

	for (uint32_t i = 0; i < FSK_FILTER_BUF_SZ * 2; ++i) {
		conv_buf[i] = 0;
	}

	arm_conv_f32(f->s1, FSK_FILTER_BUF_SZ, f->calc_buf, FSK_FILTER_BUF_SZ,
			conv_buf);
	arm_rms_f32(conv_buf, FSK_FILTER_BUF_SZ * 2, &(f->y1));
	if ((f->signal_detected == 0 && f->y1 >= f->threshold_high)
			|| (f->signal_detected == 1 && f->y1 >= f->threshold_low)) {
		if (f->signal_detected == 0) {
				f->signal_detected = 1;
			}

		// Bitshift a 1 into the current bit position (which is
		// unnecessary), and or it together with the complete byte#
		f->byte |= (0b1 << (7 - f->bit_cnt));
		f->bit_cnt++;
	}

	// not 1 or 0 detected therefore transmission complete!
	if (f->y0 <= f->threshold_low && f->y1 <= f->threshold_low) {
		f->signal_detected = 0;
	}
}

void FSK_Filter_update(FSK_Filter *f, float new_val) {
	FSK_Filter_addVal(f, new_val);

	f->skip_Ts_idle_CNT++;
	f->T_bit_Counter++;

	if ((f->signal_detected == 0 && f->skip_Ts_idle_CNT >= f->skip_Ts_idle)
			|| (f->signal_detected == 1 && f->T_bit_Counter >= FSK_FILTER_BUF_SZ)) {
		f->skip_Ts_idle_CNT = 0;
		f->T_bit_Counter = 0;

		FSK_Filter_conv(f);
	}

	if (f->y0 > maxY0)
		maxY0 = f->y0;
	if (f->y1 > maxY1)
		maxY1 = f->y1;
}

uint8_t FSK_Filter_isByteFinished(FSK_Filter *f) {
	if (f->bit_cnt > 7) {
		f->bit_cnt = 0;
		return 1;
	}
	return 0;
}
