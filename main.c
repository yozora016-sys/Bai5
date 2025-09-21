#include "stm32f10x.h"
#include <string.h>

volatile char rxBuffer[32];   
volatile uint8_t rxIndex = 0;
volatile uint8_t commandReceived = 0;

void GPIO_Config(void);
void USART_Config(void);
void USART_SendString(char *str);

int main(void) {
    GPIO_Config();
    USART_Config();

    USART_SendString("Hello from STM32!\r\n");

    while (1) {
        if (commandReceived) {
            commandReceived = 0;

            if (strcmp((char*)rxBuffer, "ON") == 0) {
                GPIO_ResetBits(GPIOC, GPIO_Pin_13);  
                USART_SendString("LED ON\r\n");
            } else if (strcmp((char*)rxBuffer, "OFF") == 0) {
                GPIO_SetBits(GPIOC, GPIO_Pin_13);     
                USART_SendString("LED OFF\r\n");
            } else {
                USART_SendString("Unknown command\r\n");
            }

            rxIndex = 0; // reset buffer
            memset((char*)rxBuffer, 0, sizeof(rxBuffer));
        }
    }
}

/* -------------------- Cau hình GPIO -------------------- */
void GPIO_Config(void) {
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_13;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &gpio);
    GPIO_SetBits(GPIOC, GPIO_Pin_13); 
}

/* -------------------- Cau hình USART1 -------------------- */
void USART_Config(void) {
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // PA9 (TX) - Output AF Push Pull
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // PA10 (RX) - Input floating
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    // USART1 config
    usart.USART_BaudRate = 9600;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &usart);
    USART_Cmd(USART1, ENABLE);

    // Enable RX interrupt
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // NVIC config
    nvic.NVIC_IRQChannel = USART1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

/* -------------------- Gui chuoi UART -------------------- */
void USART_SendString(char *str) {
    while (*str) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *str++);
    }
}

/* -------------------- Ngat USART1 -------------------- */
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        char c = USART_ReceiveData(USART1);
        if (c == '\r' || c == '\n') {
            commandReceived = 1;
        } else {
            if (rxIndex < sizeof(rxBuffer) - 1) {
                rxBuffer[rxIndex++] = c;
            }
        }
    }
}
