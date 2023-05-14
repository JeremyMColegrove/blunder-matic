#include "utils.h"

std::random_device rd;
std::mt19937_64 gen(rd());
// Generate a sparse 64-bit integer
uint64_t random_sparse_64() {
    uint64_t r = 0;
    for (int i = 0; i < 64; i += 5) {
        r |= static_cast<uint64_t>(1) << (gen() % 64);
    }
    return r;
}
uint64_t encodeMove(int from, int to, int piece_type, int captured_piece, int promotion_piece, bool en_passant, bool castling, bool double_push) {
        return (static_cast<uint64_t>(from) << MOVE_FROM_SHIFT) |
           (static_cast<uint64_t>(to) << MOVE_TO_SHIFT) |
           (static_cast<uint64_t>(piece_type) << PIECE_TYPE_SHIFT) |
           (static_cast<uint64_t>(captured_piece) << CAPTURE_FLAG_SHIFT) |
           (static_cast<uint64_t>(promotion_piece) << PROMOTION_PIECE_SHIFT) |
           (static_cast<uint64_t>(en_passant ? 1 : 0) << EN_PASSANT_FLAG_SHIFT) |
           (static_cast<uint64_t>(castling) << CASTLING_SHIFT) |
           (static_cast<uint64_t>(double_push) << DOUBLE_PUSH_SHIFT);
}

int decodeMoveFrom(uint64_t move) {
    return (move >> MOVE_FROM_SHIFT) & 0x3F;
}

int decodeMoveTo(uint64_t move) {
    return (move >> MOVE_TO_SHIFT) & 0x3F;
}

int decodePieceType(uint64_t move) {
    return (move >> PIECE_TYPE_SHIFT) & 0xF;
}

int decodeCapturePiece(uint64_t move) {
    return (move >> CAPTURE_FLAG_SHIFT) & 0xF;
}

int decodePromotionPiece(uint64_t move) {
    return (move >> PROMOTION_PIECE_SHIFT) & 0xF;
}

bool decodeEnPassantFlag(uint64_t move) {
    return (move >> EN_PASSANT_FLAG_SHIFT) & 0x1;
}

bool decodeCastling(uint64_t move) {
    return (move >> CASTLING_SHIFT) & 0x1;
}

bool decodeDoublePushFlag(uint64_t move) {
    return (move >> DOUBLE_PUSH_SHIFT) & 0x1;
}
