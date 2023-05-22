#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H


#include <cstdint>
#include <iostream>

struct TTEntry {
    uint64_t hash;
    int depth;
    int value;
    uint32_t move;
    bool cut;
};

class TranspositionTable {
    public:
        void addTranspositionTableEntry(uint64_t hash, int depth, int value, uint32_t best_move, bool cut);
        // Add an entry to the transposition table.
        void clear();

        std::optional<TTEntry> probeTranspositionTable(uint64_t hash);

    private:
        // Declare the fixed size transposition table as an array
        static const size_t TRANSPOSITION_TABLE_SIZE = 1 << 20; // 1 million entries
        std::array<std::atomic<TTEntry>, TRANSPOSITION_TABLE_SIZE> transposition_table;
};

#endif