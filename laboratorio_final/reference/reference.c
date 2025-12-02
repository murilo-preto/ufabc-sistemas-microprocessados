/* Definições de Protocolo */
char rxBuffer[5]; // O que eu recebo
char txBuffer[5]; // O que eu envio
uint8_t rxByte;   // Byte auxiliar se usar interrupção byte-a-byte

// Flags de Estado
int modo_remoto = 0; // 0=Local, 1=Mostrando Crono do Outro, 2=Mostrando ADC do Outro
uint32_t timer_comunicacao = 0;

// --------------------------------------------------------------------

// DENTRO DO WHILE(1), na parte de leitura de botões:

uint32_t now = HAL_GetTick();

// --- Botão A2: Quero jogar MEU Cronômetro na tela dele ---
if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == 0) { // Botão pressionado (assumindo pull-up/logic 0)
    if (now - timer_comunicacao > 200) {
        // Envia pedido: "Mostre meu CRONOMETRO"
        HAL_UART_Transmit(&huart1, (uint8_t*)"rqCRN", 5, 10);
        timer_comunicacao = now;
    }
}

// --- Botão A3: Quero jogar MEU ADC na tela dele ---
else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == 0) {
    if (now - timer_comunicacao > 200) {
        // Envia pedido: "Mostre meu ADC"
        HAL_UART_Transmit(&huart1, (uint8_t*)"rqADC", 5, 10);
        timer_comunicacao = now;
    }
}

// Se ninguém apertar nada e não estivermos recebendo dados externos, volta ao normal
if (modo_remoto == 0) {
    // ... Aqui fica sua lógica normal de mostrar seus dados no seu display ...
}

// ----------------------------------

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    // --- CASO 1: O colega quer mostrar o CRONÔMETRO dele aqui ---
    if (strncmp(rxBuffer, "rqCRN", 5) == 0) {
        modo_remoto = 1; // Travo meu display para o colega
        // Respondo: "Ok, manda o dado do cronômetro" [cite: 30]
        HAL_UART_Transmit(&huart1, (uint8_t*)"rqcrn", 5, 10);
    }

    // --- CASO 2: O colega quer mostrar o ADC dele aqui ---
    else if (strncmp(rxBuffer, "rqADC", 5) == 0) {
        modo_remoto = 2; // Travo meu display para o colega
        // Respondo: "Ok, manda o dado do ADC" [cite: 31]
        HAL_UART_Transmit(&huart1, (uint8_t*)"rqadc", 5, 10);
    }

    // --- CASO 3: O colega me pediu dados (Recepção do "rqcrn" ou "rqadc") ---
    // Isso acontece depois que EU apertei o botão e ele aceitou.
    else if (strncmp(rxBuffer, "rqcrn", 5) == 0) {
        char msg[5];
        // Formata e envia meu cronômetro: "3584c"
        Formata_Crono_Envio(msg); 
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, 5, 10);
    }
    else if (strncmp(rxBuffer, "rqadc", 5) == 0) {
        char msg[5];
        // Formata e envia meu ADC: "1024a"
        Formata_ADC_Envio(msg);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, 5, 10);
    }

     // --- CASO 4: Recebi DADOS para mostrar (O colega mandou "XXXXc" ou "XXXXa") ---
    else {
        // Verifica o último caractere (token) [cite: 37]
        if (rxBuffer[4] == 'c') {
            modo_remoto = 1; // Garante modo remoto
            Mostrar_Display_Remoto(rxBuffer); // Joga no display
            Piscar_LED(GPIOB, GPIO_PIN_13);   // Pisca LED 3 (PB13) [cite: 25]
        }
        else if (rxBuffer[4] == 'a') {
            modo_remoto = 2; // Garante modo remoto
            Mostrar_Display_Remoto(rxBuffer); // Joga no display
            Piscar_LED(GPIOB, GPIO_PIN_12);   // Pisca LED 4 (PB12) [cite: 25]
        }
    }

    // IMPORTANTE: Reativa a escuta para os próximos 5 bytes
    HAL_UART_Receive_IT(&huart1, (uint8_t*)rxBuffer, 5);
  }
}
/* USER CODE END 4 */