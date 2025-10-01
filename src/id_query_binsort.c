#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

/* Holds pointer to the copy and length */
struct binsort_data {
    struct record *rs;
    int n;
};

/* Comparison function for qsort/bsearch */
static int cmp_record_id(const void *a, const void *b) {
    const struct record *ra = a;
    const struct record *rb = b;
    if (ra->osm_id < rb->osm_id) return -1;
    else if (ra->osm_id > rb->osm_id) return 1;
    else return 0;
}

/* Build a sorted index */
static void *mk_binsort(const struct record *rs, int n) {
    struct binsort_data *data = malloc(sizeof(*data));
    if (!data) return NULL;

    data->n = n;
    /* Copy the entire dataset so we can sort without destroying the original */
    data->rs = malloc(n * sizeof(struct record));
    if (!data->rs) { free(data); return NULL; }
    memcpy(data->rs, rs, n * sizeof(struct record));

    /* Sort by osm_id */
    qsort(data->rs, data->n, sizeof(struct record), cmp_record_id);
    return data;
}

/* Frees the index */
static void free_binsort(void *p) {
    struct binsort_data *data = p;
    free(data->rs);
    free(data);
}

/* Slå et ID op med binærsøgning */
static const struct record *lookup_binsort(void *p, int64_t needle) {
    struct binsort_data *data = p;
    /* Lav en “nøgle”-record med kun osm_id sat */
    struct record key = { .osm_id = needle };
    /* bsearch returnerer pointer til matching element eller NULL */
    return bsearch(&key, data->rs, data->n,
                   sizeof(struct record), cmp_record_id);
}

/* Main program: use id_query_loop() to run interactively */
int main(int argc, char **argv) {
    return id_query_loop(argc, argv,
                         mk_binsort,       /* build index */
                         free_binsort,     /* cleanup */
                         lookup_binsort);  /* binary search */
}