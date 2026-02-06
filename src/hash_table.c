#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>  
#include <math.h>
#include <stdio.h>
#include <math.h>

#include "hash_table.h"
#include "prime.h"

#define HT_INITIAL_BASE_SIZE 50
#define HT_PRIME_1 211
#define HT_PRIME_2 223
#define HT_LOAD_FACTOR_UP 70
#define HT_LOAD_FACTOR_DOWN 10

// Global tombstone
static ht_item HT_DELETED_ITEM = {NULL, NULL};

static ht_item* ht_new_item(const char* key, const char* value) {
    ht_item* item = malloc(sizeof(ht_item));
    if (!item) return NULL;
    item->key   = strdup(key);
    item->value = strdup(value);
    return item;
}

static void delete_item(ht_item* item) {
    if (!item) return;
    free(item->key);
    free(item->value);
    free(item);
}
static int ht_hash(const char* s, int a, int m) {
    unsigned long h = 0;
    while (*s) {
        h = (h * a + (unsigned char)*s) % m;
        s++;
    }
    return (int) h;
}

static int ht_get_hash(const char* s, const int num_buckets, const int attempt) {
    const int hash_a = ht_hash(s, HT_PRIME_1, num_buckets);
    const int hash_b = ht_hash(s, HT_PRIME_2, num_buckets);
    return (hash_a + attempt * (hash_b + 1)) % num_buckets;
}

// ────────────────────────────────────────────────
//                  Resizing logic
// ────────────────────────────────────────────────

static hash_table* ht_new_sized(const int base_size) {
    hash_table* ht = malloc(sizeof(hash_table));
    if (!ht) return NULL;

    ht->base_size = base_size;
    ht->size      = next_prime(base_size);
    ht->count     = 0;
    ht->items     = calloc((size_t)ht->size, sizeof(ht_item*));

    if (!ht->items) {
        free(ht);
        return NULL;
    }
    return ht;
}

hash_table* new_hash_table(void) {
    return ht_new_sized(HT_INITIAL_BASE_SIZE);
}

static void ht_resize(hash_table* ht, const int base_size) {
    if (base_size < HT_INITIAL_BASE_SIZE) return;

    hash_table* new_ht = ht_new_sized(base_size);
    if (!new_ht) return;

    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) {
            ht_insert(new_ht, item->key, item->value);
        }
    }

    // Swap internals
    ht->base_size = new_ht->base_size;
    ht->count     = new_ht->count;

    const int tmp_size = ht->size;
    ht->size = new_ht->size;
    new_ht->size = tmp_size;

    ht_item** tmp_items = ht->items;
    ht->items = new_ht->items;
    new_ht->items = tmp_items;

    delete_ht(new_ht);  // safe now — only contains swapped empty shell
}

static void ht_resize_up(hash_table* ht) {
    const int new_base = ht->base_size * 2;
    ht_resize(ht, new_base);
}

static void ht_resize_down(hash_table* ht) {
    const int new_base = ht->base_size / 2;
    if (new_base < HT_INITIAL_BASE_SIZE) return;
    ht_resize(ht, new_base);
}

// ────────────────────────────────────────────────
//                  Public API
// ────────────────────────────────────────────────

void ht_insert(hash_table* ht, const char* key, const char* value) {
    if (!ht || !key || !value) return;

    const int load = (ht->count * 100) / ht->size;
    if (load > HT_LOAD_FACTOR_UP) {
        ht_resize_up(ht);
    }

    ht_item* item = ht_new_item(key, value);
    if (!item) return;

    int attempt = 0;
    int index = ht_get_hash(key, ht->size, attempt);

    while (ht->items[index] != NULL) {
        ht_item* cur = ht->items[index];

        // Replace if same key
        if (cur != &HT_DELETED_ITEM && strcmp(cur->key, key) == 0) {
            delete_item(cur);
            ht->items[index] = item;
            return;
        }

        // Otherwise keep probing
        attempt++;
        index = ht_get_hash(key, ht->size, attempt);
    }

    // Empty slot found
    ht->items[index] = item;
    ht->count++;
}

char* ht_search(hash_table* ht, const char* key) {
    if (!ht || !key) return NULL;

    int attempt = 0;
    int index = ht_get_hash(key, ht->size, attempt);
    ht_item* item = ht->items[index];

    while (item != NULL) {
        if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0) {
            return item->value;
        }
        attempt++;
        index = ht_get_hash(key, ht->size, attempt);
        item = ht->items[index];
    }
    return NULL;
}

void ht_delete(hash_table* ht, const char* key) {
    if (!ht || !key) return;

    const int load = (ht->count * 100) / ht->size;
    if (load < HT_LOAD_FACTOR_DOWN) {
        ht_resize_down(ht);
    }

    int attempt = 0;
    int index = ht_get_hash(key, ht->size, attempt);
    ht_item* item = ht->items[index];

    while (item != NULL) {
        if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0) {
            delete_item(item);
            ht->items[index] = &HT_DELETED_ITEM;
            ht->count--;
            return;
        }
        attempt++;
        index = ht_get_hash(key, ht->size, attempt);
        item = ht->items[index];
    }
}

void delete_ht(hash_table* ht) {
    if (!ht) return;
    for (int i = 0; i < ht->size; i++) {
        if (ht->items[i] != NULL && ht->items[i] != &HT_DELETED_ITEM) {
            delete_item(ht->items[i]);
        }
    }
    free(ht->items);
    free(ht);
}