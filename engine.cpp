#include "engine.h"
#include "utils.h"
#include "printers.h"

 int char_pieces[128] = {
        ['P'] = P, ['N'] = N, ['B'] = B, ['R'] = R, ['Q'] = Q, ['K'] = K,
        ['p'] = p, ['n'] = n, ['b'] = b, ['r'] = r, ['q'] = q, ['k'] = k,
    };

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
        board.en_passant_square = rank * 8 + file;
    } else {
        board.en_passant_square = no_square;
    }

    // Parse 50-move counter
    fenStream >> board.half_move_counter;

    // Parse full-move counter
    fenStream >> board.full_move_counter;

    return board;
}

void make_move(ChessBoard &board, uint64_t move) {
    int from_square = decode_move_from(move);
    int to_square = decode_move_to(move);
    int piece = decode_piece_type(move);
    int captured_piece = decode_capture_piece(move);
    int castling = decode_castling(move);
    int enpassant = decode_en_passant_flag(move);
    int promotion_piece = decode_promotion_piece(move);
    bool double_push = decode_double_push_flag(move);

    
    // Clear the moving piece from the origin square
    clear_bit(board.bitboards[piece], from_square);


    // Remove the captured piece if any
    if (captured_piece != no_piece) {
        clear_bit(board.bitboards[captured_piece], to_square);
    }

    
    // Handle special move flags (e.g., promotion, en passant, castling)
    if (promotion_piece != no_piece) {
        set_bit(board.bitboards[promotion_piece], to_square);
    } else if (enpassant) {
        // En passant special move
        if (board.white_to_move) {
            clear_bit(board.bitboards[p], to_square + 8);
        } else {
            clear_bit(board.bitboards[P], to_square - 8);
        }
        // Set the moving piece in the destination square
        set_bit(board.bitboards[piece], to_square);
    } else {
        // Set the moving piece in the destination square
        set_bit(board.bitboards[piece], to_square);
    }

    // enpassant
    if (double_push) {
                std::cout << "Double push is true" << std::endl;

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
        std::cout << "Castling is true" << std::endl;
            switch (to_square) {
                case g1:
                    clear_bit(board.bitboards[R], h1);
                    set_bit(board.bitboards[R], f1);
                    break;
                case c1:
                    clear_bit(board.bitboards[R], a1);
                    set_bit(board.bitboards[R], d1);
                    break;
                case g8:
                    clear_bit(board.bitboards[r], h8);
                    set_bit(board.bitboards[r], f8);
                    break;
                case c8:
                    clear_bit(board.bitboards[r], a8);
                    set_bit(board.bitboards[r], d8);
                    break;
            }
        }

    board.castling_rights &= castling_rights[from_square];
    board.castling_rights &= castling_rights[to_square];

    // Swap side to move
    board.white_to_move = !board.white_to_move;
}



int main() {
    ChessBoard board = create_board_from_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - - 0 1");

    uint64_t move = encode_move(e8, f8, k, no_piece, no_piece, 0, 0, 0);

    print_move(move);

    make_move(board, move);

    print_board(board);

    return 0;
}