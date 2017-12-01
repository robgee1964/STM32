/**
*  @file   Video.h
*  @author Rob Gee
*  @date   November 2017
*  @brief  Include file for compositite video signal generator
*/

#ifndef __VIDEO_H
#define __VIDEO_H

/***** Constants  *************************************************/
#define VIDEO_SYNC_TIMER      TIM1
#define VIDEO_SYNC_CHANNEL    TIM_OC1

#define FRAME_RATE            50U
#define NUM_X_PIXELS          320U
#define NUM_Y_PIXELS          240U
#define NUM_X_BYTES           (NUM_X_PIXELS/8U)

#define VIDEO_COUNTS(ms)      (((ms) * FRAME_RATE)/1000UL)

/***** Types      *************************************************/


/***** Global storage**********************************************/
extern uint8_t FrameBuff[NUM_Y_PIXELS][NUM_X_BYTES];


/***** Exported functions   ***************************************/
void VideoInit(void);
void GraphicsTick(void);
void SetPixel(uint16_t x, uint16_t y, uint8_t action);
void DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t action);
void setVerticalBlankingCallback(void (*pCallback)(uint8_t));

#endif  /*  __VIDEO_H  */


