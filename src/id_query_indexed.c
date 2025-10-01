#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

/* Structure for each index element: ID + pointer to record */
struct index_record {
    int64_t osm_id;
    const struct record *record;
};

/* Complete structure for the entire index */
struct indexed_data {
    struct index_record *irs;
    int n;
};

/* Builds a compact index from records */
static void *mk_index(const struct record *rs, int n) {
    struct indexed_data *idx = malloc(sizeof(*idx));
    if (!idx) return NULL;
    idx->n = n;
    idx->irs = malloc(n * sizeof(*idx->irs));
    if (!idx->irs) {
        free(idx);
        return NULL;
    }
    for (int i = 0; i < n; i++) {
        idx->irs[i].osm_id = rs[i].osm_id;
        idx->irs[i].record = &rs[i];  // pointer to original (constant) record
    }
    return idx;
}

/* Frees the index */
static void free_index(void *p) {
    struct indexed_data *idx = p;
    free(idx->irs);
    free(idx);
}

/* Linear search in the compact index */
static const struct record *lookup(void *p, int64_t needle) {
    struct indexed_data *idx = p;
    for (int i = 0; i < idx->n; i++) {
        if (idx->irs[i].osm_id == needle) {
            return idx->irs[i].record;
        }
    }
    return NULL;
}

/* Main function that simply forwards to id_query_loop */
int main(int argc, char **argv) {
    return id_query_loop(argc, argv, mk_index, free_index, lookup);
}