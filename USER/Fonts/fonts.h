/**
*  @file   fonts.h
*  @author Rob Gee
*  @date   November 2017
*  @brief  Include file for bit mapped fonts
*/

#ifndef FONTS_H
#define FONTS_H

#include "graphics.h"


/*-- Types -------------------------------------------------------------------*/

typedef struct 
{
   long int code;
   const tImage *image;
} tChar;

typedef struct
{
   int length;
   const tChar *chars;
} tFont;

/* Exported types -----------------------------------------------------------*/
typedef enum {MIN_FONT = 0, FIXEDSYS_8_14 = 0, 
             COURIER_NEW8_14, NOKIALARGEX_13, DEJAVUESANS6_10, MAX_FONT} fontselect_t;


#include "FixedSys8_14.h"
//#include "Terminal6_8.h"
#include "CourierNew8_14.h"
#include "NokiaLargex_13.h"
#include "DejaVueSans6x10.h"
//#include "AsciiLib.h"

#endif
