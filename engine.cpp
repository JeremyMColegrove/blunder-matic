#include "engine.h"
#include "utils.h"
#include "printers.h"
#include "moves.h"
#include "logger.h"

 int char_pieces[128] = {
        ['P'] = P, ['N'] = N, ['B'] = B, ['R'] = R, ['Q'] = Q, ['K'] = K,
        ['p'] = p, ['n'] = n, ['b'] = b, ['r'] = r, ['q'] = q, ['k'] = k,
    };

void init_occupancies(ChessBoard &board) {
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

ChessBoard createBoardFromFen(const std::string& fen) {
    initAttackTables();

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
        printBitboard(1ULL << board.en_passant_square);
        std::cout << rank << std::endl;
    } else {
        board.en_passant_square = no_square;
    }

    // Parse 50-move counter
    fenStream >> board.half_move_counter;

    // Parse full-move counter
    fenStream >> board.full_move_counter;

    init_occupancies(board);

    return board;
}

bool makeMove(ChessBoard &board, uint64_t move) {
    int from_square = decodeMoveFrom(move);
    int to_square = decodeMoveTo(move);
    int piece = decodePieceType(move);
    int captured_piece = decodeCapturePiece(move);
    bool castling = decodeCastling(move);
    int enpassant = decodeEnPassantFlag(move);
    int promotion_piece = decodePromotionPiece(move);
    bool double_push = decodeDoublePushFlag(move);
    int side = board.white_to_move ? white : black;
    
    // Clear the moving piece from the origin square
    popBit(board.bitboards[piece], from_square);


    // Remove the captured piece if any
    if (captured_piece != no_piece) {
        popBit(board.bitboards[captured_piece], to_square);
    }

    
    // Handle special move flags (e.g., promotion, en passant, castling)
    if (promotion_piece != no_piece) {
        setBit(board.bitboards[promotion_piece], to_square);
    } else if (enpassant) {
        // En passant special move
        if (board.white_to_move) {
            popBit(board.bitboards[p], to_square + 8);
        } else {
            popBit(board.bitboards[P], to_square - 8);
        }
        // Set the moving piece in the destination square
        setBit(board.bitboards[piece], to_square);
    } else {
        // Set the moving piece in the destination square
        setBit(board.bitboards[piece], to_square);
    }

    // enpassant
    if (double_push) {
        if (board.white_to_move) {
            board.en_passant_square = to_square - 8;
        } else {
            board.en_passant_square = to_square + 8;
        }
    } else {
        board.en_passant_square = no_square;
    }

    // Update castling rights
    if (castling) {
            switch (to_square) {
                case g1:
                    popBit(board.bitboards[R], h1);
                    setBit(board.bitboards[R], f1);
                    break;
                case c1:
                    popBit(board.bitboards[R], a1);
                    setBit(board.bitboards[R], d1);
                    break;
                case g8:
                    popBit(board.bitboards[r], h8);
                    setBit(board.bitboards[r], f8);
                    break;
                case c8:
                    popBit(board.bitboards[r], a8);
                    setBit(board.bitboards[r], d8);
                    break;
            }
        }

    board.castling_rights &= castling_rights[from_square];
    board.castling_rights &= castling_rights[to_square];

    board.occupancies[white] = 0;
    board.occupancies[black] = 0;
    // loop over white pieces bitboards
    for (int bb_piece = P; bb_piece <= K; bb_piece++)
        board.occupancies[white] |= board.bitboards[bb_piece];

    // loop over black pieces bitboards
    for (int bb_piece = p; bb_piece <= k; bb_piece++)
        board.occupancies[black] |= board.bitboards[bb_piece];

    // update both sides occupancies
    board.occupancies[both] = (board.occupancies[white] | board.occupancies[black]);

    // Swap side to move
    board.white_to_move = !board.white_to_move;

    // make sure that king is not exposed into a check
    if (isKingAttacked(board, board.white_to_move?white:black))
    {    
        // return illegal move
        return false;
    }
    
    return true;
}

int nodes = 0;
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

    auto start = std::chrono::steady_clock::now();

    Moves moves;
    generateMoves(board, moves);

    ChessBoard boardCopy = board;

    for (int i=0; i<moves.count; i++) {
        int move = moves.list[i];

        if (makeMove(board, move) == false) {
            board = boardCopy;
            continue;
        }

        int cummulative_nodes = nodes;

        perftHelper(board, depth - 1);

        int old_nodes = nodes - cummulative_nodes;

        board = boardCopy;

        printf("%s%s%c %d\n", 
            squaretoCoordinate(decodeMoveFrom(move)).c_str(),
            squaretoCoordinate(decodeMoveTo(move)).c_str(),
            decodePromotionPiece(move)!=no_piece ? ascii_pieces[(decodePromotionPiece(move) % 6) + 6] : ' ',
            old_nodes);
    }

    auto end = std::chrono::steady_clock::now();

    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << std::endl <<  nodes << std::endl;

    writeToLogFile("PERFT finished");

}



int main(int argc, char **argv) {
    if (argc > 2) {
        
        // set the position
        ChessBoard board = createBoardFromFen(argv[2]);

        int depth = atoi(argv[1]);

        perft(board, depth);

    } 

    return 0;
}