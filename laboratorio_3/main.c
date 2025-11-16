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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DT_D1 (50-1)                   // delay do LED D1
#define DT_D2 (75-1)                  // delay do LED D2
#define DT_D3 (100-1)                  // delay do LED D3
#define DT_D4 (125-1)                  // delay do LED D4
#define PER_PB7 800                   // tempo do ciclo (ms) do led PB7
#define PER_PB8 4000                   // tempo do ciclo (ms) do led PB8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// lembrete ‘C’: quando fn() é declarada antes de ser chamada não requer protótipo!
void apaga_leds(void)                  // apaga os LEDs da GPIOB
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15|GPIO_PIN_14|
                    GPIO_PIN_13|GPIO_PIN_12, GPIO_PIN_SET);
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
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  // para controlar tempos de piscar os LEDs e alterar duty-cycle do PWM
  uint32_t tNow=0, tinLED=0, tinD7=0, tinD8=0;
  uint16_t arrT4 = TIM4->ARR;        // lê Auto-Reload (resolução) do PWM T4
  // estados enumerados da FSM principal piscar
  static enum {PISCA1, PISCA2, PISCA3, PISCA4} sttMAINFSM=PISCA1;
  // estados enumerados das subFSM piscar
  static enum {INILED, LIGALED, DSLGLED} sttLED = INILED;
  // estados enumerados da FSM que controla PWM
  static enum {INIPWMD7, INCPWMD7, DECPWMD7}  sttPWMD7 = INIPWMD7;
  static enum {INIPWMD8, INCPWMD8, DECPWMD8}  sttPWMD8 = INIPWMD8;

  // antes de entrar no while(1), inicializar os valores analógicos
  uint16_t anD7 = 0;                 // ini anD7 (brilho LED D7)
  uint16_t anD8 = arrT4;             // ini anD8 com máximo

  // de quantos em quantos ms temos que alterar valor analógico?
  int dtD7 = (PER_PB7/2)/arrT4;      // calc delta tempo p/ PB7
  int dtD8 = (PER_PB8/2)/arrT4;      // calc delta tempo p/ PB8 = PB7

  int i = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    tNow = HAL_GetTick();              // pega o tempo nesse momento

    // FSM dedicada a controlar o piscar dos LEDs (“top-slot” - executa sempre)
    switch (sttMAINFSM) {              // maq estado principal (quem pisca)
      // tarefa #1 : piscar e controlar o estado do LED D1
      case PISCA1:
        switch (sttLED) {
          case INILED:                     // vai iniciar a máquina de estado
            apaga_leds();                  // desliga todos os LEDs
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET); // liga o LED D1
            sttLED = LIGALED;              // prox subestado da máquina
            break;
          case LIGALED:                    // estado para ligar o LED
            if ((tNow - tinLED) > DT_D1) { // se tNow - tinLED >= DT_D1
              tinLED = tNow;               // guarda tempo p/ prox mudança estado
              HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); // desliga o LED
              sttLED = DSLGLED;            // muda o prox subestado da máquina
            }
            break;
          case DSLGLED:                    // estado para desligar o LED
            if ((tNow - tinLED) > DT_D1) { // se tNow - tinLED >= DT_D1
              tinLED = tNow;               // guarda tempo p/ prox mudança estado

              if (i > 8){
                sttMAINFSM = PISCA2;         // muda o prox estado da main FSM
                sttLED = INILED;             // muda o prox subestado da máquina
              }
              else {
                i += 1;
                sttMAINFSM = PISCA1;
                sttLED = INILED;             // muda o prox subestado da máquina
              }
              sttLED = INILED;             // muda o prox subestado da máquina
              break;
            }
        };
        break;    // fim case PISCA1

      // tarefa #2 : piscar e controlar o estado do LED D2
      case PISCA2:
        i=0;
        switch (sttLED) {
          case INILED:                     // vai iniciar a máquina de estado
            apaga_leds();                  // desliga todos os LEDs
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // liga o LED D2
            sttLED = LIGALED;              // prox subestado da máquina
            break;
          case LIGALED:                    // estado para ligar o LED
            if ((tNow - tinLED) > DT_D2) { // se tNow - tinLED >= DT_D2
              tinLED = tNow;               // guarda tempo p/ prox mudança estado
              HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // desliga o LED
              sttLED = DSLGLED;            // muda o prox subestado da máquina
            }
            break;
          case DSLGLED:                    // estado para desligar o LED
            if ((tNow - tinLED) > DT_D2) { // se tNow - tinLED >= DT_D2
              tinLED = tNow;               // guarda tempo p/ prox mudança estado

              if (i > 6){
                sttMAINFSM = PISCA3;         // muda o prox estado da main FSM
                sttLED = INILED;             // muda o prox subestado da máquina
              }
              else {
                i += 1;
                sttMAINFSM = PISCA2;
                sttLED = INILED;             // muda o prox subestado da máquina
              }
              sttLED = INILED;             // muda o prox subestado da máquina
              break;
            }
        };
        break;  // fim case PISCA2

      // tarefa #3 : piscar e controlar o estado do LED D3
      case PISCA3:
        i=0;
        switch (sttLED) {
          case INILED:                     // vai iniciar a máquina de estado
            apaga_leds();                  // desliga todos os LEDs
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); // liga o LED D2
            sttLED = LIGALED;              // prox subestado da máquina
            break;
          case LIGALED:                    // estado para ligar o LED
            if ((tNow - tinLED) > DT_D3) { // se tNow - tinLED >= DT_D3
              tinLED = tNow;               // guarda tempo p/ prox mudança estado
              HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // desliga o LED
              sttLED = DSLGLED;            // muda o prox subestado da máquina
            }
            break;
          case DSLGLED:                    // estado para desligar o LED
            if ((tNow - tinLED) > DT_D3) { // se tNow - tinLED >= DT_D3
              tinLED = tNow;               // guarda tempo p/ prox mudança estado
              sttLED = INILED;             // muda o prox subestado da máquina
              sttMAINFSM = PISCA4;         // muda o prox estado da main FSM
            }
            break;
        };
        break;  // fim case PISCA3

      // tarefa #4 : piscar e controlar o estado do LED D4
      case PISCA4:
        i=0;
        switch (sttLED) {
          case INILED:                     // vai iniciar a máquina de estado
            apaga_leds();                  // desliga todos os LEDs
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // liga o LED D4
            sttLED = LIGALED;              // prox subestado da máquina
            break;
          case LIGALED:                    // estado para ligar o LED
            if ((tNow - tinLED) > DT_D4) { // se tNow - tinLED >= DT_D4
              tinLED = tNow;               // guarda tempo p/ prox mudança estado
              HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // desliga o LED
              sttLED = DSLGLED;            // muda o prox subestado da máquina
            }
            break;
          case DSLGLED:                    // estado para desligar o LED
            if ((tNow - tinLED) > DT_D4) { // se tNow - tinLED >= DT_D4
              tinLED = tNow;               // guarda tempo p/ prox mudança estado
              sttLED = INILED;             // muda o prox subestado da máquina
              sttMAINFSM = PISCA1;         // muda o prox estado da main FSM
            }
            break;
        };
        break;  // fim case PISCA4
    };  // fim do main FSM sttMAINFSM


    // FSM dedicada a controlar os LEDs com PWM_D7
    switch (sttPWMD7) {                // maq estado principal (quem pisca)
      case INIPWMD7:                     // ini PWM timer 4 p/ canal 2 (D7)
        HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2); // inicializa PWM no T4C2
        TIM4->CCR2 = anD7;               // inicia comparador PWM c/ 0
        sttPWMD7 = INCPWMD7;             // prox estado, increm INCPWMD7
        break;

      case INCPWMD7:
        if ((tNow - tinD7) > dtD7) {     // se hora de inc brilho
          tinD7 = tNow;                  // tempo p/ prox mudança anD7
          ++ anD7;                       // incrementa anD7
          if (anD7 >= arrT4) {           // se atingiu o máx brilho
            anD7 = arrT4;                // garante o máximo brilho
            sttPWMD7 = DECPWMD7;         // muda maq est p/ dec brilho
          }
          TIM4->CCR2 = anD7;             // set comparador com valor anD7
        }
        break;

      case DECPWMD7:
        if ((tNow - tinD7) > dtD7) {     // se hora de inc brilho
          tinD7 = tNow;                  // tempo p/ prox mudança anD7
          -- anD7;                       // decrementa anD7
          if (anD7 == 0) {               // se atingiu mínimo brilho
            sttPWMD7 = INCPWMD7;         // muda maq est p/ inc brilho
          }
          TIM4->CCR2 = anD7;             // set comparador com valor anD7
        }
        break;
    };  // fim da FSM que controla o PWM_D7

    // FSM dedicada a controlar os LEDs com PWM_D8 (“bottom-slot” - executa sempre)
    switch (sttPWMD8) {                // maq estado principal (quem pisca)
      case INIPWMD8:                     // ini PWM timer 4 p/ canal 3 (D8)
        HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3); // inicializa PWM no T4C3
        TIM4->CCR3 = anD8;               // inicia comparador PWM c/ ARR-1
        sttPWMD8 = DECPWMD8;             // prox estado, increm DECPWMD8
        break;

      case INCPWMD8:
        if ((tNow - tinD8) > dtD8) {     // se hora de inc brilho
          tinD8 = tNow;                  // tempo p/ prox mudança anD8
          ++ anD8;                       // incrementa anD8
          if (anD8 >= arrT4) {           // se atingiu o máx brilho
            anD8 = arrT4;                // garante o máximo brilho
            sttPWMD8 = DECPWMD8;         // muda maq est p/ dec brilho
          }
          TIM4->CCR3 = anD8;             // set comparador com valor anD8
        }
        break;

      case DECPWMD8:
        if ((tNow - tinD8) > dtD8) {     // se hora de inc brilho
          tinD8 = tNow;                  // tempo p/ prox mudança anD8
          -- anD8;                       // decrementa anD8
          if (anD8 == 0) {               // se atingiu mínimo brilho
            sttPWMD8 = INCPWMD8;         // muda maq est p/ inc brilho
          }
          TIM4->CCR3 = anD8;             // set comparador com valor anD8
        }
        break;
    };  // fim da FSM que controla o PWM_D8

  } // fim while(1)
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
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 2099;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 199;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
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
