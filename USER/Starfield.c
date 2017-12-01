/**
*  @file   Starfield.c
*  @author Rob Gee
*  @date   November 2017
*  @brief  Starfield simulation - for graphics demo
*/

/***** Include files  *********************************************/
#include <stdlib.h>
#include "stm32f10x.h"
#include "Graphics.h"

/***** Constants  *************************************************/
#define  X_STAR_MAX  NUM_X_PIXELS
#define  Y_STAR_MAX  NUM_Y_PIXELS
#define  NUM_STARS    300
#define  Z_VANISH    10000L    
#define  WARP_SPEED   500    /* pixels/second     */
#define  Z_INC        (WARP_SPEED/FRAME_RATE)

/***** Types      *************************************************/
typedef struct
{  
   int16_t  x;
   int16_t  y;
   int16_t  z;
   uint16_t  x_draw;
   uint16_t  y_draw;
   uint16_t  x_clear;
   uint16_t  y_clear;
   uint8_t   update;
} t_Star;

/***** Storage    *************************************************/
static t_Star Stars[NUM_STARS] = {0};
static uint8_t first_pass = 0;
static uint16_t draw_index = 0;
static uint16_t move_index = 0;
static uint8_t vBlankEvent = 0;

/***** Local prototypes    ****************************************/
static void InitStar(t_Star *p_star);
static void DrawStar(t_Star *p_star);
static void MoveStar(t_Star *p_star, uint16_t z_step);

/***** Exported functions  ****************************************/

/**
*  @fn     StarfieldSim
*  @brief  Simple starfield simulatiuon
*/
void StarfieldSim(void)
{
   int16_t i;

   if(first_pass == 0)
   {
      srand(0xDEADBEEF);
      first_pass = 1;
      for (i = 0; i < NUM_STARS; i++)
      {
         InitStar(&Stars[i]);   
      }
   }
   else if(IsVblankActive() != 0)
   {
      if(vBlankEvent == 0)
      {
         vBlankEvent = 1;
      }
      DrawStar(&Stars[draw_index]);
      if(++draw_index >= NUM_STARS)
         draw_index = 0;
   }
   else
   {
      vBlankEvent = 0;
      MoveStar(&Stars[move_index], Z_INC);
      if(++move_index >= NUM_STARS)
         move_index = 0;
   }
}


/***** Local    functions  ****************************************/
/**
*  @fn     InitStar
*  @param  pointer to star structure
*  @brief  Simple starfield simulatiuon
*/
static void InitStar(t_Star *p_star)
{
   /* Initialise random coordinates */
   uint32_t temp1 = rand() % X_STAR_MAX;
   uint32_t temp2 = rand() % Y_STAR_MAX;
   p_star->x = (int16_t)temp1 - X_STAR_MAX/2;
   p_star->y = (int16_t)temp2 - Y_STAR_MAX/2;
   p_star->z = (int16_t)Z_VANISH;
   p_star->x_draw = (NUM_X_PIXELS/2) - 1;
   p_star->y_draw = (NUM_Y_PIXELS/2) - 1;
   p_star->x_clear = (NUM_X_PIXELS/2) - 1;
   p_star->y_clear = (NUM_Y_PIXELS/2) - 1;
   p_star->update = 1;
}


/**
*  @fn     DrawStar
*  @param  pointer to star structure
*  @brief  Draws star on screen and
*/
static void DrawStar(t_Star *p_star)
{
   if(p_star->update == 1)
   {
      PutPixel(p_star->x_clear, p_star->y_clear, GRAPH_CLEAR);
      PutPixel(p_star->x_draw, p_star->y_draw, GRAPH_SET);
      p_star->x_clear = p_star->x_draw;
      p_star->y_clear = p_star->y_draw;
      p_star->update = 0;
   }
   else if(p_star->update == 2)
   {
      PutPixel(p_star->x_clear, p_star->y_clear, GRAPH_CLEAR);
      InitStar(p_star);
      p_star->update = 0;
   }
}


/**
*  @fn     MoveStar
*  @param  pointer to star structure
*  @brief  Moves star on screen
*/
static void MoveStar(t_Star *p_star, uint16_t z_step)
{
   int32_t x, y;
   
   if (p_star->update != 0)
   {
      return;
   }

   p_star->z -= (int16_t)z_step;
   if (p_star->z < 0)
   {
      p_star->update = 2;
      return;
   }

#ifdef FIRST_HASH   
   if(p_star->x > 0)
   {
      x = ((int32_t)(p_star->x + NUM_X_PIXELS/2)*(int32_t)(Z_VANISH - p_star->z))/Z_VANISH;      
   }
   else
   {
      x = ((int32_t)(p_star->x - NUM_X_PIXELS/2)*(int32_t)(Z_VANISH - p_star->z))/Z_VANISH;      
   }
   
   if(p_star->x > 0)
   {
      y = ((int32_t)(p_star->y + NUM_Y_PIXELS/2)*(int32_t)(Z_VANISH - p_star->z))/Z_VANISH;      
   }
   else
   {
      y = ((int32_t)(p_star->y - NUM_Y_PIXELS/2)*(int32_t)(Z_VANISH - p_star->z))/Z_VANISH;      
   }
#elif defined SECOND_HASH
   x = (2 * p_star->x * (int32_t)(Z_VANISH - p_star->z))/Z_VANISH;
   y = (2 * p_star->y * (int32_t)(Z_VANISH - p_star->z))/Z_VANISH;
#else
   x = (int32_t)(p_star->x * NUM_X_PIXELS)/p_star->z;
   y = (int32_t)(p_star->y * NUM_X_PIXELS)/p_star->z;
#endif

   /* Map coordinates into display window */
   x += NUM_X_PIXELS/2;
   y += NUM_Y_PIXELS/2;

   /* If outside display window then reset star */
   if((x <= 0) || (x >= NUM_X_PIXELS) || (y <= 0) || (y >= NUM_Y_PIXELS))
   {
      /* TODO need to delete this one next time */
      p_star->update = 2;
   }
   else if (((int16_t)x != p_star->x_draw) || ((int16_t)y != p_star->y_draw))
   {
      p_star->x_draw = (int16_t)x;
      p_star->y_draw = (int16_t)y;
      p_star->update = 1;
   }

}
 
