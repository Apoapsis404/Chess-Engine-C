#include "movegen.h"
#include "calculate.h"
#include "bitboard.h"
#include "magic.h"
#include "coordinate.h"

movegen_t *init_movegen() {
    movegen_t *movegen = malloc(sizeof(*movegen));
    if (movegen == NULL) {
        log_message(FATAL ,"MOVEGEN", "Failed to allocate space for movegen");
    }
    movegen->move_count = 0;
    return movegen;
}

void free_movegen(movegen_t *movegen) {
    free(movegen);
}

int generate_moves(Board *b) {
    b->movegen->move_count = 0;



    return 0;
}

void add_move(movegen_t *movegen, Move move){
    movegen->moves[movegen->move_count++] = move;
}

int count_trailing_zeros(BB bb) {
    if (bb == 0) {
        return sizeof(bb) * 8;
    }
    int count = 0;
    while ((bb & 1) == 0) {
        bb >>= 1;
        count++;
    }
    return count;
}

BB white_pawns_able_to_push(BB empty_bb, BB piece_bb) {
    return shift_south(empty_bb) & piece_bb;
}

BB white_pawns_able_to_double_push(BB empty_bb) {
    BB rank4 = 0x00000000ff000000;
    BB empty_rank_3 = shift_south(empty_bb & rank4) & empty_bb;
    return white_pawns_able_to_push(empty_bb, empty_rank_3);
}

BB black_pawns_able_to_push(BB empty_bb, BB piece_bb) {
    return shift_north(empty_bb) & piece_bb;
}

BB black_pawns_able_to_double_push(BB empty_bb) {
    BB rank5 = 0x000000ff00000000;
    BB empty_rank_6 = shift_north(empty_bb & rank5) & empty_bb;
    return black_pawns_able_to_push(empty_bb, empty_rank_6);
}

void single_push(BB push_bb, int promotion_rank, Board *b) {
    int from, to;
    
    while(push_bb != 0) {
        from = count_trailing_zeros(push_bb);
        to = b->white_to_move ? from + 8 : from - 8;
        // todo: Check check if()

        //todo pin check

        Move move = construct_move(0, from, to);
        if(rank_from_idx(to) == promotion_rank) {
            set_flag(&move, QUEENPROMOTIONFLAG);
            add_move(b->movegen, move);
            set_flag(&move, ROOKPROMOTIONFLAG);
            add_move(b->movegen, move);
            set_flag(&move, BISHOPPROMOTIONFLAG);
            add_move(b->movegen, move);
            set_flag(&move, KNIGHTPROMOTIONFLAG);
            add_move(b->movegen, move);
        } else {
            add_move(b->movegen, move);
        }
        push_bb &= push_bb - 1;
    }
}

void generate_pawn_moves(Board *b) {
    BB pawns_bb, push_bb, double_push_bb;
    int promotion_rank, en_passant_rank;
    BB *attacks_bb; 

    if (b->white_to_move) {
        pawns_bb = b->bb->pieceBB[WHITEPAWN];
        push_bb = white_pawns_able_to_push(b->bb->emptyBB, pawns_bb);
        double_push_bb = white_pawns_able_to_double_push(b->bb->emptyBB);
        attacks_bb = b->move_array->pawn_attacks[0];
        promotion_rank = 7;
        en_passant_rank = 4;
    } else {
        pawns_bb = b->bb->pieceBB[BLACKPAWN];
        push_bb = black_pawns_able_to_push(b->bb->emptyBB, pawns_bb);
        double_push_bb = black_pawns_able_to_double_push(b->bb->emptyBB);
        attacks_bb = b->move_array->pawn_attacks[1];
        promotion_rank = 0;
        en_passant_rank = 3;
    }

    if (pawns_bb == 0) return;

    single_push(push_bb, promotion_rank, b);

    //double_push(double_push_bb);

    //BB opponent_bb = b->white_to_move ? b->bb->pieceBB[BLACK] : b->bb->pieceBB[WHITE];
    //int en_passant_file = (int)((b->current_state & EN_PASSANT_FILE_MASK) >> 4) - 1;
    //pawn_captures(pawns_bb, attacks_bb, opponent_bb, promotion_rank, en_passant_rank, en_passant_file);
}
