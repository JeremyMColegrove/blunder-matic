#include "transposition_table.h"

void TranspositionTable::addTranspositionTableEntry(uint64_t hash, int depth, int value, uint32_t best_move, bool cut) {
    TTEntry entry = {hash, depth, value, best_move, cut};
    transposition_table[hash % TRANSPOSITION_TABLE_SIZE].store(entry, std::memory_order_relaxed);
}

void TranspositionTable::clear() {

}

std::optional<TTEntry> TranspositionTable::probeTranspositionTable(uint64_t hash) {
    TTEntry entry = transposition_table[hash % TRANSPOSITION_TABLE_SIZE].load(std::memory_order_relaxed);
    if (entry.hash == hash) {
        return entry;
    } else {
        return std::nullopt;
    }
}
