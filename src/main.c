#include <stdio.h>
#include "hash_table.h"

int main() {
    hash_table* ht = new_hash_table();
    ht_insert(ht, "name", "Alice");
    ht_insert(ht, "city", "Tokyo");
    ht_insert(ht, "lang", "C");

    printf("name -> %s\n", ht_search(ht, "name"));
    printf("city -> %s\n", ht_search(ht, "city"));

    ht_delete(ht, "city");
    printf("city after delete -> %s\n", ht_search(ht, "city") ? ht_search(ht, "city") : "(null)");

    delete_ht(ht);
    return 0;
}