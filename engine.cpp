#include "engine.h"


// Helper function to check if a bit is set at a given position
bool is_bit_set(uint64_t bitboard, int position) {
    return (bitboard & (1ULL << position)) != 0;
}

// Function to print the chess board
void print_board(ChessBoard& board) {
    std::array<std::string, 12> pieceSymbols = {
        "♙", "♘", "♗", "♖", "♕", "♔", // White pieces
        "♟", "♞", "♝", "♜", "♛", "♚"  // Black pieces
    };

    std::array<U64 *, 12> pieceBitboards = {
        &board.white_pieces.pawns,
        &board.white_pieces.knights,
        &board.white_pieces.bishops,
        &board.white_pieces.rooks,
        &board.white_pieces.queens,
        &board.white_pieces.kings,
        &board.black_pieces.pawns,
        &board.black_pieces.knights,
        &board.black_pieces.bishops,
        &board.black_pieces.rooks,
        &board.black_pieces.queens,
        &board.black_pieces.kings
    };
for (int row = 7; row >= 0; --row) {
        std::cout << (row + 1) << " ";
        for (int col = 0; col < 8; ++col) {
            int position = row * 8 + col;
            bool pieceFound = false;

            for (int i = 0; i < 12; ++i) {
                if (is_bit_set(*pieceBitboards[i], position)) {
                    std::cout << pieceSymbols[i] << " ";
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
        int rank = board.en_passant_square / 8;
        std::cout << static_cast<char>('a' + file) << (rank + 1);
    } else {
        std::cout << "-";
    }
    std::cout << std::endl;

    std::cout << "Half-move counter: " << board.half_move_counter << std::endl;
    std::cout << "Full-move counter: " << board.full_move_counter << std::endl;

}

ChessBoard create_board_from_fen(const std::string& fen) {
    ChessBoard board = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::istringstream fenStream(fen);
    std::string boardState;
    fenStream >> boardState;

    int position = 0;
    for (char c : boardState) {
        if (c >= '1' && c <= '8') {
            position += c - '0';
        } else if (c != '/') {
            switch (c) {
                case 'P': board.white_pieces.pawns   |= 1ULL << position; break;
                case 'N': board.white_pieces.knights |= 1ULL << position; break;
                case 'B': board.white_pieces.bishops |= 1ULL << position; break;
                case 'R': board.white_pieces.rooks   |= 1ULL << position; break;
                case 'Q': board.white_pieces.queens  |= 1ULL << position; break;
                case 'K': board.white_pieces.kings   |= 1ULL << position; break;
                case 'p': board.black_pieces.pawns   |= 1ULL << position; break;
                case 'n': board.black_pieces.knights |= 1ULL << position; break;
                case 'b': board.black_pieces.bishops |= 1ULL << position; break;
                case 'r': board.black_pieces.rooks   |= 1ULL << position; break;
                case 'q': board.black_pieces.queens  |= 1ULL << position; break;
                case 'k': board.black_pieces.kings   |= 1ULL << position; break;
            }
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
        int rank = enPassant[1] - '1';
        board.en_passant_square = rank * 8 + file;
    } else {
        board.en_passant_square = -1;
    }

    // Parse 50-move counter
    fenStream >> board.half_move_counter;

    // Parse full-move counter
    fenStream >> board.full_move_counter;

    return board;
}

int main() {
    std::string fen = "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 3";
    ChessBoard board = create_board_from_fen(fen);
    print_board(board);
    return 0;
}