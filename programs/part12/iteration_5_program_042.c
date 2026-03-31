// In compiler optimization (like GCC's fixup_cfg)
struct fixup_graph {
    int new_exit_index;    // Index of added exit block
    int new_entry_index;   // Index of added entry block  
    int num_vertices;      // Total basic blocks after transformation
    basic_block *vertices; // Array of basic blocks
    /* ... */
};
