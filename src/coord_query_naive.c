#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "coord_query.h"

struct naive_data {
  struct record *rs;
  int n;
};

struct naive_data* mk_naive(struct record* rs, int n) {
  struct naive_data* data = malloc(sizeof(*data));
  if (!data) {
    return NULL; // allocation failed
  }
  data->rs = rs;
  data->n = n;
  return data;
}

void free_naive(struct naive_data* data) {
  free(data);
}

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
    if (data == NULL || data->n == 0) {
        return NULL;
    }
    // Start with the first record as the best candidate
    const struct record *best_rec = &data->rs[0];
    double best_d2 = (lon - best_rec->lon) * (lon - best_rec->lon) +
                     (lat - best_rec->lat) * (lat - best_rec->lat);
    // Check each remaining record for a closer distance
    for (int i = 1; i < data->n; i++) {
        double dx = lon - data->rs[i].lon;
        double dy = lat - data->rs[i].lat;
        double d2 = dx * dx + dy * dy;
        if (d2 < best_d2) {
            best_d2 = d2;
            best_rec = &data->rs[i];
        }
    }
    // Return the closest record found after examining all records
    return best_rec;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
