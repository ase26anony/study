#include <stdlib.h>

struct fixup_graph* create_fixup_graph(int num_original_vertices, int num_extra_edges) {
    struct fixup_graph* fg = malloc(sizeof(struct fixup_graph));
    fg->new_entry_index = num_original_vertices;
    fg->new_exit_index = num_original_vertices + 1;
    fg->num_edges = num_extra_edges; // plus original edges
    fg->edge_list = malloc(fg->num_edges * sizeof(struct edge));
    return fg;
}
