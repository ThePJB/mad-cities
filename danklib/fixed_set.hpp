#pragma once

const auto empty_sentinel = -2147483648;

template<unsigned int N>
struct set_iter;

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

    int begin() {
        return contents[0];
    }


    int end() {
        return contents[size()-1];
    }

    fixed_set() {
        for (int i = 0; i < N; i++) {
            contents[i] = empty_sentinel;
        }
    }

    set_iter<N> iter() {
        return set_iter(this);
    }
};

template<unsigned int N>
struct set_iter {
    int curr_idx = 0;
    int size;
    fixed_set<N> *set;

    set_iter(fixed_set<N> *set) {
        this->set = set;
        size = set->size();
    }

    int next() {
        return set->contents[curr_idx++];
    }

    bool has_next() {
        return curr_idx < size;
    }
};