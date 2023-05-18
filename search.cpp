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

bool kingInCheck(ChessBoard &board) {
    return isSquareAttacked(board, board.white_to_move?black:white, __builtin_ctzll(board.bitboards[board.white_to_move?K:k]));
}

int negamax(ChessBoard &board, int depth, int alpha, int beta, std::vector<uint32_t> &pv, bool is_pv, int moveStartIndex, int moveEndIndex) {
    if (depth == 0) {
        return quiescence(board, alpha, beta);
    }

    // U64 hash = zobristHash(board);
    // // Transposition table lookup
    // // When you read from the transposition table, also use a modulo operation
    // {
    //     std::lock_guard<std::mutex> lock{tt_mtx};
    //     TTEntry &entry = transposition_table[hash % TRANSPOSITION_TABLE_SIZE];
    //     if (entry.hash == hash) {
    //         if (entry.depth >= depth) {
    //             if (entry.score >= beta) {
    //                 if (entry.best_move != 0xFFFFFFFF) {
    //                     pv.clear();
    //                     pv.push_back(entry.best_move);
    //                 }
    //                 return entry.score;
    //             }
    //             alpha = std::max(alpha, entry.score);
    //         }
    //     }
    // }

    bool check = kingInCheck(board);

    if (check) depth ++;

    Moves moves;
    generateMoves(board, moves);

    int best_value = -INF;
    std::vector<uint32_t> child_pv;

    ChessBoard boardCopy = board;
    bool legalMoveFound = false;
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
        }

        alpha = std::max(alpha, value);
        if (alpha >= beta) {
            break;
        }
    }

    if (!legalMoveFound) {
        if (check) {
            if (board.white_to_move &&  __builtin_ctzll(board.bitboards[q])==g6) {
                printBoard(board);
            }
            return -CHECKMATE - depth;
        } else {
            return 0;
        }
    }

    // When you store a value in the transposition table, use a modulo operation
    // if (!child_pv.empty()) {
    //     std::lock_guard<std::mutex> lock{tt_mtx};
    //     uint32_t best_move = (child_pv.empty()) ? 0xFFFFFFFF : child_pv[0];
    //     transposition_table[hash % TRANSPOSITION_TABLE_SIZE] = {depth, best_value, hash, best_move};
    // }

    pv = child_pv;
    return best_value;
}

void parallel_search(std::shared_ptr<ChessBoard> board, int depth, int alpha, int beta, int &result, std::vector<uint32_t> &pv, int moveStartIndex, int moveEndIndex) {
    result = negamax(*board, depth, alpha, beta, pv, true, moveStartIndex, moveEndIndex);
}

void search(ChessBoard &board, int depth, size_t numThreads) {

    writeToLogFile("Searching depth", depth, "on", numThreads, "threads");

    Moves moves;
    generateMoves(board, moves);

    std::vector<std::thread> threads;
    std::vector<int> results(numThreads, 0);
    std::vector<std::vector<uint32_t>> pv_lines(numThreads);
   
    int best_score = -INF;
    std::vector<uint32_t> best_pv;
    std::mutex search_mtx;

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
            std::shared_ptr<ChessBoard> new_board = std::make_shared<ChessBoard>(board);
            threads.emplace_back(parallel_search, new_board, depth, -INF, INF, std::ref(results[i]), std::ref(pv_lines[i]), start_move, end_move);
        }
    }


    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].join();
        writeToLogFile("Thread", i, "finished");

        std::unique_lock<std::mutex> lock(search_mtx);
        if (results[i] > best_score) {
            best_score = results[i];
            best_pv = pv_lines[i];
        }
        lock.unlock();
    }


    if (best_score < -CHECKMATE + 2000) {
        std::cout << "info score mate " << (best_score + CHECKMATE) << " depth " << depth << " pv ";
        // printf("info transpositions %d ttp: %.4f score mate %d depth %d nodes %d q_nodes %d time %ld pv ", cache_hit, ((float)cache_hit)/nodes, -(score + CHECKMATE) / 2 - 1, depth + 1, nodes, q_nodes, elapsed);
    } else if (best_score > CHECKMATE - 2000) {
        std::cout << "info score mate " << -(CHECKMATE - best_score) << " depth " << depth << " pv ";

        // printf("info transpositions %d ttp: %.4f score mate %d depth %d nodes %d q_nodes %d time %ld pv ", cache_hit,((float)cache_hit)/nodes, (CHECKMATE - score) / 2 + 1, depth + 1, nodes, q_nodes, elapsed);
    } else {
        std::cout << "info score cp " << best_score << " depth " << depth << " pv ";
    } 


    for (const auto &move : best_pv) {
        std::cout << squaretoCoordinate(decodeMoveFrom(move));
        std::cout << squaretoCoordinate(decodeMoveTo(move));
        int promotion = decodePromotionPiece(move);
        if (promotion != no_piece)
            std::cout << ascii_pieces[promotion];
        std::cout << " ";
    }
    std::cout << std::endl;
}
