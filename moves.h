#ifndef MOVES_H
#define MOVES_H

#include "engine.h"
#include "utils.h"


#define not_a_file 18374403900871474942ULL
#define not_h_file 9187201950435737471ULL
#define not_gh_file 4557430888798830399ULL
#define not_ab_file 18229723555195321596ULL

#define row7 65280
#define row2 71776119061217280ULL

#define castle_mask_wk 6917529027641081856
#define castle_mask_bk 96
#define castle_attack_mask_wq 864691128455135232
#define castle_piece_mask_wq 1008806316530991104
#define castle_attack_mask_bq 12
#define castle_piece_mask_bq 14

constexpr int MAX_PLY = 64;
constexpr U64 NOT_A_FILE = 0xFEFEFEFEFEFEFEFE;
constexpr U64 NOT_H_FILE = 0x7F7F7F7F7F7F7F7F;
constexpr int BOARD_SIZE = 64;
constexpr int RANKS = 8;
constexpr int FILES = 8;
constexpr U64 LSB_MASK = 0x1;

struct Moves {
    uint32_t list[MAX_PLY];
    int count;
};



void generateMoves(ChessBoard &board, Moves &moves);


void initAttackTables();

bool isSquareAttacked(ChessBoard &board, int attackingSide, int square);

bool makeMove(ChessBoard &board, uint32_t move);

void parseMoves(ChessBoard &board, const std::string &moves);

inline int getOpponentPiece(ChessBoard &board, int square);

#endif