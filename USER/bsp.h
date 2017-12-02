/**
*  @file   basp.h
*  @author Rob Gee
*  @date   November 2017
*  @brief  Include file for STM32P103 board support module
*/

#ifndef __BSP_H
#define __BSP_H


/***** Constants  *************************************************/
#define BTN_PORT                 GPIOA
#define BTN_PIN                  GPIO_Pin_0

#define LED_PORT                 GPIOC
#define LED_PIN                  GPIO_Pin_12

#define DEBUG_PORT               GPIOC
#define DEBUG_PIN_1              GPIO_Pin_7
#define DEBUG_PIN_2              GPIO_Pin_8
#define DEBUG_PIN_3              GPIO_Pin_9

#define GAME_BTN_PORT            GPIOC
#define GAME_BTN_1               GPIO_Pin_0
#define GAME_BTN_2               GPIO_Pin_1
#define GAME_BTN_3               GPIO_Pin_2

#define IS_PRESSED(PORT, BTN)    ((~(PORT) & (BTN)) != 0)
#define IS_GAME_BTN_PRESSED(BTN) ((~(BTN) & 0x07) != 0)


/***** Types      *************************************************/
typedef enum {BTN_NONE, BTN_CLICK, BTN_HOLD} t_ButtonEvent;


/***** Exported functions   ***************************************/
void GPIO_Configuration(void);
t_ButtonEvent ReadButtons(void);
uint16_t ReadGameButtons(void);


#endif  /*  __BSP_H  */
