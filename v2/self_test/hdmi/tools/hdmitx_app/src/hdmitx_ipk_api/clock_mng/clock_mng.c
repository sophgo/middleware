// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "clock_mng_local.h"
#include "clock_cfg.h"
#include "reset_mng.h"
#include "hdmi_tx_system_parameters.h"
#include "general_ops.h"
#include <math.h>
#include <float.h>


#include "util.h"


void clk_mng_write(struct hdmi_tx_app *app, uint32_t reg, uint32_t data)
{
	int ret = 0;
	fb_ioctl_data write_data;
	write_data.address = reg + PROTO_HDMI_TX_APB_CRM_CLKMGR_ADDRESS_START;
	write_data.value = data;

	if(app->hdmi_tx_driver < 0)
		return;

	LOGGER(SNPS_TRACE, "%s:addr 0x%x - value 0x%x", __func__, write_data.address, write_data.value);

	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_CORE_WRITE, &write_data);
	if(ret < 0){
		if(app->verbose)
			LOGGER(SNPS_ERROR, "CLK MNG write IOCTL error [%d]\n", ret);
	}
}

uint32_t clk_mng_read(struct hdmi_tx_app *app, uint32_t reg)
{
	int ret = 0;
	fb_ioctl_data read_data;
	read_data.address = reg + PROTO_HDMI_TX_APB_CRM_CLKMGR_ADDRESS_START;

	if(app->hdmi_tx_driver < 0)
		return 0;

	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_CORE_READ, &read_data);
	if(ret < 0){
		if(app->verbose)
			LOGGER(SNPS_ERROR, "CLK MNG read IOCTL error [%d]\n", ret);
	}

	LOGGER(SNPS_TRACE, "%s:addr 0x%x - value 0x%x", __func__,
			read_data.address, read_data.value);

	return read_data.value;
}

void clk_mng_write_mmcm(struct hdmi_tx_app *app, uint32_t reg, uint16_t mask, uint16_t value)
{
	uint32_t temp = 0;

	LOGGER(SNPS_TRACE,"addr:%02x MSB:%04x LSB:%04x\n", reg, mask, value);

	temp = clk_mng_read(app, reg);

	temp = mask & temp;
	temp |= value;

	clk_mng_write(app, reg, temp);
}


/**
 * This function takes a fixed point number and rounds it to the nearest
 * fractional precision bit.
 *
 * @param value
 * @param precision
 * @return round value
 */
uint32_t mmcm_round_frac( uint32_t value, uint32_t precision)
{
	if(value & (1 << (FRAC_PRECISION-precision - 1)))
		value += (1 <<(FRAC_PRECISION-precision -1 ));

	return value;
}


/**
 * This function calculates high_time, low_time, w_edge, and no_count
 * of a non-fractional counter based on the divide and duty cycle
 *
 * NOTE: high_time and low_time are returned as integers between 0 and 63
 *    inclusive.  64 should equal 6'b000000 (in other words it is okay to
 *    ignore the overflow)
 *
 * @param divide - Max divide is 128
 * @param duty_cycle - Duty cycle is multiplied by 100,000
 * @param divider - calculation result
 * @return error
 */
int mmcm_divider( uint32_t divide, uint32_t duty_cycle, struct divider * divider)
{
	uint32_t duty_cycle_fix;
	uint32_t mask;

	// High/Low time is initially calculated with a wider integer to prevent a
	// calculation error when it overflows to 64.
	uint32_t high_time, low_time, w_edge, no_count, temp;

	memset(divider, 0, sizeof(struct divider));

	// Duty Cycle must be between 0 and 1,000
	if(duty_cycle <=0 || duty_cycle >= 100000){
		LOGGER(SNPS_ERROR, "%s:duty_cycle %d is invalid\n", __func__, duty_cycle);
		return -1;
	}

	// Convert to FIXED_WIDTH-FRAC_PRECISION.FRAC_PRECISION fixed point
	duty_cycle_fix = (duty_cycle << FRAC_PRECISION) / 100000;

	// If the divide is 1 nothing needs to be set except the no_count bit.
	//    Other values are dummies
	if(divide == 1) {
		high_time   = 1;
		w_edge      = 0;
		low_time    = 1;
		no_count    = 1;
	} else {
		temp = mmcm_round_frac(duty_cycle_fix*divide, 1);

		// comes from above round_frac
		mask = 0x7F << (FRAC_PRECISION);
		high_time = get(temp, mask);

		// If the duty cycle * divide rounded is .5 or greater then this bit
		//    is set.
		w_edge =  get(temp, 1 << (FRAC_PRECISION-1));

		// If the high time comes out to 0, it needs to be set to at least 1
		// and w_edge set to 0
		if(high_time == 0) {
			high_time   = 1;
			w_edge      = 0;
		}

		if(high_time == divide) {
			high_time   = divide - 1;
			w_edge      = 1;
		}

		// Calculate low_time based on the divide setting and set no_count to
		//    0 as it is only used when divide is 1.
		low_time    = divide - high_time;
		no_count    = 0;
	}

	// Set the return value.
	divider->w_edge = w_edge;
	divider->no_count = no_count;
	divider->high_time = high_time;
	divider->low_time = low_time;
	return 0;
}

/**
 * This function calculates mx, delay_time, and phase_mux
 * of a non-fractional counter based on the divide and phase
 *
 * NOTE: The only valid value for the MX bits is 2'b00 to ensure the coarse mux
 * is used.
 *
 * @param divide - must be an integer (use fractional if not) assumed that
 * 		 divide already checked to be valid - Max divide is 128
 * @param phase -  Phase is given in degrees (-360,000 to 360,000)
 * @return error
 */
int mmcm_phase(uint32_t divide, int32_t phase, struct phase *result)
{
	uint32_t phase_in_cycles, phase_fixed, temp;
	uint8_t delay_time, phase_mux, mx;
	memset(result, 0, sizeof(struct phase));

	if ((phase < -360000) || (phase > 360000)) {
		LOGGER(SNPS_ERROR, "phase of $phase is not between -360000 and 360000");
		return -1;
	}

	// If phase is less than 0, convert it to a positive phase shift
	// Convert to (FIXED_WIDTH-FRAC_PRECISION).FRAC_PRECISION fixed point
	if(phase < 0)
		phase_fixed = ( (phase + 360000) << FRAC_PRECISION ) / 1000;
	else
		phase_fixed = ( phase << FRAC_PRECISION ) / 1000;

	// Put phase in terms of decimal number of vco clock cycles
	phase_in_cycles = ( phase_fixed * divide ) / 360;


	temp = mmcm_round_frac(phase_in_cycles, 3);

	// set mx to 2'b00 that the phase mux from the VCO is enabled
	mx = 0;
	phase_mux = (temp & (0x3 << (FRAC_PRECISION-2))) >> (FRAC_PRECISION-2);
	delay_time = (temp & (0x3F << (FRAC_PRECISION+1))) >> (FRAC_PRECISION+1);

	// Setup the return value
	result->reserved = 0;
	result->mx = mx;
	result->phase_mux = phase_mux;
	result->delay_time = delay_time;
	return 0;
}

/**
 * This function takes in the divide, phase, and duty cycle
 * setting to calculate the upper and lower counter registers.
 *
 * @param divide
 * @param phase
 * @param duty_cycle
 * @return error
 */
int mmcm_count_calc (uint8_t divide, int32_t phase, uint32_t duty_cycle,
		struct count_calc *count_calc)
{
	struct phase phase_calc;
	struct divider div_calc;

	memset(count_calc, 0, sizeof(struct count_calc));

	if(mmcm_divider(divide, duty_cycle, &div_calc))
	{
		LOGGER(SNPS_ERROR, "mmcm_divider");
		return -1;
	}

	if( mmcm_phase(divide, phase, &phase_calc)){
		LOGGER(SNPS_ERROR, "mmcm_phase");
		return -1;
	}

	LOGGER(SNPS_TRACE, "div:%d dc:%d phase:%d", divide, duty_cycle, phase);
	LOGGER(SNPS_TRACE, "ht:%d lt:%d ed:%d", div_calc.high_time, div_calc.low_time,
			div_calc.w_edge);
	LOGGER(SNPS_TRACE, "nc:%d mx:%d dt:%d pm:%d", div_calc.no_count,
			phase_calc.mx, phase_calc.delay_time, phase_calc.phase_mux);

	count_calc->reserved1 = 0;
	count_calc->mx = phase_calc.mx;
	count_calc->w_edge  = div_calc.w_edge;
	count_calc->no_count = div_calc.no_count;
	count_calc->delay_time = phase_calc.delay_time;
	count_calc->phase_mux = phase_calc.phase_mux;
	count_calc->reserved0 = 0;
	count_calc->high_time = div_calc.high_time;
	count_calc->low_time = div_calc.low_time;
	return 0;
}

/**
 * This function takes the divide value and outputs the necessary lock values
 *
 * @param divide - Max divide is 64
 * @return lookup table entry
 */
uint64_t mmcm_lock_lookup(uint8_t divide)
{
	uint64_t result = lookup[divide - 1];
	return result;
}


/**
 * This function takes the divide value and the bandwidth setting of the MMCM
 * and outputs the digital filter settings necessary.
 *
 * @param divide - Max divide is 64
 * @param band - BANDWIDTH - LOW or HIGH
 * @return lookup table entry
 */
uint16_t mmcm_filter_lookup(uint8_t divide, enum bandwidth band)
{
	uint16_t result = 0;
	// Set lookup_entry with the explicit bits from lookup with a part select
	if(band == LOW)
		result = lookup_low[divide - 1];
	else
		result = lookup_high[divide - 1];

	return result;
}

/**
 *  This function takes in the divide, phase, and duty cycle
 *  setting to calculate the upper and lower counter registers.
 *  for fractional multiply/divide functions.
 *
 * @param divide - Max divide is 128
 * @param phase
 * @param duty_cycle - Multiplied by 1,000
 * @param frac - Multiplied by 1000
 * @return
 */
int mmcm_frac_count_calc(uint8_t divide, int32_t phase, uint32_t duty_cycle,
		uint16_t frac, struct frac_count_calc * frac_count_calc)
{
	uint8_t lt_frac, ht_frac, wf_fall_frac, wf_rise_frac;
	uint32_t a_per_in_octets, a_phase_in_cycles;
	uint8_t pm_rise_frac_filtered, pm_fall_frac_filtered, clkout0_divide_int,
			clkout0_divide_frac;
	uint8_t even_part_high, even_part_low, odd, odd_and_frac, pm_fall, dt,
			pm_rise_frac, pm_fall_frac;
	struct divider divider;
	uint64_t dt_calc;
	struct phase phase_calc;

	memset(frac_count_calc, 0, sizeof( struct frac_count_calc));
	//convert phase to fixed
	if ((phase < -360000) || (phase > 360000)) {
		LOGGER(SNPS_ERROR, "phase of $phase is not between -360000 and 360000");
		return -1;
	}

	clkout0_divide_frac = frac / 125;
	clkout0_divide_int = divide;

	even_part_high = clkout0_divide_int >> 1;
	even_part_low = even_part_high;

	odd = clkout0_divide_int - even_part_high - even_part_low;
	odd_and_frac = (8*odd) + clkout0_divide_frac;

	lt_frac = even_part_high - (odd_and_frac <= 9);
	ht_frac = even_part_low  - (odd_and_frac <= 8);

	pm_fall =  ((odd & 0x7F) << 2 ) | (get(clkout0_divide_frac, 0x6));

	wf_fall_frac = (odd_and_frac >=2) && (odd_and_frac <=9);
	wf_rise_frac = (odd_and_frac >=1) && (odd_and_frac <=8);

	//Calculate phase in fractional cycles
	a_per_in_octets		= (8 * divide) + (frac / 125) ;
	a_phase_in_cycles	= (phase+10) * a_per_in_octets / 360000 ;
	pm_rise_frac		= ((a_phase_in_cycles & 0xFF) == 0) ? 0 :
			(a_phase_in_cycles & 0xFF) - ((a_phase_in_cycles & 0xF8) << 3);

	dt_calc = (((uint64_t)phase + 10) * a_per_in_octets / 8 )/360000 ;
	dt 	= dt_calc & 0xFF;

	pm_rise_frac_filtered = (pm_rise_frac >=8) ? (pm_rise_frac ) - 8 : pm_rise_frac ;

	pm_fall_frac		= pm_fall + pm_rise_frac;
	pm_fall_frac_filtered	= pm_fall + pm_rise_frac - ((pm_fall_frac & 0xF8) << 3);


	if(mmcm_divider(divide, duty_cycle, &divider)) {
		return -1;
	}
	if(mmcm_phase(divide, phase, &phase_calc)) {
		return -1;
	}

	frac_count_calc->frac_time = pm_fall_frac_filtered & 0x7;
	frac_count_calc->frac_wf_fall = wf_fall_frac & 1;
	frac_count_calc->time.count_calc.divide_frac = clkout0_divide_frac & 0x7;
	frac_count_calc->time.count_calc.reserved1 = 1;
	frac_count_calc->time.count_calc.wf_rise = wf_rise_frac & 1;
	frac_count_calc->time.count_calc.mx = phase_calc.mx;
	frac_count_calc->time.count_calc.w_edge = divider.w_edge;
	frac_count_calc->time.count_calc.no_count = divider.no_count;
	frac_count_calc->time.count_calc.delay_time = dt & 0x3F;
	frac_count_calc->time.count_calc.phase_mux = pm_rise_frac_filtered & 0x7;
	frac_count_calc->time.count_calc.high_time = ht_frac & 0x3F;
	frac_count_calc->time.count_calc.low_time = lt_frac & 0x3F;

	LOGGER(SNPS_TRACE, "-%d.%d p%d>>", divide, frac, phase);
	LOGGER(SNPS_TRACE, "  :DADDR_9_15 frac30to28.frac_en.wf_r_frac.dt:%d-%d-%d_%d",
			clkout0_divide_frac, 1, wf_rise_frac, dt);
	LOGGER(SNPS_TRACE, "  :DADDR_7_13 pm_f_frac_filtered_29to27.wf_f_frac_26:%d-%d",
			pm_fall_frac_filtered, wf_fall_frac);
	LOGGER(SNPS_TRACE, "  :DADDR_8_14.pm_r_frac_filt_15to13.ht_frac.lt_frac:%d-%d-%d",
			pm_rise_frac_filtered, ht_frac, lt_frac);

	return 0;

}

/**
 * Configuration of the mixed-mode clock manager (MMCM).
 * Following the document http://www.xilinx.com/support/documentation/application_notes/xapp888_7Series_DynamicRecon.pdf
 * 	Application Note: 7 Series and UltraScale FPGAs -MMCM and PLL Dynamic Reconfiguration
 *
 * @param app
 * @param cfg
 * @return
 */
int configure_mmcm(struct hdmi_tx_app *app, struct mmcm * cfg){
	count_calc_t rclkfbout, rdivclk, rclkout0, rclkout1, rclkout2, rclkout3,
		     rclkout4, rclkout5, rclkout6;
	struct frac_count_calc rclkfbout_frac_calc, rclkout0_frac_calc;
	uint64_t rlock = 0;
	uint16_t rdigital_filt = 0;
	uint32_t block_offset = 0;
	uint32_t temp = 0;

	LOGGER(SNPS_TRACE,"%s:pixel_clock - %f clkfbo_mult %02d\n", __func__,
			cfg->clock, cfg->mmcm_clkfbout_mult);

	mmcm_count_calc(cfg->mmcm_clkfbout_mult, cfg->mmcm_clkfbout_phase,
			50000, &(rclkfbout.count_calc));
	mmcm_frac_count_calc(cfg->mmcm_clkfbout_mult, cfg->mmcm_clkfbout_phase,
			50000, cfg->mmcm_clkfbout_frac, &rclkfbout_frac_calc);
	rdigital_filt = mmcm_filter_lookup(cfg->mmcm_clkfbout_mult,
			cfg->mmcm_bandwidth);
	rlock = mmcm_lock_lookup(cfg->mmcm_clkfbout_mult);
	mmcm_count_calc(cfg->mmcm_divclk_divide, 0, 50000, &(rdivclk.count_calc));
	mmcm_count_calc(cfg->mmcm_clkout0_divide, cfg->mmcm_clkout0_phase,
			cfg->mmcm_clkout0_duty, &(rclkout0.count_calc));
	mmcm_frac_count_calc(cfg->mmcm_clkout0_divide, cfg->mmcm_clkout0_phase,
			50000, cfg->mmcm_clkout0_frac, &rclkout0_frac_calc);
	mmcm_count_calc(cfg->mmcm_clkout1_divide, cfg->mmcm_clkout1_phase,
			cfg->mmcm_clkout1_duty, &(rclkout1.count_calc));
	mmcm_count_calc(cfg->mmcm_clkout2_divide, cfg->mmcm_clkout2_phase,
			cfg->mmcm_clkout2_duty, &(rclkout2.count_calc));
	mmcm_count_calc(cfg->mmcm_clkout3_divide, cfg->mmcm_clkout3_phase,
			cfg->mmcm_clkout3_duty, &(rclkout3.count_calc));
	mmcm_count_calc(cfg->mmcm_clkout4_divide, cfg->mmcm_clkout4_phase,
			cfg->mmcm_clkout4_duty, &(rclkout4.count_calc));
	mmcm_count_calc(cfg->mmcm_clkout5_divide, cfg->mmcm_clkout5_phase,
			cfg->mmcm_clkout5_duty, &(rclkout5.count_calc));
	mmcm_count_calc(cfg->mmcm_clkout6_divide, cfg->mmcm_clkout6_phase,
			cfg->mmcm_clkout6_duty, &(rclkout6.count_calc));

	//reset
	if(cfg->device == PIXEL){
		mmcm_pixel_clk_reset(true);
	}
	else if(cfg->device == AUDIO){
		block_offset = 0x200;
		mmcm_audio_clk_reset(true);
	}
	else{
		if(app->verbose)
			hdmitx_logger(SNPS_ERROR, "%s:cfg->device is not known",
					__func__);
		return -EINVAL;
	}

	// store the power bits
	clk_mng_write_mmcm(app, block_offset + DRP_POWER_REG, 0x0, 0xFFFF);

	// store clkout0 divide and phase
	if(cfg->mmcm_clkout0_frac_en == 0)
		temp = rclkout0.value;
	else
		temp = rclkout0_frac_calc.time.value;

	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT0_REG_1, 0x1000, temp & 0xFFFF);
	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT0_REG_2, 0x8000, (temp >> 16) & 0xFFFF);

	// store clkout1 divide and phase
	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT1_REG_1, 0x1000,
			(rclkout1.value) & 0xFFFF);
	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT1_REG_2, 0xFC00,
			((rclkout1.value) >> 16) & 0xFFFF);


	// store clkout2 divide and phase
	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT2_REG_1, 0x1000,
			(rclkout2.value) & 0xFFFF);
	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT2_REG_2, 0xFC00,
			((rclkout2.value) >> 16) & 0xFFFF);

	// store clkout3 divide and phase
	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT3_REG_1, 0x1000,
			(rclkout3.value) & 0xFFFF);
	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT3_REG_2, 0xFC00,
			((rclkout3.value) >> 16) & 0xFFFF);

	// store clkout4 divide and phase
	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT4_REG_1, 0x1000,
			(rclkout4.value) & 0xFFFF);
	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT4_REG_2, 0xFC00,
			((rclkout4.value) >> 16) & 0xFFFF);

	// store clkout5 divide and phase
	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT5_REG_1, 0x1000,
			(rclkout5.value) & 0xFFFF);

	if(cfg->mmcm_clkout0_frac_en == 0)
		clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT5_REG_2, 0xC000,
				((rclkout5.value) >> 16) & 0xFFFF);
	else {
		temp = get(rclkout5.value >> 16, 0xC000);
		temp |= rclkout0_frac_calc.frac_time << 11;
		temp |= rclkout0_frac_calc.frac_wf_fall << 10;
		temp |= ((rclkout5.value >> 16) & 0x3FF);
		clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT5_REG_2, 0xC000, temp);
	}

	// store clkout6 divide and phase
	clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT6_REG_1, 0x1000,
			rclkout6.value & 0xFFFF);

	if(cfg->mmcm_clkfbout_frac_en == 0)
		clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT6_REG_2, 0xC000,
				((rclkout6.value) >> 16) & 0xFFFF);
	else {
		temp = get(rclkout6.value >> 16, 0xC000);
		temp |= rclkfbout_frac_calc.frac_time << 11;
		temp |= rclkfbout_frac_calc.frac_wf_fall << 10;
		temp |= ((rclkout6.value >> 16) & 0x3FF);
		clk_mng_write_mmcm(app, block_offset + DRP_CLKOUT6_REG_2, 0xC000, temp);
	}

	// store the input divider
	clk_mng_write_mmcm(app, block_offset + DRP_DIVLOCK, 0xC000,
			(uint16_t)((get(rdivclk.value, 0xC00000) << 12) | (rdivclk.value & 0x0FFF)));

	// store the feedback divide and phase
	if(cfg->mmcm_clkfbout_frac_en == 0)
		temp = (rclkfbout.value);
	else
		temp = (rclkfbout_frac_calc.time.value);

	clk_mng_write_mmcm(app, block_offset + DRP_CLKFBOUT_REG_2, 0x8000,
			(((uint32_t) temp) >> 16) & 0xFFFF);
	clk_mng_write_mmcm(app, block_offset + DRP_CLKFBOUT_REG_1, 0x1000,
			((uint32_t) temp) & 0xFFFF);

	 // store the lock settings
	clk_mng_write_mmcm(app, block_offset + DRP_LOCK_REG_1, 0xFC00,
			(uint32_t)get64(rlock, 0x3FF00000));
	clk_mng_write_mmcm(app, block_offset + DRP_LOCK_REG_2, 0x8000,
			(uint32_t)((get64(rlock, 0x7C0000000ULL) << 10) |
				  (get64(rlock, 0x3FF))));
	clk_mng_write_mmcm(app, block_offset + DRP_LOCK_REG_3, 0x8000,
			(uint32_t)((get64(rlock, 0xF800000000) << 10) |
				  (get64(rlock, 0xFFC00))));


	 // store the filter settings
	clk_mng_write_mmcm(app, block_offset + DRP_DIGFILT_REG_1, 0x66FF,
			(get(rdigital_filt, 0x200) << 15) |
			(get(rdigital_filt, 0x180)  << 11)|
			(get(rdigital_filt, 0x40)  << 8));
	clk_mng_write_mmcm(app, block_offset + DRP_DIGFILT_REG_2, 0x666F,
			(get(rdigital_filt, 0x20) << 15) |
			(get(rdigital_filt, 0x18)  << 11)|
			(get(rdigital_filt, 0x6)  << 7)  |
			(get(rdigital_filt, 0x1)  << 4));

	// Reset clear
	if(cfg->device == PIXEL){
		mmcm_pixel_clk_reset(false);

		snps_sleep(5);

		if(video_mmcm_lock() == FALSE){
			hdmitx_logger(SNPS_ERROR, "%s: Video MMCM is not locked",__func__);
			return FALSE;
		}

		hdmitx_logger(SNPS_INFO, "%s: Video MMCM is locked",__func__);
	}
	else if(cfg->device == AUDIO){
		if(mmcm_audio_clk_div(cfg->rclk_div_factor) == FALSE){
			hdmitx_logger(SNPS_ERROR, "%s: Error setting audio clk divisor",__func__);
			return FALSE;
		}

		mmcm_audio_clk_reset(false);

		snps_sleep(5);

		if(audio_mmcm_lock() == FALSE){
			hdmitx_logger(SNPS_ERROR, "%s: Audio MMCM is not locked",__func__);
			return FALSE;
		}

		hdmitx_logger(SNPS_INFO, "%s: Audio MMCM is locked",__func__);
	}
	else{
		hdmitx_logger(SNPS_ERROR, "%s:cfg->device is not known",__func__);
		return FALSE;
	}

	return TRUE;
}


struct mmcm *get_video_mmcm_configs(double frequency){
	int i = 0;
	for(i = 0; i < sizeof(video_clock)/sizeof(struct mmcm); i++){
		if(double_is_equal(video_clock[i].clock, frequency)){
			LOGGER(SNPS_INFO, "%s:For %.3fMHz", __func__, frequency);
			return &(video_clock[i]);
		}
	}

	LOGGER(SNPS_ERROR, "%s:No configurations for %.3fMHz", __func__, frequency);
	return NULL;
}

struct mmcm *get_audio_mmcm_configs(double audio_clock_freq, uint16_t oversample_factor){
	int i = 0;
	struct mmcm * oversample_table = NULL;

	switch (oversample_factor){
		case 64  : oversample_table = audio_64_clock; break;
		case 128 : oversample_table = audio_128_clock; break;
		case 256 : oversample_table = audio_256_clock; break;
		case 512 : oversample_table = audio_512_clock; break;
		default : LOGGER(SNPS_ERROR, "%s:Invalid oversample factor %d", __func__,
									oversample_factor);
		return NULL;
	}

	for(i = 0; i < oversample_table[i].mmcm_clkfbout_mult; i++){
		if(double_is_equal(oversample_table[i].clock, audio_clock_freq)){
			LOGGER(SNPS_DEBUG, "%s:For %.2fkHz", __func__, audio_clock_freq);
			return &(oversample_table[i]);
		}
	}

	LOGGER(SNPS_ERROR, "%s:No configurations for %.2fkHz", __func__, audio_clock_freq);
	return NULL;
}



int configure_video_mmcm(struct hdmi_tx_app *app, struct mmcm * cfg){
	return configure_mmcm(app, cfg);
}

int configure_audio_mmcm(struct hdmi_tx_app *app, struct mmcm * cfg){

	LOGGER(SNPS_INFO, "%s: Clock %.2fkHz clkfbout_mult %d clkfbout_phase %d clkfbout_frac_en %d mmcm_clkfbout_frac %d bandwidth %s "
			  "divclk_divide %d clkout0_divide %d clkout0_phase %d clkout0_duty %d clkout0_frac %d clkout0_frac_en %d div_factor %d\n", __func__,
cfg->clock, cfg->mmcm_clkfbout_mult, cfg->mmcm_clkfbout_phase, cfg->mmcm_clkfbout_frac_en, cfg->mmcm_clkfbout_frac, cfg->mmcm_bandwidth == LOW ? "LOW" : "HIGH" ,
		cfg->mmcm_divclk_divide, cfg->mmcm_clkout0_divide, cfg->mmcm_clkout0_phase, cfg->mmcm_clkout0_duty, cfg->mmcm_clkout0_frac, cfg->mmcm_clkout0_frac_en, cfg->rclk_div_factor);

	return configure_mmcm(app, cfg);
}

