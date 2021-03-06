/**
*  @file   GraphText.c
*  @author Rob Gee
*  @date   November 2017
*  @brief  Text rending API for 1 bit depth graphics
*/

/***** Include files  *********************************************/
#include "stm32f10x.h"
#include "Graphics.h"
#include "GraphText.h" 
#include <string.h>
#include <stdlib.h>

/***** Constants  *************************************************/


/***** Types      *************************************************/

/***** Storage    *************************************************/

const tChar* (*GetFontChar)(u8);

// Access function for fonts
/***** Local prototypes    ****************************************/


/***** Exported functions  ****************************************/

/**
*  @fn         SetFont
*  @param[IN]  font selection (fontselect_t)
*  @brief      Simple starfield simulatiuon
*/
void SetFont(fontselect_t font)
{
   switch (font)
   {
      case FIXEDSYS_8_14:
         GetFontChar = &GetFontFixedSys8_14;
      break;

      case NOKIALARGEX_13:
          GetFontChar = &GetFontNokiaLargex_13;
      break;

      case COURIER_NEW8_14:
         GetFontChar = &GetFontCourier_new8_14;
      break;
      
      case DEJAVUESANS6_10:
         GetFontChar = &GetFontDejaVueSans6x10;
      break;

      default:
         GetFontChar = &GetFontFixedSys8_14;
      break;
   }
}

/**
*  @fn         GPutChar
*  @param[IN]  ASCII code
*  @param[IN]  action - selects plot action
*     @arg     0 - clear
*     @arg     1 - set
*     @arg     2 - OR
*  @return     number of pixels written in X direction
*  @brief      Renders single character
*/
uint16_t GPutChar(uint8_t ASCI, uint8_t action)
{
   const tChar* fontch_p;
   fontch_p = GetFontChar(ASCI);
   return PutBitmap((tImage*)fontch_p->image, action);
}

/**
*  @fn         PutText
*  @param[IN]  pointer to string to be printed
*  @param[IN]  action - selects plot action
*     @arg     0 - clear
*     @arg     1 - set
*     @arg     2 - OR
*  @return     Number of pixels written in X direction
*  @brief      Renders single character
*/
uint16_t PutText(uint8_t *str, uint8_t action)
{
   uint16_t x_pixels = 0;
 
   while(*str != 0)
   {
      x_pixels += GPutChar(*str++, action);
   }

   return x_pixels;
}


/**
*  @fn         PutInt16
*  @param[IN]  number to be printed
*  @param[IN]  action - selects plot action
*     @arg     0 - clear
*     @arg     1 - set
*  @brief      Returns pixel width written
*/
uint16_t PutInt16(uint16_t val, uint8_t action)
{
   uint8_t str[6];
   uint16_t i;
   for (i = 0; i < 5; i++)
   {
      str[4-i] = (uint8_t)((val % 10) + '0');
      val /= 10;
   }
   str[5] = 0;
   return PutText(str, action);
}

/**
*  @fn         GetTextLen
*  @param[IN]  pointer to string
*  @brief      Returns length of string in pixels, for currently selected font
*/
uint16_t GetTextLen(uint8_t *str)
{
   uint16_t x_pixels = 0;
   const tChar* fontch_p;
   
   while(*str != 0)
   {
      fontch_p = GetFontChar(*str++);
      x_pixels += fontch_p->image->width;
   }
   
   return x_pixels;
}

/**
*  @fn         GetTextHeight
*  @param[IN]  pointer to string
*  @brief      Returns height of currently selected font
*/
uint16_t GetTextHeight(uint8_t *str)
{
   const tChar* fontch_p = GetFontChar(*str); 
   uint16_t y_pixels = fontch_p->image->height;
   
   return y_pixels;
}






