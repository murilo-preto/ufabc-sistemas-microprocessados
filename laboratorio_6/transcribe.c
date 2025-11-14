/* Includes ­­­­­­­­­­­­­­­­­­­­­­­­­­*/ 
#include "main.h" 
#include "STM32f1xx_it.h" 
/* Private includes ­­­­­­­­­­­­­­­­­­*/ 

/* 
--- NEW SECTION ---
*/

void SysTick_Handler(void) 
{ 
  /* USER CODE BEGIN SysTick_IRQn 0 */ 
  /* USER CODE END SysTick_IRQn 0 */ 
  HAL_IncTick(); 
  /* USER CODE BEGIN SysTick_IRQn 1 */ 
  HAL_SYSTICK_Callback(); 
  /* USER CODE END SysTick_IRQn 1 */ 
} 

/* 
--- NEW SECTION ---
*/

/* USER CODE BEGIN Includes */ 
#include "stdint.h" 
/* USER CODE END Includes */ 

/* USER CODE BEGIN EFP */ 
void HAL_SYSTICK_Callback(void); 
/* USER CODE END EFP */ 

/* 
--- NEW SECTION ---
*/

/* USER CODE BEGIN Includes */ 
#include "funcoes_SPI_display.h" 
/* USER CODE END Includes */ 

/* USER CODE BEGIN PD */ 
#define MD_CRONO 1                     // modo cronometro decrementar 
#define DT_CRONO (100­1)               // conta 100 ms e dec o contador 
#define DT_DEB_LOW 200                 // tempo debouncing fall 
#define DT_DEB_HIGH 100                // tempo debouncing rising 
#define DTLED (125­1)                  // delay do piscar LEDs (125 on/off) 
#define DT_BA3_RESET (4000­1)          // tempo BA3 em 0 para resetar (4000 ms) 
#define DTALRMLED (75­1)               // tempo do alarme do led 
#define DTBUZ (150­1)                  // delay do buzzer (150 ms) 
#define REPETPISCA 5                   // num vz repete piscar dos leds 
#define DT_VARRE  5                    // inc varredura a cada 5 ms (~200 Hz) 
#define DIGITO_APAGADO 0x10            // kte valor p/ apagar um dígito no display 
#define NDGDSP 4                       // display com 4 dígitos 
/* USER CODE END PD */ 

/* 
--- NEW SECTION ---
*/

/* USER CODE BEGIN PV */ 
int8_t Crono[]  = {0,0,0,0};             // vetor com vals decimais do cronometro 
int8_t DspHex[] = {16,16,16,16};         // vetor valor p/ display (se=16 display off) 
int fRun = 0;                            // flag Run = 1 (modo pausado = 0) 
int fFim = 0;                            // flag Fim = 1 se chegou no 00:00 
/* USER CODE END PV */ 

/* 
--- NEW SECTION ---
*/

/* USER CODE BEGIN PFP */ 
void fn_piscaLedSeq(uint32_t t);         // prot função para piscar leds 
void fn_pisca_Leds_Buzzer(uint32_t t);   // prot fn pisca todos e toca buzzer 
void desliga_leds_buzzer(void);          // prot fn apaga leds e desliga buzzer 
/* USER CODE END PFP */ 

/* 
--- NEW SECTION ---
*/

/* USER CODE BEGIN 2 */ 

reset_pinos_emula_SPI ();            // começa SPI emulada com pinos = 'high' 
// definir os estados da FSM que controla 'debouncing' dos botões 
static enum {DB_NORMAL, DB_FALL, DB_LOW, DB_RISING} 
sttBTA1 = DB_NORMAL,                 // var estado FSM debouncing A1 
sttBTA2 = DB_NORMAL,                 // var estado FSM debouncing A2 
sttBTA3 = DB_NORMAL;                 // var estado FSM debouncing A3 
uint32_t tNow = 0,                   // tempo que representa agora 
tIN_A1 = 0,                   // salva tempo debouncing A1 
tIN_A2 = 0,                   // salva tempo debouncing A2 
tIN_A3 = 0,                   // salva tempo debouncing A3 
tIN_varre = 0;                   // salva tempo última varredura 
uint8_t ptDec = 0b1010;              // ligar pts decs min e dec seg (eg: x._x._) 
/* USER CODE END 2 */ 

/* 
--- NEW SECTION ---
*/

/* USER CODE BEGIN 3 */ 
tNow = HAL_GetTick();              // salva o tempo de agora 

// tarefa #1: polling botão PA1, debouncing fall e rise 
switch(sttBTA1) { 
  case DB_NORMAL:                    // se bt sem acionar muito tempo ­ normal 
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 0) { 
      tIN_A1 = tNow;                 // salva tempo do fall 
      ++ Crono[3];                   // incrementa unid minutos 
      if (Crono[3] > 9) {            // se maior que 9 
        Crono[3] = 0;                // volta unid min p/ 0 
      } 
      sttBTA1 = DB_FALL;             // prox estado 'FALL' 
    } 
    break; 
  case DB_FALL: 
    if ((tNow­tIN_A1)>DT_DEB_LOW) { // tempo do debouncing L? 
      sttBTA1 = DB_LOW;             // prox estado 'LOW' 
    } 
    break; 
  case DB_LOW: 
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 1) { 
      tIN_A1 = tNow;                 // salva tempo da subida 
      sttBTA1 = DB_RISING;           // prox estado 'DB_RISING' 
    } 
    break; 
  case DB_RISING: 
    if ((tNow­tIN_A1)>DT_DEB_HIGH) { // tempo debouncing H? 
      sttBTA1 = DB_NORMAL;           // prox estado 'DB_NORMAL' 
    } 
    break; 
} 

/* 
--- NEW SECTION ---
*/

// tarefa #2: polling botão PA2 e fazer debouncing fall e rise 
switch(sttBTA2) { 
  case DB_NORMAL:                    // se bt sem acionar muito tempo ­ normal 
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == 0) { 
      tIN_A2 = tNow;                 // salva tempo do fall 
      ++ Crono[1];                   // incrementa unid segundos 
      if (Crono[1] > 9) {            // se maior que 9 
        Crono[1] = 0;                // volta unid seg p/ 0 
        ++ Crono[2];                 // inc dezena de seg 
        if (Crono[2] > 5) {          // se maior que 5 (60) 
          Crono[2] = 0;              // volta p/ zero 
        } } 
      sttBTA2 = DB_FALL;             // prox estado 'FALL' 
    } 
    break; 
  case DB_FALL: 
    if ((tNow­tIN_A2)>DT_DEB_LOW) {  // tempo do debouncing L? 
      sttBTA2 = DB_LOW;              // prox estado 'LOW' 
    } 
    break; 
  case DB_LOW: 
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == 1) { 
      tIN_A2 = tNow;                 // salva tempo da subida 
      sttBTA2 = DB_RISING;           // prox estado 'DB_RISING' 
    } 
    break; 
  case DB_RISING: 
    if ((tNow­tIN_A2)>DT_DEB_HIGH) { // tempo debouncing H? 
      sttBTA2 = DB_NORMAL;           // prox estado 'DB_NORMAL' 
    } 
    break; 
} 

/* 
--- NEW SECTION ---
*/

// tarefa #3: polling botão PA3, debouncing fall e rise contar tempo em '0' 
switch(sttBTA3) { 
  case DB_NORMAL:                    // se bt sem acionar muito tempo ­ normal 
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == 0) { 
      tIN_A3 = tNow;                 // salva tempo do fall 
      // muda flag modo RUN=1 ou PAUSA=0 ­ controla o temporizador 
      if (fRun==0) fRun=1; else fRun=0; 
      sttBTA3 = DB_FALL;             // prox estado 'FALL' 
    } 
    break; 
  case DB_FALL: 
    if ((tNow­tIN_A3)>DT_DEB_LOW) {  // tempo do debouncing L? 
      sttBTA3 = DB_LOW;              // prox estado 'LOW' 
    } 
    break; 
  case DB_LOW: 
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == 1) { 
      tIN_A3 = tNow;                 // salva tempo da subida 
      sttBTA3 = DB_RISING;           // prox estado 'DB_RISING' 
    } else {                         // bt==0 testa se passou 3 seg 
      if ((tNow­tIN_A3)>DT_BA3_RESET) {  // tempo A3=low para reset? 
        tIN_A3 = tNow;               // salva tempo da subida 
        Crono[0] = 0;                // volta crono p/ zero 
        Crono[1] = 0;                // volta crono p/ zero 
        Crono[2] = 0;                // volta crono p/ zero 
        Crono[3] = 0;                // volta crono p/ zero 
        fRun=0;                      // volta o crono p/ modo pausa 
      } } 
    break; 
  case DB_RISING: 
    if ((tNow­tIN_A3)>DT_DEB_HIGH) { // tempo debouncing H? 
      sttBTA3 = DB_NORMAL;           // prox estado 'DB_NORMAL' 
    } 
    break; 
} 

/* 
--- NEW SECTION ---
*/

// tarefa #4: ver se toca alarme, ou piscar os LEDs 
if ((fRun == 1) && (fFim == 1) ){  // chegou ao final ­ alarme 
  fn_pisca_Leds_Buzzer(tNow);      // piscar LEDs + buzzer 
} else {                           // não chegou ao final, piscar LEDs 
  fn_piscaLedSeq(tNow);            // piscar LEDs em sequência 
} 

/* 
--- NEW SECTION ---
*/

// tarefa #5: ver se é hora varrer os displays 
if ((tNow­tIN_varre) < DT_VARRE) continue; 
tIN_varre = tNow;                  // registra tempo última varredura 
// transfere o conteúdo de Crono para o vetor display DspHex 
for (int i = 0; i<NDGDSP; i++) {   // transf conteúdo p/ DspHex 
  DspHex[i] = Crono[i];            // DspHex[] = Crono[] 
} 
// chama fn varredura passando o ponteiro de DspHex (tam fixo) 
mostrar_no_display(DspHex, ptDec); // chama fn para atualizar display 
  }                                    // ­­ fim do loop infinito 
/* USER CODE END 3 */ 

/* 
--- NEW SECTION ---
*/


/* USER CODE BEGIN 4 */ 
// função de call­back do Systick ­ onde implemento o timer 
void HAL_SYSTICK_Callback(void) { 
  uint16_t static contaCRN = 0;        // conta num vezes até DT­CRONO 
  if (fRun == 0) return; 

  // CRONOMETRO UP/DOWN 
  if (contaCRN >= DT_CRONO) {          // já passou contaCRN ms? 
    fFim = 0;                          // em qq caso, assume fFim=zero 
    contaCRN = 0;                      // retorna conta para zero 
    if(MD_CRONO == 0){                 // MD_CRONO = 0 incrementa o cronômetro 
      ++ Crono[0];                     // inc décimo de segundos 
      if (Crono[0] > 9){               // se > 9 
        Crono[0] = 0;                  // volta p/ zero 
        ++ Crono[1];                   // inc unidade de segundo 
        if (Crono[1] > 9){             // se > 9 
          Crono[1] = 0;                // volta p/ zero 
          ++ Crono[2];                 // inc dezena de segundos 
          if (Crono[2] > 5){           // se > 5 
            Crono[2] = 0;              // volta p/ zero 
            ++ Crono[3];               // inc unidade de minutos 
            if (Crono[3] > 9){         // se > 9 
              Crono[3] = 9;            // retém em 9 
              Crono[2] = 5;            // retém em 5 
              Crono[1] = 9;            // retém em 9 
              Crono[0] = 9;            // retém em 9 
              fFim = 1;                // quando chega ao final fFim=1 
            }  }  }  } 
    } else {                           // MD_CRONO = 1 decrementa o cronômetro 
      ­­ Crono[0];                     // dec segundos 
      if (Crono[0] < 0){               // se < 0 
        Crono[0] = 9;                  // volta p/ 9 
        ­­ Crono[1];                   // dec unidade de segundo 
        if (Crono[1] < 0){             // se < 0 
          Crono[1] = 9;                // volta p/ 9 
          ­­ Crono[2];                 // dec dez de segundos 
          if (Crono[2] < 0){           // se < 0 
            Crono[2] = 5;              // volta p/ 5 
            ­­ Crono[3];               // dec unidade de minutos 
            if (Crono[3] < 0 ){        // se < 0 
              Crono[3] = 0;            // retém em 0 
              Crono[2] = 0;            // retém em 0 
              Crono[1] = 0;            // retém em 0 
              Crono[0] = 0;            // retém em 0 
              fFim = 1;                // quando chega ao final fFim=1 
            }  }  }  } 
    } 
  } else { ++contaCRN; }               // se nao muda crono, so' inc contaCRN 
}                                      // fim função callback do SysTick 

/* 
--- NEW SECTION ---
*/

// função que pisca os LEDs D1...D4 em sequência: 
void fn_piscaLedSeq(uint32_t tNow) 
{ 
  // estados enumerados da FSM LEDs e buzzer 
  static enum {PISCA1, PISCA2, PISCA3, PISCA4} sttLEDS=PISCA1; 
  // estados enumerados das subFSM 
  static enum {INILED, LIGALED, DSLGLED} sttLED=INILED; 
  static uint32_t tIN_LED = 0;         // salvar tempo de piscar leds 
  if(fRun == 0) return;                // se não está rodando não pisca LEDs 

  // muda para um dos estados da FSM sttLEDS conforme tempo decorrido 
  switch (sttLEDS) {                   // maq estado principal (quem pisca) 
    // subtarefa #1 : piscar e controlar o estado do LED D1 
    case PISCA1: 
      switch (sttLED) { 
        case INILED:                       // vai iniciar a máquina de estado 
          desliga_leds_buzzer();           // desliga todos os LEDs 
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET); // liga o LED D1 
          sttLED = LIGALED;                // prox subestado da máquina 
          break; 
        case LIGALED:                      // estado para ligar o LED 
          if ((tNow ­ tIN_LED) > DTLED) {  // se tNow ­ tinLED >= DTLED 
            tIN_LED = tNow;                // guarda tempo p/ prox mudança estado 
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); // desliga o LED 
            sttLED = DSLGLED;              // muda o prox subestado da máquina 
          } 
          break; 
        case DSLGLED:                      // estado para desligar o LED 
          if ((tNow ­ tIN_LED) > DTLED) {  // se tNow ­ tinLED >= DTLED 
            tIN_LED = tNow;                // guarda tempo p/ prox mudança estado 
            sttLED = INILED;               // muda o prox subestado da máquina 
            sttLEDS = PISCA2;              // muda o prox estado da FSM LEDS 
          } 
          break; 
      }; 
      break;                             // fim case PISCA1 

    /* 
--- NEW SECTION ---
*/

    // subtarefa #2 : piscar e controlar o estado do LED D2 
    case PISCA2: 
      switch (sttLED) { 
        case INILED:                       // vai iniciar a máquina de estado 
          desliga_leds_buzzer();           // desliga todos os LEDs 
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // liga o LED D2 
          sttLED = LIGALED;                // prox subestado da máquina 
          break; 
        case LIGALED:                      // estado para ligar o LED 
          if ((tNow ­ tIN_LED) > DTLED) {  // se tNow ­ tinLED >= DTLED 
            tIN_LED = tNow;                // guarda tempo p/ prox mudança estado 
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // desliga o LED 
            sttLED = DSLGLED;              // muda o prox subestado da máquina 
          } 
          break; 
        case DSLGLED:                      // estado para desligar o LED 
          if ((tNow ­ tIN_LED) > DTLED) {  // se tNow ­ tinLED >= DTLED 
            tIN_LED = tNow;                // guarda tempo p/ prox mudança estado 
            sttLED = INILED;               // muda o prox subestado da máquina 
            sttLEDS = PISCA3;              // muda o prox estado da FSM LEDS 
          } 
          break; 
      }; 
      break;                             // fim case PISCA2 

    /* 
--- NEW SECTION ---
*/

    // subtarefa #3 : piscar e controlar o estado do LED D3 
    case PISCA3: 
      switch (sttLED) { 
        case INILED:                       // vai iniciar a máquina de estado 
          desliga_leds_buzzer();           // desliga todos os LEDs 
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); // liga o LED D2 
          sttLED = LIGALED;                // prox subestado da máquina 
          break; 
        case LIGALED:                      // estado para ligar o LED 
          if ((tNow ­ tIN_LED) > DTLED) {  // se tNow ­ tinLED >= DTLED 
            tIN_LED = tNow;                // guarda tempo p/ prox mudança estado 
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // desliga o LED 
            sttLED = DSLGLED;              // muda o prox subestado da máquina 
          } 
          break; 
        case DSLGLED:                      // estado para desligar o LED 
          if ((tNow ­ tIN_LED) > DTLED) {  // se tNow ­ tinLED >= DTLED 
            tIN_LED = tNow;                // guarda tempo p/ prox mudança estado 
            sttLED = INILED;               // muda o prox subestado da máquina 
            sttLEDS = PISCA4;              // muda o prox estado da FSM LEDS 
          } 
          break; 
      }; 
      break;                             // fim case PISCA3 

    /* 
--- NEW SECTION ---
*/

    // subtarefa #4 : piscar e controlar o estado do LED D4 
    case PISCA4: 
      switch (sttLED) { 
        case INILED:                       // vai iniciar a máquina de estado 
          desliga_leds_buzzer();           // desliga todos os LEDs 
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // liga o LED D4 
          sttLED = LIGALED;                // prox subestado da máquina 
          break; 
        case LIGALED:                      // estado para ligar o LED 
          if ((tNow ­ tIN_LED) > DTLED) {  // se tNow ­ tinLED >= DTLED 
            tIN_LED = tNow;                // guarda tempo p/ prox mudança estado 
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // desliga o LED 
            sttLED = DSLGLED;              // muda o prox subestado da máquina 
          } 
          break; 
        case DSLGLED:                      // estado para desligar o LED 
          if ((tNow ­ tIN_LED) > DTLED) {  // se tNow ­ tinLED >= DTLED 
            tIN_LED = tNow;                // guarda tempo p/ prox mudança estado 
            sttLED = INILED;               // muda o prox subestado da máquina 
            sttLEDS = PISCA1;              // muda o prox estado da FSM LEDS 
          } 
          break; 
      }; 
      break;                             // fim case PISCA4 
  };                                   // fim switch() da mainFSM sttLEDS que pisca LEDs 
}                                      // fim da fn_piscaLedSequencia() 

/* 
--- NEW SECTION ---
*/

// função que apaga leds e desliga buzzer 
void desliga_leds_buzzer(void)         // apaga os LEDs da GPIOB 
{ 
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_13| 
                    GPIO_PIN_12|GPIO_PIN_5, GPIO_PIN_SET); 
}                                      // fim desliga_leds_buzzer() 

/* 
--- NEW SECTION ---
*/

// função que pisca todos os LEDs juntos e liga buzzer 
void fn_pisca_Leds_Buzzer(uint32_t tNow) 
{ 
  // estados enumerados das subFSM 
  static enum {INILED, LIGALED, DSLGLED, LIGABZZ, DESLIGABZZ} sttPINS=INILED; 
  static uint32_t tIN_LED = 0;         // salvar tempo de piscar leds 
  static int conta = 0;                // conta qtas vezes piscou leds 

  switch (sttPINS) {                   // maq estado principal (pisca todos e buzzer) 
    case INILED:                         // iniciar a máquina de estado 
      desliga_leds_buzzer();             // desliga todos os LEDs 
      sttPINS = LIGALED;                 // prox estado da máquina 
      break; 
    case LIGALED:                        // estado para ligar o LED 
      if ((tNow­tIN_LED)>DTALRMLED) {    // se tNow ­ tinLED >= DTALRMLED 
        tIN_LED = tNow;                  // guarda tempo p/ prox mudança estado 
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15|GPIO_PIN_14| 
                          GPIO_PIN_13|GPIO_PIN_12, GPIO_PIN_RESET); // liga os LEDs 
        sttPINS = DSLGLED;               // prox estado da máquina 
      } 
      break; 
    case DSLGLED:                        // estado para desligar o LED 
      if ((tNow­tIN_LED)>DTALRMLED) {    // se tNow ­ tinLED >= DTALRMLED 
        desliga_leds_buzzer();           // desliga todos os LEDs 
        tIN_LED = tNow;                  // guarda tempo p/ prox estado 
        if (conta < REPETPISCA) {        // guard condition: Ñ executou REP vezes? 
          ++ conta;                      // inc conta e pisca leds juntos 
          sttPINS = LIGALED;             // prox subestado da máquina 
        } else {                         // repetiu REP vezes, buzzer 
          conta = 0;                     // volta conta p/ 0 
          sttPINS = LIGABZZ;             // prox estado da FSM LEDS 
        } 
      } 
      break; 
    case LIGABZZ:                        // estado para ligar o BUZZER 
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); // liga buzz 
      tIN_LED = tNow;                    // guarda tempo p/ prox estado 
      sttPINS = DESLIGABZZ;              // prox estado da FSM LEDS 
      break; 
    case DESLIGABZZ:                     // estado para desligar o BUZZER 
      if ((tNow ­ tIN_LED) > DTBUZ) {    // se tNow ­ tinLED >= DTBUZ 
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); // liga buzz 
        sttPINS = LIGALED;               // prox estado da FSM LEDS 
      } 
      break; 
  } 
}                                      // fim fn_fn_pisca_Leds_Buzzer() 
/* USER CODE END 4 */ 

