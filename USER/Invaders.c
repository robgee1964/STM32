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
#define BOTTOM_ROW_Y          (NUM_Y_PIXELS-13U)
#define LASER_WIDTH           13U
#define LASER_STATUS_Y        BOTTOM_ROW_Y
#define LASER_GAP             7U
#define NUM_LASERS            3U
#define FONT_WIDTH            6U
                              
#define MAX_GAME_X            (NUM_X_PIXELS-MARGIN)
#define MIN_GAME_X            (MARGIN)

#define ALIENS_PER_ROW         11U
#define NUM_ALIEN_ROWS         5U


#define LASER_ADD          1
#define LASER_REMOVE       0

#define INITIAL_LASERS     3

/* Animation paramters     */
#define LASER_SPEED           50U        /* 100 pixels per second */
#define MISSILE_SPEED_LASER   100U
#define MISSILE_SPEED_WIGGLE  100U
#define MISSILE_SPEED_CROSS   100U

#define ALIEN_ANIMATION_INTERVAL    240U
#define ALIEN_ANIMATION_INC         VIDEO_COUNTS(ALIEN_ANIMATION_INTERVAL)

#define LASER_INC             VIDEO_COUNTS(LASER_SPEED)
#define MISSILE_INC_LASER     VIDEO_COUNTS(MISSILE_SPEED_LASER)
#define MISSILE_INC_WIGGLE    VIDEO_COUNTS(MISSILE_SPEED_WIGGLE)
#define MISSILE_INC_CROSS     VIDEO_COUNTS(MISSILE_SPEED_CROSS)

#define ALIEN_DELAY_BASE      150U           /* Base delay between alien movements */
#define ALIEN_DELAY_INC       VIDEO_COUNTS(ALIEN_DELAY_BASE)

#define ALIEN_STEP            2              /* Use for left-right motion of aliens */

#define ALIEN_DESTROY_TIME    200
#define ALIEN_DESTROY_COUNTS  VIDEO_COUNTS(ALIEN_DESTROY_TIME)

/* Controls                 */
#define BTN_FIRE           GAME_BTN_1
#define BTN_LEFT           GAME_BTN_3
#define BTN_RIGHT          GAME_BTN_2

/***** Types      *************************************************/
typedef enum {SPRITE_PRESENT = 0, SPRITE_HIT = 1, SPRITE_DESTROYED} tSpriteState;
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
   int16_t  alien_step; 
   uint16_t alien_x;
   uint16_t alien_new_x;
   uint16_t alien_y;
   uint16_t alien_new_y;
   uint8_t  i_alien_bmp;
   uint8_t  alien_step_timer;
   uint8_t  alien_step_interval;
   uint8_t  alien_anim_timer;
   uint8_t  num_lasers;
   uint8_t  level;
   uint8_t  num_living_aliens;
   uint8_t  num_alien_columns;
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
void GameScreenInit(uint16_t level);
void DrawLaserStatus(uint8_t action);
void DrawLaser(uint16_t button_status);
uint8_t DrawAliens(void);
uint8_t AnimateAliens(void);
uint8_t MoveAliens(void);
void DrawShelter(uint8_t n);
void DestroyAlien(uint16_t row, uint16_t column);
void InitialiseObjects(void);

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
               if(alien_redraw == 1)
               {
                  if(DrawAliens() == 1)
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
            if(alien_redraw == 0)
            {
               if(AnimateAliens() == 1)
                  alien_redraw = 1;
                  
               if(++gameCtx.alien_step_timer >= gameCtx.alien_step_interval)
               {
                  gameCtx.alien_step_timer = 0;
                  if(MoveAliens() == 1)
                     game_state = GAME_END;
                  else
                     alien_redraw = 1;
               }
            }
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
void GameScreenInit(uint16_t level)
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
void DrawLaserStatus(uint8_t action)
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
*  @brief      Draws active laser on screen
*/
void DrawLaser(uint16_t button_status)
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
         if((x_new = gameCtx.laser_x + LASER_INC) < (MAX_GAME_X-Laser.width))
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
*  @fn         DrawAliens
*  @return     1 if complete, 0 otherwise
*  @brief      Renders aliens on screen
*              Designed to be called over more than one blanking interval
*/
uint8_t DrawAliens(void)
{
   static int16_t row = NUM_ALIEN_ROWS-1;
   uint8_t count;
   uint8_t complete = 0;
   uint16_t x;
   uint16_t y = gameCtx.alien_y + Aliens[row].y_ofst;
   uint16_t new_x, new_y;
   uint8_t w = Aliens[row].pSprite->width-1;
   uint8_t h = Aliens[row].pSprite->height-1;
   new_y = gameCtx.alien_new_y + Aliens[row].y_ofst;
   
   /* Count up or down depending on direction, if moving right->left then redraw leftmost items first */
   if(gameCtx.alien_new_x < gameCtx.alien_x)
   {
      x = gameCtx.alien_x + Aliens[row].x_ofst;
      new_x = gameCtx.alien_new_x + Aliens[row].x_ofst;
      for(count = 0; count < gameCtx.num_alien_columns; count++)
      {
         /* Erase bitmaps at old position */
         FillRectangle(x, y, x+w, y+h, GRAPH_CLEAR);
         /* draw new bit map */
         GotoXY(new_x, new_y);
         PutBitmap((tImage*)Aliens[row].pSprite, GRAPH_SET);
         new_x += Aliens[row].x_interval;
         x += Aliens[row].x_interval;
      }
   }
   else
   {
      count = gameCtx.num_alien_columns;
      x = Aliens[row].x_ofst + (Aliens[row].x_interval * gameCtx.num_alien_columns);
      new_x = gameCtx.alien_new_x + x;
      x += gameCtx.alien_x;
      do
      {
         count--;
         new_x -= Aliens[row].x_interval;
         x -= Aliens[row].x_interval;
         FillRectangle(x, y, x+w, y+h, GRAPH_CLEAR);
         GotoXY(new_x, new_y);
         PutBitmap((tImage*)Aliens[row].pSprite, GRAPH_SET);
      } while(count > 0);
   }
   
   if(--row < 0)
   {
      complete = 1;
      row = NUM_ALIEN_ROWS-1;
      gameCtx.alien_x = gameCtx.alien_new_x;
      gameCtx.alien_y = gameCtx.alien_new_y;
   }
   
   return complete;
}


/**
*  @fn         AnimateAliens
*  @brief      Carried out bitmap swapping for alien animation
*/
uint8_t AnimateAliens(void)
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
uint8_t MoveAliens(void)
{
   uint16_t x, y;
   uint8_t game_over = 0;
   uint16_t w;
   /* See if we are going to hit the screen edges */
   x = gameCtx.alien_x + gameCtx.alien_step;
   w = (gameCtx.num_alien_columns * (HORIZ_SPACING-1)) - 1;
   if((x >= MIN_GAME_X) && (x + w <= MAX_GAME_X))
   {
      gameCtx.alien_new_x = x;
      gameCtx.alien_new_y = gameCtx.alien_y;
   }
   else
   {
      y = gameCtx.alien_y + Aliens[4].pSprite->height;
      gameCtx.alien_step = 0 - gameCtx.alien_step;
      x = gameCtx.alien_x + gameCtx.alien_step;
      gameCtx.alien_new_x = x;
      if((y+INVADER_Y_OFST(4)) >= (LASER_Y-Aliens[4].pSprite->height))
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
void DrawShelter(uint8_t n)
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
void DestroyAlien(uint16_t row, uint16_t column)
{
   if(Aliens[row].sprite_state[column] == SPRITE_PRESENT)
   {
      Aliens[row].sprite_state[column] = SPRITE_HIT;
      gameCtx.destroyed_per_column[column]++;
      Aliens[row].num_destroyed++;
   }
}


/**
*  @fn         InitialiseObjects
*  @brief      Initialises game variables
*/
void InitialiseObjects(void)
{
   uint16_t n,i;
   
/* Initialise game play variables */
   gameCtx.i_alien_bmp = 0;
   gameCtx.alien_anim_timer = 0;
   gameCtx.level = 0;
   gameCtx.laser_x = (NUM_X_PIXELS-LASER_WIDTH)/2U;
   gameCtx.laser_inc = (NUM_X_PIXELS-LASER_WIDTH)/2U;
   gameCtx.num_living_aliens = ALIENS_PER_ROW * NUM_ALIEN_ROWS;
   gameCtx.alien_step_timer = 0;
   gameCtx.alien_step_interval = (gameCtx.num_living_aliens >> 1) + ALIEN_DELAY_INC;
   gameCtx.num_alien_columns = ALIENS_PER_ROW;
   gameCtx.alien_x = LEFT_OFFSET;
   gameCtx.alien_y = TOP_ROW_OFFSET;
   gameCtx.alien_new_x = LEFT_OFFSET;
   gameCtx.alien_new_y = TOP_ROW_OFFSET;
   gameCtx.alien_step = 0-ALIEN_STEP;
   
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
