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
    while (bb > 0) {
        bb &= (bb - 1);
        count++;
    }
    return count;
}

int count_1s(BB b) {
  int r;
  for(r = 0; b; r++, b &= b - 1);
  return r;
}

const int bit_table[] = {
    63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
    51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
    26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
    58, 20, 37, 17, 36, 8
};

int pop_first_bit(BB *bb){
    uint64_t b = *bb ^ (*bb - 1);
    uint32_t fold = (uint32_t)((b & 0xffffffff) ^ (b >> 32));
    *bb &= (*bb - 1);
    return bit_table[(fold * 0x783a9b23) >> 26];
}

uint64_t generate_occupancy(int index, int bits_in_mask, uint64_t attack_mask) {
    int count, square;
    uint64_t occupancy = 0ULL;
    for (count = 0; count < bits_in_mask; count++){
        square = pop_first_bit(&attack_mask);
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

uint64_t rook_mask(int square) {
    uint64_t mask = 0;
    int rank = square / 8;
    int file = square % 8;

    int r, f;

    for (r = rank + 1; r <= 6; r++) mask |= (1ULL << (file + r * 8));
    for (r = rank - 1; r >= 1; r--) mask |= (1ULL << (file + r * 8));
    for (f = file + 1; f <= 6; f++) mask |= (1ULL << (f + rank * 8));
    for (f = file - 1; f >= 1; f--) mask |= (1ULL << (f + rank * 8));

    return mask;
}

void generate_all_rook_masks(uint64_t *rook_masks) {
    log_message(DEBUG, "MAGIC", "Generating all rook masks");
    for (int square = 0; square < 64; square++){
        rook_masks[square] = rook_mask(square);
    }
}

uint64_t bishop_mask(int square) {
    uint64_t mask = 0ULL;
    int rank = square / 8;
    int file = square % 8;
    int r, f;

    for (r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++) mask |= (1ULL << (f + r * 8));
    for (r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--) mask |= (1ULL << (f + r * 8));
    for (r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++) mask |= (1ULL << (f + r * 8));
    for (r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--) mask |= (1ULL << (f + r * 8));

    return mask;
}

void generate_all_bishop_masks(uint64_t *bishop_masks) {
    log_message(DEBUG, "MAGIC", "Generating all bishop masks");
    for (int square = 0; square < 64; square++){
        bishop_masks[square] = bishop_mask(square);
    }
}

BB rook_attack(int square, BB block){
    BB attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;
    int r, f;

    for (r = rank + 1; r <= 7; r++) {
        attacks |= (1ULL << (file + r * 8));
        if (((1ULL << (file + r * 8)) & block) != 0) break;
    }
    for (r = rank - 1; r >= 0; r--) {
        attacks |= (1ULL << (file + r * 8));
        if (((1ULL << (file + r * 8)) & block) != 0) break;
    }
    for (f = file + 1; f <= 7; f++) {
        attacks |= (1ULL << (f + rank * 8));
        if (((1ULL << (f + rank * 8)) & block) != 0) break;
    }
    for (f = file - 1; f >= 0; f--) {
        attacks |= (1ULL << (f + rank * 8));
        if (((1ULL << (f + rank * 8)) & block) != 0) break;
    }
    
    return attacks;
}
BB bishop_attack(int square, BB block) {
    BB attacks = 0;
    int rank = square / 8;
    int file = square % 8;
    int r, f;

    for (r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++) {
        attacks |= (1ULL << (f + r * 8));
        if (((1ULL << (f + r * 8)) & block) != 0) break;
    }
    for (r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (f + r * 8));
        if (((1ULL << (f + r * 8)) & block) != 0) break;
    }
    for (r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (f + r * 8));
        if (((1ULL << (f + r * 8)) & block) != 0) break;
    }
    for (r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (f + r * 8));
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

magic_entry_t init_magic_entry(BB mask, BB magic){
    magic_entry_t magic_entry;
    magic_entry.magic = magic;
    magic_entry.mask = mask;
    return magic_entry;
}

void free_magic_entry(magic_entry_t *entry) {
    free(entry);
}

void binprintf(BB v)
{
    uint64_t mask=1ULL<<(63);
    while(mask) {
        printf("%d", (v&mask ? 1 : 0));
        mask >>= 1;
    }
    printf("\n");
}

magic_entry_t find_magic(int square, int relevant_bits, bool is_rook) {
    int array_size = 4096;
    BB occupancies[array_size];
    BB attacks[array_size];
    BB used_attacks[array_size];

    bool is_error = false;
    bool fail = false;
    magic_entry_t entry = init_magic_entry(0, 0);
    

    BB attack_mask = is_rook ? rook_mask(square) : bishop_mask(square);
    int occupancy_indices = count_1s(attack_mask); //bit_count(attack_mask);

    int bits = (1 << occupancy_indices);

    if (bits > array_size) {
        logf_message(FATAL, "MAGIC", "Occupancy indecies outnumber relevant bits for square: %d", square);
        printf("Bits: %d\n", bits);
        printf("Mask: %lu\n", attack_mask);
        is_error = true;
        goto clean_up;
    }

    int idx, k, magic_idx;
    for(idx = 0; idx < bits; idx++){
        occupancies[idx] = generate_occupancy(idx, occupancy_indices, attack_mask);
        attacks[idx] = is_rook ? rook_attack(square, occupancies[idx]) : bishop_attack(square, occupancies[idx]);
    }

    BB magic;
    for (k = 0; k < 10000000; k++){
        magic = generate_magic_number();
        if(bit_count((attack_mask  * magic) & 0xFF00000000000000ULL) < 6) continue;

        for(idx =  0; idx < array_size; idx++) {
            used_attacks[idx] = 0ULL;
        }
    
        for (idx = 0, fail = false; !fail && idx < bits; idx++){
            magic_idx = transform_key(occupancies[idx], magic, relevant_bits);
            if (used_attacks[magic_idx] == 0ULL) used_attacks[magic_idx] = attacks[idx];
            else if(used_attacks[magic_idx] != attacks[idx]) fail = true;
        }
        if (!fail) {
            entry.magic = magic;
            entry.mask = attack_mask;
            goto clean_up;
        }
    }
    is_error = true;
clean_up:
    if (is_error) {
        logf_message(ERROR, "MAGIC", "Failed to find magic number for square %d", square);
    }

    return entry;
} 

const int rook_bits[] = { 
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12 
};

const int bishop_bits[] = { 
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6 
};

/* Expects rook_magic_entries and bishop_magic_entries to be NULL */
void init_magic_bitboards(magic_entry_t *rook_magic_entries, magic_entry_t *bishop_magic_entries) {
    log_message(INFO, "MAGIC", "Starting to generate magic bitboards!");

    int square;
    log_message(INFO, "MAGIC", "Finding all magic bitboards for rooks");
    for (square = 0; square < 64; square++) {
        printf("Finding magic rook entries for square %d\n", square);    
        rook_magic_entries[square] = find_magic(square, rook_bits[square], true);
    }

    log_message(INFO, "MAGIC", "Finding all magic bitboards for bishop");
    for (square = 0; square < 64; square++) {
        printf("Finding magic bishop entries for square %d\n", square);    
        bishop_magic_entries[square] = find_magic(square, bishop_bits[square], false);
    }
}