#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "coord_query.h"

struct kd_node {
    const struct record *record;
    int axis; // 0 = lon, 1 = lat
    struct kd_node *left;
    struct kd_node *right;
};

static int cmp_longitude(const void *a, const void *b) {
    const struct record * const *ra = a;
    const struct record * const *rb = b;
    if ((*ra)->lon < (*rb)->lon) return -1;
    if ((*ra)->lon > (*rb)->lon) return 1;
    return 0;
}


static int cmp_latitude(const void *a, const void *b) {
    const struct record * const *ra = a;
    const struct record * const *rb = b;
    if ((*ra)->lat < (*rb)->lat) return -1;
    if ((*ra)->lat > (*rb)->lat) return 1;
    return 0;
}

static struct kd_node* build_kdtree(const struct record **points, int len, int depth) {
    if (len <= 0) {
        return NULL;
    }
    int axis = depth % 2;
    if (axis == 0) {
        qsort(points, len, sizeof(*points), cmp_longitude);
    } else {
        qsort(points, len, sizeof(*points), cmp_latitude);
    }
    int mid = len / 2;
    struct kd_node *node = malloc(sizeof(*node));
    if (!node) {
        return NULL;
    }
    node->record = points[mid];
    node->axis = axis;
    node->left = build_kdtree(points, mid, depth + 1);
    node->right = build_kdtree(points + mid + 1, len - mid - 1, depth + 1);
    return node;
}

static void *mk_kdtree(const struct record *rs, int n) {
    if (n <= 0 || rs == NULL) {
        return NULL;
    }
    const struct record **points = malloc(n * sizeof(*points));
    if (!points) {
        return NULL;
    }
    for (int i = 0; i < n; i++) {
        points[i] = &rs[i];
    }
    struct kd_node *root = build_kdtree(points, n, 0);
    free(points);
    return root;
}

static void search_kdtree(const struct kd_node *node, double qlon, double qlat, const struct record **best, double *best_dist) {
    if (node == NULL) {
        return;
    }
    double dx = qlon - node->record->lon;
    double dy = qlat - node->record->lat;
    double d2 = dx * dx + dy * dy;
    if (d2 < *best_dist) {
        *best_dist = d2;
        *best = node->record;
    }
    double diff;
    if (node->axis == 0) {
        diff = qlon - node->record->lon;
    } else {
        diff = qlat - node->record->lat;
    }
    struct kd_node *near_child = diff <= 0 ? node->left : node->right;
    struct kd_node *far_child = diff <= 0 ? node->right : node->left;
    search_kdtree(near_child, qlon, qlat, best, best_dist);
    if (diff * diff < *best_dist) {
        search_kdtree(far_child, qlon, qlat, best, best_dist);
    }
}

static const struct record *lookup_kdtree(void *tree, double lon, double lat) {
    struct kd_node *root = (struct kd_node *) tree;
    if (root == NULL) {
        return NULL;
    }
    const struct record *best = NULL;
    double best_dist = __DBL_MAX__;
    search_kdtree(root, lon, lat, &best, &best_dist);
    return best;
}

static void free_kdtree(void *index) {
    struct kd_node *root = (struct kd_node*) index;
    if (!root) {
        return;
    }
    struct kd_node *left = root->left;
    struct kd_node *right = root->right;
    free(root);
    if (left) {
        free_kdtree(left);
    }
    if (right) {
        free_kdtree(right);
    }
}

int main(int argc, char **argv) {
    return coord_query_loop(argc, argv,
                            (mk_index_fn)mk_kdtree,
                            (free_index_fn)free_kdtree,
                            (lookup_fn)lookup_kdtree);
}