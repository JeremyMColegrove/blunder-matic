#include "search.h"

constexpr int NUM_THREADS = 4;



struct TTEntry {
    int depth;
    int score;
    uint32_t best_move;
};

std::mutex tt_mtx;
std::unordered_map<U64, TTEntry> transposition_table;

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

    // Transposition table lookup
    {
        std::lock_guard<std::mutex> lock(tt_mtx);
        auto entry_iter = transposition_table.find(zobristHash(board));
        if (entry_iter != transposition_table.end()) {
            TTEntry &entry = entry_iter->second;
            if (entry.depth >= depth) {
                if (entry.score >= beta) {
                    pv.clear();
                    pv.push_back(entry.best_move);
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

    // Transposition table store
    {
        std::lock_guard<std::mutex> lock(tt_mtx);
        transposition_table[zobristHash(board)] = {depth, best_value, child_pv[0]};
    }

    pv = child_pv;
    return best_value;
}

void parallel_search(std::shared_ptr<ChessBoard> board, int depth, int alpha, int beta, int &result, std::vector<uint32_t> &pv) {
    result = negamax(*board, depth, alpha, beta, pv, true);
}

void search(ChessBoard &board, int depth) {

    writeToLogFile("Searching board with Zobrist", zobristHash(board), "depth", depth);

    Moves moves;
    generateMoves(board, moves);
    std::vector<std::thread> threads;
    std::vector<int> results(NUM_THREADS, 0);
    std::vector<std::vector<uint32_t>> pv_lines(NUM_THREADS);
   
    int best_score = -INT_MAX;
    std::vector<uint32_t> best_pv;
    std::mutex search_mtx;

    ChessBoard boardCopy = board;
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (i < moves.count) {
            // ChessBoard new_board = board;
            std::shared_ptr<ChessBoard> new_board = std::make_shared<ChessBoard>(board);
            if (makeMove(*new_board, moves.list[i]) == false) {
                board = boardCopy;
                continue;
            }
            
            threads.emplace_back(parallel_search, new_board, depth - 1, -INT_MAX, -best_score, std::ref(results[i]), std::ref(pv_lines[i]));
        }
    }

    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].join();
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
