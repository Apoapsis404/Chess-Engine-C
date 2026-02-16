#include "logging/lutil.h"
#include "calculate.h"

#include <stdlib.h>
#include <time.h>


void free_move_arrays(move_arrays* move_arrays){
    //free(move_arrays->king_moves);
    //free(move_arrays->knight_moves);
    //free(move_arrays->pawn_attacks);
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
    log_message(DEBUG, "CALCULATE", "Finished calculating king moves");
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

    log_time_stop(DEBUG, "CALCULATE", &time_it);

    return move_array;
}