/*
 * periph_init.c
 *
 */

#include "periph_init.h"
#include <stm32f4xx.h>
#include "config.h"
#include "ADC.h"

static void RCC_Init(void);
static void GPIO_Init(void);
static void TIM_Init(void);

void System_Init(void)
{
	SCB->CPACR |= (0xFU << 20);   // FPU Activate
	__DSB();
	__ISB();

	RCC_Init();
	GPIO_Init();
	ADC_Init();
	TIM_Init();
}

static void RCC_Init(void)
{
	/*Clock Source HSI*/
	WriteRegisterField(&RCC->CR, RCC_CR_HSION_Pos, RCC_CR_HSION_Msk, 1U); // HSI ON
	while((RCC->CR & (1U << RCC_CR_HSIRDY_Pos)) == 0U); // Wait for HSI Ready

	/*APB1 Power Enable for Power Configuration*/
	WriteRegisterField(&RCC->APB1ENR, RCC_APB1ENR_PWREN_Pos, RCC_APB1ENR_PWREN_Msk, 1U);

	/*Power Configuration for 84MHz*/
	WriteRegisterField(&PWR->CR, PWR_CR_VOS_Pos, PWR_CR_VOS_Msk, 2U); // Power Scale 2 for 84MHz

	/*Flash Latency*/
	WriteRegisterField(&FLASH->ACR, FLASH_ACR_LATENCY_Pos, FLASH_ACR_LATENCY_Msk, 2U); // Two Wait States for 84MHz

	/*PLL OFF Verification*/
	if (RCC->CR & ((1U) << RCC_CR_PLLON_Pos))
	{
		WriteRegisterField(&RCC->CR, RCC_CR_PLLON_Pos, RCC_CR_PLLON_Msk, 0U); // Reset PLL
	}
	while((RCC->CR & ((1U) << RCC_CR_PLLRDY_Pos)) != 0U); // Wait for PLL not Ready

	/*PLL Source HSI*/
	WriteRegisterField(&RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC_Pos, RCC_PLLCFGR_PLLSRC_Msk, 0U);

	/*Configuration of PLL for 84MHz*/
	WriteRegisterField(&RCC->PLLCFGR, RCC_PLLCFGR_PLLM_Pos, RCC_PLLCFGR_PLLM_Msk, 16U); // M = 16
	WriteRegisterField(&RCC->PLLCFGR, RCC_PLLCFGR_PLLN_Pos, RCC_PLLCFGR_PLLN_Msk, 336U); // N = 336
	WriteRegisterField(&RCC->PLLCFGR, RCC_PLLCFGR_PLLP_Pos, RCC_PLLCFGR_PLLP_Msk, 1U); // P = 4
	WriteRegisterField(&RCC->PLLCFGR, RCC_PLLCFGR_PLLQ_Pos, RCC_PLLCFGR_PLLQ_Msk, 7U); // Q = 7

	/*AHB, APB1 & APB2 Prescaler*/
	WriteRegisterField(&RCC->CFGR, RCC_CFGR_HPRE_Pos, RCC_CFGR_HPRE_Msk, 0U); // AHB not devided
	WriteRegisterField(&RCC->CFGR, RCC_CFGR_PPRE1_Pos, RCC_CFGR_PPRE1_Msk, 4U); // APB1 devided by 2
	WriteRegisterField(&RCC->CFGR, RCC_CFGR_PPRE2_Pos, RCC_CFGR_PPRE2_Msk, 0U); // APB2 not devided

	/*PLL Turn ON*/
	WriteRegisterField(&RCC->CR , RCC_CR_PLLON_Pos, RCC_CR_PLLON_Msk, 1U); // PLL Set
	while((RCC->CR & ((1U) << RCC_CR_PLLRDY_Pos)) == 0U); // Wait for PLL Ready

	/*Selecting PLL As System Clock*/
	WriteRegisterField(&RCC->CFGR, RCC_CFGR_SW_Pos, RCC_CFGR_SW_Msk, 2U);
	while((RCC->CFGR & RCC_CFGR_SWS_Msk) != (2U << RCC_CFGR_SWS_Pos)); // PLL As SystemCLK Verification

	/*System Clock Update*/
	SystemCoreClockUpdate();

}

static void GPIO_Init(void)
{
	/*GPIO Clock Enable*/
	WriteRegisterField(&RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN_Pos, RCC_AHB1ENR_GPIOAEN_Msk, 1U); // GPIO A Enable
	WriteRegisterField(&RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN_Pos, RCC_AHB1ENR_GPIOBEN_Msk, 1U); // GPIO B Enable
	WriteRegisterField(&RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN_Pos, RCC_AHB1ENR_GPIOCEN_Msk, 1U); // GPIO C Enable
	WriteRegisterField(&RCC->AHB1ENR, RCC_AHB1ENR_GPIOEEN_Pos, RCC_AHB1ENR_GPIOEEN_Msk, 1U); // GPIO E Enable

	/*PA1 PA2 Configuration*/
	WriteRegisterField(&GPIOA->MODER, GPIO_MODER_MODER1_Pos, GPIO_MODER_MODER1_Msk, 3U); // PA1 Analog Mode
	WriteRegisterField(&GPIOA->MODER, GPIO_MODER_MODER2_Pos, GPIO_MODER_MODER2_Msk, 3U); // PA2 Analog Mode

	/*PC13 Configuration*/
	WriteRegisterField(&GPIOC->MODER, GPIO_MODER_MODER13_Pos, GPIO_MODER_MODER13_Msk, 0U); // PC13 Input
	WriteRegisterField(&GPIOC->PUPDR, GPIO_PUPDR_PUPD13_Pos, GPIO_PUPDR_PUPD13_Msk, 2U); // PC13 Pull Down

	/*PE11 Configuration*/
	WriteRegisterField(&GPIOE->MODER, GPIO_MODER_MODER11_Pos, GPIO_MODER_MODER11_Msk, 2U); // PE11 Alternative Function
	WriteRegisterField(&GPIOE->OTYPER, GPIO_OTYPER_OT11_Pos, GPIO_OTYPER_OT11_Msk, 0U);
	WriteRegisterField(&GPIOE->PUPDR, GPIO_PUPDR_PUPD11_Pos, GPIO_PUPDR_PUPD11_Msk, 0U); // PE11 Push Pull
	WriteRegisterField(&GPIOE->OSPEEDR, GPIO_OSPEEDR_OSPEED11_Pos, GPIO_OSPEEDR_OSPEED11_Msk, 3U); // Very High Speed
	WriteRegisterField(&GPIOE->AFR[1], GPIO_AFRH_AFSEL11_Pos, GPIO_AFRH_AFSEL11_Msk, 1U); // Alternative Function 01

	/*PA7 Configuration*/
	WriteRegisterField(&GPIOA->MODER, GPIO_MODER_MODER7_Pos, GPIO_MODER_MODER7_Msk, 2U); // PA7 Alternative Function
	WriteRegisterField(&GPIOA->OTYPER, GPIO_OTYPER_OT7_Pos, GPIO_OTYPER_OT7_Msk, 0U);
	WriteRegisterField(&GPIOA->PUPDR, GPIO_PUPDR_PUPD7_Pos, GPIO_PUPDR_PUPD7_Msk, 0U); // PA7 Push Pull
	WriteRegisterField(&GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED7_Pos, GPIO_OSPEEDR_OSPEED7_Msk, 3U); // Very High Speed
	WriteRegisterField(&GPIOA->AFR[0], GPIO_AFRL_AFSEL7_Pos, GPIO_AFRL_AFSEL7_Msk, 1U); // Alternative Function 01

	/*PA8 Configuration*/
	WriteRegisterField(&GPIOA->MODER, GPIO_MODER_MODER8_Pos, GPIO_MODER_MODER8_Msk, 2U); // PA8 Alternative Function
	WriteRegisterField(&GPIOA->OTYPER, GPIO_OTYPER_OT8_Pos, GPIO_OTYPER_OT8_Msk, 0U);
	WriteRegisterField(&GPIOA->PUPDR, GPIO_PUPDR_PUPD8_Pos, GPIO_PUPDR_PUPD8_Msk, 0U); // PA8 Push Pull
	WriteRegisterField(&GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED8_Pos, GPIO_OSPEEDR_OSPEED8_Msk, 3U); // Very High Speed
	WriteRegisterField(&GPIOA->AFR[1], GPIO_AFRH_AFSEL8_Pos, GPIO_AFRH_AFSEL8_Msk, 1U); // Alternative Function 01

	/*PB0 Configuration*/
	WriteRegisterField(&GPIOB->MODER, GPIO_MODER_MODER0_Pos, GPIO_MODER_MODER0_Msk, 2U); // PB0 Alternative Function
	WriteRegisterField(&GPIOB->OTYPER, GPIO_OTYPER_OT0_Pos, GPIO_OTYPER_OT0_Msk, 0U);
	WriteRegisterField(&GPIOB->PUPDR, GPIO_PUPDR_PUPD0_Pos, GPIO_PUPDR_PUPD0_Msk, 0U); // PB0 Push Pull
	WriteRegisterField(&GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED0_Pos, GPIO_OSPEEDR_OSPEED0_Msk, 3U); // Very High Speed
	WriteRegisterField(&GPIOB->AFR[0], GPIO_AFRL_AFSEL0_Pos, GPIO_AFRL_AFSEL0_Msk, 1U); // Alternative Function 01
}

static void TIM_Init(void)
{
	WriteRegisterField(&RCC->APB2ENR, RCC_APB2ENR_TIM1EN_Pos, RCC_APB2ENR_TIM1EN_Msk, 1U); // TIM1 Clock Enable

	/*PWM 10KHz Frequency Configuration*/
	WriteRegisterField(&TIM1->PSC, TIM_PSC_PSC_Pos, TIM_PSC_PSC_Msk, 0U); // Counter Clock Prescaler
	WriteRegisterField(&TIM1->ARR, TIM_ARR_ARR_Pos, TIM_ARR_ARR_Msk, COUNTER_MAX_VAL-1); // Counter Max Value
	WriteRegisterField(&TIM1->CCMR1, TIM_CCMR1_OC1M_Pos, TIM_CCMR1_OC1M_Msk, 6U); // CH1 PWM Mode 1
	WriteRegisterField(&TIM1->CCMR1, TIM_CCMR1_OC1PE_Pos, TIM_CCMR1_OC1PE_Msk, 1U); // CH1 Pre-load Enable

	WriteRegisterField(&TIM1->CCMR1, TIM_CCMR1_OC2M_Pos, TIM_CCMR1_OC2M_Msk, 6U); // CH2 PWM Mode 1
	WriteRegisterField(&TIM1->CCMR1, TIM_CCMR1_OC2PE_Pos, TIM_CCMR1_OC2PE_Msk, 1U); // CH2 Pre-load Enable

	/*Pre-load changes duty cycle for next PWM period rather than the current one */

	WriteRegisterField(&TIM1->CCER, TIM_CCER_CC1E_Pos, TIM_CCER_CC1E_Msk, 1U); // PA8 Output
	WriteRegisterField(&TIM1->CCER, TIM_CCER_CC1NE_Pos, TIM_CCER_CC1NE_Msk, 1U); // PA7 Output
	WriteRegisterField(&TIM1->CCER, TIM_CCER_CC2E_Pos, TIM_CCER_CC2E_Msk, 1U); // PE11 Output
	WriteRegisterField(&TIM1->CCER, TIM_CCER_CC2NE_Pos, TIM_CCER_CC2NE_Msk, 1U); // PB0 Output
	WriteRegisterField(&TIM1->CR1, TIM_CR1_CKD_Pos, TIM_CR1_CKD_Msk, 0U); // Deadtime Period
	WriteRegisterField(&TIM1->BDTR, TIM_BDTR_DTG_Pos, TIM_BDTR_DTG_Msk, 84U); // DeadTime 1 MicroSecond  Configuration

	NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn); // Timer 1 Interruption Activate
	WriteRegisterField(&TIM1->EGR, TIM_EGR_UG_Pos, TIM_EGR_UG_Msk, 1U); // Register Update
	WriteRegisterField(&TIM1->BDTR, TIM_BDTR_MOE_Pos, TIM_BDTR_MOE_Msk, 1U); // Outputs Enable
	WriteRegisterField(&TIM1->CR1, TIM_CR1_CEN_Pos, TIM_CR1_CEN_Msk, 1U); // TIM 1 Enable
}
