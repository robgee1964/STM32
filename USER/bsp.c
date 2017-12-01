/**
*  @file   bsp.c
*  @author Rob Gee
*  @date   November 2017
*  @brief  Simple board support functions for Olimex STM32P103
*/

/***** Include files  *********************************************/
#include "stm32f10x.h"
#include "Video.h"
#include "bsp.h"

/***** Constants  *************************************************/
#define BTN_DEBOUNCE_TIME  60UL
#define BTN_CLICK_TIME     300UL
#define BTN_HOLD_TIME      800UL

/***** Types      *************************************************/

/***** Storage    *************************************************/

/***** Local prototypes    ****************************************/

/***** Exported functions  ****************************************/
/**
*  @fn     GPIO_Configuration
*  @brief  GPIO configuration for Olimex STM32P103 board
*/
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); 						 
  /**
  *  LED_STAT -> PC12
  */			
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; ; 
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /**
  *  Game Buttons -> PC0-2
  */			
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; ; 
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Button - PA0       */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; ; 
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_SetBits(GPIOC, GPIO_Pin_12);       /* LED off */

}

/**
*  @fn     ReadButtons
*  @return Button event
*  @brief  Reads user button(s) on Olimex STM32P103 board
*/
t_ButtonEvent ReadButtons(void)
{
   static uint8_t btnPressed = 0;
   static uint16_t btnTimer = 0;
   t_ButtonEvent event = BTN_NONE;

   if(btnPressed == 0)
   {
      if (GPIO_ReadInputDataBit(BTN_PORT, BTN_PIN) != 0)
      {
         btnTimer = 0;
         btnPressed = 1;
      }
   }
   else if(btnPressed == 1)
   {
      if(++btnTimer >= VIDEO_COUNTS(BTN_DEBOUNCE_TIME))
      {
         if(GPIO_ReadInputDataBit(BTN_PORT, BTN_PIN) == 0)
         {
            if(btnTimer < VIDEO_COUNTS(BTN_CLICK_TIME))
            {
               event = BTN_CLICK;  
            }
            btnTimer = 0;
            btnPressed = 0;
         }
         else if(btnTimer > VIDEO_COUNTS(BTN_HOLD_TIME))
         {
            event = BTN_HOLD;
            btnTimer = 0;
         }
      }
   }
   
   return event;
}

#pragma inline
/**
*  @fn     ReadGameButtons
*  @return Button event
*  @brief  Reads user button(s) on Olimex STM32P103 board
*/
uint16_t ReadGameButtons(void)
{
   return GPIO_ReadInputData(GPIOC);   
}

#pragma no_inline


/***** Local    functions  ****************************************/
