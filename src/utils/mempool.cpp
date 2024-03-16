#include "mempool.hpp"
#include "json.hpp"

using json = nlohmann::json;

uint32_t getLastBlock(String str) {
    json j = json::parse(str);

    return j[0]["height"].template get<int>();
}

int getBTCPrice(String str) {
    json j = json::parse(str);

    return j["USD"].template get<int>();
}