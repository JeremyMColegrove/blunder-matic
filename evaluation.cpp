#include "evaluation.h"

int piece_square_table[6][64] = {
    // Pawn
    { 0,  0,  0,  0,  0,  0,  0,  0,
      5, 10, 10,-20,-20, 10, 10,  5,
      5, -5,-10,  0,  0,-10, -5,  5,
      0,  0,  0, 20, 20,  0,  0,  0,
      5,  5, 10, 25, 25, 10,  5,  5,
      10,10, 20, 30, 30, 20, 10, 10,
      50,50, 50, 50, 50, 50, 50, 50,
      0,  0,  0,  0,  0,  0,  0,  0 },
      
    // Knight
    {-50,-40,-30,-30,-30,-30,-40,-50,
     -40,-20,  0,  5,  5,  0,-20,-40,
     -30,  5, 10, 15, 15, 10,  5,-30,
     -30,  0, 15, 20, 20, 15,  0,-30,
     -30,  5, 15, 20, 20, 15,  5,-30,
     -30,  0, 10, 15, 15, 10,  0,-30,
     -40,-20,  0,  0,  0,  0,-20,-40,
     -50,-40,-30,-30,-30,-30,-40,-50},
     
    // Bishop
    {-20,-10,-10,-10,-10,-10,-10,-20,
     -10,  5,  0,  0,  0,  0,  5,-10,
     -10, 10, 10, 10, 10, 10, 10,-10,
     -10,  0, 10, 10, 10, 10,  0,-10,
     -10,  5,  5, 10, 10,  5,  5,-10,
     -10,  0,  5, 10, 10,  5,  0,-10,
     -10,  0,  0,  0,  0,  0,  0,-10,
     -20,-10,-10,-10,-10,-10,-10,-20},
     
    // Rook
    {  0,  0,  0,  5,  5,  0,  0,  0,
      -5,  0,  0,  0,  0,  0,  0, -5,
      -5,  0,  0,  0,  0,  0,  0, -5,
      -5,  0,  0,  0,  0,  0,  0, -5,
      -5,  0,  0,  0,  0,  0,  0, -5,
      -5,  0,  0,  0,  0,  0,  0, -5,
       5, 10, 10, 10, 10, 10, 10,  5,
       0,  0,  0,  0,  0,  0,  0,  0},
      
    // Queen
    {-20,-10,-10, -5, -5,-10,-10,-20,
     -10,  0,  0,  0,  0,  0,  0,-10,
     -10,  5,  5,  5,  5,  5,  0,-10,
       0,  0,  5,  5,  5,  5,  0, -5,
      -5,  0,  5,  5,  5,  5,  0, -5,
     -10,  0,  5,  5,  5,  5,  0,-10,
     -10,  0,  0,  0,  0,  0,  0,-10,
     -20,-10,-10, -5, -5,-10,-10,-20},
      
    // King (this table should be used in the endgame)
    {-50,-40,-30,-20,-20,-30,-40,-50,
     -30,-20,-10,  0,  0,-10,-20,-30,
     -30,-10, 20, 30, 30, 20,-10,-30,
     -30,-10, 30, 40, 40, 30,-10,-30,
     -30,-10, 30, 40, 40, 30,-10,-30,
     -30,-10, 20, 30, 30, 20,-10,-30,
     -30,-30,  0,  0,  0,  0,-30,-30,
     -50,-30,-30,-30,-30,-30,-30,-50}
};

int piece_values[12] = {
    100, 300, 300, 500, 900, 10000,
    -100, -300, -300, -500, -900, -10000 // Negative for black pieces as we are assuming the point of view of white
};

int evaluate(ChessBoard &board) {
    int score = 0;

    for (int i = 0; i < 12; i++) {
        U64 bb = board.bitboards[i];
        while (bb) {
            int index = __builtin_ctzll(bb);
            score += piece_values[i];
            if(i < 6) // For white pieces
                score -= piece_square_table[i][index];
            else // For black pieces
                score += piece_square_table[i-6][flip(index)];
            bb &= bb - 1;
        }
    }

    return score;
}