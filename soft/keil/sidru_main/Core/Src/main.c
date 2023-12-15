/* USER CODE BEGIN Header */
/*
* main.c
* Основной файл программы контроллера светодиодов ws2812b
* СветоИздучающего Декоративного Радиоуправляемого Устройства (СИДРУ)
* Проект "КомБО" (Открытые системы беспроводной коммуникации)
* 
* Микроконтроллер:  stm32f030f4p6
* Приемопередатчик: nrf24l01+
*
* Подключение периферии:
* NRF24L01		STM32
* CE					PA3(9)					
* CSN					PA4(10)					
* SCK					PA5(11)					
* MOSI				PA7(13)					
* MISO				PA6(12)					
* IRQ					PA2(8)				
*
* WS2812			PB1(14)				
*
*   Краткое описание работы устройства:
* При включении питания адресным светодиодам отправляется команда на зажжение 
* белого цвета в течение 50 мс для проверки работоспособности.
* В процессе работы устройство отправляет запросы пульту управления и принимает 
* от него команды включения/выключения, либо смены режима.
* При отсутствии команд устройство циклически отправляет команды на светодиоды 
* в соответствии с текущим режимом.
*
* Алгоритм управления адресными светодиодами позаимствован с 
* https://kawaii.computer/stm32/2020/06/07/ws2812b-stm32f0-circular-dma.html
* Для управления светодиодами используется DMA и ШИМ (таймер 3 канал 4)
*
* Примечания:
* - Проект сгенерирован в STM32CubeMX, структура проекта, основные настройки и инициализация периферии 
*   созданы автоматически.
* - Для МК stm32f030f4p6 - без перемычки на BOOT (1) - старт в режиме загрузчика, можно подключиться
*   по USART и через SWD, с перемычкой - нормальная загрузка, можно подключиться только через SWD.
* - Алгоритм светового эффекта "радуга" позаимствован с репозитория:
* 	https://github.com/hubmartin/WS2812B_STM32F4/
*
*		Автор: Мелехин В.Н. (MelexinVN)
*/

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//объявляем переменные и массивы
uint32_t millis = 0;				//милисекунды
uint32_t temp_millis = 0;		//вспомогательная переменная для организации пауз
uint32_t led_millis = 0;		//вспомогательная переменная для организации пауз
uint32_t pause_millis = 0;	//вспомогательная переменная для организации пауз
uint32_t led_pause = 10;		//значение паузы для светоэффектов

//внешние переменные и массивы
extern volatile uint8_t f_rx, f_tx;			//флаги приема и передачи радиомодуля
extern uint8_t tx_buf[TX_PLOAD_WIDTH];	//буфер передачи радиомодуля
extern uint8_t rx_buf[TX_PLOAD_WIDTH];	//буфер приема радиомодуля

extern uint32_t grb[LED_COUNT];					//массив состояния светодиодов
//Каждый элемент представляет собой значение GRB для соответствующего индекса светодиода, 
//например, 0x00ffffff - полностью белый, 0x00ff0000 - полностью зеленый, и тд.

extern uint16_t grb_offset;							//смещение в массиве GRB при выводе светодиодов
extern uint8_t buffer[BUFFER_LEN];			//буфер 

uint8_t led_mode = 0;										//режим световых эффектов
uint8_t save_mode = 1;									//сохраненное значение режима

uint8_t f_pause = 0;										//флаг включения паузы

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_IWDG_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//Подпрограмма обработки прерываний от таймера 1
//происходит 1 раз в милисекунду
void TIM1_callback(void)
{
	//если флаг прерывания таймера поднят
	if(LL_TIM_IsActiveFlag_UPDATE(TIM1))			
	{
		//считаем текущую милисекунду
		millis++;
		//опускаем флаг прерывания
		LL_TIM_ClearFlag_UPDATE(TIM1);					
	}
}

//Процедура приема радиомодуля
void nrf24l01_receive(void)
{
	if(f_rx)	//если флаг приема поднят (флаг поднимается по внешнему прерыванию от радиомодуля)
	{
		if (rx_buf[0] == OUR_ADDR)	//если первый принятый байт совпадает с адресом устройства
		{
			if (rx_buf[1] == ON_OFF)	//если команда включения/выключения
			{
				if (led_mode == 0)			//если светодиоды отключены (нулевой режим)
				{
					led_mode = save_mode;	//включаем светодиоды в сохраненном режиме
				}
				else										//иначе
				{
					save_mode = led_mode;	//сохраняем значение текущего модуля
					led_mode = 0;					//выключаем светодиоды (устанавливаем нулевой режим)
				}
				f_pause = 1;						//поднимаем флаг паузы для предотвращения приема лишних команд
				pause_millis = millis;	//сохраняем текущее время
			}
			else if (rx_buf[1] == MODE_UP)	//если команда смены режима
			{
				if (led_mode)									//если устройство включено
				{
					if (led_mode < MODES_COUNT) led_mode++;	//если режим не крайний, присваиваем переменной следующее значение
					else led_mode = 1;											//если режим крайний, включаем первый режим
					f_pause = 1;														//поднимаем флаг паузы
					pause_millis = millis;									//сохраняем текущее время
				}
			}
		}
		f_rx = 0;										//опускаем флаг приема
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, 3);

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_IWDG_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

	//Включаем SPI
	LL_SPI_Enable(SPI1);
	
	//Включаем внешнее прерывание
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_2);
	LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_2);
	
	//Конфигурируем таймер 1
	LL_TIM_EnableIT_UPDATE(TIM1);
	LL_TIM_EnableCounter(TIM1);

	//Конфигурируем таймер 3
	LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH4);
	LL_TIM_EnableAllOutputs(TIM3);
	LL_TIM_EnableDMAReq_CC4(TIM3);
	LL_TIM_EnableCounter(TIM3);
	
	//Конфигурируем DMA
	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)&TIM3->CCR4);
	LL_DMA_EnableIT_HT(DMA1, LL_DMA_CHANNEL_3); 
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);

	//Инициализируем радиомодуль
	nrf24_init();						
	
	//Моргаем светодиодами
	blink_leds();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//если прошло больше 50 мс
		if (millis-temp_millis > 50)
		{
			//отсылаем запрос
			tx_buf[0] = REMOTE_ADDR;	//заполняем поле адреса
			nrf24_send(tx_buf);				//отправляем посылку
			
			temp_millis = millis;			//сохраняем значение времени
		}
		//если режим 0 (выключено)
		if(led_mode == 0)
		{
			//заполняем буфер
			for(uint8_t i = 0; i < LED_COUNT; i++)
			{
				grb[i] = new_color(0, 0, 0);	//заполняем буфер нулевыми данными
			}
		}
		//далее в зависимости от режима заполняем массив соответсвующим образом
		else if(led_mode == 1)
		{
			//радуа
			rainbow(15);
		}
		//остальные режимы требуют некоторой паузы
		//если прошло времени больше паузы
		if (millis-led_millis > led_pause)
		{
			if (led_mode == 2)
			{
				//бегущий зеленый
				green_run();
			}
			
			else if (led_mode == 3)
			{
				//бегущий красный
				red_run();
			}

			else if (led_mode == 4)
			{
				//бегущий синий
				blue_run();
			}

			else if (led_mode == 5)
			{
				//бегущие красный и синий
				red_blue_run();
			}

			else if (led_mode == 6)
			{
				//бегущие красный и зеленый
				red_green_run();
			}

			else if (led_mode == 7)
			{
				//бегущие синий и зеленый
				blue_green_run();
			}

			if (led_mode == 8)
			{
				led_pause = 200;		//устанавливаем новое значение паузы
				//случайные цвета
				random_noise();
			}
			else led_pause = 10;	//возвращаем  значение паузы по умолчанию
			
			led_millis = millis;	//сохраняем значение времени
		}
		
		__disable_irq();				//отключаем все прерывания
		led_update();						//отправляем данные светодиодам
		__enable_irq();					//включаем все прерывания

		if (f_pause)						//если поднят флаг паузы
		{												
			//если прошло достаточно времени
			if (millis-pause_millis > DELAY) 
			{
				f_pause = 0;				//опускаем флаг паузы
			}
		}
		else										//если опущен флаг паузы
		{
			nrf24l01_receive();		//обрабатываем прием радиомодуля
		}
		
		//Далее алгоритм смены режимов ДЛЯ ДЕМОНСТРАЦИИ устройства
		//если решим не нулевой
		if (led_mode)
		{	//если прошло достаточно времени
			if (millis - pause_millis > DELAY*20) 
			{
				if (led_mode < MODES_COUNT) led_mode++;	//если режим не крайний, присваиваем переменной следующее значение
				else led_mode = 1;											//если режим крайний, включаем первый режим
				pause_millis = millis;									//сохраняем значение времени
			}
		}
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		LL_IWDG_ReloadCounter(IWDG);//сбрасываем сторожевой таймер
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
  {
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_LSI_Enable();

   /* Wait till LSI is ready */
  while(LL_RCC_LSI_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_6);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_Init1msTick(48000000);
  LL_SetSystemCoreClock(48000000);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  LL_IWDG_Enable(IWDG);
  LL_IWDG_EnableWriteAccess(IWDG);
  LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_32);
  LL_IWDG_SetReloadCounter(IWDG, 4095);
  while (LL_IWDG_IsReady(IWDG) != 1)
  {
  }

  LL_IWDG_ReloadCounter(IWDG);
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  LL_SPI_InitTypeDef SPI_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SPI1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**SPI1 GPIO Configuration
  PA5   ------> SPI1_SCK
  PA6   ------> SPI1_MISO
  PA7   ------> SPI1_MOSI
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 7;
  LL_SPI_Init(SPI1, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
  LL_SPI_EnableNSSPulseMgt(SPI1);
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);

  /* TIM1 interrupt Init */
  NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0);
  NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 47999;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM1, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM1);
  LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM1);
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  /* TIM3 DMA Init */

  /* TIM3_CH4_UP Init */
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_VERYHIGH);

  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_CIRCULAR);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PDATAALIGN_HALFWORD);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MDATAALIGN_BYTE);

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 59;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM3);
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH4);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 0;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH4, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH4);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM3);
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**TIM3 GPIO Configuration
  PB1   ------> TIM3_CH4
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* Init with LL driver */
  /* DMA controller clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

  /* DMA interrupt init */
  /* DMA1_Channel2_3_IRQn interrupt configuration */
  NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0);
  NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

  /**/
  LL_GPIO_ResetOutputPin(CE_GPIO_Port, CE_Pin);

  /**/
  LL_GPIO_ResetOutputPin(CSN_GPIO_Port, CSN_Pin);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE2);

  /**/
  LL_GPIO_SetPinPull(IRQ_GPIO_Port, IRQ_Pin, LL_GPIO_PULL_NO);

  /**/
  LL_GPIO_SetPinMode(IRQ_GPIO_Port, IRQ_Pin, LL_GPIO_MODE_INPUT);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_2;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  GPIO_InitStruct.Pin = CE_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(CE_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = CSN_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(CSN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  NVIC_SetPriority(EXTI2_3_IRQn, 0);
  NVIC_EnableIRQ(EXTI2_3_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

