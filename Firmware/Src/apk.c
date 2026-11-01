/*
 * apk.c
 *
 */

#include "apk.h"
#include "config.h"
#include <stdint.h>
#include <stm32f4xx.h>
#include <math.h>
#include <stdlib.h>

uint16_t Amp = 0;
uint16_t Freq = 0;

static volatile uint16_t IRQCounter = 0;

static volatile uint8_t FromDC = 0; // Flag for Conversion Mode Switch

static volatile float m = 0.0f;
static volatile uint8_t SinTableSize = 0;

static int16_t SinRefTable[SIN_TABLE_MAX];

void Sin_Init(void)
{
    for (uint8_t i = 0; i < SIN_TABLE_MAX; i++)
    {
        SinRefTable[i] = (int16_t)((float)COUNTER_MAX_VAL * sinf((2.0f * (float)M_PI * i) / SIN_TABLE_MAX));
    }
}

void DC_AC_Conversion(void)
{
    static uint16_t OldFreq = 0;

    m = (float)Amp / (float)MAX_DC_VAL; // Amplitude variation

    if ((abs((int)OldFreq - (int)Freq) >= 2)) // Avoid ADC Small variations and recalculation for same Frequency
    {
        WriteRegisterField(&TIM1->DIER, TIM_DIER_UIE_Pos, TIM_DIER_UIE_Msk, 0U); // TIM interrupt Disable
        uint8_t OldSize = SinTableSize;
        SinTableSize = (uint8_t)(FREQ_PWM / Freq); // Calculate Number of PWM Periods in desired period
        if (OldSize != 0U)
        {
            IRQCounter = (uint16_t)(((uint32_t)IRQCounter * SinTableSize) / OldSize);   // Same phase for new Frequency
        }
        else
        {
            IRQCounter = 0;
        }
        OldFreq = Freq; // Old Frequency Memorization
    }

    if ((FromDC != 0U) && (SinTableSize != 0U)) // Start from SIN Value when switching from DC-DC to DC-AC
    {
        if ((GPIOC->IDR & GPIO_IDR_ID12_Msk) == 0U)
        {
            IRQCounter = SinTableSize / 4U; // Positive DC
        }
        else
        {
            IRQCounter = (3U * SinTableSize) / 4U; // Negative DC
        }
        FromDC = 0U; // From DC Reset
    }
    WriteRegisterField(&TIM1->DIER, TIM_DIER_UIE_Pos, TIM_DIER_UIE_Msk, 1U); // TIM INT Enable
}

void DC_DC_Conversion(void)
{
	if (((TIM1->DIER & TIM_DIER_UIE_Msk) != 0U) && (SinTableSize != 0U)) // Coming from DC/AC mode
	{
		uint16_t Target;
		if ((GPIOC->IDR & GPIO_IDR_ID12_Msk) == 0U)
		{
			Target = SinTableSize / 4U; // Positive DC
		}
		else
		{
			Target = (3U * SinTableSize) / 4U; // Negative DC
		}
		while (IRQCounter != Target) {}        // Wait for SIN to get DC Value
	}

	WriteRegisterField(&TIM1->DIER, TIM_DIER_UIE_Pos, TIM_DIER_UIE_Msk, 0U); // TIM INT Disable
	static uint32_t CCR_Val = 0;
	FromDC = 1U; // Flag Set
	m = (float)Amp / (float)MAX_DC_VAL; // Calculate Amplitude Variation
	if ((GPIOC->IDR & GPIO_IDR_ID12_Msk )== 0U) // Positive DC Output
	{
		CCR_Val = (uint32_t)(m*(float)COUNTER_MAX_VAL);
		TIM1->CCR1 = (CCR_Val << TIM_CCR1_CCR1_Pos); // Constant PWM for DC-DC Conversion
		TIM1->CCR2 = 0U; // PWM2 Disable
	}
	else // Negative DC Output
	{
		CCR_Val = (uint32_t)(m*(float)COUNTER_MAX_VAL);
		TIM1->CCR1 = 0U; // PWM1 Disable
		TIM1->CCR2 = (CCR_Val << TIM_CCR2_CCR2_Pos); // Constant PWM for DC-DC Conversion
	}

}

void TIM1_UP_TIM10_IRQHandler(void)
{
	static uint32_t CCR1_Val = 0;
	static uint32_t CCR2_Val = 0;
	if ((TIM1->SR & TIM_SR_UIF_Msk) == TIM_SR_UIF)
	{
		WriteRegisterField(&TIM1->SR, TIM_SR_UIF_Pos, TIM_SR_UIF_Msk, 0U); // Interrupt Flag Reset

		int16_t s = SinRefTable[((uint32_t)IRQCounter * SIN_TABLE_MAX) / SinTableSize]; // Calculate Corresponding SIN value based on output frequency
		CCR1_Val = (uint32_t)((COUNTER_MAX_VAL + m*(float)s) / 2); // Calculate PWM1 Duty cycle
		WriteRegisterField(&TIM1->CCR1, TIM_CCR1_CCR1_Pos, TIM_CCR1_CCR1_Msk, CCR1_Val); // Setting PWM 1 Duty cycle
		CCR2_Val = (uint32_t)((COUNTER_MAX_VAL - m*(float)s) / 2); // Calculate PWM2 Duty Cycle
		WriteRegisterField(&TIM1->CCR2, TIM_CCR2_CCR2_Pos, TIM_CCR2_CCR2_Msk, CCR2_Val); // Setting PWM2 Duty cycle

		IRQCounter++; // Number of iterations
		if(IRQCounter >= SinTableSize) // End of Sin reference Period
		{
			IRQCounter = 0; // Iteration counter reset
		}
	}
}

