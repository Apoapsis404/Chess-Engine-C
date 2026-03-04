#include "logging/lutil.h"
#include "calculate.h"
#include "magic.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>


void free_move_arrays(move_arrays* move_arrays){
    int i;
    for (i = 0; i < 64; i++) {
        free(move_arrays->bishop_attacks[i]);
        free(move_arrays->rook_attacks[i]);
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
    attacks = (east | west) << 8;
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
    int square, idx, relevant_bits_count, occupancy_variations, occupancy, magic_index;
    for(square = 0; square < 64; square++){
        relevant_bits_count = rook_bits[square];
        occupancy_variations = 1 << relevant_bits_count;

        move_array->rook_attacks[square] = malloc(sizeof(BB) * occupancy_variations);

        for(idx = 0; idx < occupancy_variations; idx++) {
            occupancy = generate_occupancy(idx, relevant_bits_count, rook_mask(square));
            magic_index = (int)((occupancy * rook_magic_entries[square].magic) >> (64 - relevant_bits_count));
            move_array->rook_attacks[square][magic_index] = rook_attack(square, occupancy);
        }
    }
}

void calculate_all_bishop_attacks(move_arrays *move_array, magic_entry_t *bishop_magic_entries) {
    int square, idx, relevant_bits_count, occupancy_variations, occupancy, magic_index;
    for(square = 0; square < 64; square++) {
        relevant_bits_count = bishop_bits[square];
        occupancy_variations = 1 << relevant_bits_count;

        move_array->bishop_attacks[square] = malloc(sizeof(BB) * occupancy_variations);

        for(idx = 0; idx < occupancy_variations; idx++) {
            occupancy = generate_occupancy(idx, relevant_bits_count, bishop_mask(square));
            magic_index = (occupancy * bishop_magic_entries[square].magic) >> (64 - relevant_bits_count);
            move_array->bishop_attacks[square][magic_index] = bishop_attack(square, occupancy);
        }
    }
}

move_arrays* init_move_arrays(bool read_in_calcs){
    if (read_in_calcs) log_message(WARNING, "CALCULATE", "Cannot read in calcs because they do not exist");
    move_arrays* move_array = malloc(sizeof(*move_array));

    if (move_array == NULL){
        log_message(ERROR, "CALCULATE", "FATAL: failed to allocate move_array");
        exit(EXIT_FAILURE);
    }

    clock_t time_it;
    log_time_start(DEBUG, "CALCULATE", &time_it);

    calculate_king_moves(move_array);
    calculate_knight_moves(move_array);
    calculate_pawn_attacks(move_array);

    magic_entry_t *rook_magic_entries = malloc(sizeof(magic_entry_t) * 64);
    magic_entry_t *bishop_magic_entries = malloc(sizeof(magic_entry_t) * 64);

    init_magic_bitboards(rook_magic_entries, bishop_magic_entries);

    printf("Magic: %lu, Mask: %lu\n", rook_magic_entries[1].magic, rook_magic_entries[1].mask);

    calculate_all_rook_attacks(move_array, rook_magic_entries);
    calculate_all_bishop_attacks(move_array, bishop_magic_entries);

    free(rook_magic_entries);
    free(bishop_magic_entries);

    log_time_stop(DEBUG, "CALCULATE", &time_it);

    return move_array;
}