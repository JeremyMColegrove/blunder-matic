#include "moves.h"
#include "printers.h"

uint64_t pawn_attack_table[2][64];

void generate_moves(ChessBoard &board, Moves *moves) {
    // calculate moves


}


// Generate attack bitboard for pawn at the given position and side
uint64_t generate_pawn_attack(int side, int position) {
    uint64_t attacks = 0;

    if (side == white) { // White
        if (position >= 8) { // Exclude 7th and 8th ranks
            attacks |= ((1ULL << (position - 7)) & NOT_A_FILE) | ((1ULL << (position - 9)) & NOT_H_FILE);
        }
    } else { // Black
        if (position < 56) { // Exclude 1st and 2nd ranks
            attacks |= ((1ULL << (position+9)) & NOT_A_FILE) | ((1ULL << (position+7)) & NOT_H_FILE);
        }
    }
    return attacks;
}

// Initialize precomputed attack table for pawns
void init_pawn_attack_table() {
    memset(pawn_attack_table, 2*64, sizeof(uint64_t));
    for (int side = 0; side < 2; ++side) {
        for (int square = 0; square < 64; ++square) {
            pawn_attack_table[side][square] = generate_pawn_attack(side, square);
        }
    }
    print_bitboard(pawn_attack_table[black][d8]);

}

void init_moves() {
    init_pawn_attack_table();
}