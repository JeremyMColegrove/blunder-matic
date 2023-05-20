#include "uci.h"

int main() {
    clearLogs();

    std::string line;
    ChessBoard board;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                        std::istream_iterator<std::string>{}};

        if (tokens.empty()) {
            continue;
        }

        if (tokens[0] == "uci") {
            std::cout << "id name blunder-matic" << std::endl;
            std::cout << "id author Jeremy Colegrove" << std::endl;

            // print the available options here
            // for example: "option name Hash type spin default 1 min 1 max 1024"

            std::cout << "uciok" << std::endl;
        } else if (tokens[0] == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (tokens[0] == "ucinewgame") {
            board = createBoardFromFen(STARTING_FEN);
        } else if (tokens[0] == "position") {
            // process the position command
            // "position startpos moves e2e4 e7e5"
            // or: "position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4 e7e5"
            int movesIndex = -1;
            if (tokens[1] == "startpos") {
                board = createBoardFromFen(STARTING_FEN);
                movesIndex = 2;
            } else if (tokens[1] == "fen") {
                std::string fen;
                int i = 2;
                while (tokens[i] != "moves" && i < tokens.size()) {
                    fen += tokens[i++] + " ";
                    movesIndex = i;
                }
                board = createBoardFromFen(fen);
            }
            if (tokens[movesIndex] == "moves") {
                for (int i = movesIndex + 1; i < tokens.size(); ++i) {
                    // add to repitition array
                }
            }
        } else if (tokens[0] == "go") {
            
        } else if (tokens[0] == "quit") {
            break;
        } else if (tokens[0] == "setoption") {
            std::string name, value;
            for (int i = 1; i < tokens.size(); ++i) {
                if (tokens[i] == "name") {
                    name = tokens[++i];
                } else if (tokens[i] == "value") {
                    value = tokens[++i];
                }
            }

            // set option here with 'name' and 'token'
        }
    }
}