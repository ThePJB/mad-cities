#pragma once

const auto empty_sentinel = -2147483648;

template<unsigned int N>
struct fixed_set {
    int contents[N] = {empty_sentinel};

    bool contains(int x) {
        for (int i = 0; i < N; i++) {
            if (contents[i] == x) {
                return true;
            }
        }
        return false;
    }

    void put(int x) {
        if (contains(x)) {
            return;
        }

        for (int i = 0; i < N; i++) {
            if (contents[i] == empty_sentinel) {
                contents[i] = x;
            }
        }
    }
};