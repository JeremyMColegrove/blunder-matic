#ifndef UTILS_H
#define UTILS_H

#include "engine.h"
#include <cstdint>
#include <random>
#include <unordered_set>

uint64_t random_sparse_64();

// pops bit at square
#define pop_bit(x, square) (x &= ~(1ULL << (square)))
// is faster than pop_bit
#define pop_lsb(bitboard) (bitboard &= (bitboard-1))
//Returns the bit on the square
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
//Turns the bit at square to 1
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))


constexpr uint64_t MOVE_FROM_SHIFT = 0;
constexpr uint64_t MOVE_TO_SHIFT = 6;
constexpr uint64_t PIECE_TYPE_SHIFT = 12;
constexpr uint64_t CAPTURE_FLAG_SHIFT = 16;
constexpr uint64_t PROMOTION_PIECE_SHIFT = 20;
constexpr uint64_t EN_PASSANT_FLAG_SHIFT = 24;
constexpr uint64_t CASTLING_SHIFT = 25;
constexpr uint64_t DOUBLE_PUSH_SHIFT = 26;

uint64_t encode_move(int from, int to, int piece_type, int captured_piece, int promotion_piece, bool en_passant, int castling, bool double_push);

int decode_move_from(uint64_t move);

int decode_move_to(uint64_t move);

int decode_piece_type(uint64_t move);

int decode_capture_piece(uint64_t move);

int decode_promotion_piece(uint64_t move);

bool decode_en_passant_flag(uint64_t move);

int decode_castling(uint64_t move);

bool decode_double_push_flag(uint64_t move);



#endif