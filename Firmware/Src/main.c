/**
 ******************************************************************************
 * @file    main.c
 * @author  Badis Kaouel
 * @date    October 2026
 * @brief   Static power converter controlled by STM32F401VC
 *
 * DC/DC chopper and DC/AC single-phase inverter (H-bridge) driven by TIM1
 * complementary PWM outputs with hardware dead-time.
 *
 * Features:
 *  - 10 kHz complementary PWM with 1 us dead-time (TIM1 CH1/CH1N, CH2/CH2N)
 *  - Unipolar sinusoidal PWM (SPWM) for DC/AC mode
 *  - Output amplitude and frequency set by potentiometers (ADC1)
 *  - Operating mode selected by push button (PC13)
 *  - DC/DC output polarity (positive/negative) selected by button (PC12)
 *  - Bare-metal register programming with CMSIS (no HAL)
 *
 * Target : STM32F401VC @ 84 MHz (HSI + PLL)
 * Tested : Proteus simulation, STM32F407G-DISC1 board (PWM outputs)
 *
 * @copyright Copyright (c) 2026 Badis Kaouel
 *            Licensed under the MIT License. See LICENSE file for details.
 ******************************************************************************
 */

#include "main.h"
#include "periph_init.h"
#include "ADC.h"
#include "config.h"
#include "apk.h"

int main(void)
{

	System_Init();
	Sin_Init();
    /* Loop forever */
	while(1)
	{
		ADC_Read(&Amp, &Freq);
		switch((GPIOC->IDR & GPIO_IDR_ID13_Msk) == ((1U) << GPIO_IDR_ID13_Pos)) // Button Press Check for Conversion Mode
		{
		case DC_DC_Converter :
			DC_DC_Conversion();
			break;
		case DC_AC_Inverter :
			DC_AC_Conversion();
			break;
		}
	}
}

