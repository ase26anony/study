// In GCC's cfg.c / cfganal.c
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    fixup_vertex_p vertices;
    fixup_edge_p edge_list;
    /* ... */
};
