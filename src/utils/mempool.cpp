#include "mempool.hpp"

#define SERIALDEBUG

String getBTCPrice(String str) {
    int index = str.indexOf("\"price\":"); //"price": "67618.73",

    String price;

    if (index == -1) {
        return "";
    } else {
        index += 9;
        int i = 0;
        while (true) {
            if (isDigit(str[index + i])) {
                price += str[index + i];
            } else if (str[index + i] == '.') {
                price += ",";
            } else {
                break;
            }
            i++;
        }
    }

    return price;
}

String getLastBlock(String str) {
    std::string cStr = str.c_str();
    int index = cStr.rfind("\"height\":");

    String lastBlock;

    if (index == -1) {
        return "";
    } else {
        index += 9;
        int i = 0;
        while (isDigit(str[index + i])) {
            lastBlock += str[index + i];
            i++;
        }
    }

    return lastBlock;
}