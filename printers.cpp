#include "printers.h"

const char ascii_pieces[] = "PNBRQKpnbrqk-";

void printBitboard(uint64_t bitboard) {
    for (int i=0; i<64; i++)
    {
        if ((i & 7) == 0) {
            std::cout << std::endl;
            std::cout << 8-(i/8) << "\t\t";
        }
        if (get_bit(bitboard, i)) std::cout << "1  ";
        else std::cout << "0  ";
    }
    std::cout << "\n\n\t\ta  b  c  d  e  f  g  h" << std::endl;
}

void printMoveHeader() {
    printf("from\tto\tpiece\tcapturing\tpromoted\tenp\tcastling\tdouble_push\n");
}



void printMove(uint64_t move) {
    int from_square = decode_move_from(move);
    int to_square = decode_move_to(move);
    int piece = decode_piece_type(move);
    int captured_piece = decode_capture_piece(move);
    int castling = decode_castling(move);
    int enpassant = decode_en_passant_flag(move);
    int promotion_piece = decode_promotion_piece(move);
    bool double_push = decode_double_push_flag(move);
    
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

void printMoves(Moves &moves) {
    printMoveHeader();

    for (int i=0; i<moves.count; i++) {
        printMove(moves.list[i]);
    }
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
                if (get_bit(board.bitboards[i], position)) {
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
    if (board.en_passant_square != -1) {
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