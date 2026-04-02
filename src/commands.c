#include "commands.h"
#include "ui/ui.h"
#include "logging/lutil.h"
#include "board.h"
#include "fen.h"
#include "move.h"
#include "coordinate.h"
#include "movegen.h"
#include "sutil.h"

#include <string.h>

#define QUIT  "q"
#define RESET "rst"
#define MOVE  "mv"
#define BB_FUNCS "bb"
#define CHANGE_LEVEL "lvl"
#define GET_FEN "fen"

typedef int (*command_fn)(BString args, ui_t *ui);

typedef struct {
    const char *name;
    command_fn handler;
    const char *help;
} command_def_t;

static int bstring_equal(BString token, const char *command) {
    size_t len = strlen(command);
    return token.count == len && strncmp(token.string, command, len) == 0;
}

static void handle_fen(BString bs, Board *b){
    (void)bs;
    String s = get_fen(b);
    logf_message(INFO, "EVAL", "Current FEN: %s", s.string);
    free_string(&s);
}

static int get_color(BString bs){
    int ret = -1;
    if (strncmp(bs.string, "w", 1) == 0) {
        ret = WHITE;
    } else if (strncmp(bs.string, "b", 1) == 0){
        ret = BLACK;
    }
    return ret;
}

static int get_piece(BString bs) {
    int ret = -1;
    if (strncmp(bs.string, "k", 1) == 0) {
        ret = KING;
    } else if (strncmp(bs.string, "p", 1) == 0) {
        ret = PAWN;
    } else if (strncmp(bs.string, "n", 1) == 0) {
        ret = KNIGHT;
    } else if (strncmp(bs.string, "b", 1) == 0) {
        ret = BISHOP;
    } else if (strncmp(bs.string, "r", 1) == 0) {
        ret = ROOK;
    } else if (strncmp(bs.string, "q", 1) == 0) {
        ret = QUEEN;
    } else if (strncmp(bs.string, "o", 1) == 0) {
        ret = 7;
    }
    return ret;
}

PIECE *move_board(Board *b, int color, PIECE from_piece, PIECE target_piece, int from_sq, int to_sq){
    PIECE *board = calloc(64, sizeof(PIECE));
    int from, to;
    Move move;
    PIECE piece;
    for(size_t i = 0; i < b->movegen->move_count; i++) {
        move = b->movegen->moves[i];
        from = get_from(move);
        to = get_to(move);
        piece = b->board[from];

        if (!piece_is_color(piece, color)) {
            logf_message(DEBUG, "EVAL_MOVE", "Piece is wrong color. Color: %s", (piece & COLORMASK) ? "Black" : "White");
            continue;
        }

        if (from_sq != -1 && from != from_sq){
            logf_message(DEBUG, "EVAL_MOVE", "Wrong from square: %d", from);
            continue;
        }

        if (to_sq != -1 && to != to_sq) {
            logf_message(DEBUG, "EVAL_MOVE", "Wrong to square: %d", to);
            continue;
        }

        if (from_piece && b->board[from] != from_piece) {
            logf_message(DEBUG, "EVAL_MOVE", "Wrong piece : %s%s", get_piece_color_name(piece).string, get_piece_name(piece & PIECEMASK).string);
            continue;
        }

        if (target_piece && b->board[to] != target_piece) {
            logf_message(DEBUG, "EVAL_MOVE", "Wrong target piece : %s%s", get_piece_color_name(b->board[to]).string, get_piece_name(b->board[to] & PIECEMASK).string);
            continue;
        }

        board[from] = piece;
        if (move_is_capture(move)) {
            board[to] = 16;
        } else {
            board[to] = CAPTURED_PIECE;
        }
    }
    return board;
}

static int handle_move(BString bs, Board *b){
    if (strlen(bs.string) == 4) {
        Move move = string_to_move(bs);
        if (move == NULLMOVE) return 1;
        return (int)make_move(b, move);
    }

    int color = b->white_to_move ? WHITE : BLACK;

    if (is_square(&bs)) {
        logf_message(INFO, "EVAL", "Got square: %s", bs.string);
        printf("\nPrinting move board!\n");
        PIECE *board_of_moves = move_board(b, color, NONE, NONE, -1, -1);
        print_board(board_of_moves);
        free(board_of_moves);
        printf("\n");
        return 0;
    }
 
    int piece;
    bs.string++;
    piece = get_piece(bs);

    if (piece == -1) {
        logf_message(ERROR, "EVAL", "Unknown piece: %s", bs.string);
        return 0;
    }
    return 0;
}

static void handle_bb_cmds(BString bs, ui_t *ui){
    if (bs.string[0] == '\0') {
        ui->draw_bb = !(ui->draw_bb);
        log_message(DEBUG, "EVAL", "Changing draw bitboard setting");
        return;
    }

    int color = get_color(bs);
    int piece;

    if (color != -1){
        bs.string++;
    }
    piece = get_piece(bs);

    if (piece == -1) {
        logf_message(ERROR, "EVAL", "Unknown bitboard: %s", bs.string);
        return;
    }

    if (piece == 7) {
        if (color != -1) {
            ui->bb = ui->b->bb->occupiedBB & ~ui->b->bb->pieceBB[color ? BLACK : WHITE];
        } else {
            ui->bb = ui->b->bb->occupiedBB;
        }
        return;
    }

    if (color == -1) {
        ui->bb = ui->b->bb->pieceBB[WHITE | piece] | ui->b->bb->pieceBB[BLACK | piece];
    } else {
        ui->bb = ui->b->bb->pieceBB[color | piece];
    }
}

static int handle_quit_cmd(BString args, ui_t *ui) {
    (void)args;
    (void)ui;
    return 1;
}

static int handle_reset_cmd(BString args, ui_t *ui){
    (void)args;
    if (ui->b->board != NULL){
        free(ui->b->board);
    }
    ui->b->board = parse_fen(DEFAULTFEN);
    log_message(INFO, "EVAL", "Resetting board!");
    return 0;
}

static int handle_move_cmd(BString args, ui_t *ui){
    log_message(INFO, "EVAL", "Handling move command");
    return handle_move(args, ui->b);
}

static int handle_bb_cmd(BString args, ui_t *ui){
    handle_bb_cmds(args, ui);
    return 0;
}

static int handle_change_level_cmd(BString args, ui_t *ui){
    (void)ui;
    set_log_level_from_string(args.string);
    return 0;
}

static int handle_get_fen_cmd(BString args, ui_t *ui){
    handle_fen(args, ui->b);
    return 0;
}

static const command_def_t command_table[] = {
    { QUIT, handle_quit_cmd, "Quit REPL" },
    { RESET, handle_reset_cmd, "Reset board to default position" },
    { MOVE, handle_move_cmd, "Make a move" },
    { BB_FUNCS, handle_bb_cmd, "Toggle or select bitboard view" },
    { CHANGE_LEVEL, handle_change_level_cmd, "Change logging level" },
    { GET_FEN, handle_get_fen_cmd, "Display current FEN string" },
};

int eval(BString *bs, ui_t *ui){
    BString token = bstring_next(bs, ' ');
    if (token.count == 0) {
        return 0;
    }

    for (size_t i = 0; i < sizeof(command_table) / sizeof(command_table[0]); ++i) {
        if (bstring_equal(token, command_table[i].name)) {
            return command_table[i].handler(bstring_next(bs, ' '), ui);
        }
    }

    fprintf(stderr, "Unknown command: %.*s\n", (int)token.count, token.string);
    logf_message(WARNING, "EVAL", "Unknown command: %.*s", (int)token.count, token.string);
    return 0;
}

int process_command_line(ui_t *ui, const char *command){
    if (command == NULL || command[0] == '\0') {
        return 0;
    }

    String s = { 0 };
    if (string_append_many(&s, command, strlen(command)) == -1) {
        log_message(ERROR, "EVAL", "Failed to allocate temporary command buffer");
        return 0;
    }

    BString bs = bstring_from_string(&s);
    int result = eval(&bs, ui);
    free_string(&s);
    return result;
}
