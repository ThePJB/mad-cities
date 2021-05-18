#pragma once

const auto empty_sentinel = -2147483648;

template<unsigned int N>
struct fixed_set {
    int contents[N];

    bool contains(int x) {
        for (int i = 0; i < N; i++) {
            if (contents[i] == x) {
                return true;
            }
        }
        return false;
    }

    int size() {
        for (int i = 0; i < N; i++) {
            if (contents[i] == empty_sentinel) {
                return i;
            }
        }
        return N;
    }

    void put(int x) {
        if (contains(x)) {
            return;
        }

        for (int i = 0; i < N; i++) {
            if (contents[i] == empty_sentinel) {
                contents[i] = x;
                return;
            }
        }
    }

    fixed_set() {
        for (int i = 0; i < N; i++) {
            contents[i] = empty_sentinel;
        }
    }
};