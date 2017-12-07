/**
*  @file   Invaders.c
*  @author Rob Gee
*  @date   November 2017
*  @brief  Space invaders type game
*/

/***** Include files  *********************************************/
#include "stm32f10x.h"
#include <stdlib.h>
#include <string.h>
#include "bsp.h"
#include "Graphics.h"
#include "GraphText.h"
#include "Sprites.h"
#include "Invaders.h"

/***** Constants  *************************************************/
#define MAX_UINT16            0xffffU
#define MARGIN                40U
#define TOP_ROW_OFFSET        35U
#define HORIZ_SPACING         20U
#define VERT_SPACING          15U
#define LEFT_OFFSET           (MARGIN+20U)
#define INVADER_Y_OFST(row)   ((row)*VERT_SPACING)
#define SHELTER_MARGIN        (MARGIN+30U)
#define SHELTER_WIDTH         22U
#define NUM_SHELTERS          4U
#define SHELTER_SPACING       ((NUM_X_PIXELS - (2 * SHELTER_MARGIN) - SHELTER_WIDTH)/(NUM_SHELTERS-1))
#define SHELTER_Y             (NUM_Y_PIXELS-56U)
#define LASER_HEIGHT          8U
#define LASER_Y               (NUM_Y_PIXELS-28U)
#define LASER_MISSILE_REST_Y  (LASER_Y)
#define BOTTOM_ROW_Y          (NUM_Y_PIXELS-13U)
#define LASER_WIDTH           13U
#define LASER_STATUS_Y        BOTTOM_ROW_Y
#define LASER_GAP             7U
#define NUM_LASERS            3U
#define FONT_WIDTH            6U
#define FONT_HEIGHT           10U
#define LASER_MISILE_LEN      3U

#define MIN_GAME_Y            20U                              
#define MAX_GAME_X            (NUM_X_PIXELS-MARGIN)
#define MIN_GAME_X            (MARGIN)

#define ALIENS_PER_ROW        11U
#define NUM_ALIEN_ROWS        5U
#define MAX_ALIEN_ROW         (NUM_ALIEN_ROWS-1U)
#define MAX_ALIEN_COLUMN      (ALIENS_PER_ROW-1U)


#define LASER_ADD          1
#define LASER_REMOVE       0

#define INITIAL_LASERS     3

/* Animation paramters     */
#define LASER_SPEED           50U        /* 100 pixels per second */
#define MISSILE_SPEED_LASER   250U
#define MISSILE_SPEED_WIGGLE  200U
#define MISSILE_SPEED_CROSS   200U

#define ALIEN_ANIMATION_INTERVAL    240U
#define ALIEN_ANIMATION_INC         VIDEO_COUNTS(ALIEN_ANIMATION_INTERVAL)

#define LASER_INC             (LASER_SPEED/FRAME_RATE)
#define MISSILE_INC_LASER     (MISSILE_SPEED_LASER/FRAME_RATE)
#define MISSILE_INC_WIGGLE    (MISSILE_SPEED_WIGGLE/FRAME_RATE)
#define MISSILE_INC_CROSS     (MISSILE_SPEED_CROSS/FRAME_RATE)

#define ALIEN_DELAY_BASE      60U           /* Base delay between alien movements */
#define ALIEN_DELAY_INC       VIDEO_COUNTS(ALIEN_DELAY_BASE)

#define ALIEN_STEP            2              /* Use for left-right motion of aliens */

#define ALIEN_DESTROY_TIME    300
#define ALIEN_DESTROY_COUNTS  VIDEO_COUNTS(ALIEN_DESTROY_TIME)
#define SAUCER_DESTROY_TIME   500
#define SAUCER_DESTROY_COUNTS VIDEO_COUNTS(SAUCER_DESTROY_TIME)


/* Controls                 */
#define BTN_FIRE           GAME_BTN_1
#define BTN_LEFT           GAME_BTN_3
#define BTN_RIGHT          GAME_BTN_2

#define TIMING_TEST
#define KILL_ALIEN_ROWS


/***** Types      *************************************************/
typedef enum {SPRITE_PRESENT = 0, SPRITE_HIT = 1, ALIEN_DESTROYED=(SPRITE_HIT+ALIEN_DESTROY_COUNTS)} tSpriteState;
typedef enum {FALSE, TRUE} tBool;


typedef struct
{
   const tImage*  pSprite;
   uint16_t       x_ofst;
   uint16_t       y_ofst;
   uint16_t       x_interval;
   uint8_t        num_destroyed;   
   tSpriteState   sprite_state[ALIENS_PER_ROW];    
} tAlienRow;


/***** Storage    *************************************************/
static tAlienRow Aliens[NUM_ALIEN_ROWS] = {
      {&Invader30pt_1, 2, INVADER_Y_OFST(0), HORIZ_SPACING},
      {&Invader20pt_1, 0, INVADER_Y_OFST(1), HORIZ_SPACING},
      {&Invader20pt_1, 0, INVADER_Y_OFST(2), HORIZ_SPACING},
      {&Invader10pt_1, 0, INVADER_Y_OFST(3), HORIZ_SPACING},
      {&Invader10pt_1, 0, INVADER_Y_OFST(4), HORIZ_SPACING}
      };

static u8   *pBmpShelters;

static struct
{
   tImage   shelters[NUM_SHELTERS];
   uint16_t laser_x;
   int16_t  laser_inc;
   uint16_t  laser_missile_y;
   int16_t  alien_step; 
   uint16_t alien_x;
   uint16_t alien_new_x;
   uint16_t alien_y;
   uint16_t alien_new_y;
   uint8_t  num_living_aliens;
   uint8_t  i_alien_bmp;
   uint8_t  alien_step_timer;
   uint8_t  alien_step_interval;
   uint8_t  alien_anim_timer;
   uint8_t  num_lasers;
   uint8_t  level;
   uint8_t  left_alien_column;
   uint8_t  right_alien_column;
   uint8_t  top_alien_row;
   uint8_t  bottom_alien_row;
   uint8_t  destroyed_per_column[ALIENS_PER_ROW];
} gameCtx;

/* Sprite table for aliens animation  */
const tImage* pAlienSprites[][2] = {
      {&Invader30pt_1, &Invader30pt_2},
      {&Invader20pt_1, &Invader20pt_2},
      {&Invader20pt_2, &Invader20pt_1},
      {&Invader10pt_1, &Invader10pt_2},
      {&Invader10pt_2, &Invader10pt_1}};





/***** Local prototypes    ****************************************/
static void GameScreenInit(uint16_t level);
static void DrawLaserStatus(uint8_t action);
static void DrawLaser(uint16_t button_status);
static uint16_t FireLaser(uint16_t button_status);
static tBool CheckLaserHits(uint16_t laser_x);
static uint8_t DrawAliens(void);
static uint8_t AnimateAliens(void);
static uint8_t MoveAliens(void);
static void DrawShelter(uint8_t n);
static void KillAlien(uint16_t row, uint16_t column);
static void DrawDyingAliens(void);
static void InitialiseObjects(void);
static void TestKillAliens(uint16_t button_status);

/***** Exported functions  ****************************************/

/**
*  @fn         InvadersGame
*  @param[IN]  Button event
*  @brief      Space invaders type game
*/
void InvadersGame(t_ButtonEvent button_event)
{
   static uint8_t vBlankEvent = 0;
   static uint8_t alien_redraw = 0;
   static enum {GAME_INIT, GAME_PLAY, GAME_END} game_state = GAME_INIT;
   uint16_t btn_status;

   if (IsVblankActive() != 0)
   {
      if (vBlankEvent == 0)
      {
         /* vertical blanking active - do the screen writes */
         vBlankEvent = 1;   
         btn_status = ReadGameButtons();
         switch(game_state)
         {
            case GAME_INIT:
               InitialiseObjects();
               GameScreenInit(1);
               game_state = GAME_PLAY;
            break;
            
            case GAME_PLAY:
               btn_status = ReadGameButtons();
               DrawLaser(btn_status);
               FireLaser(btn_status);
               if(1)
               {
//                  TestKillAliens(ReadGameButtons());
                  DrawDyingAliens();
               }
               if(alien_redraw == 1)
               {
                  while(DrawAliens() == 0);
                  {
                     alien_redraw = 0;
                  }
               }
            break;
            
            case GAME_END:
            break;   
         }
      }
   }
   else
   {
      if(vBlankEvent == 1)
      {
         vBlankEvent = 0;     
         if(game_state == GAME_PLAY)
         {
            /* Screen being rendered - do non display related tasks such as hit detection */
            if((AnimateAliens() == 1) && (alien_redraw == 0))
            {
               alien_redraw = 1;
            }
            if(1)
            {
               if(gameCtx.alien_step_timer >= gameCtx.alien_step_interval)
               {
                  gameCtx.alien_step_timer = 0;
                  if(MoveAliens() == 1)
                     game_state = GAME_END;
                  else
                     alien_redraw = 1;
               }
            }
            gameCtx.alien_step_timer++;
         }
      }
   }
}

/***** Local    functions  ****************************************/

/**
*  @fn         GameScreenInit
*  @param[IN]  level
*  @brief      Initialises game screen for current level
*/
static void GameScreenInit(uint16_t level)
{
   uint16_t row, count;
   uint16_t x, y;
   for (row = 0; row < NUM_ALIEN_ROWS; row++)
   {
      x = gameCtx.alien_x + Aliens[row].x_ofst;
      y = gameCtx.alien_y + Aliens[row].y_ofst;
      Aliens[row].num_destroyed = 0;
      for(count = 0; count < ALIENS_PER_ROW; count++)
      {
         Aliens[row].sprite_state[count] = SPRITE_PRESENT;
         GotoXY(x, y);
         PutBitmap((tImage*)Aliens[row].pSprite, GRAPH_SET);
         x += Aliens[row].x_interval;
       }
   }
   PutLine(MARGIN, BOTTOM_ROW_Y-2, NUM_X_PIXELS-MARGIN, BOTTOM_ROW_Y-2, GRAPH_SET);
   
   for(count = 0; count < NUM_SHELTERS; count++)
   {
      DrawShelter(count);
   }
   
   for(count = 0; count < NUM_LASERS; count++)
   {
      DrawLaserStatus(LASER_ADD);
   }

   GotoXY(gameCtx.laser_x, LASER_Y);
   PutBitmap((tImage*)&Laser, GRAPH_SET);

   SetFont(DEJAVUESANS6_10);
   GotoXY(MARGIN, 0);
   PutText("SCORE<1>", GRAPH_SET);
   GotoXY((NUM_X_PIXELS-(8*FONT_WIDTH))/2, 0);
   PutText("HI-SCORE", GRAPH_SET);
   GotoXY(NUM_X_PIXELS-MARGIN-(8*FONT_WIDTH), 0);
   PutText("SCORE<2>", GRAPH_SET);
   GotoXY(MARGIN+16, 10);
   PutText("0000", GRAPH_SET);
   GotoXY((NUM_X_PIXELS-(4*FONT_WIDTH))/2, 10);
   PutText("9990", GRAPH_SET);
   GotoXY(NUM_X_PIXELS-MARGIN-(9*FONT_WIDTH), BOTTOM_ROW_Y);
   PutText("CREDIT 03", GRAPH_SET);
}

/**
*  @fn         DrawLaserStatus
*  @param[IN]  action - add or remove
*  @brief      Draws number of lasers remaining
*/
static void DrawLaserStatus(uint8_t action)
{
   int16_t x;
   if (action == LASER_REMOVE)
   {
      gameCtx.num_lasers--;
      x = MARGIN + ((Laser.width + LASER_GAP)* gameCtx.num_lasers);
      FillRectangle(x, LASER_STATUS_Y, x + Laser.width -1, LASER_STATUS_Y + Laser.height -1, GRAPH_CLEAR);
   }
   else if(action == LASER_ADD)
   {
      x = MARGIN + ((Laser.width + LASER_GAP)* gameCtx.num_lasers);
      GotoXY(x, LASER_STATUS_Y);
      PutBitmap((tImage*)&Laser, GRAPH_SET);
      gameCtx.num_lasers++;
   }
}


/**
*  @fn         DrawLaser
*  @param[IN]  button status
*  @brief      Draws active laser on screen
*/
static void DrawLaser(uint16_t button_status)
{
   int16_t x_new;
   uint8_t redraw = 0;
   if(IS_GAME_BTN_PRESSED(button_status))
   {
      if(IS_PRESSED(button_status, BTN_LEFT))
      {
         if((x_new = gameCtx.laser_x - LASER_INC) >= MIN_GAME_X)
            redraw = 1;
      }
      else if(IS_PRESSED(button_status, BTN_RIGHT))
      {
         if((x_new = gameCtx.laser_x + LASER_INC) <= (MAX_GAME_X-Laser.width+1))
            redraw = 1;
      }
      if(redraw == 1)
      {
         FillRectangle(gameCtx.laser_x, LASER_Y, gameCtx.laser_x+Laser.width-1, LASER_Y+Laser.height-1, GRAPH_CLEAR);
         gameCtx.laser_x = (uint16_t)x_new;
         GotoXY(gameCtx.laser_x, LASER_Y);
         PutBitmap((tImage*)&Laser, GRAPH_SET);
      }
   }
}

/**
*  @fn         FireLaser
*  @param[IN]  button status
*  @return     laser x position or 0xffff if laser inactive
*  @brief      Starts laser missile on screen
*/
static uint16_t FireLaser(uint16_t button_status)
{
   static uint16_t button_status_prev = 0x07;
   static uint16_t missile_x;
   tBool hit;

   /* Animate missile  */
   if(gameCtx.laser_missile_y != LASER_MISSILE_REST_Y)
   {
      PutVline(missile_x, gameCtx.laser_missile_y, LASER_MISILE_LEN, GRAPH_CLEAR);
      
      if((hit = CheckLaserHits(missile_x)) == FALSE)
         gameCtx.laser_missile_y-=MISSILE_INC_LASER;

      if((gameCtx.laser_missile_y <= MIN_GAME_Y) || (hit == TRUE))
      {
         gameCtx.laser_missile_y = LASER_MISSILE_REST_Y;
      }
      else
         PutVline(missile_x, gameCtx.laser_missile_y, LASER_MISILE_LEN, GRAPH_SET);

   }

   if((IS_PRESSED(button_status, BTN_FIRE)) && (!IS_PRESSED(button_status_prev, BTN_FIRE)))
   {
      if(gameCtx.laser_missile_y == LASER_MISSILE_REST_Y)
      {
         /* Start laser missile */
         missile_x = gameCtx.laser_x + (Laser.width>>1);
         gameCtx.laser_missile_y -= LASER_MISILE_LEN;
         PutVline(missile_x, gameCtx.laser_missile_y, LASER_MISILE_LEN, GRAPH_SET);
         CheckLaserHits(MAX_UINT16);
      }
   }
   button_status_prev = button_status;
   if(gameCtx.laser_missile_y != LASER_MISSILE_REST_Y)
      return missile_x;
   else
      return MAX_UINT16;
}

/**
*  @fn         CheckLaserHits
*  @param[IN]  laser missile x coordinate, passing in 0xffff resets
*  @return     TRUE if laser missile hit alien
*  @brief      Checks if missile hit alien
*/
static tBool CheckLaserHits(uint16_t laser_x)
{
   uint16_t x_left, x_right;
   uint16_t y_hit_bottom, y_hit_top;
   uint16_t column;
   static uint16_t row = MAX_ALIEN_ROW;
   static uint16_t row_prev = 0;
   tBool hit = FALSE;
   
   if(laser_x == MAX_UINT16)
      row = gameCtx.bottom_alien_row;
   else
   {
      /* Work out lowest y coordinate of aliens array */
      y_hit_top = gameCtx.alien_y + Aliens[row].y_ofst - (LASER_MISILE_LEN-1);
      y_hit_bottom = y_hit_top + (Aliens[row].pSprite->height-1) + (LASER_MISILE_LEN - 1);
      if(gameCtx.laser_missile_y <= y_hit_bottom)
      {
         x_left = gameCtx.alien_x + Aliens[row].x_ofst;
         for(column = gameCtx.left_alien_column; column <= gameCtx.right_alien_column; column++)
         {
            if(Aliens[row].sprite_state[column] == SPRITE_PRESENT)
            {
               x_right = x_left + Aliens[row].pSprite->width-1;
               if((laser_x >= x_left) && (laser_x <= x_right))
               {
                  hit = TRUE;
                  KillAlien(row, column);
                  row = gameCtx.bottom_alien_row;
                  break;
               }
            }
            x_left += HORIZ_SPACING;
         }
         if((hit == FALSE) && (gameCtx.laser_missile_y < y_hit_top))
         {
            if(row > gameCtx.top_alien_row)
               row--;
         }
      }
   }
   if(row_prev != row)
   {
      FillRectangle((NUM_X_PIXELS/2 - ((FONT_WIDTH*5)/2)), BOTTOM_ROW_Y, (NUM_X_PIXELS/2 + 1 + ((FONT_WIDTH*5)/2)), 
               BOTTOM_ROW_Y+FONT_HEIGHT/2, GRAPH_CLEAR);
      GotoXY((NUM_X_PIXELS/2 - ((FONT_WIDTH*5)/2)), BOTTOM_ROW_Y);
      PutInt16(row, GRAPH_SET);
   }
   row_prev = row;
   return hit;
}


/**
*  @fn         DrawAliens
*  @return     1 if complete, 0 otherwise
*  @brief      Renders aliens on screen
*              Designed to be called over more than one blanking interval
*/
static uint8_t DrawAliens(void)
{
   static int8_t row = MAX_ALIEN_ROW;
   uint8_t count;
   uint8_t complete = 0;
   uint16_t x;
   uint16_t y = gameCtx.alien_y + Aliens[row].y_ofst;
   uint16_t new_x, new_y;
   uint8_t w = Aliens[row].pSprite->width-1;
   uint8_t h = Aliens[row].pSprite->height-1;
   new_y = gameCtx.alien_new_y + Aliens[row].y_ofst;

   #ifdef TIMING_TEST
   GPIO_ResetBits(DEBUG_PORT, DEBUG_PIN_1);
   #endif
   
   x = gameCtx.alien_x + Aliens[row].x_ofst;
   new_x = gameCtx.alien_new_x + Aliens[row].x_ofst;
   for(count = gameCtx.left_alien_column; count <= gameCtx.right_alien_column; count++)
   {
      if(Aliens[row].sprite_state[count] == SPRITE_PRESENT)
      {
         /* Erase bitmaps at old position */
         FillRectangle(x, y, x+w, y+h, GRAPH_CLEAR);
         /* draw new bit map */
         GotoXY(new_x, new_y);
         PutBitmap((tImage*)Aliens[row].pSprite, GRAPH_SET);
      }
      else if(Aliens[row].sprite_state[count] < ALIEN_DESTROYED)
      {
         /* Erase bitmaps at old position */
         FillRectangle(x-Aliens[row].x_ofst, y, 
                     x+InvaderExplode.width-Aliens[row].x_ofst-1, 
                       y+InvaderExplode.height-1, GRAPH_CLEAR);
         /* draw new bit map */
         GotoXY(new_x-Aliens[row].x_ofst, new_y);
         PutBitmap((tImage*)&InvaderExplode, GRAPH_SET);
      }
      new_x += Aliens[row].x_interval;
      x += Aliens[row].x_interval;
   }
   
   if(--row < gameCtx.top_alien_row)
   {
      complete = 1;
      row = gameCtx.bottom_alien_row;
      gameCtx.alien_x = gameCtx.alien_new_x;
      gameCtx.alien_y = gameCtx.alien_new_y;
   }
   #ifdef TIMING_TEST
   GPIO_SetBits(DEBUG_PORT, DEBUG_PIN_1);
   #endif
   
   return complete;
}


/**
*  @fn         AnimateAliens
*  @brief      Carried out bitmap swapping for alien animation
*/
static uint8_t AnimateAliens(void)
{
   uint8_t alien_redraw = 0;
   uint16_t count;
   
   if(++gameCtx.alien_anim_timer >= ALIEN_ANIMATION_INC)
   {
      gameCtx.alien_anim_timer = 0;
      if(gameCtx.i_alien_bmp == 0)
      {
         gameCtx.i_alien_bmp = 1;
         alien_redraw = 1;
      }
      else
      {
         gameCtx.i_alien_bmp = 0;
         alien_redraw = 1;
      }
      for(count = 0 ; count < NUM_ALIEN_ROWS; count++)
      {  
         Aliens[count].pSprite = pAlienSprites[count][gameCtx.i_alien_bmp];
      }
   }
   
   return alien_redraw;
}


/**
*  @fn         MoveAliens
*  @return     1 if game over
*  @brief      Calculates new position for aliens
*/
static uint8_t MoveAliens(void)
{
   uint16_t x, y;
   uint8_t game_over = 0;
   uint16_t w;
   uint16_t column, row, alien_col_left, alien_col_right;
   tBool finding_left = TRUE;
   tBool finding_top = TRUE;
   uint16_t bottom_alien_row;
   uint16_t top_alien_row;

   alien_col_left = gameCtx.left_alien_column;
   alien_col_right = gameCtx.right_alien_column;
  
   /* Check if left most or rightmost columns are empty */
   for(column = gameCtx.left_alien_column; column <= gameCtx.right_alien_column; column++)
   {
      if(finding_left == TRUE)
      {
         if(gameCtx.destroyed_per_column[column] < NUM_ALIEN_ROWS)
            finding_left = FALSE;
         else
         {
            alien_col_left++;
         }
      }
      if(finding_left == FALSE)
      {
         if(gameCtx.destroyed_per_column[alien_col_right] < NUM_ALIEN_ROWS)
         {
            gameCtx.right_alien_column = alien_col_right;
            break;
         }
         else
            alien_col_right--;
      }
   }
   gameCtx.alien_x += (HORIZ_SPACING*(alien_col_left - gameCtx.left_alien_column));
   gameCtx.left_alien_column = alien_col_left;

   
   /* See if any rows have been emptied */
   bottom_alien_row = gameCtx.bottom_alien_row;
   top_alien_row = gameCtx.top_alien_row;
   for(row = gameCtx.top_alien_row; row <= gameCtx.bottom_alien_row; row++)
   {
      if(finding_top == TRUE)
      {
         if(Aliens[row].num_destroyed < ALIENS_PER_ROW)
            finding_top = FALSE;
         else
            top_alien_row++;
      }
      if(finding_top == FALSE)
      {
         if(Aliens[bottom_alien_row].num_destroyed < ALIENS_PER_ROW)
         {
            gameCtx.bottom_alien_row = bottom_alien_row;
            break;
         }
         else
            bottom_alien_row--;
      }
   }
   gameCtx.top_alien_row = top_alien_row;
   
#if(0)   
   row = NUM_ALIEN_ROWS-1;
   do
   {
      if(finding_bottom == TRUE)
      {
         if(Aliens[row].num_destroyed < ALIENS_PER_ROW)
            finding_bottom = FALSE;
         else
         {
            bottom_alien_row--;
         }
      }
      else if(finding_bottom == FALSE)
      {
         if(Aliens[top_alien_row].num_destroyed < ALIENS_PER_ROW)
         {
            gameCtx.top_alien_row = top_alien_row;
            break;
         }
         else
            top_alien_row++;
      }
   }
   while(row-- > gameCtx.top_alien_row);
   gameCtx.bottom_alien_row = bottom_alien_row;
#endif   
   
   /* See if we are going to hit the screen edges */
   x = gameCtx.alien_x + gameCtx.alien_step;
   w = ((alien_col_right-alien_col_left) * (HORIZ_SPACING)) + 
                     Aliens[bottom_alien_row].pSprite->width-1;
   if((x >= MIN_GAME_X) && (x + w <= MAX_GAME_X))
   {
      gameCtx.alien_new_x = x;
      gameCtx.alien_new_y = gameCtx.alien_y;
   }
   else
   {
      y = gameCtx.alien_y + Aliens[bottom_alien_row].pSprite->height;
      gameCtx.alien_step = 0 - gameCtx.alien_step;
      x = gameCtx.alien_x + gameCtx.alien_step;
      gameCtx.alien_new_x = x;
      if((y+INVADER_Y_OFST(bottom_alien_row)) >= (LASER_Y-Aliens[bottom_alien_row].pSprite->height))
      {
         game_over = 1;
      }
      else
      {
         gameCtx.alien_new_y = y;
      }
   }
   
   return game_over;
}



/**
*  @fn         DrawLaser
*  @param[IN]  Shelter to draw/update
*  @brief      Draws active laser on screen
*/
static void DrawShelter(uint8_t n)
{
   uint16_t x = SHELTER_MARGIN + (n * SHELTER_SPACING);
   FillRectangle(x, SHELTER_Y, x+Shelter.width-1, SHELTER_Y+Shelter.height-1, GRAPH_CLEAR);
   GotoXY(x, SHELTER_Y);
   PutBitmap(&gameCtx.shelters[n], GRAPH_SET);
}

/**
*  @fn         DrawLaser
*  @param[IN]  row
*  @param[IN]  column
*  @brief      Draws active laser on screen
*/
static void KillAlien(uint16_t row, uint16_t column)
{
   if(Aliens[row].sprite_state[column] == SPRITE_PRESENT)
   {
      Aliens[row].sprite_state[column] = SPRITE_HIT;
   }
}


/**
*  @fn         DrawDyingAliens
*  @brief      Draws active laser on screen
*/
static void DrawDyingAliens(void)
{
   uint16_t x, y, row, column;
   
   y = gameCtx.alien_y + (gameCtx.top_alien_row * VERT_SPACING);
   for(row = gameCtx.top_alien_row; row <= gameCtx.bottom_alien_row; row++)
   {
      x = gameCtx.alien_x;
      for(column = gameCtx.left_alien_column; column <= gameCtx.right_alien_column; column++)
      {
         switch(Aliens[row].sprite_state[column])
         {
            case SPRITE_PRESENT:
            case ALIEN_DESTROYED:
            break;
            
            case SPRITE_HIT:
               /* draw exploding sprite */
               FillRectangle(x, y, x+InvaderExplode.width-1, y+InvaderExplode.height-1, GRAPH_CLEAR);
               GotoXY(x, y);
               PutBitmap((tImage*)&InvaderExplode, GRAPH_SET);
               Aliens[row].sprite_state[column]++;
            break;
            
            default:
               if(++Aliens[row].sprite_state[column] >= ALIEN_DESTROYED)
               {
                  FillRectangle(x, y, x+InvaderExplode.width-1, y+InvaderExplode.height-1, GRAPH_CLEAR);
                  gameCtx.destroyed_per_column[column]++;
                  Aliens[row].num_destroyed++;
                  gameCtx.num_living_aliens--;
                  gameCtx.alien_step_interval = (gameCtx.num_living_aliens/2) + ALIEN_DELAY_INC;
               }
            break;
         }
         x += HORIZ_SPACING;
      }
      y += VERT_SPACING;
   }
}


/**
*  @fn         InitialiseObjects
*  @brief      Initialises game variables
*/
static void InitialiseObjects(void)
{
   uint16_t n,i;
   
/* Initialise game play variables */
   gameCtx.i_alien_bmp = 0;
   gameCtx.alien_anim_timer = 0;
   gameCtx.level = 0;
   gameCtx.laser_x = (NUM_X_PIXELS-LASER_WIDTH)/2U;
   gameCtx.laser_inc = (NUM_X_PIXELS-LASER_WIDTH)/2U;
   gameCtx.laser_missile_y = LASER_MISSILE_REST_Y;
   gameCtx.num_living_aliens = ALIENS_PER_ROW * NUM_ALIEN_ROWS;
   gameCtx.alien_step_timer = 0;
   gameCtx.alien_step_interval = (gameCtx.num_living_aliens/2) + ALIEN_DELAY_INC;
   gameCtx.alien_x = LEFT_OFFSET;
   gameCtx.alien_y = TOP_ROW_OFFSET;
   gameCtx.alien_new_x = LEFT_OFFSET;
   gameCtx.alien_new_y = TOP_ROW_OFFSET;
   gameCtx.alien_step = 0-ALIEN_STEP;
   gameCtx.top_alien_row = 0;
   gameCtx.bottom_alien_row = MAX_ALIEN_ROW;
   gameCtx.right_alien_column = MAX_ALIEN_COLUMN;
   gameCtx.left_alien_column = 0;
   
   for (i = 0; i < ALIENS_PER_ROW; i++) 
   {
      gameCtx.destroyed_per_column[i] = 0;
   }
   
/* Initialise shelter sprites in RAM */  
   n = ((Shelter.width+7)>>3) * (Shelter.height);
   pBmpShelters = malloc(n * NUM_SHELTERS);
 
   for (i = 0; i < NUM_SHELTERS; i++)
   {
      gameCtx.shelters[i].width = Shelter.width;
      gameCtx.shelters[i].height = Shelter.height;
      gameCtx.shelters[i].bitmap = pBmpShelters+(i * n);
      memcpy((void*)gameCtx.shelters[i].bitmap, (void*)Shelter.bitmap, n);
   }
   
}


/**
*  @fn         InitialiseObjects
*  @param[IN]  button_status
*  @brief      Initialises game variables
*/
static void TestKillAliens(uint16_t button_status)
{
   static uint16_t button_status_prev;
   #ifdef KILL_RANDOM_ALIENS
   uint16_t row;
   uint16_t alien_number, column;
   #elif defined KILL_ALIEN_COLUMNS
   uint16_t row;
   static uint16_t kill_count = 0;
   static uint8_t kill_col = 0;
   #elif defined KILL_ALIEN_ROWS
   static uint16_t kill_count = 0;
   static uint8_t kill_row = 0;
   uint16_t column;
   #endif
   
   if(gameCtx.num_living_aliens > 0)
   {
      if((IS_PRESSED(button_status, BTN_FIRE)) && !(IS_PRESSED(button_status_prev, BTN_FIRE)))
      {
         #ifdef KILL_RANDOM_ALIENS
         while(1)
         {
            alien_number = rand() % (ALIENS_PER_ROW*NUM_ALIEN_ROWS);
            row = alien_number % NUM_ALIEN_ROWS;
            column = alien_number % ALIENS_PER_ROW;
            if(Aliens[row].sprite_state[column] == SPRITE_PRESENT)
            {
               KillAlien(row, column);
               break;
            }
         }
         #elif defined KILL_ALIEN_COLUMNS
         for(row = 0; row < NUM_ALIEN_ROWS; row++)
         {
            if(Aliens[row].sprite_state[kill_col] == SPRITE_PRESENT)
            {
               KillAlien(row, kill_col);
               if(++kill_count >= NUM_ALIEN_ROWS)
               {
                  kill_count = 0;
                  if(kill_col == gameCtx.left_alien_column)
                     kill_col = gameCtx.right_alien_column;
                  else
                     kill_col = gameCtx.left_alien_column;
               }
               break;
            }
         }
         #elif defined KILL_ALIEN_ROWS
         for(column = 0; column < ALIENS_PER_ROW; column++)
         {
            if(Aliens[kill_row].sprite_state[column] == SPRITE_PRESENT)
            {
               KillAlien(kill_row, column);
               if(++kill_count >= ALIENS_PER_ROW)
               {
                  kill_count = 0;
                  if(kill_row == gameCtx.top_alien_row)
                     kill_row = gameCtx.bottom_alien_row;
                  else
                     kill_row = gameCtx.top_alien_row;
               }
            }
         }
         #endif
      }
   }
   
   button_status_prev = button_status;
}
