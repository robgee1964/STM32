/**
*  @file   main.c
*  @author Rob Gee
*  @date   November 2017
*  @brief  compositite video signal generator, runs on Olimex STM32P103
*/


/***** Include files  *********************************************/
#include "stm32f10x.h"
#include <stdio.h>
#include "bsp.h"
#include "Graphics.h"
#include "Starfield.h"
#include "BmpTest.h"
#include "Invaders.h"

/***** Constants  *************************************************/
#define TEST_DELAY         3U       /* Units of 20ms  */
#define NUM_LINES          16U
#define X_STEP             (NUM_X_PIXELS/NUM_LINES)
#define Y_STEP             (NUM_Y_PIXELS/NUM_LINES)


/***** Types      *************************************************/

/***** Storage    *************************************************/


/***** Local prototypes    ****************************************/
static void AnimateTestPattern(void);

/***** Exported functions  ****************************************/

/**
*  @fn        main
*  @brief     Main function
*/
int main(void)
{
   static uint8_t TickEvent = 0; 
   static enum {FIRST_TEST = 0, LINE_DRAW = 0, STARFIELD, TEXT_DRAW, GRAPH_OBJ, SPACE_GAME, LAST_TEST} test_state = SPACE_GAME;  
   t_ButtonEvent button_event;
   /* pointer alignment test  */

   SystemInit();
	GPIO_Configuration();
   GraphicsInit();

   #ifdef TEST
   PutRectangle(0,0,319,239,1);
   PutRectangle(10,10,309,229,1);
   PutLine(10,10,309,229,1);
   PutLine(10,229,309,10,1);
   PutLine(10,10,150,229,1);
   PutLine(10,229,150,10,1);

   PutLine(300,220,20,20,1);
   PutLine(300,20,20,220,1);
   PutLine(300,20,150,200,1);
   PutLine(300,220,150,20,1);
   PutCircle(159,119,50,1);
   #endif

   /* Draw initial pattern */   

    /* Infinite loop */
   while (1)
   {
      if(IsVblankActive() == 0)
      {
         if(TickEvent == 0)
         {
            TickEvent = 1;
            if ((button_event = ReadButtons()) == BTN_CLICK)
            {
               ClearScreen();
               if(++test_state == LAST_TEST)
               {
                  test_state = FIRST_TEST;
               }
            }
         }
      }
      else
         TickEvent = 0;

      switch(test_state)
      {
         case LINE_DRAW:
            AnimateTestPattern();
         break;
         
         case STARFIELD:
            StarfieldSim();
         break;
         
         case TEXT_DRAW:
            TextTest(button_event);
         break;  

         case GRAPH_OBJ:
            GraphTest(button_event);
         break;

         case SPACE_GAME:
            InvadersGame(button_event);
         break;
      }
      GraphicsTick();
   }
}



/**
*  @fn     AnimateTestPattern
*  @brief  Simple animated test patttern
*/
static void AnimateTestPattern(void)
{
   static enum {TOP_RIGHT, RIGHT_BOTTOM, BOTTOM_LEFT, LEFT_TOP} displayState = TOP_RIGHT;
   static uint16_t x_draw = X_STEP, y_draw = Y_STEP;
   static uint16_t x_clear = X_STEP;
   static uint16_t y_clear = Y_STEP * (NUM_LINES-1);
   static uint16_t delay = 0;
   static uint8_t vBlankEvent = 0;

   if(IsVblankActive() != 0)
   {
      if(vBlankEvent == 0)
      {
         vBlankEvent = 1;  
         if(++delay >= TEST_DELAY)
         {
            delay = 0;
            switch(displayState)
            {
               case TOP_RIGHT:
                  /* Erase an old line */
                  PutLine(0, y_clear, x_clear, 0, GRAPH_CLEAR);
                  y_clear -= Y_STEP;
                  x_clear += X_STEP;
                  /* Draw a new line  */
                  PutLine(x_draw, 0, NUM_X_PIXELS-1, y_draw, GRAPH_SET);
                  x_draw += X_STEP;
                  y_draw += Y_STEP;
                  /* Check for next state */
                  if (x_draw >= NUM_X_PIXELS)
                  {
                     displayState = RIGHT_BOTTOM;
                     x_clear = X_STEP;
                     y_clear = Y_STEP;
                     x_draw = X_STEP * (NUM_LINES-1);
                     y_draw = Y_STEP;         
                  }
               break;
         
               case RIGHT_BOTTOM:
                  /* Erase an old line */
                  PutLine(x_clear, 0, NUM_X_PIXELS-1, y_clear, GRAPH_CLEAR);
                  y_clear += Y_STEP;
                  x_clear += X_STEP;
                  /* Draw a new line  */
                  PutLine(NUM_X_PIXELS-1, y_draw, x_draw, NUM_Y_PIXELS-1, GRAPH_SET);
                  x_draw -= X_STEP;
                  y_draw += Y_STEP;
                  /* Check for next state */
                  if (x_draw == 0)
                  {
                     displayState = BOTTOM_LEFT;
                     x_clear = X_STEP * (NUM_LINES-1);
                     y_clear = Y_STEP;
                     x_draw = X_STEP * (NUM_LINES-1);
                     y_draw = Y_STEP * (NUM_LINES-1);         
                  }
               break;
         
               case BOTTOM_LEFT:
                  /* Erase an old line */
                  PutLine(NUM_X_PIXELS-1, y_clear, x_clear, NUM_Y_PIXELS-1, GRAPH_CLEAR);
                  y_clear += Y_STEP;
                  x_clear -= X_STEP;
                  /* Draw a new line  */
                  PutLine(x_draw, NUM_Y_PIXELS-1, 0, y_draw, GRAPH_SET);
                  x_draw -= X_STEP;
                  y_draw -= Y_STEP;
                  /* Check for next state */
                  if (x_draw == 0)
                  {
                     displayState = LEFT_TOP;
                     x_clear = X_STEP * (NUM_LINES-1);
                     y_clear = Y_STEP * (NUM_LINES-1);
                     x_draw = X_STEP;
                     y_draw = Y_STEP * (NUM_LINES-1);         
                  }
               break;
         
               case LEFT_TOP:
                  /* Erase an old line */
                  PutLine(x_clear, NUM_Y_PIXELS-1, 0, y_clear, GRAPH_CLEAR);
                  y_clear -= Y_STEP;
                  x_clear -= X_STEP;
                  /* Draw a new line  */
                  PutLine(0, y_draw, x_draw, 0, GRAPH_SET);
                  x_draw += X_STEP;
                  y_draw -= Y_STEP;
                  /* Check for next state */
                  if (x_draw >= NUM_X_PIXELS)
                  {
                     displayState = TOP_RIGHT;
                     x_clear = X_STEP;
                     y_clear = Y_STEP * (NUM_LINES-1);
                     x_draw = X_STEP;
                     y_draw = Y_STEP;         
                  }
               break;
            }
         }          
      }
   }
   else
   {
      vBlankEvent = 0;
   }
}





#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

