#pragma once
#include <vector>
#include "Terminal.h"
#include "Map.h"

class TerminalSystem {
public:
    void generate(const Map& map, float tileSize);
    void update(float dt);
    void reset() { m_terminals.clear(); }

    bool isSignalRevealed() const;
    const std::vector<Terminal>& getTerminals() const { return m_terminals; }
    std::vector<Terminal>& getTerminals() { return m_terminals; }

private:
    std::vector<Terminal> m_terminals;
};