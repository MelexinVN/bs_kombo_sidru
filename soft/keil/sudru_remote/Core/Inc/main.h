/* USER CODE BEGIN Header */
/*
* main.h
* заголовочный файл основной прораммы контроллера светодиодов ws2812b
* СветоИзлучающего Декоративного Радиоуправляемого Устройства (СИДРУ)
*
* Микроконтроллер:  stm32f030f4p6
* Приемопередатчик: nrf24l01+
*
* Примечания:
* - Проект сгенерирован в STM32CubeMX, структура проекта, основные настройки и инициализация периферии 
*   созданы автоматически.
*
* Автор: Мелехин В.Н. (MelexinVN)
*/
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_ll_iwdg.h"
#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_spi.h"
#include "stm32f0xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Добавление пользовательских библиотек
#include "spi.h"						//библиотека интерфейса SPI
#include "kombo_nrf24.h"		//библиотека радиомодуля nrf24l01
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin LL_GPIO_PIN_1
#define LED_GPIO_Port GPIOA
#define IRQ_Pin LL_GPIO_PIN_2
#define IRQ_GPIO_Port GPIOA
#define IRQ_EXTI_IRQn EXTI2_3_IRQn
#define CE_Pin LL_GPIO_PIN_3
#define CE_GPIO_Port GPIOA
#define CSN_Pin LL_GPIO_PIN_4
#define CSN_GPIO_Port GPIOA
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif
/* USER CODE BEGIN Private defines */
//Выбор микроконтроллера и периферийной библиотеки
#define STM32_LL					//stm32, библиотека LL

//Адреса устройств
#define OUR_ADDR 		0xA2	//адрес данного устройства
#define SID_ADDR		0xA1	//адрес пульта управления

//Коды команд
#define ON_OFF 			0xC0	//включение/выключение
#define MODE_UP 		0xC3	//смена режима

//Макросы управления светодиодом
#define LED_ON()		LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin)		//включение светодиода
#define LED_OFF()		LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin)	//выключение светодиода
#define LED_TGL()		LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin)				//смена состояния светодиода

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
