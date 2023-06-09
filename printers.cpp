#include "printers.h"


void printBitboard(uint32_t bitboard) {
    for (int i=0; i<64; i++)
    {
        if ((i & 7) == 0) {
            std::cout << std::endl;
            std::cout << 8-(i/8) << "\t\t";
        }
        if (getBit(bitboard, i)) std::cout << "1  ";
        else std::cout << "0  ";
    }
    std::cout << "\n\n\t\ta  b  c  d  e  f  g  h" << std::endl;
}

void printMoveHeader() {
    printf("from\tto\tpiece\tcapturing\tpromoted\tenp\tcastling\tdouble_push\n");
}

std::string squaretoCoordinate(int square) {
    std::string result = "";
    result += static_cast<char>(square % 8 + 'a');
    result += static_cast<char>(8 - square / 8 + '0');
    return result;
}



void printMoveDetailed(uint32_t move) {
    int from_square = decodeMoveFrom(move);
    int to_square = decodeMoveTo(move);
    int piece = decodePieceType(move);
    int captured_piece = decodeCapturePiece(move);
    int castling = decodeCastling(move);
    int enpassant = decodeEnPassantFlag(move);
    int promotion_piece = decodePromotionPiece(move);
    bool double_push = decodeDoublePushFlag(move);
    
    printf("%c%c\t%c%c\t%c\t%c\t\t%c\t\t%d\t%d\t\t%d\n", 
        static_cast<char>(from_square % 8 + 'a'),
        static_cast<char>(8 - from_square / 8 + '0'),
        static_cast<char>(to_square % 8 + 'a'),
        static_cast<char>(8 - to_square / 8 + '0'),
        ascii_pieces[piece],
        ascii_pieces[captured_piece],
        ascii_pieces[promotion_piece],
        enpassant,
        castling,
        double_push);
}

void printMovesDetailed(Moves &moves) {
    printMoveHeader();

    for (int i=0; i<moves.count; i++) {
        printMoveDetailed(moves.list[i]);
    }

    std::cout << "Moves: " << moves.count << std::endl;
}

void printMove(uint32_t move) {
    std::cout << squaretoCoordinate(decodeMoveFrom(move));
    std::cout << squaretoCoordinate(decodeMoveTo(move));
    int promotion = decodePromotionPiece(move);
    if (promotion != no_piece)
        std::cout << ascii_pieces[promotion];
}

void printPVLine(std::vector<uint32_t> pv) {
    for (const auto &move : pv) {
        printMove(move);
        std::cout << " ";
    }
    std::cout << std::endl;
}
// Function to print the chess board
void printBoard(ChessBoard& board) {
    std::array<std::string, 12> pieceSymbols = {
        "♟", "♞", "♝", "♜", "♛", "♚",  // Black pieces
        "♙", "♘", "♗", "♖", "♕", "♔", // White pieces
    };

    for (int row = 0; row < 8; row++) {
        std::cout << 8 - row << " ";
        for (int col = 0; col < 8; ++col) {
            int position = row * 8 + col;
            bool pieceFound = false;

            for (int i = 0; i < 12; ++i) {
                if (getBit(board.bitboards[i], position)) {
                    std::cout << ascii_pieces[i] << " ";
                    pieceFound = true;
                    break;
                }
            }

            if (!pieceFound) {
                std::cout << ". ";
            }
        }
        std::cout << std::endl;
    }

    std::cout << "  a b c d e f g h" << std::endl;

    // Print additional variables
    std::cout << "Side to move: " << (board.white_to_move ? "White" : "Black") << std::endl;
    std::cout << "Castling rights: ";
    if (board.castling_rights) {
        if (board.castling_rights & 1) std::cout << "K";
        if (board.castling_rights & 2) std::cout << "Q";
        if (board.castling_rights & 4) std::cout << "k";
        if (board.castling_rights & 8) std::cout << "q";
    } else {
        std::cout << "-";
    }
    std::cout << std::endl;

    std::cout << "En passant square: ";
    if (board.en_passant_square != no_square) {
        int file = board.en_passant_square % 8;
        int rank = (8 - board.en_passant_square / 8);
        std::cout << static_cast<char>('a' + file) << (rank);
    } else {
        std::cout << "-";
    }
    std::cout << std::endl;

    std::cout << "Half-move counter: " << board.half_move_counter << std::endl;
    std::cout << "Full-move counter: " << board.full_move_counter << std::endl;
}