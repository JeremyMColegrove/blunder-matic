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
U64 en_passant_keys[8];
U64 side_key[2];

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

    for (size_t file = 0; file < 8; ++file) {
        en_passant_keys[file] = dist(rng);
    }

    for (size_t i = 0; i < 2; ++i) {
        side_key[i] = dist(rng);
    }
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
        hash ^= side_key[0];
    } else {
        hash ^= side_key[1];
    }

    hash ^= castling_keys[board.castling_rights];

    if (board.en_passant_square != -1) {
        int file = board.en_passant_square % 8;
        hash ^= en_passant_keys[file];
    }

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

    return board;
}


U64 nodes = 0ULL;
void perftHelper(ChessBoard &board, int depth) {
    if (depth == 0) {
        nodes ++;
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
    nodes = 0;
    Moves moves;
    generateMoves(board, moves);

    ChessBoard boardCopy = board;

    for (int i=0; i<moves.count; i++) {
        int move = moves.list[i];

        if (makeMove(board, move) == false) {
            board = boardCopy;
            continue;
        }

        U64 cummulative_nodes = nodes;

        perftHelper(board, depth - 1);

        U64 old_nodes = nodes - cummulative_nodes;

        board = boardCopy;

        printf("%s%s%c ", 
            squaretoCoordinate(decodeMoveFrom(move)).c_str(),
            squaretoCoordinate(decodeMoveTo(move)).c_str(),
            decodePromotionPiece(move)!=no_piece ? ascii_pieces[(decodePromotionPiece(move) % 6) + 6] : ' ');

        std::cout << old_nodes << std::endl;
    }

    std::cout << std::endl <<  nodes << std::endl;

    writeToLogFile("PERFT finished");

}




int main(int argc, char **argv) {
    clearLogs();

    // uint32_t seqn = htonl(atoi(argv[1]));
    // printMoveHeader();
    // printMove(seqn);
    ChessBoard board = createBoardFromFen("6k1/5p2/1p5p/p4Np1/5q2/Q6P/PPr5/3R3K w - - 1 0");

    // // Moves moves;
    // // generateMoves(board, moves);

    // // printMoves(moves);
    // // printBoard(board);
    // // perft(board, 5);
    // // std::cout << kingInCheck(board) << std::endl;
    search(board, 3, 2);



    // search(board, 8, 2);
    // printBoard(board);

    // parseMoves(board, "e1g1");

    // Moves moves;
    // generateMoves(board, moves);

    // printBoard(board);

    // printMoves(moves);

    // if (argc > 2) {
        
    //     // set the position
    //     ChessBoard board = createBoardFromFen(argv[2]);

    //     int depth = atoi(argv[1]);

    //     if (argc > 3) {
    //         parseMoves(board, argv[3]);
    //     }

    //     perft(board, depth);

    // } 

    return 0;
}