#pragma once

#include "rng.hpp"

const uint32_t empty_bucket = 267349487;


template<typename T>
struct bucket {
    T item;
    uint32_t key = empty_bucket;
};

// only good for one set so be careful
template<typename T>
struct hash_result {
    bool empty;
    bucket<T> *ptr;

    hash_result(uint32_t desired_hash, bucket<T> *b) {
        ptr = b;
        empty = b->key == empty_bucket;
    }
};

template<typename T>
struct dict {
    bucket<T> *items;
    int backing_size;
    int amount;

    const static int default_backing_size = 10;
    const float high_water_mark = 0.5;

    dict() {
        items = (bucket<T> *)malloc(sizeof(bucket<T>) * default_backing_size);
        backing_size = default_backing_size;
        amount = 0;
        for (int i = 0; i < backing_size; i++) {
            items[i] = bucket<T>();
        }
    }

    hash_result<T> get_bucket(uint32_t original_key) {
        auto rehashed_key = original_key;
        auto index = rehashed_key % backing_size;
        while (items[index].key != original_key) {
            
            if (items[index].key == empty_bucket) {
                // bucket is available
                return hash_result(original_key, &items[index]);
            }

            rehashed_key = hash(rehashed_key);
            index = rehashed_key % backing_size;
        }

        // found it
        return hash_result(original_key, &items[index]);
    }

    // ah am i doing forbidden things with amount?
    // since i call set, we will see
    void rebuild() {

        const auto old_size = backing_size;
        backing_size *= 2;
        const auto old_items = items;
        amount = 0;
        
        // zeroize new items
        items = (bucket<T> *)malloc(sizeof(bucket<T>) * backing_size);
        for (int i = 0; i < backing_size; i++) {
            items[i] = bucket<T>();
        }

        // repopulate
        for (int i = 0; i < old_size; i++) {
            if (old_items[i].key != empty_bucket) {
                set(old_items[i].key, old_items[i].item);
            }
        }

        free(old_items);
    }

    void set(uint32_t key, T item) {
        const auto result = get_bucket(key);
        result.ptr->item = item;
        result.ptr->key = key;
        amount++;
        if (amount > high_water_mark * backing_size) {
            rebuild();
        }
    }

    bool contains(uint32_t key) {
        return get_bucket(key)->empty;
    }

    T *get(uint32_t key) {
        const auto result = get_bucket(key);
        if (result.empty) {
            printf("uh oh key error with key %u\n", key);
            return NULL;
        }
        return &result.ptr->item;
    }

    void destroy() {
        if (items) {
            free(items);
            items = NULL;
        }
    }
};