#include "mempool.hpp"

String getBTCPrice(String str) {
    int index = str.indexOf("\"USD\":");

    String price;

    if (index == -1) {
        return "";
    } else {
        index += 6;
        int i = 0;
        while (isDigit(str[index + i])) {
            price += str[index + i];
            i++;
        }
    }

    return price;
}

String getLastBlock(String str) {
    int index = str.indexOf("\"height\":");

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