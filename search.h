#ifndef SEARCH_H
#define SEARCH_H

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <unordered_map>
#include "moves.h"
#include "engine.h"
#include "util.h"
#include "logger.h"
#include "printers.h"
#include "evaluation.h"
#include <atomic>

#define CHECKMATE 50000
#define INF 999999
void search(ChessBoard &board, int depth, size_t numThreads);

bool kingInCheck(ChessBoard &board);
#endif