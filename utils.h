#ifndef UTILS_H
#define UTILS_H

#include "engine.h"
#include <cstdint>
#include <random>
#include <unordered_set>

uint32_t random_sparse_64();

// pops bit at square
#define popBit(x, square) (x &= ~(1ULL << (square)))
// is faster than pop_bit
#define popLsb(bitboard) (bitboard &= (bitboard-1))
//Returns the bit on the square
#define getBit(bitboard, square) ((bitboard) & (1ULL << (square)))
//Turns the bit at square to 1
#define setBit(bitboard, square) ((bitboard) |= (1ULL << (square)))



constexpr uint32_t MOVE_FROM_SHIFT = 0;
constexpr uint32_t MOVE_TO_SHIFT = 6;
constexpr uint32_t PIECE_TYPE_SHIFT = 12;
constexpr uint32_t CAPTURE_FLAG_SHIFT = 16;
constexpr uint32_t PROMOTION_PIECE_SHIFT = 20;
constexpr uint32_t EN_PASSANT_FLAG_SHIFT = 24;
constexpr uint32_t CASTLING_SHIFT = 25;
constexpr uint32_t DOUBLE_PUSH_SHIFT = 26;

uint32_t encodeMove(int from, int to, int piece_type, int captured_piece, int promotion_piece, bool en_passant, bool castling, bool double_push);

int decodeMoveFrom(uint32_t move);

int decodeMoveTo(uint32_t move);

int decodePieceType(uint32_t move);

int decodeCapturePiece(uint32_t move);

int decodePromotionPiece(uint32_t move);

bool decodeEnPassantFlag(uint32_t move);

bool decodeCastling(uint32_t move);

bool decodeDoublePushFlag(uint32_t move);



#endif