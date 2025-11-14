/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdint.h"                    // porque uso: uint8_t, int16_t, uint32_t
#include <string.h>                    // para criar uma string de msgs
#include <stdlib.h>                    // porque vou usar rand()
#include "funcoes_SPI_display.h"       // funções de display 7-segm e conversores
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/*  -=-=-=- Algumas definicoes desse projeto (Prof. J Ranhel):  -=-=-=-
"rqcrn" requisita o dado de seu kit (ao receber essa solicitação você deve enviar o dado do cronômetro)
"rqadc" requisita o dado de seu kit (ao receber essa solicitação você deve enviar o dado do ADC)
"rqCRN" requisita que a placa do colega mostre seu CRONOMETRO;
"rqADC" requisita que a placa do colega mostre seu ADC, em Volts.milivolts;
"XXXXa" seu kit recebe / envia 'a' + os dígitos do ADC em ASCII
"XXXXc" seu kit recebe / envia 'c' + os dígitos do cronometro em ASCII
~ A mensagem é montada do LSD BufOUT[0] p/ MSD BufOUT[3], c/ 5 caracteres.
~ Para receber um valor é necessário requisitá-lo.
~ O intervalo entre solicitacoes deve ser < 100 ms (para obter val decimo seg).
~ Para nao ter que escrever:
BufOUT[0]='r'; BufOUT[1]='q'; BufOUT[2]='s'; BufOUT[3]='r'; BufOUT[4]='v'; */

#define RQDCRN "rqcrn"       // define a string para pedir leitura cronometro
#define RQDADC "rqadc"       // define a string para pedir leitura adc
#define RQSCRN "rqCRV"       // define a string para solicitar servico CRONO
#define RQSADC "rqADC"       // define a string para descartar servico ADC

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

// macro = função que copia uma string (n chars) para o BufOUT[] (n items)
// char *strncpy(char *dest, const char *src, size_t n)
// copies up to n characters from the string pointed to by *src to 'dest'.
// strlen() function calc the length of a string (returns a size_t ( uint )
#define STR_BUFF(str) do { \
    const char *src = str; \
    strncpy((char *)(BufOUT), src, sizeBuffs); \
} while (0)

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

// essas macros definem kts e delays mais comuns no projeto
#define MD_CRONO 0             // cronômetro =0 incrementa, =1 decrementa
#define DT_MUX_DISP 6          // dt = ~7ms para multiplex display (142 vz/s)
#define DT_ADC 200-1           // aqui É O SEU samples/seg de acordo com seu RA
#define DT_CRONO 100-1         // dt = 99 ms (== 100 e ajusta crono, reset p/ 00)
#define DT_DISPLAY 4000-1      // dt para mudar display a cada 4000 ms
#define DT_LEDS 150-1          // intervalo tempo para piscar leds

#define PA1 1u                 // em que pino o PA1 está (qtos bits deslocar)
#define PA2 2u                 // em que pino o PA2 está (qtos bits deslocar)
#define PA3 3u                 // em que pino o PA3 está (qtos bits deslocar)
#define DT_CKECKEYS 4          // intervalo de tempo p/ verificar teclas/botões
#define PA1CHECK 0xFFFFFFFFu   // check estado default do botão PA1 (tudo '1')
#define PA2CHECK 0xFFFFFFFFu   // check estado default do botão PA2 (tudo '1')
#define PA3CHECK 0xFFFFFFFFu   // check estado default do botão PA3 (tudo '1')

// para funcoes_display
#define NDIGSDISP 4            // quatro digitos nos displays
#define NUMSEGS 7              // ligar 7 segs (leds) em cada display
#define DIG_APAGADO 0x10       // kte valor p/ apagar um dígito no display
// para interrupções
#define DT_DEBOUNCING 250-1    // delay P/ debouncing - ~250 ms (no xx_it.c)
#define DT_EFEITO_INI 75-1     // delay p/ efeito inicial nos LEDs ~75 ms

//#define GPIOA_IDR *((uint32_t *)0x40010C08ul) // end. reg GPIOB Input Data
//#define GPIOB_ODR *((uint32_t *)0x40010C0Cul) // end. reg GPIOB Output Data

// vals enumerados dos SINAIS emitidos pelos botões
enum {A1_FALL, A1_DOWN, A1_UP, A2_FALL, A2_DOWN, A2_UP, A3_FALL, A3_DOWN, A3_UP, NO_BOT};
// valores enumerados dos sinais/msgs a serem ENVIADAS
enum {sndNADA, sndCRN, sndADC, sndREQCRN, sndREQADC, sndREQSRV, sndREQOFF, sndMSGNSV};
// valores enumerados dos sinais/msgs que foram RECEBIDAS
enum {rcvNADA, rcvCRN, rcvADC, rcvREQCRN, rcvREQADC, rcvREQSRV, rcvREQOFF, rcvMSGNSV};
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
