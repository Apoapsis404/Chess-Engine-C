#include "logging/lutil.h"
#include "calculate.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>


void free_move_arrays(move_arrays* move_arrays){
    int i;
    for (i = 0; i < 64; i++) {
        free(move_arrays->bishop_attacks[i].piece_attack);
        free(move_arrays->rook_attacks[i].piece_attack);
    }
    free(move_arrays);
}

BB king_attacks(BB king_bb){
    BB attacks = shift_east(king_bb) | shift_west(king_bb);
    king_bb |= attacks;
    attacks |= shift_north(king_bb) | shift_south(king_bb);
    return attacks;
}

void calculate_king_moves(move_arrays* move_array){
    for(int i = 0; i < 64; i++){
        BB king_bb = 1ULL << i;
        move_array->king_moves[i] = king_attacks(king_bb);
    }
}

BB knight_attacks(BB knight_bb) {
    BB west, east, attacks;
    west = shift_west(knight_bb);
    east = shift_east(knight_bb);
    attacks = (east | west) << 16;
    attacks |= (east | west) >> 16;
    west = shift_west(west);
    east = shift_east(east);
    attacks |= (east | west) << 8;
    attacks |= (east | west) >> 8;
    return attacks;
}

void calculate_knight_moves(move_arrays *move_array) {
    for (int i = 0; i < 64; i++) {
        BB knight_bb = 1ULL << i;
        move_array->knight_moves[i] = knight_attacks(knight_bb);
    }
}

BB pawn_attacks(BB pawn_bb, int color) {
    if (color == WHITE) {
        return shift_northeast(pawn_bb) | shift_northwest(pawn_bb);
    } else {
        return shift_southwest(pawn_bb) | shift_southwest(pawn_bb);
    }
}

void calculate_pawn_attacks(move_arrays *move_array) {
    for (int i = 0; i < 64; i++) {
        BB pawn_bb = 1ULL << i;
        move_array->pawn_attacks[0][i] = pawn_attacks(pawn_bb, WHITE);
        move_array->pawn_attacks[1][i] = pawn_attacks(pawn_bb, BLACK);
    }
}


void calculate_all_rook_attacks(move_arrays *move_array, magic_entry_t *rook_magic_entries) {
    int square, idx, relevant_bits_count, occupancy_variations;
    BB occupancy;
    uint64_t magic_index;
    for(square = 0; square < 64; square++){
        relevant_bits_count = rook_bits[square];
        occupancy_variations = 1 << relevant_bits_count;

        move_array->rook_attacks[square].size = occupancy_variations;
        move_array->rook_attacks[square].piece_attack = malloc(sizeof(BB) * occupancy_variations);

        for(idx = 0; idx < occupancy_variations; idx++) {
            occupancy = generate_occupancy(idx, relevant_bits_count, rook_mask(square));
            magic_index = (occupancy * rook_magic_entries[square].magic) >> (64 - relevant_bits_count);
            move_array->rook_attacks[square].piece_attack[magic_index] = rook_attack(square, occupancy);
        }
    }
}

void calculate_all_bishop_attacks(move_arrays *move_array, magic_entry_t *bishop_magic_entries) {
    int square, idx, relevant_bits_count, occupancy_variations;
    BB occupancy;
    uint64_t magic_index;
    for(square = 0; square < 64; square++) {
        relevant_bits_count = bishop_bits[square];
        occupancy_variations = 1 << relevant_bits_count;

        move_array->bishop_attacks[square].size = occupancy_variations;
        move_array->bishop_attacks[square].piece_attack = malloc(sizeof(BB) * occupancy_variations);

        for(idx = 0; idx < occupancy_variations; idx++) {
            occupancy = generate_occupancy(idx, relevant_bits_count, bishop_mask(square));
            magic_index = (occupancy * bishop_magic_entries[square].magic) >> (64 - relevant_bits_count);
            move_array->bishop_attacks[square].piece_attack[magic_index] = bishop_attack(square, occupancy);
        }
    }
}

int triangular_index(int sq1, int sq2) {
    int d = sq1 - sq2;
    d &= d >> 31;
    sq2 += d;
    sq1 -= d;
    sq2 *= sq2 ^ 127;
    return (sq2 >> 1) + sq1;
}

void calculate_all_inbetween(move_arrays *move_array) {
    int sq1, sq2, tri_idx;

    for(int i = 0; i < 65*64/2; i++) {
        move_array->triangle_inbetween[i] = 0ULL;
    }

    for(sq1 = 0; sq1 < 64; sq1++) {
        
        for(sq2 = 0; sq2 < 64; sq2++) {
            tri_idx = triangular_index(sq1, sq2);
            if(move_array->triangle_inbetween[tri_idx] != 0ULL) continue;

            move_array->triangle_inbetween[tri_idx] = in_between(sq1, sq2);
        }
    }
}

#define ARRAY_SIZE 64

#define OPEN_FILE_ERROR -1
#define READ_FROM_FILE_ERROR -2
#define WRITE_TO_FILE_ERROR -3


int read_in_magics(char *filename, move_arrays *move_array) {
    int rc;
    int retval = 0;
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        logf_message(ERROR, "CALC_READ", "Error: Failed to open file %s", filename);
        return OPEN_FILE_ERROR;
    }

    rc = fread(move_array->rook_magic_entries, sizeof(magic_entry_t), ARRAY_SIZE, f);
    if (rc != ARRAY_SIZE) {
        logf_message(ERROR, "CALC_READ_MAGIC", "Failed to read in rook magic entries! Read count: %d", rc);
        goto cleanup;
    }
    rc = fread(move_array->bishop_magic_entries, sizeof(magic_entry_t), ARRAY_SIZE, f);
    if (rc != ARRAY_SIZE) {
        logf_message(ERROR, "CALC_READ_MAGIC", "Failed to read in bishop magic entries! Read count: %d", rc);
        goto cleanup;
    }

cleanup:
    if (f) fclose(f);
    return retval;

}

int read_in_calcs(char *filename, move_arrays *move_array) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        logf_message(ERROR, "CALC_READ", "Error: Failed to open file %s", filename);
        return OPEN_FILE_ERROR;
    }

    (void) move_array;
    return 0;
}

int save_magics(char *filename, move_arrays *move_array){
    int retval = 0;
    int wc;
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        logf_message(ERROR, "CALC_SAVE_MAGIC", "Error: Failed to open file %s", filename);
        return OPEN_FILE_ERROR;
    }

    wc = fwrite(move_array->rook_magic_entries, sizeof(magic_entry_t), ARRAY_SIZE, f);
    if (wc != ARRAY_SIZE) {
        logf_message(ERROR, "CALC_SAVE_MAGIC", "Failed to write rook magic entries to file: Write count: %d", wc);
        retval = WRITE_TO_FILE_ERROR;
        goto cleanup;
    }

    wc = fwrite(move_array->bishop_magic_entries, sizeof(magic_entry_t), ARRAY_SIZE, f);
    if (wc != ARRAY_SIZE) {
        logf_message(ERROR, "CALC_SAVE_MAGIC", "Failed to write bishop magic entries to file: Write count: %d", wc);
        retval = WRITE_TO_FILE_ERROR;
        goto cleanup;
    }

cleanup:
    if (f) fclose(f);
    return retval;
}

int save_calcs(char *filename, move_arrays *move_array) {
    int retval = 0;
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        logf_message(ERROR, "CALC_SAVE", "Error: Failed to open file %s", filename);
        return OPEN_FILE_ERROR;
    }

    size_t wc;

    // Saving king moves
    wc = fwrite(move_array->king_moves, sizeof(BB), ARRAY_SIZE, f);
    if (wc != ARRAY_SIZE) {
        logf_message(ERROR, "CALC_SAVE", "Failed to write king moves to file: Write count: %d", wc);
        retval = WRITE_TO_FILE_ERROR;
        goto cleanup;
    }

    // Saving knight moves
    wc = fwrite(move_array->knight_moves, sizeof(BB), ARRAY_SIZE, f);
    if (wc != ARRAY_SIZE) {
        logf_message(ERROR, "CALC_SAVE", "Failed to write knight moves to file: Write count: %d", wc);
        retval = WRITE_TO_FILE_ERROR;
        goto cleanup;
    }

    // Saving white pawn attacks
    wc = fwrite(move_array->pawn_attacks[0], sizeof(BB), ARRAY_SIZE, f);
    if (wc != ARRAY_SIZE) {
        logf_message(ERROR, "CALC_SAVE", "Failed to write white pawn attacks to file: Write count: %d", wc);
        retval = WRITE_TO_FILE_ERROR;
        goto cleanup;
    }

    // Saving black pawn attacks
    wc = fwrite(move_array->pawn_attacks[1], sizeof(BB), ARRAY_SIZE, f);
    if (wc != ARRAY_SIZE) {
        logf_message(ERROR, "CALC_SAVE", "Failed to write black pawn attacks to file: Write count: %d", wc);
        retval = WRITE_TO_FILE_ERROR;
        goto cleanup;
    }

    // Saving rook moves
    for (int i = 0; i < ARRAY_SIZE; i++) {
        size_t size = move_array->rook_attacks[i].size;
        wc = fwrite(&size, sizeof(size_t), 1, f);
        wc = fwrite(move_array->rook_attacks[i].piece_attack, sizeof(BB), size, f);
        if (wc != size) {
            logf_message(ERROR, "CALC_SAVE", "Failed to write rook moves to file: Write count: %d", wc);
            retval = WRITE_TO_FILE_ERROR;
            goto cleanup;
        }
    }
    // Saving bishop moves
    for (int i = 0; i < ARRAY_SIZE; i++) {
        size_t size = move_array->bishop_attacks[i].size;
        wc = fwrite(&size, sizeof(size_t), 1, f);
        wc = fwrite(move_array->bishop_attacks[i].piece_attack, sizeof(BB), size, f);
        if (wc != size) {
            logf_message(ERROR, "CALC_SAVE", "Failed to write bishop moves to file: Write count: %d", wc);
            retval = WRITE_TO_FILE_ERROR;
            goto cleanup;
        }
    }

cleanup:
    if (f) fclose(f);
    return retval;
}



move_arrays* init_move_arrays(bool read_in_calcs){
    move_arrays* move_array = malloc(sizeof(*move_array));
    
    if (move_array == NULL){
        log_message(ERROR, "CALC", "FATAL: failed to allocate move_array");
        exit(EXIT_FAILURE);
    }
    
    clock_t time_it;
    log_time_start(DEBUG, "CALC", &time_it);
    
    if (read_in_calcs && read_in_magics("magic_calcs.bin", move_array) == 0) {
        log_message(INFO, "CALC", "Succesfully read in calcs");
    } else {
        init_magic_bitboards(move_array->rook_magic_entries, move_array->bishop_magic_entries);
    }

    calculate_king_moves(move_array);
    calculate_knight_moves(move_array);
    calculate_pawn_attacks(move_array);

    calculate_all_rook_attacks(move_array, move_array->rook_magic_entries);
    calculate_all_bishop_attacks(move_array, move_array->bishop_magic_entries);

    calculate_all_inbetween(move_array);

    log_time_stop(DEBUG, "CALC", &time_it);

    return move_array;
}

BB get_inbetween(int sq1, int sq2, move_arrays *move_array) {
    return move_array->triangle_inbetween[triangular_index(sq1, sq2)];
}

BB get_inbetween_inclusive(int sq1, int sq2, move_arrays *move_array){
    return get_inbetween(sq1, sq2, move_array) | 1ULL << sq1 | 1ULL << sq2;
}