#include "magic.h"

#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// Random 


uint64_t rnd64(uint64_t n){

    const uint64_t z = 0x9FB21C651E98DF25;

    n ^= ((n << 49) | (n >> 15)) ^ ((n << 24) | (n >> 40));
    n *= z;
    n ^= n >> 35;
    n *= z;
    n ^= n >> 28;

    return n;
}

uint64_t generate_magic_number() {
    return rnd64(rand()) & rnd64(rand()) & rnd64(rand());
}

static int bit_count(BB bb){
    int count = 0;
    while (bb != 0) {
        bb &= bb - 1;
        count++;
    }
    return count;
}

int bit_table[] = {
    63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
    51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
    26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
    58, 20, 37, 17, 36, 8
};

int pop_first_bit(BB *bb, int *bit_table){
    uint64_t b = *bb ^ (*bb - 1);
    uint32_t fold = (uint32_t)((b & 0xffffffff) ^ (b >> 32));
    *bb &= *bb - 1;
    return bit_table[(fold * 0x783a9b23) >> 26];
}

uint64_t generate_occupancy(int index, int bits_in_mask, uint64_t attack_mask, int *bit_table) {
    uint64_t occupancy = 0;
    for (int count = 0; count < bits_in_mask; count++){
        int square = pop_first_bit(&attack_mask, bit_table);
        if ((index & (1 << count)) != 0) {
            occupancy |= (1ULL << square);
        }
    }
    return occupancy;
}

typedef struct {
    BB rook_masks[64];
    BB bishop_masks[64];
} magic_masks_t;

static uint64_t rook_mask(int square) {
    uint64_t mask = 0;
    int rank = square / 8;
    int file = square & 8;

    for (int r = rank + 1; r <= 6; r++) mask |= (1ULL << (file + r * 8));
    for (int r = rank - 1; r >= 1; r--) mask |= (1ULL << (file + r * 8));
    for (int f = file + 1; f <= 6; f++) mask |= (1ULL << (f + rank * 8));
    for (int f = file - 1; f >= 1; f--) mask |= (1ULL << (f + rank * 8));

    return mask;
}

void generate_all_rook_masks(uint64_t *rook_masks) {
    log_message(DEBUG, "MAGIC", "Generating all rook masks");
    for (int square = 0; square < 64; square++){
        rook_masks[square] = rook_mask(square);
    }
}

static uint64_t bishop_mask(int square) {
    uint64_t mask = 0;
    int rank = square / 8;
    int file = square & 8;

    for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++) mask |= (1ULL << (f + r * 8));
    for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--) mask |= (1ULL << (f + r * 8));
    for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++) mask |= (1ULL << (f + r * 8));
    for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--) mask |= (1ULL << (f + r * 8));

    return mask;
}

void generate_all_bishop_masks(uint64_t *bishop_masks) {
    log_message(DEBUG, "MAGIC", "Generating all bishop masks");
    for (int square = 0; square < 64; square++){
        bishop_masks[square] = bishop_mask(square);
    }
}

static BB rook_attack(int square, BB block){
    BB attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;

    for (int r = rank + 1; r <= 7; r++) {
        attacks |= (1ULL << (file + r * 8));
        if (((1ULL << (file + r * 8)) & block) != 0) break;
    }
    for (int r = rank - 1; r >= 0; r--) {
        attacks |= (1ULL << (file + r * 8));
        if (((1ULL << (file + r * 8)) & block) != 0) break;
    }
    for (int f = file + 1; f <= 7; f++) {
        attacks |= (1ULL << (f + rank * 8));
        if (((1ULL << (f + rank * 8)) & block) != 0) break;
    }
    for (int f = file - 1; f >= 0; f--) {
        attacks |= (1ULL << (f + rank * 8));
        if (((1ULL << (f + rank * 8)) & block) != 0) break;
    }
    
    return attacks;
} 
static BB bishop_attack(int square, BB block) {
    BB attacks = 0;
    int rank = square / 8;
    int file = square % 8;

    for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++) {
        attacks |= (1UL << (f + r * 8));
        if (((1ULL << (f + r * 8)) & block) != 0) break;
    }
    for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1UL << (f + r * 8));
        if (((1ULL << (f + r * 8)) & block) != 0) break;
    }
    for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1UL << (f + r * 8));
        if (((1ULL << (f + r * 8)) & block) != 0) break;
    }
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1UL << (f + r * 8));
        if (((1ULL << (f + r * 8)) & block) != 0) break;
    }

    return attacks;
}

static int transform_key(uint64_t key, uint64_t magic, int bits) {
    return (int)((key * magic) >> (64 - bits));
}

void fill_array(uint64_t *array, int array_size, uint64_t fill){
    for (int i = 0; i < array_size; i++){
        array[i] = fill; 
    }
}

magic_entry_t *init_magic_entry(BB mask, BB magic){
    magic_entry_t *magic_entry = malloc(sizeof(magic_entry_t));
    magic_entry->magic = magic;
    magic_entry->mask = mask;
    return magic_entry;
}

void free_magic_entry(magic_entry_t *entry) {
    free(entry);
}

magic_entry_t *find_magic(int square, int relevant_bits, bool is_rook, magic_masks_t *masks) {
    int array_size = 1 << relevant_bits;
    BB *occupancies = calloc(sizeof(BB), array_size);
    BB *attacks = calloc(sizeof(BB), array_size);
    BB *used_attacks = calloc(sizeof(BB), array_size);

    bool is_error = false;
    magic_entry_t *entry = init_magic_entry(0, 0);
    
    BB attack_mask = is_rook ? masks->rook_masks[square] : masks->bishop_masks[square];
    int occupancy_indices = bit_count(attack_mask);

    for(int idx = 0; idx < (1 << occupancy_indices); idx++){
        occupancies[idx] = generate_occupancy(idx, occupancy_indices, attack_mask, bit_table);
        attacks[idx] = is_rook ? rook_attack(square, occupancies[idx]) : bishop_attack(square, occupancies[idx]);
    }

    for (int k = 0; k < 10000; k++){
        BB magic = generate_magic_number();
        if(bit_count((attack_mask  * magic) & 0xFF00000000000000) < 6) continue;

        fill_array(used_attacks, array_size, 0ULL);
    
        bool fail = false;
        for (int idx = 0; idx < (1 << occupancy_indices); idx++){
            int magic_idx = transform_key(occupancies[idx], magic, relevant_bits);
            if (used_attacks[magic_idx] == 0) {
                used_attacks[magic_idx] = attacks[idx];
            } else if (used_attacks[magic_idx] != attacks[idx]) {
                fail = true;
                break;
            }
        }
        if (!fail) {
            entry->magic = magic;
            entry->mask = attack_mask;
            goto clean_up;
        }
    }
    is_error = true;
clean_up:
    if (is_error) {
        logf_message(ERROR, "MAGIC", "Failed to find magic number for square %d", square);
    }

    free(occupancies);
    free(used_attacks);
    free(attacks);

    return entry;
} 
int rook_bits[] = { 
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12 
};

int bishop_bits[] = { 
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6 
};

void init_magic_bitboards() {
    magic_entry_t *rook_magic_entries = malloc(sizeof(magic_entry_t) * 64);
    magic_entry_t *bishop_magic_entries = malloc(sizeof(magic_entry_t) * 64);

    log_message(INFO, "MAGIC", "Starting to generate magic bitboards!");

    magic_masks_t *masks = malloc(sizeof(magic_masks_t));
    generate_all_rook_masks(masks->rook_masks);
    generate_all_bishop_masks(masks->bishop_masks);

    for (int square = 0; square < 64; square++) {
        printf("Finding magic entries for square %d\n", square);    
        rook_magic_entries[square] = *find_magic(square, rook_bits[square], true, masks);
        bishop_magic_entries[square] = *find_magic(square, bishop_bits[square], false, masks);
    }
    free_magic_entry(rook_magic_entries);
    free_magic_entry(bishop_magic_entries);
    free(masks);
}