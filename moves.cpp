#include "moves.h"
#include "printers.h"


U64 knightMasks[64];
U64 kingMasks[64];
U64 bishopAttackMasks[64];
U64 rookAttackMasks[64];

// Declare the attack tables as global arrays
U64 rookAttackTable[64][4096];
U64 bishopAttackTable[64][512];

// pre-calculate all of the pawn moves so we just do simple lookups when generating the moves
U64 pawnAttackTable[2][64];
U64 pawnDoubleTable[2][64];
U64 pawnSingleTable[2][64];
U64 pawnEnpassantTable[2][64][8];

const U64 magicR[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};

const U64 magicB[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};




// Generate attack bitboard for pawn at the given position and side
uint64_t generatePawnAttacks(int side, int square) {
        uint64_t attacks = 0;

        if (side == white) { // White
            if (square >= 8) { // Exclude 7th and 8th ranks
                attacks |= ((1ULL << (square - 7)) & NOT_A_FILE) | ((1ULL << (square - 9)) & NOT_H_FILE);
            }
        } else { // Black
            if (square < 56) { // Exclude 1st and 2nd ranks
                attacks |= ((1ULL << (square+9)) & NOT_A_FILE) | ((1ULL << (square+7)) & NOT_H_FILE);
            }
        }
        return attacks;
}

U64 generatePawnDoublePushes(int side, int square) {
    U64 mask = 0ULL;
    if (side == white && (square / 8) == 6) {
        mask |= (1ULL << (square - 16));
    } else if (side==black && (square / 8) == 1) {
        mask |= (1ULL << (square + 16));
    }
    return mask;
}

U64 generatePawnSinglePushes(int side, int square) {
    U64 mask = 0ULL;
    // Precompute single push moves
    if (side==white && square > 7) {
        mask |= (1ULL << (square - 8));
    } else if (side==black && square < 56) {
        mask |= (1ULL << (square + 8));
    }
    return mask;
}

U64 generateKnightMasks(int square) {
    //Generate the knight tables
    U64 board = 0UL;
    U64 attacks = 0UL;
    set_bit(board, square);

    //Knight attacks  attacks//

    if (board >> 15 & not_a_file) attacks |= board >> 15;
    if (board >> 17 & not_h_file) attacks |= board >> 17;

    if (board >> 6 & not_ab_file)  attacks |= board >> 6;
    if (board >> 10 & not_gh_file) attacks |= board >> 10;


    if (board << 15 & not_h_file) attacks |= board << 15;
    if (board << 17 & not_a_file) attacks |= board << 17;

    if (board << 6 & not_gh_file)  attacks |= board << 6;
    if (board << 10 & not_ab_file) attacks |= board << 10;

    return attacks;
}

U64 generateKingMasks(int square) {
    //Generate the kings tables
    U64 white_board = 0UL;
    U64 attacks = 0UL;
    set_bit(white_board, square);

    //Left
    if (white_board >> 1 & not_h_file) attacks |= white_board >> 1;
    if (white_board >> 9 & not_h_file) attacks |= white_board >> 9;
    if (white_board >> 7 & not_a_file) attacks |= white_board >> 7;

    if (white_board << 1 & not_a_file) attacks |= white_board << 1;
    if (white_board << 9 & not_a_file) attacks |= white_board << 9;
    if (white_board << 7 & not_h_file) attacks |= white_board << 7;

    attacks |= white_board >> 8;
    attacks |= white_board << 8;

    return attacks;
}

uint64_t generateBishopAttacks(int square, uint64_t occupancy) {
    uint64_t attacks = 0;
    int rank = square / RANKS;
    int file = square % FILES;

    // Up-right diagonal
    for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f) {
        int sq = r * 8 + f;
        set_bit(attacks, sq);
        if (get_bit(occupancy, sq)) break;
    }

    // Up-left diagonal
    for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f) {
        int sq = r * 8 + f;
        set_bit(attacks, sq);
        if (get_bit(occupancy, sq)) break;
    }

    // Down-right diagonal
    for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f) {
        int sq = r * 8 + f;
        set_bit(attacks, sq);
        if (get_bit(occupancy, sq)) break;
    }

    // Down-left diagonal
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f) {
        int sq = r * 8 + f;
        set_bit(attacks, sq);
        if (get_bit(occupancy, sq)) break;
    }

    return attacks;
}

uint64_t generateRookAttacks(int square, uint64_t occupancy) {
    uint64_t attacks = 0;
    int rank = square / 8;
    int file = square % 8;

    // Up
    for (int r = rank + 1; r < 8; ++r) {
        int sq = r * 8 + file;
        set_bit(attacks, sq);
        if (get_bit(occupancy, sq)) break;
    }

    // Down
    for (int r = rank - 1; r >= 0; --r) {
        int sq = r * 8 + file;
        set_bit(attacks, sq);
        if (get_bit(occupancy, sq)) break;
    }

    // Right
    for (int f = file + 1; f < 8; ++f) {
        int sq = rank * 8 + f;
        set_bit(attacks, sq);
        if (get_bit(occupancy, sq)) break;
    }

    // Left
    for (int f = file - 1; f >= 0; --f) {
        int sq = rank * 8 + f;
        set_bit(attacks, sq);
        if (get_bit(occupancy, sq)) break;
    }

    return attacks;
}

// the attack mask does not include corner squares or edge squares, for indexing into the magic bitboard
U64 generateRookAttackMask(int square) {
    U64 mask = 0;
    int rank = square / 8, file = square % 8;
    for (int r = rank + 1; r < 7; ++r) mask |= (1ULL << (r * 8 + file));
    for (int r = rank - 1; r > 0; --r) mask |= (1ULL << (r * 8 + file));
    for (int f = file + 1; f < 7; ++f) mask |= (1ULL << (rank * 8 + f));
    for (int f = file - 1; f > 0; --f) mask |= (1ULL << (rank * 8 + f));
    return mask;
}

U64 generateBishopAttackMask(int square) {
    U64 mask = 0;
    int rank = square / 8, file = square % 8;
    for (int r = rank + 1, f = file + 1; r < 7 && f < 7; ++r, ++f) mask |= (1ULL << (r * 8 + f));
    for (int r = rank - 1, f = file + 1; r > 0 && f < 7; --r, ++f) mask |= (1ULL << (r * 8 + f));
    for (int r = rank + 1, f = file - 1; r < 7 && f > 0; ++r, --f) mask |= (1ULL << (r * 8 + f));
    for (int r = rank - 1, f = file - 1; r > 0 && f > 0; --r, --f) mask |= (1ULL << (r * 8 + f));
    return mask;
}

const int bishopRelevantBits[64] = {
            6, 5, 5, 5, 5, 5, 5, 6,
            5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 7, 7, 7, 7, 5, 5,
            5, 5, 7, 9, 9, 7, 5, 5,
            5, 5, 7, 9, 9, 7, 5, 5,
            5, 5, 7, 7, 7, 7, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5,
            6, 5, 5, 5, 5, 5, 5, 6,
    };
    const int rookRelevantBits[64] = {
            12, 11, 11, 11, 11, 11, 11, 12,
            11, 10, 10, 10, 10, 10, 10, 11,
            11, 10, 10, 10, 10, 10, 10, 11,
            11, 10, 10, 10, 10, 10, 10, 11,
            11, 10, 10, 10, 10, 10, 10, 11,
            11, 10, 10, 10, 10, 10, 10, 11,
            11, 10, 10, 10, 10, 10, 10, 11,
            12, 11, 11, 11, 11, 11, 11, 12,
    };

U64 setOccupancy(int index, int bits_in_mask, U64 attack_mask) {
    U64 occupancy = 0ULL;

    for (int i=0; i<bits_in_mask; i++)
    {
        int square = __builtin_ctzll(attack_mask);
        pop_bit(attack_mask, square);
        if (index & (1ULL << i))
        {
            occupancy |= 1ULL << square;
        }
    }
    return occupancy;
}

void initAttackTables() {
    // generate all masks
    for (int square = 0; square < BOARD_SIZE; square++) {
        bishopAttackMasks[square] = generateBishopAttackMask(square);
        rookAttackMasks[square] = generateRookAttackMask(square);
        kingMasks[square] = generateKingMasks(square);
        knightMasks[square] = generateKnightMasks(square);
        for (int side=0; side < 2; side++) {
            pawnAttackTable[side][square] = generatePawnAttacks(side, square);
            pawnDoubleTable[side][square] = generatePawnDoublePushes(side, square);
            pawnSingleTable[side][square] = generatePawnSinglePushes(side, square);
        }
    }

    // generate magic bitboards for sliding pieces
    for (int square=0; square<64; square++)
    {
        // get our rook attack mask
        U64 rook_attack = rookAttackMasks[square];
        // get our bishop attack mask
        U64 bishop_attack = bishopAttackMasks[square];

        // get the number of bits in the rook mask
        int rook_relevant_bit_count = __builtin_popcountll(rook_attack);

        //get the number of bits in the bishop mask
        int bishop_relevant_bit_count = __builtin_popcountll(bishop_attack);

        // get the number of rook indicies
        int rook_occupancy_indicies = (1 << rook_relevant_bit_count);

        // get the number of bishop indicies
        int bishop_occupancy_indicies = (1 << bishop_relevant_bit_count);

        //Construct all attacks for this square for the rook
        for (int index=0; index<rook_occupancy_indicies; index++)
        {
            //The rooks occupancy mask
            U64 rook_occupancy = setOccupancy(index, rook_relevant_bit_count, rook_attack);
            //The rooks magic index
            int magic_index = (rook_occupancy * magicR[square]) >> (64 - rookRelevantBits[square]);
            //Insert the attack into our table
            rookAttackTable[square][magic_index] = generateRookAttacks(square, rook_occupancy);
        }

        //Construct all attacks for this square for the rook
        for (int index=0; index<bishop_occupancy_indicies; index++)
        {
            //The rooks occupancy mask
            U64 bishop_occupancy = setOccupancy(index, bishop_relevant_bit_count, bishop_attack);
            //The rooks magic index
            int magic_index = (bishop_occupancy * magicB[square]) >> (64 - bishopRelevantBits[square]);
            //Insert the attack into our table
            bishopAttackTable[square][magic_index] = generateBishopAttacks(square, bishop_occupancy);
        }
    }
}

U64 getRookAttacks(int square, U64 occupancy) {
    occupancy &= rookAttackMasks[square];
    occupancy *= magicR[square];
    occupancy >>= 64 - rookRelevantBits[square];
    return rookAttackTable[square][occupancy];
}

U64 getBishopAttacks(int square, U64 occupancy) {
    occupancy &= bishopAttackMasks[square];
    occupancy *= magicB[square];
    occupancy >>= 64 - bishopRelevantBits[square];
    return bishopAttackTable[square][occupancy];
}

U64 getQueenAttacks(int square, U64 occupancy) {
    return getRookAttacks(square, occupancy) | getBishopAttacks(square, occupancy);
}

// U64 getMoves(int piece, int square, ChessBoard &board) {
//     // go through all of the bitboards and check which piece is on the piece, assuming it is not an invalid piece
//     U64 attacks = 0ULL;
//     U64 piece_bitboard = (1ULL << square);
//     int side = board.white_to_move ? white : black;

//     switch (piece) {
//         case r:
//         case R:
//             return getRookAttacks(square, board.occupancies[both]) & ~board.occupancies[side];
//         case b:
//         case B:
//             return getBishopAttacks(square, board.occupancies[both]) & ~board.occupancies[side];
//         case q:
//         case Q:
//             return (getBishopAttacks(square, board.occupancies[both]) | getRookAttacks(square, board.occupancies[both])) & ~board.occupancies[side];
//         case p:
//             //  single pushes
//             attacks |= ((piece_bitboard << 8) & ~board.occupancies[both]) | (pawnAttackTable[side][square] & board.occupancies[side ^ 1]);
//             //  double pushes
//             attacks |= ((((piece_bitboard & row7) << 8) & ~board.occupancies[both]) << 8) & ~board.occupancies[both];
//             // handle enpassants here as well
//             if (board.en_passant_square != no_square) {
//                 attacks |= pawnAttackTable[side][square] & (1ULL << board.en_passant_square);
//             }
//             return attacks;
//         case P:
//             // single pushes
//             attacks |= ((piece_bitboard >> 8) & ~board.occupancies[both]) | (pawnAttackTable[side][square] & board.occupancies[side ^ 1]);
//             // double pushes
//             attacks |= ((((piece_bitboard & row2) >> 8) & ~board.occupancies[both]) >> 8) & ~board.occupancies[both];

//             // handle enpassants here as well
//             if (board.en_passant_square != no_square) {
//                 // check if enpassant square and capture mask collide
//                 attacks |= pawnAttackTable[side][square] & (1ULL << board.en_passant_square);
//             }
//             return attacks;
//         case n:
//         case N:
//             return knightMasks[square] & ~board.occupancies[side];
//         case k:
//         case K:
//             return kingMasks[square] & ~board.occupancies[side];
//     }
//     return attacks;
// }

inline int getOpponentPiece(ChessBoard &board, int square) {
    int offset = board.white_to_move?6:0;
    // go through opponent bitboards
    if (get_bit(board.occupancies[board.white_to_move?black:white], square)) {
        // search opponents bitboards
        for (int i=0; i<6; i++) {
            if (get_bit(board.bitboards[i + offset], square)) {
                return (offset + i) % 12;
            }
        }
    }

    return no_piece;
}



void generateMoves(ChessBoard &board, Moves &moves) {
    // calculate moves
    moves.count = 0;

    int square, side;

    side = (board.white_to_move) ? white : black;
    // get the right pieces (for easier lookup later)
    int pawn, knight, bishop, rook, queen, king;
    U64 *opponent_bitboards;
    if (board.white_to_move == false) {
        pawn = p, knight=n, bishop=b, rook=r, queen=q, king=k;
        opponent_bitboards=board.bitboards;
    } else {
        pawn = P, knight=N, bishop=B, rook=R, queen=Q, king=K;
        opponent_bitboards=board.bitboards+6;
    }

    //KING MOVES
    U64 empty_squares = ~board.occupancies[side];
    int king_square = __builtin_ctzll(board.bitboards[king]);
    U64 king_moves = kingMasks[king_square] & empty_squares;

    // get all of the possible destinations for the king
    while (king_moves) {
        square = __builtin_ctzll(king_moves);

        // encode the move
        int captured_piece = getOpponentPiece(board, square);
        moves.list[moves.count++] = encode_move(king_square, square, king, captured_piece, no_piece, false, false, false);
        pop_lsb(king_moves);
    }


    // PAWN MOVES
    U64 pawns = board.bitboards[pawn];
    U64 doublePushes, enpassantBoard = 0ULL, attacks, pawnMoves, singlePushes;

    // enpassant board
    if (board.en_passant_square != no_square) {
        enpassantBoard |= 1ULL << board.en_passant_square;
    }

    while (pawns) {
        square = __builtin_ctzll(pawns);

        // get the procomputed pawn move masks
        singlePushes = pawnSingleTable[side][square] & ~board.occupancies[both];
        if (singlePushes > 0) {
            doublePushes = pawnDoubleTable[side][square] & ~board.occupancies[both];
        }

        attacks = pawnAttackTable[side][square] & (board.occupancies[side^1] | enpassantBoard);
        pawnMoves = singlePushes | doublePushes | attacks;

        // go through each move and add to list
        while (pawnMoves) {
            int destination = __builtin_ctzll(pawnMoves);
            

            // getOpponentPiece is expensive, only compute if we know its a capture

            bool is_capture = (1ULL << destination) & attacks ;

            int captured_piece = no_piece;
            if (is_capture) {
                captured_piece = getOpponentPiece(board, destination);
            }

            // check if pawn is going to promoted square
            bool is_promotion_square = side == white ? (destination >= 0 && destination <= 7) : (destination >= 56 && destination <= 63);

            if (is_promotion_square) {
                moves.list[moves.count++] = encode_move(square, destination, pawn, captured_piece, rook, false, false, false);
                moves.list[moves.count++] = encode_move(square, destination, pawn, captured_piece, bishop, false, false, false);
                moves.list[moves.count++] = encode_move(square, destination, pawn, captured_piece, queen, false, false, false);
                moves.list[moves.count++] = encode_move(square, destination, pawn, captured_piece, knight, false, false, false);
            } else if (destination == board.en_passant_square) {
                moves.list[moves.count++] = encode_move(square, destination, pawn, captured_piece, no_piece, true, false, false);
            } else {
                moves.list[moves.count++] = encode_move(square, destination, pawn, captured_piece, no_piece, false, false, abs(square-destination)==16);
            }

            pop_lsb(pawnMoves);
        }

        pop_lsb(pawns);
    }

    
}



