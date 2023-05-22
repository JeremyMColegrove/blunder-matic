#include "search.h"


// {hash, depth, value, best_move, cut};
struct TTEntry {
    U64 hash;
    int depth;
    int value;
    uint32_t move;
    bool cut;
};


// Declare the fixed size transposition table as an array
constexpr size_t TRANSPOSITION_TABLE_SIZE = 1 << 20; // 1 million entries
std::array<std::atomic<TTEntry>, TRANSPOSITION_TABLE_SIZE> transposition_table;

// create atomic variables for the search
std::atomic<size_t> searchNodes(0);
std::atomic<size_t> ttHits(0);

std::atomic<size_t> movetime(0);
std::atomic<bool> killSwitch(false);
std::chrono::time_point<std::chrono::steady_clock>  searchStart;

int quiescence(ChessBoard &board, int alpha, int beta, std::vector<uint32_t> &pv, int ply) {

    searchNodes ++;

    // if either king is in check, look at all of the moves
    bool inCheck = (isSquareAttacked(board, white, __builtin_ctzll(board.bitboards[k])) || isSquareAttacked(board, black, __builtin_ctzll(board.bitboards[K])));

    if (!inCheck) {
        int standPat = evaluate(board);

        if (standPat >= beta) {
            return beta;
        }
        if (alpha < standPat) {
            alpha = standPat;
        }
    }

    Moves moves;
    generateMoves(board, moves); // Assuming you have an optional flag for generating only capture moves in your generateMoves function

    ChessBoard boardCopy = board;
    int bestValue = -INF;
    uint32_t bestMove;
    bool legalMoveFound = false;
    for (size_t i = 0; i < moves.count; ++i) {
        uint32_t move = moves.list[i];

        // skip non-captures
        if (!inCheck && decodeCapturePiece(move) == no_piece)
            continue;

        
        // skip illegal moves
        if (makeMove(board, move) == false) {
            board = boardCopy;
            continue;
        }

        std::vector<uint32_t> child_pv;
        int value = -quiescence(board, -beta, -alpha, child_pv, ply + 1);

        legalMoveFound = true;

        board = boardCopy;

        if (value > bestValue) {
            bestValue = value;
            bestMove = move;
            pv = child_pv;
            pv.emplace(pv.begin(), bestMove);
        }

        if (value >= beta) {
            return beta;
        }
        if (value > alpha) {
            alpha = value;
        }
    }

    if (!legalMoveFound) {
        if (inCheck) {
            return -CHECKMATE + ply;
        } else {
            return 0;
        }
    }

    return bestValue;
}

bool kingInCheck(ChessBoard &board) {
    return isSquareAttacked(board, board.white_to_move?black:white, __builtin_ctzll(board.bitboards[board.white_to_move?K:k]));
}

// Add an entry to the transposition table.
void addTTEntry(U64 hash, int depth, int value, uint32_t best_move, bool cut) {
    TTEntry entry = {hash, depth, value, best_move, cut};
    transposition_table[hash % TRANSPOSITION_TABLE_SIZE].store(entry, std::memory_order_relaxed);
}

// Look up an entry in the transposition table.
std::optional<TTEntry> probeTT(U64 hash) {
    TTEntry entry = transposition_table[hash % TRANSPOSITION_TABLE_SIZE].load(std::memory_order_relaxed);
    if (entry.hash == hash) {
        return entry;
    } else {
        return std::nullopt;
    }
}

int negamax(ChessBoard &board, int depth, int alpha, int beta, std::vector<uint32_t> &pv, bool is_pv, int moveStartIndex, int moveEndIndex) {
    
    searchNodes ++;

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - searchStart).count();

    if (elapsed > movetime) {
        killSwitch = true;
        return evaluate(board);
    }

    if (depth == 0) {
        return evaluate(board);//quiescence(board, alpha, beta, pv, 1);
    }

    // thread friendly transposition table lookup
    std::optional<TTEntry> opt_entry = probeTT(board.hash);
    if (opt_entry.has_value() && opt_entry->depth >= depth) {
        if (opt_entry->cut) {
            if (opt_entry->value <= alpha) {
                ttHits ++;
                return alpha;
            }
        } else {
            if (opt_entry->value >= beta) {
                ttHits ++;
                return beta;
            }
        }
    }

    bool check = kingInCheck(board);

    if (check) depth ++;

    Moves moves;
    generateMoves(board, moves);

    int best_value = -INF;
    std::vector<uint32_t> child_pv;

    ChessBoard boardCopy = board;
    bool legalMoveFound = false, cut = false;
    for (size_t i = moveStartIndex; i < moves.count && i < moveEndIndex; ++i) {
        uint32_t move = moves.list[i];

        // skip illegal moves
        if (makeMove(board, move) == false) {
            board = boardCopy;
            continue;
        }

        legalMoveFound = true;
        std::vector<uint32_t> tmp_pv;

        int value;
        if (i == 0 || !is_pv) {
            value = -negamax(board, depth - 1, -beta, -alpha, tmp_pv, is_pv, 0, INF);
        } else {
            value = -negamax(board, depth - 1, -alpha - 1, -alpha, tmp_pv, false, 0, INF);

            if (alpha < value && value < beta) {
                value = -negamax(board, depth - 1, -beta, -alpha, tmp_pv, true, 0, INF);
            }
        }

        board = boardCopy;
    
        if (value > best_value) {
            best_value = value;
            child_pv = tmp_pv;
            child_pv.emplace(child_pv.begin(), move);

            if (value > alpha) {
                cut = false;
            }
        }

        alpha = std::max(alpha, value);
        if (alpha >= beta) {
            cut = true;
            break;
        }   
    }

    if (!legalMoveFound) {
        if (check) {
            return -CHECKMATE - depth;
        } else {
            return 0;
        }
    }

    if (!child_pv.empty()) {
        addTTEntry(board.hash, depth, best_value, child_pv[0], cut);
    }

    pv = child_pv;
    return best_value;
}

void parallel_search(std::shared_ptr<ChessBoard> board, int depth, int alpha, int beta, int &result, std::vector<uint32_t> &pv, int moveStartIndex, int moveEndIndex) {
    result = negamax(*board, depth, alpha, beta, pv, true, moveStartIndex, moveEndIndex);
}


void search(ChessBoard &board, int depth, size_t movetime_, size_t numThreads) {

    writeToLogFile("Searching depth", depth, "on", numThreads, "threads");

    movetime = movetime_;

    Moves moves;

    generateMoves(board, moves);

    int best_score = -INF;
    std::mutex search_mtx;

    int moves_per_thread = moves.count / numThreads;
    int remaining_moves = moves.count % numThreads;
    std::vector<uint32_t> best_pv;

    searchStart = std::chrono::steady_clock::now();

    killSwitch = false;

    for (int currDepth = 1; currDepth <= depth; currDepth ++) {

        // reset all counters
        searchNodes = 0, ttHits = 0;

        std::vector<std::thread> threads;
        std::vector<int> results(numThreads, 0);
        std::vector<std::vector<uint32_t>> pv_lines(numThreads);

        for (int i = 0; i < numThreads; ++i) {
            int start_move = i * moves_per_thread;
            int end_move = (i + 1) * moves_per_thread;

            if (i == numThreads - 1) {
                // Assign remaining moves to the last thread
                end_move += remaining_moves;
            }

            std::shared_ptr<ChessBoard> new_board = std::make_shared<ChessBoard>(board);
            threads.emplace_back(parallel_search, new_board, currDepth, -INF, INF, std::ref(results[i]), std::ref(pv_lines[i]), start_move, end_move);
        }

        for (size_t i = 0; i < threads.size(); ++i) {
            threads[i].join();

            std::unique_lock<std::mutex> lock(search_mtx);
            if (results[i] > best_score) {
                best_score = results[i];
                best_pv = pv_lines[i];
            }
            lock.unlock();
        }

        if (killSwitch) break;

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - searchStart).count();

        if (abs(best_score) > CHECKMATE - 2000) {
            std::cout << "info score mate " << (best_pv.size() / 2) + 1 << " depth " << currDepth << " nodes " << searchNodes << " time " << elapsed << " pv ";
        } else {
            std::cout << "info score cp " << best_score << " depth " << currDepth << " nodes " << searchNodes << " time " << elapsed << " pv ";
        } 

        printPVLine(best_pv);
    }

    // finally at the end, print the move
    std::cout << "bestmove ";
    printMove(best_pv[0]);
    std::cout << std::endl;
}
