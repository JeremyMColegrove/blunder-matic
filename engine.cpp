#include "engine.h"
#include "utils.h"
#include "printers.h"
#include "moves.h"
#include "logger.h"
#include "search.h"

 int char_pieces[128] = {
        ['P'] = P, ['N'] = N, ['B'] = B, ['R'] = R, ['Q'] = Q, ['K'] = K,
        ['p'] = p, ['n'] = n, ['b'] = b, ['r'] = r, ['q'] = q, ['k'] = k,
    };

U64 piece_keys[12][64];
U64 castling_keys[16];
U64 enpassant_keys[65];
U64 side_key;

void initOccupancies(ChessBoard &board) {
    board.occupancies[white] = 0ULL;
    board.occupancies[black] = 0ULL;
    board.occupancies[both] = 0ULL;

    for (int i = 0; i < 12; i++) {
        U64 bitboard = board.bitboards[i];
        while (bitboard) {
            int square = __builtin_ctzll(bitboard);

            if (i < 6) {
                setBit(board.occupancies[white], square);
            } else {
                setBit(board.occupancies[black], square);
            }

            popLsb(bitboard);
        }   
    }

    board.occupancies[both] = board.occupancies[white] | board.occupancies[black];
}

void initZobristKeys() {
    writeToLogFile("Initializing Zobrist keys");
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist;

    for (size_t pt = 0; pt < 12; ++pt) {
        for (size_t sq = 0; sq < 64; ++sq) {
            piece_keys[pt][sq] = dist(rng);
        }
    }

    for (size_t cr = 0; cr < 16; ++cr) {
        castling_keys[cr] = dist(rng);
    }

    for (size_t square = 0; square < 65; ++square) {
        enpassant_keys[square] = dist(rng);
    }

    side_key = dist(rng);
}

U64 zobristHash(const ChessBoard &board) {
    U64 hash = 0;

    for (size_t pt = 0; pt < 12; ++pt) {
        for (size_t sq = 0; sq < 64; ++sq) {
            if (board.bitboards[pt] & (1ULL << sq)) {
                hash ^= piece_keys[pt][sq];
            }
        }
    }

    if (board.white_to_move) {
        hash ^= side_key;
    }

    hash ^= castling_keys[board.castling_rights];

    hash ^= enpassant_keys[board.en_passant_square];
    

    return hash;
}


ChessBoard createBoardFromFen(const std::string& fen) {
    initAttackTables();
    initZobristKeys();

    writeToLogFile("Creating board with FEN: ", fen);

    ChessBoard board = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::istringstream fenStream(fen);
    std::string boardState;
    fenStream >> boardState;

    int position = 0;
    for (char c : boardState) {
        if (c >= '1' && c <= '8') {
            position += c - '0';
        } else if (c != '/') {
            int piece = char_pieces[c];
            board.bitboards[piece] |= 1ULL << position;
            position++;
        }
    }

    // Parse side to move
    std::string sideToMove;
    fenStream >> sideToMove;
    board.white_to_move = (sideToMove == "w");

    // Parse castling rights
    std::string castlingRights;
    fenStream >> castlingRights;
    board.castling_rights = 0;
    for (char c : castlingRights) {
        switch (c) {
            case 'K': board.castling_rights |= 1; break;
            case 'Q': board.castling_rights |= 2; break;
            case 'k': board.castling_rights |= 4; break;
            case 'q': board.castling_rights |= 8; break;
        }
    }

    // Parse en passant square
    std::string enPassant;
    fenStream >> enPassant;
    if (enPassant != "-") {
        int file = enPassant[0] - 'a';
        int rank = enPassant[1] - '0';
        board.en_passant_square = (8 - rank) * 8 + file;
    } else {
        board.en_passant_square = no_square;
    }

    // Parse 50-move counter
    fenStream >> board.half_move_counter;

    // Parse full-move counter
    fenStream >> board.full_move_counter;

    initOccupancies(board);

    board.hash = zobristHash(board);

    return board;
}


U64 perftNodes = 0ULL;
void perftHelper(ChessBoard &board, int depth) {
    if (depth == 0) {
        perftNodes ++;
        return;
    }

    Moves moves;
    generateMoves(board, moves);

    ChessBoard boardCopy = board;
    for (int i=0; i<moves.count; i++) {
        int move = moves.list[i];

        if (makeMove(board, move) == false) {
            board = boardCopy;
            continue;
        }

        perftHelper(board, depth - 1);

        board = boardCopy;
    }
}

void perft(ChessBoard &board, int depth) {

    writeToLogFile("Starting PERFT with depth", depth);
    perftNodes = 0;
    Moves moves;
    generateMoves(board, moves);

    ChessBoard boardCopy = board;

    for (int i=0; i<moves.count; i++) {
        int move = moves.list[i];

        if (makeMove(board, move) == false) {
            board = boardCopy;
            continue;
        }

        U64 cummulative_nodes = perftNodes;

        perftHelper(board, depth - 1);

        U64 old_nodes = perftNodes - cummulative_nodes;

        board = boardCopy;

        printf("%s%s%c ", 
            squaretoCoordinate(decodeMoveFrom(move)).c_str(),
            squaretoCoordinate(decodeMoveTo(move)).c_str(),
            decodePromotionPiece(move)!=no_piece ? ascii_pieces[(decodePromotionPiece(move) % 6) + 6] : ' ');

        std::cout << old_nodes << std::endl;
    }

    std::cout << std::endl <<  perftNodes << std::endl;

    writeToLogFile("PERFT finished");

}

