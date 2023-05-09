#ifndef UTILS_H
#define UTILS_H

#include "engine.h"

constexpr uint64_t MOVE_FROM_SHIFT = 0;
constexpr uint64_t MOVE_TO_SHIFT = 6;
constexpr uint64_t PIECE_TYPE_SHIFT = 12;
constexpr uint64_t CAPTURE_FLAG_SHIFT = 15;
constexpr uint64_t PROMOTION_PIECE_SHIFT = 19;
constexpr uint64_t EN_PASSANT_FLAG_SHIFT = 23;
constexpr uint64_t CASTLING_SHIFT = 24;

uint64_t encode_move(int from, int to, int piece_type, int captured_piece, int promotion_piece, bool en_passant, int castling);

int decode_move_from(uint64_t move);

int decode_move_to(uint64_t move);

int decode_piece_type(uint64_t move);

int decode_capture_piece(uint64_t move);

int decode_promotion_piece(uint64_t move);

bool decode_en_passant_flag(uint64_t move);

int decode_castling(uint64_t move);

// Helper function to check if a bit is set at a given position
bool is_bit_set(uint64_t bitboard, int position);

void set_bit(U64 &bitboard, int position);

void clear_bit(U64 &bitboard, int position);


#endif