#pragma once
#include <vector>
#include "DataLog.h"
#include "Map.h"

class DataLogSystem {
public:
    void generate(const Map& map, float tileSize);
    void reset() { m_logs.clear(); }

    const std::vector<DataLog>& getLogs() const { return m_logs; }
    std::vector<DataLog>& getLogs() { return m_logs; }

private:
    std::vector<DataLog> m_logs;
};