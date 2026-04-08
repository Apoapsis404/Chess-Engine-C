#ifndef SPRITES_H
#define SPRITES_H

#include <raylib.h>



typedef struct sprite_map {
    Texture2D sprites;
    Rectangle *sprite_corners;
} sprite_map;

#endif //SPRITES_H;
