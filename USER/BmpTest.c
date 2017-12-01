/**
*  @file   BmpTest.c
*  @author Rob Gee
*  @date   November 2017
*  @brief  Bitmap rendering test
*/

/***** Include files  *********************************************/
#include "stm32f10x.h"
#include <stdlib.h>
#include <string.h>
#include "bsp.h"
#include "Graphics.h"
#include "GraphText.h"
#include "BmpTest.h"
#include "Sprites.h"


/***** Constants  *************************************************/
#define MAX_PIXEL_SPEED    200U
#define MIN_PIXEL_SPEED    50U
#define MIN_DIRCHG_TIME    1000U
#define MAX_DIRCHG_TIME    5000U

#define TIMING_TEST

#define ANIMATE_STEP       240U


/***** Types      *************************************************/

/***** Storage    *************************************************/
static const uint8_t* TestStrings[] = {"Will this do?", "Hello World", "Analogue Heaven", "PAL video", "Who needs LCDs?", 
                                       "Yet another pointless project", "Bring back the speccy",
                                       "The answer is 42", "He's dead Jim", "I blame Brexit", 0};                                        
static struct
{
   tImage* pImage; 
   uint16_t x;
   uint16_t y;
   uint16_t strlen;
   uint16_t strhgt;
   int16_t x_inc;
   int16_t y_inc;
   uint16_t testTimer;
   uint8_t string_idx;
   fontselect_t font;
   uint8_t test_str[30];
} TestCtx = {0};                                        

static uint8_t first_text_pass = 0;
static uint8_t first_graph_pass = 0;

/***** Local prototypes    ****************************************/
static void setNextTextTestStep(fontselect_t font);
static void setNextGraphTestStep(void);
static void setTextTestString(fontselect_t font);
static void setObjectSpeed(void);
static void moveText(void);
static void moveGraphic(void);

/***** Exported functions  ****************************************/
/**
*  @fn         TextTest
*  @param[IN]  Button event
*  @brief      Tests out text rendering functions
*/
void TextTest(t_ButtonEvent button_event)
{
   static uint8_t vBlankEvent = 0;
   static uint8_t freeze = 0;
   static enum {TEST_START = 0, TEST_FIXED_SYS = 0, TEST_COURIER, TEST_NOKIA, TEST_END} test_state = TEST_NOKIA;

   first_graph_pass = 0;
   
   if (first_text_pass == 0)
   {
      first_text_pass = 1;
      srand(0xCAFEBABE);
//      setNextTestStep(FIXEDSYS_8_14);
      setNextTextTestStep(NOKIALARGEX_13);
//      test_state = TEST_FIXED_SYS;
      test_state = TEST_NOKIA;
   }
   
   if (IsVblankActive() != 0)
   {
      if (vBlankEvent == 0)
      {
         vBlankEvent = 1;
         if(freeze == 0)
         {
            TestCtx.testTimer++;
            moveText();
            if(button_event == BTN_HOLD)
               freeze = 1;
         }
         else
         {
            if(button_event == BTN_HOLD)
               freeze = 0;
         }
         switch(test_state)
         {
            case TEST_FIXED_SYS:
               if(TestCtx.testTimer >= VIDEO_COUNTS(MAX_DIRCHG_TIME))
               {
                  setNextTextTestStep(COURIER_NEW8_14);
                  test_state = TEST_COURIER;
               }
            break;
            
            case TEST_COURIER:
               if(TestCtx.testTimer >= VIDEO_COUNTS(MAX_DIRCHG_TIME))
               {
                  setNextTextTestStep(NOKIALARGEX_13);
                  test_state = TEST_NOKIA;
               }
            break;
            
            case TEST_NOKIA:
               if(TestCtx.testTimer >= VIDEO_COUNTS(MAX_DIRCHG_TIME))
               {
                  setNextTextTestStep(FIXEDSYS_8_14);
                  test_state = TEST_FIXED_SYS;
 //                 setNextTestStep(NOKIALARGEX_13);
 //                 test_state = TEST_NOKIA;

               }
            break;
          }
       }
   }
   else
   {
      vBlankEvent = 0;
   }
}


/**
*  @fn     GraphTest
*  @param[IN]  Button event
*  @brief  Tests out bitmap rendering functions
*/
void GraphTest(t_ButtonEvent button_event)
{
   static uint8_t vBlankEvent = 0;
   static uint8_t freeze = 0;
   static enum {TEST_START = 0, TEST_INVADER_10 = 0, TEST_END} test_state = TEST_START;

   first_text_pass = 0;
   
   if (first_graph_pass == 0)
   {
      first_graph_pass = 1;
      srand(0xCAFEBABE);
      setObjectSpeed();
      TestCtx.testTimer = 0;
      TestCtx.pImage = (tImage*)&Invader10pt_1;
      test_state = TEST_START;
   }
   
   if (IsVblankActive() != 0)
   {
      if (vBlankEvent == 0)
      {
         vBlankEvent = 1;
         if(freeze == 0)
         {
            TestCtx.testTimer++;
            moveGraphic();
            if(button_event == BTN_HOLD)
               freeze = 1;
         }
         else
         {
            if(button_event == BTN_HOLD)
               freeze = 0;
         }
         switch(test_state)
         {
            case TEST_INVADER_10:
               if(TestCtx.testTimer >= VIDEO_COUNTS(MAX_DIRCHG_TIME))
               {
                  setNextGraphTestStep();
               }
            break;
            
          }
       }
   }
   else
   {
      vBlankEvent = 0;
   }
}


/***** Local functions  *********************************************/

/**
*  @fn     setNextTextTestStep
*  @fn     font
*  @brief  Moves onto next test
*/
static void setNextTextTestStep(fontselect_t font)
{
   setTextTestString(font);
   setObjectSpeed();
   TestCtx.testTimer = 0;
   if(++TestCtx.font >= MAX_FONT)
      TestCtx.font = MIN_FONT;
   if(TestStrings[++TestCtx.string_idx] == 0)
      TestCtx.string_idx = 0;
}

/**
*  @fn     setNextGraphTestStep
*  @fn     font
*  @brief  Moves onto next test
*/
static void setNextGraphTestStep(void)
{
   ClearScreen();
   setObjectSpeed();
   TestCtx.testTimer = 0;
   TestCtx.x = (NUM_X_PIXELS-1 - TestCtx.pImage->width)/2;
   TestCtx.y = (NUM_Y_PIXELS-1 - TestCtx.pImage->height)/2;
   GotoXY(TestCtx.x, TestCtx.y);
   PutBitmap(TestCtx.pImage, GRAPH_SET);
}


/**
*  @fn     setTextTest
*  @param  font to use
*  @param  string to use
*  @brief  Tests out text rendering functions
*/
static void setTextTestString(fontselect_t font)
{
   ClearScreen();
   SetFont(font);
   strcpy((void*)TestCtx.test_str, (void*)TestStrings[TestCtx.string_idx]);
   TestCtx.strlen = GetTextLen(TestCtx.test_str);
   TestCtx.strhgt = GetTextHeight(TestCtx.test_str);
   TestCtx.x = (NUM_X_PIXELS-1 - TestCtx.strlen)/2;
   TestCtx.y = (NUM_Y_PIXELS-1 - TestCtx.strhgt)/2;
   
   GotoXY(TestCtx.x, TestCtx.y);
   PutText(TestCtx.test_str, GRAPH_SET);
}

/**
*  @fn     setObjectSpeed
*  @brief  sets test x and y speeds, randomly
*/
static void setObjectSpeed(void)
{
   uint16_t spdrange = (MAX_PIXEL_SPEED - MIN_PIXEL_SPEED) * 2;

   /* Determine x speed */
   int16_t spd = rand() % spdrange;
   if (spd >= spdrange/2)
      spd = spd/2 + MIN_PIXEL_SPEED;
   else
      spd = -spd/2 - MIN_PIXEL_SPEED;

      TestCtx.x_inc = spd/(int16_t)FRAME_RATE;
   
   /* Determine y speed */
   spd = rand() % spdrange;
   if (spd >= spdrange/2)
      spd = spd/2 + MIN_PIXEL_SPEED;
   else
      spd = -spd/2 - MIN_PIXEL_SPEED;

      TestCtx.y_inc = spd/(int16_t)FRAME_RATE;
   
}


/**
*  @fn     moveText
*  @brief  Moves text according to x and y speeds
*/
static void moveText(void)
{
   /* Erase text first */
   uint16_t x = TestCtx.x;
   uint16_t y = TestCtx.y;
   uint8_t redraw = 0;
   int16_t test_coord;

   /* Work out new coordinates and change direction */
   test_coord = TestCtx.x + TestCtx.x_inc;
   if ((test_coord >= 0) && ((test_coord + TestCtx.strlen -1) < NUM_X_PIXELS))
   {  
      TestCtx.x = test_coord;
      redraw = 1;
   }
   else
   {
      TestCtx.x_inc = -TestCtx.x_inc;
   }
   
   test_coord = TestCtx.y + TestCtx.y_inc;
   if ((test_coord >= 0) && ((test_coord + TestCtx.strhgt -1) < NUM_Y_PIXELS))
   {  
      TestCtx.y = test_coord;
      redraw = 1;
   }
   else
   {
      TestCtx.y_inc = -TestCtx.y_inc;
   }
   
   if (redraw == 1)
   {  
      #ifdef TIMING_TEST
      GPIO_ResetBits(LED_PORT, LED_PIN);
      #endif
      GotoXY(x, y);
      PutText(TestCtx.test_str, GRAPH_CLEAR);
      GotoXY(TestCtx.x, TestCtx.y);
      PutText(TestCtx.test_str, GRAPH_SET);
      #ifdef TIMING_TEST
      GPIO_SetBits(LED_PORT, LED_PIN);
      #endif
   }
}

/**
*  @fn     moveGraphic
*  @brief  Moves Graphic according to x and y speeds
*/
static void moveGraphic(void)
{
   uint16_t x = TestCtx.x;
   uint16_t y = TestCtx.y;
   uint8_t redraw = 0;
   int16_t test_coord;
   static uint8_t anim_timer = 0;

   /* Work out new coordinates and change direction */
   test_coord = TestCtx.x + TestCtx.x_inc;
   if ((test_coord >= 0) && ((test_coord + TestCtx.pImage->width -1) < NUM_X_PIXELS))
   {  
      TestCtx.x = test_coord;
      redraw = 1;
   }
   else
   {
      TestCtx.x_inc = -TestCtx.x_inc;
   }
   
   test_coord = TestCtx.y + TestCtx.y_inc;
   if ((test_coord >= 0) && ((test_coord + TestCtx.pImage->height -1) < NUM_Y_PIXELS))
   {  
      TestCtx.y = test_coord;
      redraw = 1;
   }
   else
   {
      TestCtx.y_inc = -TestCtx.y_inc;
   }
   
   if (redraw == 1)
   {  
      if(++anim_timer >= VIDEO_COUNTS(ANIMATE_STEP))
      {
         anim_timer = 0;
         if(TestCtx.pImage == &Invader10pt_1)
            TestCtx.pImage = (tImage*)&Invader10pt_2;
         else            
            TestCtx.pImage = (tImage*)&Invader10pt_1;
      }
      #ifdef TIMING_TEST
      GPIO_ResetBits(LED_PORT, LED_PIN);
      #endif
      FillRectangle(x, y, x+TestCtx.pImage->width -1, y+TestCtx.pImage->height -1, GRAPH_CLEAR);
      GotoXY(TestCtx.x, TestCtx.y);
      PutBitmap(TestCtx.pImage, GRAPH_SET);
      #ifdef TIMING_TEST
      GPIO_SetBits(LED_PORT, LED_PIN);
      #endif
   }}


