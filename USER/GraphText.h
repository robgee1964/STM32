/**
*  @file   GraphText.h
*  @author Rob Gee
*  @date   November 2017
*  @brief  Include file for Text rendering API
*/

#ifndef __GRAPHTEXT_H 
#define __GRAPHTEXT_H

/* Includes ------------------------------------------------------------------*/
#include "fonts.h"

/* Private define ------------------------------------------------------------*/
#define DISP_ORIENTATION  90  /* angle 0 90 */ 
#define LEFT_RIGHT         0
#define TOP_BOTTOM         1
#define FILL_DIRECTION  LEFT_RIGHT
//#define FILL_DIRECTION  TOP_BOTTOM

#if  ( DISP_ORIENTATION == 90 ) || ( DISP_ORIENTATION == 270 )

#define  MAX_X  320
#define  MAX_Y  240   

#elif  ( DISP_ORIENTATION == 0 ) || ( DISP_ORIENTATION == 180 )

#define  MAX_X  240
#define  MAX_Y  320   

#endif

void SetFont(fontselect_t font);
uint16_t PutChar(uint8_t ASCI, uint8_t action);
uint16_t PutText(uint8_t *str, uint8_t action);
uint16_t PutInt16(uint16_t val, uint8_t action);
uint16_t GetTextLen(uint8_t *str);
uint16_t GetTextHeight(uint8_t *str);


#endif   /* __GRAPHTEXT_H   */





