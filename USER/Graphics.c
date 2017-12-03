/**
*  @file   Video.c
*  @author Rob Gee
*  @date   November 2017
*  @brief  compositite video signal generator, for STM32
*/

/***** Include files  *********************************************/
#include <stm32f10x.h>
#include <string.h>
#include "bsp.h"
#include "Video.h"
#include "Graphics.h"

/***** Constants  *************************************************/
#define TIMING_TEST

/***** Types      *************************************************/

/***** Storage    *************************************************/
static volatile uint8_t vBlankActive = 0;

static struct
{
   uint8_t *pBuff;    /* pointer to frame buffer  */
   uint16_t x_draw;
   uint16_t y_draw;
   uint8_t  bit_pos;
} GraphCtx;


/***** Local prototypes    ****************************************/
static void VblankCallback(uint8_t event);
static void PutHline(uint16_t x, uint16_t y, uint16_t len, uint8_t action);
static void plot8points(uint16_t cx, uint16_t cy, uint16_t x, uint16_t y, uint16_t Action);
static void plot4points(uint16_t cx, uint16_t cy, uint16_t x, uint16_t y, uint16_t Action);


/***** Exported functions  ****************************************/
/**
*  @fn     graphicsInit
*  @brief  Initialises graphics system
*/
void GraphicsInit(void)
{
   VideoInit();
   setVerticalBlankingCallback(VblankCallback);
}

/**
*  @fn     GraphicsTick
*  @brief  Called every tick for graphical object animation and update
*/
void GraphicsTick(void)
{
   if(vBlankActive != 0)
   {
      /* Carry out FrameBuffer updates */
   }
}


/**
*  @fn        PutPixel
*  @param[IN] x coordinate
*  @param[IN] y coordinate
*  @param[IN] action, 1 = set, 0 = clear
*  @brief     Sets pixel at specified location
*/
void PutPixel(uint16_t x, uint16_t y, uint8_t action)
{
   /* X Byte index and bit mask */
   uint16_t x_index = x >> 3U; 
   uint16_t bit_pos = x & 7;
   uint8_t  mask = (0x80 >> bit_pos);
   
   if (action == 1)
   {
      FrameBuff[y][x_index] |= mask;
   }
   else if (action == 0)
   {
      FrameBuff[y][x_index] &= ~mask;
   }
}


/**
*  @fn        PutLine
*  @param[IN] x coordinate, 1st point
*  @param[IN] y coordinate, 1st point
*  @param[IN] x coordinate, 2nd point
*  @param[IN] y coordinate, 2nd point
*  @param[IN] action, 1 = set, 0 = clear
*  @brief     Draws line between specified points
              x2 >= x1
*/
void PutLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t action)
{
   /* Bressenhans algorithm works by traversing the longest difference on a pixel
    by pixel basis, using a roll over counter to determine when to increment the 
    counter for the short difference
   */
   uint16_t rollcount;
   uint16_t Xstep, Ystep;
   uint16_t Xstart, Ystart;
   int16_t inc = 1;
   uint16_t count;
   
   /* Establish if we are going to have to decrement on rollover */
   if(x2 > x1)
   {
      Xstep = x2 - x1;
   }
   else
   {
      Xstep = x1 - x2;
      inc = 0 - inc;
   }
   if(y2 > y1)
   {
      Ystep = y2 - y1;
   }
   else
   {
      Ystep = y1 - y2;
      inc = 0 - inc;
   }

   if (Xstep > Ystep)
   {
      if (x2 > x1)
         Xstart = x1;
      else
         Xstart = x2;

      if(y2 > y1)
      {
         if(inc == -1)
            Ystart = y2;
         else
            Ystart = y1;
      }
      else
      {
         if(inc == -1)
            Ystart = y1;
         else
            Ystart = y2;
      }
      PutPixel(Xstart, Ystart, action);      // draw the first pixel on its own,

      // X increment larger, so traverse in X direction
      rollcount = Xstep >> 1;
      count = Xstep;
      while(count--)
      {
         rollcount += Ystep;
         if (rollcount >= Xstep)
         {
             rollcount -= Xstep;
             Ystart += inc;
         }
         Xstart++;
         PutPixel(Xstart, Ystart, action);
      }
   }
   else
   {
      if (y2 > y1)
         Ystart = y1;
      else
         Ystart = y2;

      if(x2 > x1)
      {
         if(inc == -1)
            Xstart = x2;
         else
            Xstart = x1;
      }
      else
      {
         if(inc == -1)
            Xstart = x1;
         else
            Xstart = x2;
      }

      rollcount = Ystep >> 1;
      count = Ystep;
      while(count--)
      {
         rollcount += Xstep;
         if (rollcount >= Ystep)
         {
             rollcount -= Ystep;
             Xstart += inc;
         }
         Ystart++;
         PutPixel(Xstart, Ystart, action);
      }
   }
}

/**
*  @fn        PutVline
*  @param[IN] x coordinate
*  @param[IN] y coordinate
*  @param[IN] length of line
*  @param[IN] action, 1 = set, 0 = clear
*  @brief     Draws vertical line starting from specified point
*/
void PutVline(uint16_t x, uint16_t y, uint16_t len, uint8_t action)
{
   /* To do a vertical line just work identify start byte then increment by NUM_X_BYTES each time */
   uint16_t x_index = x >> 3U; 
   uint16_t bit_pos = x & 7;
   uint8_t  mask = (0x80 >> bit_pos);
   uint8_t *pFrameBuff = &FrameBuff[y][x_index];

   if(action == 0)
   {
      mask = ~mask;
   }

   while(len-- > 0)
   {
      if(action == 1)
         *pFrameBuff |= mask;
      else if(action == 0)
         *pFrameBuff &= mask;
      pFrameBuff += NUM_X_BYTES;
   }

}



/**
*  @fn        PutRectangle
*  @param[IN] x coordinate, 1st point
*  @param[IN] y coordinate, 1st point
*  @param[IN] x coordinate, 2nd point
*  @param[IN] y coordinate, 2nd point
*  @param[IN] action, 1 = set, 0 = clear
*  @brief     Draws rectangle between specified points
*/
void PutRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t action)
{
   PutHline(x1, y1, x2-x1+1, action);     /* top            */
   PutVline(x1, y1+1, y2-y1-1, action);   /* left side      */
   PutVline(x2, y1+1, y2-y1-1, action);   /* right side     */
   PutHline(x1, y2,  x2-x1+1, action);    /* bottom         */
}

/**
*  @fn        FillRectangle
*  @param[IN] x coordinate, 1st point
*  @param[IN] y coordinate, 1st point
*  @param[IN] x coordinate, 2nd point
*  @param[IN] y coordinate, 2nd point
*  @param[IN] action, 1 = set, 0 = clear
*  @brief     Fills rectangle between specified points
*/
void FillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t action)
{
   uint16_t count = y2-y1+1;
   uint16_t i;

   for(i = 0; i < count; i++)
   {
      PutHline(x1, y1+i, x2-x1+1, action);  
   }
}

/*****************************************************************************/
/**
*  @fn        PutCircle
*  @param[IN] cx  - centre coordinate
*  @param[IN] cy  - centre coordinate
*  @param[IN] radius
*  @param[IN] action, 1 = set, 0 = clear
*  @brief     Sub-function for circle drawing
*             It is based on Bressenhams circle algorithm, and adapted from 
*             sample code given at http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
*/
void PutCircle(uint16_t cx, uint16_t cy, uint16_t radius, uint16_t action)
{
   int16_t error = 0-radius;
   int16_t x = radius;
   int16_t y = 0;
   
   // The following while loop may altered to 'while (x > y)' for a
   // performance benefit, as long as a call to 'plot4points' follows
   // the body of the loop. This allows for the elimination of the
   // '(x != y') test in 'plot8points', providing a further benefit.
   //
   // For the sake of clarity, this is not shown here.
   while (x >= y)
   {
      plot8points(cx, cy, x, y, action);
      
      error += y;
      ++y;
      error += y;
      
      // The following test may be implemented in assembly language in
      // most machines by testing the carry flag after adding 'y' to
      // the value of 'error' in the previous step, since 'error'
      // nominally has a negative value.
      if (error >= 0)
      {
         --x;
         error -= x;
         error -= x;
      }
   }
}

/**
*  @fn         PutBitmap
*  @param[IN]  pointer to image data
*  @param[IN]  action - selects plot action
*     @arg     0 - clear
*     @arg     1 - set
*  @return     Number of pixels written in X direction
*  @brief      Renders single character
*/
uint8_t PutBitmap(tImage* pImage, uint8_t action)
{
   
   uint16_t n_fb;
   uint16_t n_img;
   uint16_t i_img;
   uint16_t i_fb;
   uint8_t* p_img = (uint8_t*)pImage->bitmap;
    /* Return with pointer at next character location */
   uint8_t* p_finish;
   uint8_t mask;
   uint16_t rows = pImage->height;

   #ifdef TIMING_TEST
   GPIO_ResetBits(LED_PORT, LED_PIN);
   #endif

   /* Work out how many frame buffer bytes we will be writing to */
   n_fb = (GraphCtx.bit_pos + pImage->width +7) >> 3;
   n_img = (pImage->width+7) >> 3;
   p_finish = GraphCtx.pBuff + ((GraphCtx.bit_pos + pImage->width)>>3);

   while(rows--)
   {   
      i_img = 0; i_fb = 0;
      do
      {
         mask = 0;
         if(i_fb != 0)
         {
            mask |= (p_img[i_img] << (8 - GraphCtx.bit_pos));  /* rightmost section of prev image byte */
            i_img++;
         }
 
         if(i_img < n_img)
         {
            mask |= p_img[i_img] >> GraphCtx.bit_pos;       /* leftmost section of image byte */
         }       

         if (action == GRAPH_SET) 
            GraphCtx.pBuff[i_fb] |= mask;
         else if (action == GRAPH_CLEAR)
            GraphCtx.pBuff[i_fb] &= ~mask;
   
         i_fb++;

      }
      while(i_fb < n_fb);
      p_img += n_img;                        /* next row in bit map      */
      GraphCtx.pBuff += (NUM_X_PIXELS/8U);   /* next row in frame buffer */
   }

   GraphCtx.pBuff = p_finish;
   GraphCtx.bit_pos = (GraphCtx.bit_pos + pImage->width) & 0x7;

   #ifdef TIMING_TEST
   GPIO_SetBits(LED_PORT, LED_PIN);
   #endif

   return pImage->width;
}

/**
*  @fn         GotoXY
*  @param[IN]  Xcoordinate
*  @param[IN]  Ycoordinate
*  @brief      Sets working location
*/
void GotoXY(uint16_t Xpos, uint16_t Ypos)
{
   GraphCtx.y_draw = Ypos;
   GraphCtx.x_draw = Xpos;
   GraphCtx.bit_pos = (uint8_t)(Xpos & 0x7);
   GraphCtx.pBuff = &FrameBuff[Ypos][Xpos >> 3];
}



/*****************************************************************************/
/**
*  @fn        ClearScreen
*  @brief     Clear frame buffer
*/
void ClearScreen(void)
{
   memset(FrameBuff, 0, sizeof(FrameBuff));
}


#pragma inline
/*****************************************************************************/
/**
*  @fn        IsVblankActive
*  @brief     Returns vertical blanking status
*/
uint8_t IsVblankActive(void)
{
   return vBlankActive;
}

#pragma no_inline

/***** Local    functions  ****************************************/
/**
*  @fn        VblankCallback
*  @param[IN] 1 for start of blanking interval, 0 for end
*  @brief     Callback function for vertical blanking
               Called at start and end of vertical blanking period
*/
static void VblankCallback(uint8_t event)
{
   vBlankActive = event;
}



/**
*  @fn        PutHline
*  @param[IN] x coordinate
*  @param[IN] y coordinate
*  @param[IN] length of line
*  @param[IN] action, 1 = set, 0 = clear
*  @brief     Draws horizontal line starting from specified point
*/
static void PutHline(uint16_t x, uint16_t y, uint16_t len, uint8_t action)
{
   /* To do a vertical line identify start byte then just keep filling the row */
   uint16_t x_index = x >> 3U; 
   uint16_t bit_pos = x & 7;
   uint8_t  s_mask, e_mask;
   uint8_t *pFrameBuff = &FrameBuff[y][x_index];

   /* write non byte aligned pixels at start of line */
   if(bit_pos > 0)
   {
      s_mask = 0xff >> bit_pos;
      /* Special case for short lines */
      if((bit_pos+len) < 8) 
      {
         s_mask &= 0xff << (8 - (bit_pos+len));
         len = 0;
      }
      else
      {
         len -= (8 - bit_pos);
      }

      if (action == 1)
         *pFrameBuff |= s_mask;
      else if (action == 0)
         *pFrameBuff &= ~s_mask;

      pFrameBuff++;
   }

   while(len >= 8)
   {
      if(action == 1)
         *pFrameBuff = 0xff;
      else if (action == 0)
         *pFrameBuff = 0;

      len -= 8;
      pFrameBuff++;
   }

   /* write non byte aligned pixels at end of line */
   if (len > 0)
   {
//      pFrameBuff++;
      e_mask = 0xff << (8 - len);
      if(action == 1)
         *pFrameBuff |= e_mask;
      else if(action == 0)
         *pFrameBuff &= ~e_mask;     
   }
}


/**
*  @fn        plot8points
*  @param[IN] cx
*  @param[IN] cy
*  @param[IN] x
*  @param[IN] y
*  @param[IN] action, 1 = set, 0 = clear
*  @brief     Sub-function for circle drawing
*             Its is based on Bressenhams circle algorithm, and adapted from 
*             sample code given at http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
*/
static void plot8points(uint16_t cx, uint16_t cy, uint16_t x, uint16_t y, uint16_t action)
{
   plot4points(cx, cy, x, y, action);
   if (x != y) 
      plot4points(cx, cy, y, x, action);
}
 
/**
*  @fn        plot4points
*  @param[IN] cx
*  @param[IN] cy
*  @param[IN] x
*  @param[IN] y
*  @param[IN] action, 1 = set, 0 = clear
*  @brief     Sub-function for circle drawing
*             Its is based on Bressenhams circle algorithm, and adapted from 
*             sample code given at http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
*/
static void plot4points(uint16_t cx, uint16_t cy, uint16_t x, uint16_t y, uint16_t action)
{
   // The '(x != 0 && y != 0)' test in the last line of this function
   // may be omitted for a performance benefit if the radius of the
   // circle is known to be non-zero.
   PutPixel(cx + x, cy + y, action);
   if (x != 0) 
      PutPixel(cx - x, cy + y, action);
   if (y != 0) 
      PutPixel(cx + x, cy - y, action);
   if (x != 0 && y != 0) 
      PutPixel(cx - x, cy - y, action);
}









