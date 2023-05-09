#include "engine.h"


// Move encoding (in bits):
//  0- 5: origin square (6 bits)
//  6-11: destination square (6 bits)
// 12-14: moving piece (3 bits)
// 15-17: captured piece (3 bits)
// 18-20: special move flag (3 bits)
constexpr uint64_t ENCODE_MOVE(int from, int to, int piece, int captured, MoveFlag flag) {
    return (static_cast<uint64_t>(from) |
            (static_cast<uint64_t>(to) << 6) |
            (static_cast<uint64_t>(piece) << 12) |
            (static_cast<uint64_t>(captured) << 15) |
            (static_cast<uint64_t>(flag) << 18));
}

constexpr int MOVE_FROM(uint64_t move) { return move & 0x3F; }
constexpr int MOVE_TO(uint64_t move) { return (move >> 6) & 0x3F; }
constexpr int MOVE_PIECE(uint64_t move) { return (move >> 12) & 0x07; }
constexpr int MOVE_CAPTURED(uint64_t move) { return (move >> 15) & 0x07; }
constexpr MoveFlag MOVE_FLAG(uint64_t move) { return static_cast<MoveFlag>((move >> 18) & 0x07); }


uint64_t *get_piece_bitboard(ChessBoard &board, int piece) {
    switch (piece) {
        case 0: return &board.white_pieces.pawns;
        case 1: return &board.white_pieces.knights;
        case 2: return &board.white_pieces.bishops;
        case 3: return &board.white_pieces.rooks;
        case 4: return &board.white_pieces.queens;
        case 5: return &board.white_pieces.kings;
        case 6: return &board.black_pieces.pawns;
        case 7: return &board.black_pieces.knights;
        case 8: return &board.black_pieces.bishops;
        case 9: return &board.black_pieces.rooks;
        case 10: return &board.black_pieces.queens;
        case 11: return &board.black_pieces.kings;
        default: return nullptr; // This should never happen if the piece value is valid
    }
}

// Helper function to check if a bit is set at a given position
bool is_bit_set(uint64_t bitboard, int position) {
    return (bitboard & (1ULL << position)) != 0;
}

void print_bitboard(uint64_t bitboard) {
    for (int i=0; i<64; i++)
    {
        if ((i & 7) == 0) {
            std::cout << std::endl;
            std::cout << 8-(i/8) << "\t\t";
        }
        if (is_bit_set(bitboard, i)) std::cout << "1  ";
        else std::cout << "0  ";
    }
    std::cout << "\n\n\t\ta  b  c  d  e  f  g  h" << std::endl;
}


void print_move(uint64_t move) {
    int from_square = MOVE_FROM(move);
    int to_square = MOVE_TO(move);
    int piece = MOVE_PIECE(move);
    int captured_piece = MOVE_CAPTURED(move);
    MoveFlag flag = MOVE_FLAG(move);

    std::cout << "Move: "
              << static_cast<char>(from_square % 8 + 'a')
              << static_cast<char>(8 - from_square / 8 + '0')
              << static_cast<char>(to_square % 8 + 'a')
              << static_cast<char>(8 - to_square / 8 + '0')
              << " (piece: " << piece
              << ", captured piece: " << captured_piece
              << ", flag: " << static_cast<int>(flag) << ")"
              << std::endl;
}

// Function to print the chess board
void print_board(ChessBoard& board) {
    std::array<std::string, 12> pieceSymbols = {
        "♟", "♞", "♝", "♜", "♛", "♚",  // Black pieces
        "♙", "♘", "♗", "♖", "♕", "♔", // White pieces
    };

    std::array<U64*, 12> pieceBitboards = {
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

    for (int row = 0; row < 8; row++) {
        std::cout << 8 - row << " ";
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


void set_bit(U64 &bitboard, int position) {
    bitboard |= (1ull << position);
}

void clear_bit(U64 &bitboard, int position) {
    bitboard &= ~(1ull << position);
}


void make_move(ChessBoard &board, uint64_t move) {
    int from_square = MOVE_FROM(move);
    int to_square = MOVE_TO(move);
    int piece = MOVE_PIECE(move);
    int captured_piece = MOVE_CAPTURED(move);
    MoveFlag flag = MOVE_FLAG(move);

    // Clear the moving piece from the origin square
    clear_bit(*get_piece_bitboard(board, piece), from_square);

    // Set the moving piece in the destination square
    set_bit(*get_piece_bitboard(board, piece), to_square);

    // Remove the captured piece if any
    if (captured_piece != -1) {
        clear_bit(*get_piece_bitboard(board, captured_piece), to_square);
    }

    // Handle special move flags (e.g., promotion, en passant, castling)
    if (flag != MoveFlag::NORMAL) {
        // Implement the logic for handling special moves
        switch (flag) {
            case MoveFlag::PROMOTION_QUEEN:
                std::cout << "ITS HERE" << std::endl;
                clear_bit(*get_piece_bitboard(board, piece), to_square);
                set_bit(*get_piece_bitboard(board, 5), to_square);
        }
    }

    // Update en passant square, castling rights, side to move, and move counters
    // ...

    // Swap side to move
    board.white_to_move = !board.white_to_move;
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
    ChessBoard board;

    board = create_board_from_fen("8/4P3/8/8/8/8/8/8 w - - 0 1");

    // Example move: e2e4 (White pawn moves from e2 to e4)
    int from_square = e7; // e2
    int to_square = e8; // e4
    int piece = 0; // White pawn
    int captured_piece = -1; // No piece captured
    MoveFlag flag = MoveFlag::PROMOTION_QUEEN; // Normal move

    uint64_t move = ENCODE_MOVE(from_square, to_square, piece, captured_piece, flag);
    print_move(move);
    make_move(board, move);

    print_board(board);

    return 0;
}