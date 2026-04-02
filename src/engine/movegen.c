#include "movegen.h"
#include "calculate.h"
#include "magic.h"
#include "coordinate.h"
//#include "ui/ui.h"

#include <stdio.h>

movegen_t *init_movegen() {
    movegen_t *movegen = malloc(sizeof(*movegen));
    if (movegen == NULL) {
        log_message(FATAL ,"MOVEGEN", "Failed to allocate space for movegen");
    }
    movegen->move_count = 0;
    movegen->pin_bb = 0ULL;
    movegen->checking_pieces = 0ULL;
    return movegen;
}

void free_movegen(movegen_t *movegen) {
    free(movegen);
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

BB get_rook_moves_from_square(int square, move_arrays *move_array, BB occupied_bb) {
    magic_entry_t entry = move_array->rook_magic_entries[square];
    BB occupancy = (occupied_bb & entry.mask);
    occupancy *= entry.magic;
    occupancy >>= (64 - rook_bits[square]);
    return move_array->rook_attacks[square].piece_attack[occupancy]; 
}

BB get_bishop_moves_from_square(int square, move_arrays *move_array, BB occupied_bb) {
    magic_entry_t entry = move_array->bishop_magic_entries[square];
    BB occupancy = (occupied_bb & entry.mask);
    occupancy *= entry.magic;
    occupancy >>= (64 - bishop_bits[square]);
    return move_array->bishop_attacks[square].piece_attack[occupancy];
}

/*
PINS
*/

BB xray_rook_attacks(int square, Board *b, BB blockers) {
    BB attacks = get_rook_moves_from_square(square, b->move_array, b->bb->occupiedBB);
    blockers &= attacks;
    return attacks ^ get_rook_moves_from_square(square, b->move_array, b->bb->occupiedBB ^ blockers);
}

BB xray_bishop_attacks(int square, Board *b, BB blockers) {
    BB attacks = get_bishop_moves_from_square(square, b->move_array, b->bb->occupiedBB);
    blockers &= attacks;
    return attacks ^ get_bishop_moves_from_square(square, b->move_array, b->bb->occupiedBB ^ blockers);
}

BB get_bishop_pins(BB bishop_bb, Board *b, BB enemy_pieces, int king_square) {
    BB pin_bb = 0ULL;
    BB king_between = 0ULL;

    int from;
    BB new_pin;

    while (bishop_bb != 0) {
        from = count_trailing_zeros(bishop_bb);
        new_pin = xray_bishop_attacks(from, b, enemy_pieces);
        pin_bb |= new_pin;
        if (king_square != -1 && new_pin != 0 && (new_pin & (1ULL << king_square)) != 0) {
            king_between |= get_inbetween_inclusive(from, king_square, b->move_array);
        }
        bishop_bb &= bishop_bb - 1;
    }
    if (king_square == -1) {
        return pin_bb;
    }
    return (pin_bb & king_between) | king_between;
}

BB get_rook_pins(BB rook_bb, Board *b, BB enemy_pieces, int king_square) {
    BB pin_bb = 0ULL;
    BB king_between = 0ULL;

    int from;
    BB new_pin;

    while (rook_bb != 0) {
        from = count_trailing_zeros(rook_bb);
        new_pin = xray_rook_attacks(from, b, enemy_pieces);
        pin_bb |= new_pin;
        if (king_square != -1 && new_pin != 0 && (new_pin & (1ULL << king_square)) != 0) {
            king_between |= get_inbetween_inclusive(from, king_square, b->move_array);
        }
        rook_bb &= rook_bb - 1;
    }
    if (king_square == -1) {
        return pin_bb;
    }
    return (pin_bb & king_between) | king_between;
}

BB get_queen_pins(BB queen_bb, Board *b, BB enemy_pieces, int king_square) {
    BB pin_bb = 0ULL;
    BB king_between = 0ULL;

    int from;
    BB new_pin;

    while (queen_bb != 0) {
        from = count_trailing_zeros(queen_bb);
        new_pin = xray_bishop_attacks(from, b, enemy_pieces);
        new_pin |= xray_rook_attacks(from, b, enemy_pieces);
        pin_bb |= new_pin;
        if (king_square != -1 && new_pin != 0 && (new_pin & (1ULL << king_square)) != 0) {
            king_between |= get_inbetween_inclusive(from, king_square, b->move_array);
        }
        queen_bb &= queen_bb - 1;
    }
    if (king_square == -1) {
        return pin_bb;
    }
    return (pin_bb & king_between) | king_between;
}

BB pinned(int king_square, int from, move_arrays *move_array) {
    BB lines = (rank_from_idx(from) == rank_from_idx(king_square)) ||
               (file_from_idx(from) == file_from_idx(king_square)) ?
               get_rook_moves_from_square(king_square, move_array, 0ULL) & get_rook_moves_from_square(from, move_array, 0ULL) :
               get_bishop_moves_from_square(king_square, move_array, 0ULL) & get_bishop_moves_from_square(from, move_array, 0ULL);
    return lines;
}

BB get_pin_bb(int king_square, Board *b) {
    BB enemy_pieces = b->bb->pieceBB[b->white_to_move ? WHITE : BLACK];
    BB pin_bb = 0ULL;

    pin_bb |= get_rook_pins(b->bb->pieceBB[b->white_to_move ? BLACKROOK : WHITEROOK], b, enemy_pieces, king_square);
    pin_bb |= get_bishop_pins(b->bb->pieceBB[b->white_to_move ? BLACKBISHOP : WHITEBISHOP], b, enemy_pieces, king_square);
    pin_bb |= get_queen_pins(b->bb->pieceBB[b->white_to_move ? BLACKQUEEN : WHITEQUEEN], b, enemy_pieces, king_square);
    return pin_bb;
}

/*
END PINS
*/

BB white_pawns_able_to_push(BB empty_bb, BB piece_bb) {
    return shift_south(empty_bb) & piece_bb;
}

BB white_pawns_able_to_double_push(BB empty_bb, BB piece_bb) {
    BB rank4 = 0x00000000ff000000;
    BB empty_rank_3 = shift_south((empty_bb & rank4)) & empty_bb;
    return white_pawns_able_to_push(empty_rank_3, piece_bb);
}

BB black_pawns_able_to_push(BB empty_bb, BB piece_bb) {
    return shift_north(empty_bb) & piece_bb;
}

BB black_pawns_able_to_double_push(BB empty_bb, BB piece_bb) {
    BB rank5 = 0x000000ff00000000;
    BB empty_rank_6 = shift_north((empty_bb & rank5)) & empty_bb;
    return black_pawns_able_to_push(empty_rank_6, piece_bb);
}

void single_push(BB push_bb, int promotion_rank, Board *b) {
    int from, to;
    Move move;
    
    while(push_bb != 0) {
        from = count_trailing_zeros(push_bb);
        to = b->white_to_move ? from + 8 : from - 8;
        // todo: Check check if()
        if (b->check && ((1ULL << to) & b->movegen->checking_pieces) == 0) {
            push_bb &= push_bb - 1;
            continue;
        }

        if ((b->movegen->pin_bb & (1ULL << from)) != 0 && (pinned(b->white_to_move ? b->king_square[WHITE_KING_SQUARE] : b->king_square[BLACK_KING_SQUARE], from, b->move_array) & (1ULL << to)) == 0){
            logf_message(DEBUG, "MOVEGEN_PAWN", "Pawn at square %s is pinned", square_name_from_idx(from).string);
            push_bb &= push_bb - 1;
            continue;
        }

        move = construct_move(0, from, to);
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

void double_push(BB double_push_bb, Board *b) {
    int from, to;

    Move move;
    int to_offset = b->white_to_move ? 16 : -16;

    while (double_push_bb != 0) {
        from = count_trailing_zeros(double_push_bb);
        to = from + to_offset;
        
        //TODO: Add check
        if (b->check && ((1ULL << to) & b->movegen->checking_pieces) == 0) {
            double_push_bb &= double_push_bb - 1;
            continue;
        }

        if ((b->movegen->pin_bb & (1ULL << from)) != 0 && (pinned(b->white_to_move ? b->king_square[WHITE_KING_SQUARE] : b->king_square[BLACK_KING_SQUARE], from, b->move_array) & (1ULL << to)) == 0){
            logf_message(DEBUG, "MOVEGEN_PAWN", "Pawn at square %s is pinned", square_name_from_idx(from).string);
            double_push_bb &= double_push_bb - 1;
            continue;
        }

        move = construct_move(DOUBLEPAWNPUSHFLAG, from ,to);
        add_move(b->movegen, move);
        double_push_bb &= double_push_bb - 1;
    }
}

void handle_en_passant(int from, int en_passant_rank, BB en_passant_bb) {
    (void)from;
    (void)(en_passant_bb);
    (void)(en_passant_rank);
    log_message(WARNING, "MOVEGEN", "En passant not implemented!");
}

void pawn_captures(BB pawns_bb, BB *attacks_bb, BB opponent_bb, int promotion_rank, int en_passant_rank, int en_passant_file, bool white_to_move, Board *b) {
    int from, to;
    BB attacks, en_passant_bb;
    Move move;

    while (pawns_bb != 0) {
        from = count_trailing_zeros(pawns_bb);

        attacks = (attacks_bb[from] & opponent_bb);

        en_passant_bb = attacks_bb[from] & calculate_enpassantbb(en_passant_file, white_to_move);
        if (en_passant_bb != 0 && rank_from_idx(from) == en_passant_rank) {
            handle_en_passant(from, en_passant_rank, en_passant_bb);
        }

        //TODO: Handle check
        if (b->check) {
            attacks &= b->movegen->checking_pieces;
        }

        if ((b->movegen->pin_bb & (1ULL << from)) != 0) {
            logf_message(DEBUG, "MOVEGEN_PAWN", "Pawn at square %s is pinned", square_name_from_idx(from).string);
            attacks &= pinned(b->white_to_move ? b->king_square[WHITE_KING_SQUARE] : b->king_square[BLACK_KING_SQUARE], from, b->move_array);
        }

        while (attacks != 0) {
            to = count_trailing_zeros(attacks);
            move = construct_move(CAPTURESFLAG, from, to);

            if (rank_from_idx(to) == promotion_rank) {
                set_flag(&move, QUEENPROMOTIONCAPTUREFLAG);
                add_move(b->movegen, move);
                set_flag(&move, ROOKPROMOTIONCAPTUREFLAG);
                add_move(b->movegen, move);
                set_flag(&move, BISHOPPROMOTIONCAPTUREFLAG);
                add_move(b->movegen, move);
                set_flag(&move, KNIGHTPROMOTIONCAPTUREFLAG);
                add_move(b->movegen, move);
            } else {
                add_move(b->movegen, move);
            }
            attacks &= attacks - 1;
        }
        pawns_bb &= pawns_bb - 1;
    }
}

void generate_pawn_moves(Board *b) {
    BB pawns_bb, push_bb, double_push_bb;
    int promotion_rank, en_passant_rank;
    BB *attacks_bb; 

    if (b->white_to_move) {
        pawns_bb = b->bb->pieceBB[WHITEPAWN];
        push_bb = white_pawns_able_to_push(b->bb->emptyBB, pawns_bb);
        double_push_bb = white_pawns_able_to_double_push(b->bb->emptyBB, pawns_bb);
        attacks_bb = b->move_array->pawn_attacks[0];
        promotion_rank = 7;
        en_passant_rank = 4;
    } else {
        pawns_bb = b->bb->pieceBB[BLACKPAWN];
        push_bb = black_pawns_able_to_push(b->bb->emptyBB, pawns_bb);
        double_push_bb = black_pawns_able_to_double_push(b->bb->emptyBB, pawns_bb);
        attacks_bb = b->move_array->pawn_attacks[1];
        promotion_rank = 0;
        en_passant_rank = 3;
    }

    if (pawns_bb == 0) return;

    single_push(push_bb, promotion_rank, b);

    double_push(double_push_bb, b);

    BB opponent_bb = b->white_to_move ? b->bb->pieceBB[BLACK] : b->bb->pieceBB[WHITE];
    int en_passant_file = (int)((b->current_state & EN_PASSANT_FILE_MASK) >> 4) - 1;
    pawn_captures(pawns_bb, attacks_bb, opponent_bb, promotion_rank, en_passant_rank, en_passant_file, b->white_to_move, b);
}

void generate_knight_moves(Board *b) {
    BB knight_bb = b->bb->pieceBB[b->white_to_move ? WHITEKNIGHT : BLACKKNIGHT];
    BB friendly_pieces = b->bb->pieceBB[b->white_to_move ? WHITE : BLACK];
    BB enemy_pieces = b->bb->pieceBB[b->white_to_move ? BLACK : WHITE];

    int from, to;
    BB moves_bb;
    Move move;

    while (knight_bb != 0) {
        from = count_trailing_zeros(knight_bb);
        moves_bb = (b->move_array->knight_moves[from] & ~friendly_pieces);

        //TODO: Handle Check
        if (b->check) {
            moves_bb &= b->movegen->checking_pieces;
        }

        if ((b->movegen->pin_bb & (1ULL << from)) != 0) {
            logf_message(DEBUG, "MOVEGEN_KNIGHT", "Knight at square %s is pinned", square_name_from_idx(from).string);
            moves_bb &= pinned(b->king_square[b->white_to_move ? WHITE_KING_SQUARE : BLACK_KING_SQUARE], from, b->move_array);
        }

        while (moves_bb != 0) {
            to = count_trailing_zeros(moves_bb);
            move = construct_move(0, from, to);
            if ((enemy_pieces & (1ULL << to)) != 0) {
                set_flag(&move, CAPTURESFLAG);
            }
            add_move(b->movegen, move);
            moves_bb &= moves_bb - 1;
        }
        knight_bb &= knight_bb - 1;
    }
}

void generate_rook_moves(Board *b) {
    BB rook_bb = b->bb->pieceBB[b->white_to_move ? WHITEROOK : BLACKROOK];
    BB friendly_pieces = b->bb->pieceBB[b->white_to_move ? WHITE : BLACK];
    BB emeny_pieces = b->bb->pieceBB[b->white_to_move ? BLACK : WHITE];

    int from, to;
    BB moves_bb;
    Move move = 0;

    while (rook_bb != 0) {
        from = count_trailing_zeros(rook_bb);
        moves_bb = (get_rook_moves_from_square(from, b->move_array, b->bb->occupiedBB) & ~friendly_pieces);
        
        //TODO: Handle check
        if (b->check) {
            moves_bb &= b->movegen->checking_pieces;
        }

        if ((b->movegen->pin_bb & (1ULL << from)) != 0) {
            logf_message(DEBUG, "MOVEGEN_ROOK", "Rook at square %s is pinned", square_name_from_idx(from).string);
            moves_bb &= pinned(b->king_square[b->white_to_move ? WHITE_KING_SQUARE : BLACK_KING_SQUARE], from, b->move_array);
        }

        while (moves_bb != 0) {
            to = count_trailing_zeros(moves_bb);
            move = construct_move(0, from, to);
            if ((emeny_pieces & (1ULL << to)) != 0) {
                set_flag(&move, CAPTURESFLAG);
            }
            add_move(b->movegen, move);
            moves_bb &= moves_bb - 1;
        }
        rook_bb &= rook_bb - 1;
    }
}

void generate_bishop_moves(Board *b) {
    BB bishop_bb = b->bb->pieceBB[b->white_to_move ? WHITEBISHOP : BLACKBISHOP];
    BB friendly_pieces = b->bb->pieceBB[b->white_to_move ? WHITE : BLACK];
    BB emeny_pieces = b->bb->pieceBB[b->white_to_move ? BLACK : WHITE];

    int from, to;
    BB moves_bb;
    Move move = 0;

    while (bishop_bb != 0) {
        from = count_trailing_zeros(bishop_bb);
        moves_bb = (get_bishop_moves_from_square(from, b->move_array, b->bb->occupiedBB) & ~friendly_pieces);
        
        //TODO: Handle check
        if (b->check) {
            moves_bb &= b->movegen->checking_pieces;
        }

        if ((b->movegen->pin_bb & (1ULL << from)) != 0) {
            logf_message(DEBUG, "MOVEGEN_BISHOP", "Bishop at square %s is pinned", square_name_from_idx(from).string);
            moves_bb &= pinned(b->king_square[b->white_to_move ? WHITE_KING_SQUARE : BLACK_KING_SQUARE], from, b->move_array);
        }

        while (moves_bb != 0) {
            to = count_trailing_zeros(moves_bb);
            move = construct_move(0, from, to);
            if ((emeny_pieces & (1ULL << to)) != 0) {
                set_flag(&move, CAPTURESFLAG);
            }
            add_move(b->movegen, move);
            moves_bb &= moves_bb - 1;
        }
        bishop_bb &= bishop_bb - 1;
    }
}

void generate_queen_moves(Board *b) {
    BB queen_bb = b->bb->pieceBB[b->white_to_move ? WHITEQUEEN : BLACKQUEEN];
    BB friendly_pieces = b->bb->pieceBB[b->white_to_move ? WHITE : BLACK];
    BB enemy_pieces = b->bb->pieceBB[b->white_to_move ? BLACK : WHITE];

    int from, to;
    BB moves_bb;
    Move move;

    while (queen_bb != 0) {
        from = count_trailing_zeros(queen_bb);
        moves_bb = (get_rook_moves_from_square(from, b->move_array, b->bb->occupiedBB) |
                    get_bishop_moves_from_square(from, b->move_array, b->bb->occupiedBB)) &
                    ~friendly_pieces;
         
        //TODO: Handle Check
        if (b->check) {
            moves_bb &= b->movegen->checking_pieces;
        }

        if ((b->movegen->pin_bb & (1ULL << from)) != 0) {
            logf_message(DEBUG, "MOVEGEN_QUEEN", "Queen at square %s is pinned", square_name_from_idx(from).string);
            moves_bb &= pinned(b->king_square[b->white_to_move ? WHITE_KING_SQUARE : BLACK_KING_SQUARE], from, b->move_array);
        }



        while(moves_bb != 0) {
            to = count_trailing_zeros(moves_bb);
            move = construct_move(0, from, to);
            if ((enemy_pieces & (1ULL << to)) != 0) {
                set_flag(&move, CAPTURESFLAG);
            }
            add_move(b->movegen, move);
            moves_bb &= moves_bb - 1;
        }
        queen_bb &= queen_bb - 1;
    }
}

BB attacks_to(BB occ, int square, BB *piece_bb, move_arrays *move_array) {
    BB knights, kings, bishopsQueens, rooksQueens;
    knights = piece_bb[WHITEKNIGHT] | piece_bb[BLACKKNIGHT];
    kings = piece_bb[WHITEKING] | piece_bb[BLACKKING];
    rooksQueens = piece_bb[WHITEQUEEN] | piece_bb[BLACKQUEEN];
    bishopsQueens = piece_bb[WHITEQUEEN] | piece_bb[BLACKQUEEN];
    rooksQueens |= piece_bb[WHITEROOK] | piece_bb[BLACKROOK];
    bishopsQueens |= piece_bb[WHITEBISHOP] | piece_bb[BLACKBISHOP];

    return (move_array->pawn_attacks[0][square] & piece_bb[BLACKPAWN])
        | (move_array->pawn_attacks[1][square] & piece_bb[WHITEPAWN])
        | (move_array->knight_moves[square] & knights)
        | (move_array->king_moves[square] & kings)
        | (get_bishop_moves_from_square(square, move_array, occ) & bishopsQueens)
        | (get_rook_moves_from_square(square, move_array, occ) & rooksQueens);
}

bool calculate_castling_rights(Board *b, bool white_to_move, bool king_side) {
    if (b->check) return false;
    bool attacked = false;

    BB occ = b->bb->occupiedBB;

    if (white_to_move) {
        if (king_side) {
            attacked = ((attacks_to(occ, 5, b->bb->pieceBB, b->move_array) | 
                         attacks_to(occ, 6, b->bb->pieceBB, b->move_array)) &
                         b->bb->pieceBB[BLACK]) != 0;
            return (occ & WHITE_KINGSIDE_EMPTY) == 0 && !attacked;
        } else {
            attacked = ((attacks_to(occ, 2, b->bb->pieceBB, b->move_array) |
                         attacks_to(occ, 3, b->bb->pieceBB, b->move_array)) & 
                         b->bb->pieceBB[BLACK]) != 0;
            return (occ & WHITE_QUEENSIDE_EMPTY) == 0 && !attacked;
        }
    } else {
        if (king_side) {
            attacked = ((attacks_to(occ, 61, b->bb->pieceBB, b->move_array) | 
                         attacks_to(occ, 62, b->bb->pieceBB, b->move_array)) &
                         b->bb->pieceBB[WHITE]) != 0;
            return (occ & BLACK_KINGSIDE_EMPTY) == 0 && !attacked;
        } else {
            attacked = ((attacks_to(occ, 58, b->bb->pieceBB, b->move_array) |
                         attacks_to(occ, 59, b->bb->pieceBB, b->move_array)) & 
                         b->bb->pieceBB[WHITE]) != 0;
            return (occ & BLACK_QUEENSIDE_EMPTY) == 0 && !attacked;
        }
    }
}

void generate_king_moves(Board *b) {
    int piece_color = b->white_to_move ? WHITE : BLACK;
    int from, to;

    BB piece_bb = b->bb->pieceBB[piece_color];
    BB opponent_bb = b->bb->pieceBB[b->white_to_move ? BLACK : WHITE];

    BB king_bb = b->bb->pieceBB[piece_color | KING];

    if (king_bb == 0) return;

    from = count_trailing_zeros(king_bb);
    BB moves_bb = b->move_array->king_moves[from] & ~piece_bb;
    
    if (b->check) {
        moves_bb ^= b->movegen->checking_pieces & moves_bb;
        moves_bb &= ~get_pin_bb(-1, b);
    }

    Move move;
    while (moves_bb != 0) {
        to = count_trailing_zeros(moves_bb);

        if ((attacks_to(0ULL, to, b->bb->pieceBB, b->move_array) & opponent_bb) != 0){
            moves_bb &= moves_bb - 1;
            continue;
        }

        move = construct_move(0, from, to);
        if((opponent_bb & (1ULL << to)) != 0) {
            set_flag(&move, CAPTURESFLAG);
        }

        add_move(b->movegen, move);
        moves_bb &= moves_bb - 1;
    }

    // Castling
    if (b->white_to_move) {
        if ((b->current_state & CASTLING_RIGHTS_MASK & CASTLING_WHITE_KINGSIDE) != 0){
            // White king Side
            if (calculate_castling_rights(b, true, true)) {
                move = construct_move(KINGCASLTEFLAG, from, 6);
                add_move(b->movegen, move);
            }
        }
        if ((b->current_state & CASTLING_RIGHTS_MASK & CASTLING_WHITE_QUEENSIDE) != 0){
            // White queen Side
            if (calculate_castling_rights(b, true, false)) {
                move = construct_move(KINGCASLTEFLAG, from, 2);
                add_move(b->movegen, move);
            }
        }
    } else {
        if ((b->current_state & CASTLING_RIGHTS_MASK & CASTLING_BLACK_KINGSIDE) != 0){
            // Black king Side
            if (calculate_castling_rights(b, false, true)) {
                move = construct_move(KINGCASLTEFLAG, from, 62);
                add_move(b->movegen, move);
            }
        }
        if ((b->current_state & CASTLING_RIGHTS_MASK & CASTLING_BLACK_QUEENSIDE) != 0){
            // Black queen Side
            if (calculate_castling_rights(b, false, false)) {
                move = construct_move(KINGCASLTEFLAG, from, 58);
                add_move(b->movegen, move);
            }
        }
    }

}

BB handle_check(Board *b) {
    BB cpieces = 0ULL;
    int king_square = b->king_square[b->white_to_move ? WHITE_KING_SQUARE : BLACK_KING_SQUARE];
    BB attacks = attacks_to(b->bb->occupiedBB, king_square, b->bb->pieceBB, b->move_array) & b->bb->pieceBB[b->white_to_move ? BLACK : WHITE];
    int attack_from_square = count_trailing_zeros(attacks);

    cpieces |= b->move_array->triangle_inbetween[triangular_index(king_square, attack_from_square)];
    
    attacks &= attacks - 1;
    // If there are remaining attacks it is a double check and only king moves are allowed
    if (attacks != 0) {
        printf("It is a double check!\n");
        return 0ULL;
    }
    return cpieces;
}


int generate_moves(Board *b) {
    log_message(INFO, "MOVEGEN", "Generating moves");
    
    // Reset movegen
    b->movegen->move_count = 0;
    b->check = false;
    b->movegen->checking_pieces = UINT64_MAX;

    // Check pins and check
    b->movegen->pin_bb = get_pin_bb(b->king_square[b->white_to_move ? WHITE_KING_SQUARE : BLACK_KING_SQUARE], b);

    if ((attacks_to(b->bb->occupiedBB, b->king_square[b->white_to_move ? WHITE_KING_SQUARE : BLACK_KING_SQUARE], b->bb->pieceBB, b->move_array) & b->bb->pieceBB[b->white_to_move ? BLACK : WHITE]) != 0) {
        logf_message(DEBUG, "MOVEGEN", "The %s king is in check", b->white_to_move ? "white" : "black");
        b->check = true;
        b->movegen->checking_pieces = handle_check(b);
    }

    generate_pawn_moves(b);
    generate_king_moves(b);
    generate_knight_moves(b);
    generate_rook_moves(b);
    generate_bishop_moves(b);
    generate_queen_moves(b);

    return 0;
}

void dump_moves(movegen_t *movegen) {
    for (size_t i = 0; i < movegen->move_count; i++) {
        print_move(movegen->moves[i]);
    }
}
