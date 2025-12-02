/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "funcoes_SPI_display.h"
#include "stm32f1xx_it.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DT_VARRE  5             // inc varredura a cada 5 ms (~200 Hz)
#define DIGITO_APAGADO 0x10     // kte valor p/ apagar um dígito no display
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int8_t ValAdc[] = {0,0,0,0};           // vetor com vals BCD do conv ADC
int8_t ValTime[] = {0,0,0,0};           // vetor com vals BCD do conv ADC
int state_machine = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  uint16_t val7seg = 0x00FF,           // inicia 7-seg com 0xF (tudo apagado)
  decPoint = 0x7FFF,           // liga o ponto decimal da unidade
  serial_data = 0x01FF;           // dado a serializar (dig | val7seg)
  uint32_t tNow = 0,                   // tempo que representa agora
  tIN_varre = 0;              // registra tempo última varredura
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  reset_pinos_emula_SPI ();            // começa SPI emulada com pinos = 'high'
  // definir os estados da FSM começando sttVarre varrendo milésimo (LSD)
  static enum {DIG_UNI, DIG_DECS, DIG_CENS, DIG_MILS} sttVARRE=DIG_MILS;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    tNow = HAL_GetTick();              // salva o tempo de agora
    state_machine = get_state();

    // tarefa #1: se (get_stt_ADC=1) dispara uma conversão ADC
    if (state_machine == 0) {
      // dispara por software uma conversão ADC
      HAL_ADC_Start_IT(&hadc1);        // dispara ADC p/ conversão por IRQ
    }

    // tarefa #2: enquanto tNow não contar novo delta_T p/ atualizar display
    if ((tNow - tIN_varre) > DT_VARRE) {
      continue;                        // próxima iteração
    }
    // se deu tempo de nova atualização no display
    tIN_varre = tNow;                  // salva tIN p/ prox tempo varredura
    time_update_values();
    switch (sttVARRE) {                // qual DIG vai varrer?
      case DIG_MILS:                     // se varre milésimo (LSD)
        sttVARRE = DIG_CENS;             // ajusta p/ prox digito
        serial_data = 0x0008;            // display #1 do painel 4x7-seg
        if (state_machine == 0){
          val7seg = conv_7_seg(ValAdc[0]); // dígito menos significativo
        }
        else{
          val7seg = conv_7_seg(ValTime[0]); // dígito menos significativo
        }
        break;
      case DIG_CENS:                     // varre o centésimo
        sttVARRE = DIG_DECS;             // ajusta p/ prox digito
        serial_data = 0x00004;           // display #2
        // se centésimo, décimo ou a unidade != 0, mostrar dígito
        if (state_machine == 0){
          if (ValAdc[1]>0 || ValAdc[2]>0 || ValAdc[3]>0) {
            val7seg = conv_7_seg(ValAdc[1]);// valor do centésimo
          }
          else {                         // senão, apagar o dígito
            val7seg = conv_7_seg(DIGITO_APAGADO);
          }
        }
        else{
          if (ValAdc[1]>0 || ValAdc[2]>0 || ValAdc[3]>0) {
            val7seg = conv_7_seg(ValTime[1]);// valor do centésimo
            val7seg &= decPoint;
          }
          else {                         // senão, apagar o dígito
            val7seg = conv_7_seg(DIGITO_APAGADO);
          }
        }

        break;
      case DIG_DECS:                     // varre o décimo
        sttVARRE = DIG_UNI;              // ajusta p/ prox digito
        serial_data = 0x0002;            // display #3
        // se o décimo ou a unidade são diferentes de zero
        if (state_machine == 0){
          if (ValAdc[2]>0 || ValAdc[3]>0) {
            val7seg = conv_7_seg(ValAdc[2]); // mostrar décimo
          } else {                         // senão apagar dígito
            val7seg = conv_7_seg(DIGITO_APAGADO);
          }
        }
        else{
          if (ValAdc[2]>0 || ValAdc[3]>0) {
            val7seg = conv_7_seg(ValTime[2]); // mostrar décimo
          }
          else {                         // senão apagar dígito
            val7seg = conv_7_seg(DIGITO_APAGADO);
          }
        }
        break;
      case DIG_UNI:                      // varre MSD - unidade
        sttVARRE = DIG_MILS;             // ajusta p/ prox digito
        serial_data = 0x0001;            // display #4
        //
        if (state_machine == 0){
          if (ValAdc[3] > 0) {             // se unidade > 0
            val7seg = conv_7_seg(ValAdc[3]); // ligar o display unidade
            val7seg &= decPoint;           // liga o ponto decimal
          } else {                         // se unidade =0, dig apagado
            val7seg = conv_7_seg(DIGITO_APAGADO);
          }
        }
        else{
          if (ValTime[3] > 0) {             // se unidade > 0
          val7seg = conv_7_seg(ValTime[3]); // ligar o display unidade
            val7seg &= decPoint;           // liga o ponto decimal
          }
          else{
            val7seg = conv_7_seg(DIGITO_APAGADO);
          }
        }
        break;
    }  // fim case
    serial_data |= val7seg;            // OR com val7seg = dado a serializar
    serializar(serial_data);           // serializa dado p/74HC595 (shift reg)
  }    // -- fim do loop infinito
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
    |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* EXTI1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB5 PB6 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// fn que atende ao callback da ISR do conversor ADC1
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  uint16_t val_adc = 0;                // define var para ler ADC
  if(hadc->Instance == ADC1) {         // se veio ADC1
    val_adc = HAL_ADC_GetValue(&hadc1);// capta valor adc
    // converter o valor lido em valores hexa p/ display
    int mVolt = val_adc*3300/4095;     // converte p/ INT 32 bits
    ValAdc[3] = mVolt/1000;            // MSD unidade (8 bits bcd)
    ValAdc[2] = (mVolt%1000)/100;      // décimo
    ValAdc[1] = (mVolt%100)/10;        // centésimo
    ValAdc[0] = mVolt%10;              // LSD milésimo
    if (mVolt > 2000) {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    }
    else {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
    }
  }
  set_stt_ADC(0);                      // muda get_stt_ADC p/ 0
}

void time_update_values()
{
  uint32_t time_now;                // define var para ler ADC
  uint16_t sec1, sec2, min, cent, total_sec;
  if(1) {         // se veio ADC1
    time_now = HAL_GetTick();// capta valor adc
    // converter o valor lido em valores hexa p/ display

    // int time_now_32 = time_now * (3300/4095);     // converte p/ INT 32 bits
    total_sec = time_now / 1000;
    min = total_sec / 60;
    sec1 = ((total_sec - (60*min))/10);
    sec2 = total_sec % 10;
    cent = (time_now % 1000) / 100;
    ValTime[3] = min;// time_now / 1000;            // MSD unidade (8 bits bcd)
    ValTime[2] = sec1;// (time_now_32%1000)/100;      // décimo
    ValTime[1] = sec2;// (time_now_32%100)/10;        // centésimo
    ValTime[0] = cent; // time_now_32%10;              // LSD milésimo
  }
  set_stt_ADC(0);                      // muda get_stt_ADC p/ 0
}


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
#ifdef USE_FULL_ASSERT
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
