#include "search.h"




struct TTEntry {
    int depth;
    int score;
    U64 hash;
    uint32_t best_move;
};

std::mutex tt_mtx;
// Declare the fixed size transposition table as an array
constexpr size_t TRANSPOSITION_TABLE_SIZE = 1 << 20; // 1 million entries
std::array<TTEntry, TRANSPOSITION_TABLE_SIZE> transposition_table;




int quiescence(ChessBoard &board, int alpha, int beta) {


    int standPat = evaluate(board);

    if (standPat >= beta) {
        return beta;
    }
    if (alpha < standPat) {
        alpha = standPat;
    }

    Moves moves;
    generateMoves(board, moves); // Assuming you have an optional flag for generating only capture moves in your generateMoves function

    ChessBoard boardCopy = board;
    for (size_t i = 0; i < moves.count; ++i) {
        uint32_t move = moves.list[i];

        // skip non-captures
        if (decodeCapturePiece(move) == no_piece)
            continue;

        // skip illegal moves
        if (makeMove(board, move) == false) {
            board = boardCopy;
            continue;
        }

        int value = -quiescence(board, -beta, -alpha);

        board = boardCopy;

        if (value >= beta) {
            return beta;
        }
        if (value > alpha) {
            alpha = value;
        }
    }
    return alpha;
}


int negamax(ChessBoard &board, int depth, int alpha, int beta, std::vector<uint32_t> &pv, bool is_pv) {
    if (depth == 0) {
        return quiescence(board, alpha, beta);
    }

    U64 hash = zobristHash(board);
    // Transposition table lookup
    {
        // When you read from the transposition table, also use a modulo operation
    {
        std::lock_guard<std::mutex> lock{tt_mtx};
        TTEntry &entry = transposition_table[hash % TRANSPOSITION_TABLE_SIZE];
        if (entry.hash == hash) {
            if (entry.depth >= depth) {
                if (entry.score >= beta) {
                    if (entry.best_move != 0xFFFFFFFF) {
                        pv.clear();
                        pv.push_back(entry.best_move);
                    }
                    return entry.score;
                }
                alpha = std::max(alpha, entry.score);
            }
        }
    }

    Moves moves;
    generateMoves(board, moves);

    int best_value = INT_MIN;
    std::vector<uint32_t> child_pv;

    ChessBoard boardCopy = board;
    for (size_t i = 0; i < moves.count; ++i) {
        uint32_t move = moves.list[i];

        // skip illegal moves
        if (makeMove(board, move) == false) {
            board = boardCopy;
            continue;
        }

        std::vector<uint32_t> tmp_pv;

        int value;
        if (i == 0 || !is_pv) {
            value = -negamax(board, depth - 1, -beta, -alpha, tmp_pv, is_pv);
        } else {
            value = -negamax(board, depth - 1, -alpha - 1, -alpha, tmp_pv, false);

            if (alpha < value && value < beta) {
                value = -negamax(board, depth - 1, -beta, -alpha, tmp_pv, true);
            }
        }

        board = boardCopy;

        if (value > best_value) {
            best_value = value;
            child_pv = tmp_pv;
            child_pv.emplace(child_pv.begin(), move);
        }

        alpha = std::max(alpha, value);
        if (alpha >= beta) {
            break;
        }
    }

    // When you store a value in the transposition table, use a modulo operation
    U64 hash = zobristHash(board);
    {
        std::lock_guard<std::mutex> lock{tt_mtx};
        uint32_t best_move = (child_pv.empty()) ? 0xFFFFFFFF : child_pv[0];
        transposition_table[hash % TRANSPOSITION_TABLE_SIZE] = {depth, best_value, hash, best_move};
    }

    pv = child_pv;
    return best_value;
}
}
void parallel_search(ChessBoard board, int depth, int alpha, int beta, int &result, std::vector<uint32_t> &pv) {
    result = negamax(board, depth, alpha, beta, pv, true);
}

void search(ChessBoard &board, int depth, size_t numThreads) {

    writeToLogFile("Searching depth", depth, "on", numThreads, "threads");

    Moves moves;
    generateMoves(board, moves);

    std::vector<std::thread> threads;
    std::vector<int> results(numThreads, 0);
    std::vector<std::vector<uint32_t>> pv_lines(numThreads);
   
    int best_score = -INT_MAX;
    std::vector<uint32_t> best_pv;
    std::mutex search_mtx;

    ChessBoard boardCopy = board;
    int moves_per_thread = moves.count / numThreads;
    int remaining_moves = moves.count % numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int start_move = i * moves_per_thread;
        int end_move = (i + 1) * moves_per_thread;

        if (i == numThreads - 1) {
            // Assign remaining moves to the last thread
            end_move += remaining_moves;
        }

        if (start_move < moves.count) {
            threads.emplace_back([&, start_move, end_move]() {
                writeToLogFile("Thread", i, "starting, searching", end_move-start_move, "moves");
                for (int j = start_move; j < end_move; ++j) {
                    ChessBoard new_board = board;
                    if (makeMove(new_board, moves.list[j]) == false) {
                        continue;
                    }
                    
                    int result;
                    std::vector<uint32_t> pv;
                    parallel_search(new_board, depth - 1, -INT_MAX, -best_score, result, pv);

                    std::unique_lock<std::mutex> lock(search_mtx);
                    if (result > best_score) {
                        best_score = result;
                        best_pv = pv;
                        best_pv.emplace(best_pv.begin(), moves.list[j]);
                    }
                    lock.unlock();
                }
            });
        }
    }


    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].join();
        writeToLogFile("Thread", i, "finished");

        std::unique_lock<std::mutex> lock(search_mtx);
        if (results[i] > best_score) {
            best_score = results[i];
            best_pv = pv_lines[i];
            best_pv.emplace(best_pv.begin(), moves.list[i]);
        }
        lock.unlock();
    }

    std::cout << "Best score: " << best_score << std::endl;
    writeToLogFile("Best score found:", best_score);
    std::cout << "Best PV: ";
    for (const auto &move : best_pv) {
        std::cout << squaretoCoordinate(decodeMoveFrom(move));
        std::cout << squaretoCoordinate(decodeMoveFrom(move));
        int promotion = decodePromotionPiece(move);
        if (promotion != no_piece)
            std::cout << ascii_pieces[promotion];
        std::cout << " ";
    }
    std::cout << std::endl;
}
