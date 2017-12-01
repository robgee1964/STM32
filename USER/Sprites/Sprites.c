
/*******************************************************************************
* image
* filename: Sprite.c
* name: Image
*
* preset name: Monochrome_TB
* data block size: 8 bit(s), uint8_t
* RLE compression enabled: no
* conversion type: Monochrome, Edge 128
* bits per pixel: 1
*
* preprocess:
*  main scan direction: top_to_bottom
*  line scan direction: forward
*  inverse: no
*******************************************************************************/

/*
 typedef struct {
     const uint8_t *data;
     uint16_t width;
     uint16_t height;
     uint8_t dataSize;
     } tImage;
*/
#include <stdint.h>
#include "Sprites.h"

static const uint8_t image_data_Invader10pt_1[16] = {
    0x0f, 0x00,
    0x7f, 0xe0,
    0xff, 0xf0,
    0xe6, 0x70,
    0xff, 0xf0,
    0x19, 0x80,
    0x36, 0xc0,
    0xc0, 0x30
};
const tImage Invader10pt_1 = { image_data_Invader10pt_1, 12, 8,
    8 };

    static const uint8_t image_data_Invader10pt_2[16] = {
    0x0f, 0x00,
    0x7f, 0xe0,
    0xff, 0xf0,
    0xe6, 0x70,
    0xff, 0xf0,
    0x39, 0xc0,
    0x66, 0x60,
    0x30, 0xc0
};
const tImage Invader10pt_2 = { image_data_Invader10pt_2, 12, 8,
    8 };

static const uint8_t image_data_Invader20pt_1[16] = {
    0x20, 0x80,
    0x11, 0x00,
    0x3f, 0x80,
    0x6e, 0xc0,
    0xff, 0xe0,
    0xbf, 0xa0,
    0xa0, 0xa0,
    0x1b, 0x00
};
const tImage Invader20pt_1 = { image_data_Invader20pt_1, 11, 8,
    8 };

static const uint8_t image_data_Invader20pt_2[16] = {
    0x20, 0x80,
    0x91, 0x20,
    0xbf, 0xa0,
    0xee, 0xe0,
    0xff, 0xe0,
    0x7f, 0xc0,
    0x20, 0x80,
    0x40, 0x40
};
const tImage Invader20pt_2 = { image_data_Invader20pt_2, 11, 8,
    8 };

static const uint8_t image_data_Invader30pt_1[8] = {
    0x18,
    0x3c,
    0x7e,
    0xdb,
    0xff,
    0x5a,
    0x81,
    0x42
};
const tImage Invader30pt_1 = { image_data_Invader30pt_1, 8, 8,
    8 };

static const uint8_t image_data_Invader30pt_2[8] = {
    0x18,
    0x3c,
    0x7e,
    0xdb,
    0xff,
    0x24,
    0x5a,
    0xa5
};
const tImage Invader30pt_2 = { image_data_Invader30pt_2, 8, 8,
    8 };


static const uint8_t image_data_InvaderExplode[16] = {
    0x08, 0x80,
    0x45, 0x10,
    0x20, 0x20,
    0x10, 0x40,
    0xc0, 0x18,
    0x10, 0x40,
    0x25, 0x20,
    0x48, 0x90
};
const tImage InvaderExplode = { image_data_InvaderExplode, 13, 8,
    8 };

static const uint8_t image_data_Laser[16] = {
    0x02, 0x00,
    0x07, 0x00,
    0x07, 0x00,
    0x7f, 0xf0,
    0xff, 0xf8,
    0xff, 0xf8,
    0xff, 0xf8,
    0xff, 0xf8
};
const tImage Laser = { image_data_Laser, 13, 8,
    8 };

static const uint8_t image_data_LaserExplode1[18] = {
    0x00, 0x00,
    0x02, 0x00,
    0x00, 0x10,
    0x02, 0xa0,
    0x12, 0x00,
    0x01, 0xb0,
    0x45, 0xa8,
    0x1f, 0xe4,
    0x3f, 0xf5
};
const tImage LaserExplode1 = { image_data_LaserExplode1, 16, 9,
    8 };

static const uint8_t image_data_LaserExplode2[16] = {
    0x10, 0x04,
    0x82, 0x19,
    0x10, 0xc0,
    0x02, 0x02,
    0x4b, 0x31,
    0x21, 0xc4,
    0x1f, 0xf0,
    0x37, 0xf2
};
const tImage LaserExplode2 = { image_data_LaserExplode2, 16, 8,
    8 };

static const uint8_t image_data_MissileCross[6] = {
    0x40,
    0x40,
    0xe0,
    0x40,
    0x40,
    0x40
};
const tImage MissileCross = { image_data_MissileCross, 3, 6,
    8 };


static const uint8_t image_data_MissileWiggle1[7] = {
    0x20,
    0x40,
    0x80,
    0x40,
    0x20,
    0x40,
    0x80
};
const tImage MissileWiggle1 = { image_data_MissileWiggle1, 3, 7,
    8 };

static const uint8_t image_data_MissileWiggle2[7] = {
    0x80,
    0x40,
    0x20,
    0x40,
    0x80,
    0x40,
    0x20
};
const tImage MissileWiggle2 = { image_data_MissileWiggle2, 3, 7,
    8 };

static const uint8_t image_data_Shelter[48] = {
    0x0f, 0xff, 0xc0,
    0x1f, 0xff, 0xe0,
    0x3f, 0xff, 0xf0,
    0x7f, 0xff, 0xf8,
    0xff, 0xff, 0xfc,
    0xff, 0xff, 0xfc,
    0xff, 0xff, 0xfc,
    0xff, 0xff, 0xfc,
    0xff, 0xff, 0xfc,
    0xff, 0xff, 0xfc,
    0xff, 0xff, 0xfc,
    0xff, 0xff, 0xfc,
    0xfe, 0x03, 0xfc,
    0xfc, 0x01, 0xfc,
    0xf8, 0x00, 0xfc,
    0xf8, 0x00, 0xfc
};
const tImage Shelter = { image_data_Shelter, 22, 16,
    8 };

static const uint8_t image_data_Spaceship[14] = {
    0x07, 0xe0,
    0x1f, 0xf8,
    0x3f, 0xfc,
    0x6d, 0xb6,
    0xff, 0xff,
    0x39, 0x9c,
    0x10, 0x08
};
const tImage Spaceship = { image_data_Spaceship, 16, 7,
    8 };

 


