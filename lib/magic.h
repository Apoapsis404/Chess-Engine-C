#ifndef MAGIC_H
#define MAGIC_H

#include "bitboard.h"

typedef struct {
    BB mask;
    BB magic;
} magic_entry_t;

void free_magic_entry(magic_entry_t *entry);
void init_magic_bitboards();



#endif //MAGIC_H;