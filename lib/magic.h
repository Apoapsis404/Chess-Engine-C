#ifndef MAGIC_H
#define MAGIC_H

#include "bitboard.h"

typedef struct {
    BB mask;
    BB magic;
} magic_entry_t;

void free_magic_entry(magic_entry_t *entry);
magic_entry_t find_magic(int square, int relevant_bits, bool is_rook);
void init_magic_bitboards(magic_entry_t *rook_magic_entries, magic_entry_t *bishop_magic_entries);
uint64_t generate_occupancy(int index, int bits_in_mask, uint64_t attack_mask);

extern const int rook_bits[];
extern const int bishop_bits[]; 

uint64_t bishop_mask(int square);
uint64_t rook_mask(int square);
BB rook_attack(int square, BB block);
BB bishop_attack(int square, BB block);




#endif //MAGIC_H;