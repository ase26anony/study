/* In GCC's cfgrtl.c or profile.c */
struct fixup_graph {
    int new_exit_index;    /* Index of added exit vertex */
    int new_entry_index;   /* Index of added entry vertex */
    int num_vertices;      /* Total vertices in fixup graph */
    fixup_vertex_p vertices; /* Array of vertices */
    fixup_edge_p edge_list;  /* Edge list */
    int num_edges;          /* Edge count */
    /* ... */
};
