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
    int side = board.white_to_move?white:black;

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
            board.en_passant_square = to_square + 8;
        } else {
            board.en_passant_square = to_square - 8;
        }
    } else {
        board.en_passant_square = no_square;
    }

    // Update castling rights
    if (castling) {
            switch (to_square) {
                case g1:
                    if (isSquareAttacked(board, black, f1) || isSquareAttacked(board, black, g1) || isSquareAttacked(board, black, e1)) 
                        return false;
                    popBit(board.bitboards[R], h1);
                    setBit(board.bitboards[R], f1);
                    break;
                case c1:
                    if (isSquareAttacked(board, black, c1) || isSquareAttacked(board, black, d1) || isSquareAttacked(board, black, e1)) 
                        return false;
                    popBit(board.bitboards[R], a1);
                    setBit(board.bitboards[R], d1);
                    break;
                case g8:
                    if (isSquareAttacked(board, white, f8) || isSquareAttacked(board, white, g8) || isSquareAttacked(board, white, e8)) 
                        return false;
                    popBit(board.bitboards[r], h8);
                    setBit(board.bitboards[r], f8);
                    break;
                case c8:
                    if (isSquareAttacked(board, white, c8) || isSquareAttacked(board, white, d8) || isSquareAttacked(board, white, e8)) 
                        return false;
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
    int kingSquare = __builtin_ctzll(board.white_to_move?board.bitboards[k]:board.bitboards[K]);
    if (isSquareAttacked(board, board.white_to_move?white:black, kingSquare))
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

    std::cout << std::endl <<  nodes << std::endl;

    writeToLogFile("PERFT finished");

}

int getPieceOnSquare(ChessBoard &board, int square) {
    for (int index = 0; index < 12; index++) {
        U64 bitboard = board.bitboards[index];
        if (getBit(bitboard, square)) return index;
    }
    return no_piece;
}

void parseMove(ChessBoard &board, const std::string &move) {
    int from = (move[0] - 'a') + (8 - (move[1] - '0')) * 8;
    int to = (move[2] - 'a') + (8 - (move[3] - '0')) * 8;
    
    int piece = getPieceOnSquare(board, from);

    if (piece == no_piece) {
        writeToLogFile("Tried to move piece that doesn't exist:", move);
    }

    // if theres a promotion piece
    int promotionPiece = no_piece;
    if (move.length() == 5) {
        switch (move[4]) {
            case 'r':
                promotionPiece = r;
                break;
            case 'b':
                promotionPiece = b;
                break;
            case 'q':
                promotionPiece = q;
                break;
            case 'n':
                promotionPiece = n;
                break;
            case 'R':
                promotionPiece = R;
                break;
            case 'B':
                promotionPiece = B;
                break;
            case 'Q':
                promotionPiece = Q;
                break;
            case 'N':
                promotionPiece = N;
                break;
        }
    }
    
    bool castling = false;
    bool doublePush = false;
    if (piece == k || piece == K) {
        if (abs(to - from) == 2) {
            castling = true;
        }
    } else if (piece == p || piece == P) {
        if (abs(from - to) == 16) {
            doublePush = true;
        }
    }

    U64 encodedMove = encodeMove(
        from, 
        to, 
        piece, 
        getPieceOnSquare(board, to), 
        promotionPiece, 
        (to==board.en_passant_square), 
        castling,
        doublePush
        );

    ChessBoard copy = board;
    if (makeMove(board, encodedMove) == false) {
        writeToLogFile("Tried to parse illegal move:", move);
        board = copy;
    }
}


void parseMoves(ChessBoard &board, const std::string &moves) {
    writeToLogFile("Parsing moves:", moves);
    std::stringstream ss(moves);
    std::string move;

    while (ss >> move) {
        parseMove(board, move);
    }
}

int main(int argc, char **argv) {
    clearLogs();

    // ChessBoard board = createBoardFromFen("r3k2r/8/8/5r2/6R1/8/8/R3K2R w KQkq - 0 1");

    // printBoard(board);

    // parseMoves(board, "e1g1");

    // Moves moves;
    // generateMoves(board, moves);

    // printBoard(board);

    // printMoves(moves);

    if (argc > 2) {
        
        // set the position
        ChessBoard board = createBoardFromFen(argv[2]);

        int depth = atoi(argv[1]);

        if (argc > 3) {
            parseMoves(board, argv[3]);
        }

        perft(board, depth);

    } 

    return 0;
}