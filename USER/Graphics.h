/**
*  @file   Graphics.h
*  @author Rob Gee
*  @date   November 2017
*  @brief  Include file for graphics functions
*/

#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include "Display.h"

/***** Constants  *************************************************/
#define GRAPH_SET    1U
#define GRAPH_CLEAR  0U
#define GRAPH_OR     2U

/***** Types      *************************************************/
typedef struct
{
   const uint8_t* bitmap; 
   uint8_t  width;
   uint8_t  height;
   uint8_t  datasize;
} tImage;


/***** Exported functions   ***************************************/
void GraphicsInit(void);
void GraphicsTick(void);
void PutPixel(uint16_t x, uint16_t y, uint8_t action);
void PutLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t action);
void PutVline(uint16_t x, uint16_t y, uint16_t len, uint8_t action);
void PutRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t action);
void FillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t action);
void PutCircle(uint16_t cx, uint16_t cy, uint16_t radius, uint16_t action);
uint8_t PutBitmap(tImage* pImage, uint8_t action);
void GotoXY(uint16_t Xpos, uint16_t Ypos);
void ClearScreen(void);
uint8_t IsVblankActive(void);

#endif  /*  __GRAPHICS_H  */


