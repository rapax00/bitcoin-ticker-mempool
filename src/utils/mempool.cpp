#include "mempool.hpp"
#include "json.hpp"

using json = nlohmann::json;

String lastBlock(String str) {
    Serial.printf("Dentro funcion\n");

    json j = json::parse(str);

    auto lastBlock = j["blocks"].template get<std::vector<json>>().back();

    Serial.printf("Last block: %s\n", lastBlock.dump().c_str());

    return "";
}