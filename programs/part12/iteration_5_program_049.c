// Example: Creating a normalized CFG with single entry/exit
struct fixup_graph* create_normalized_cfg(struct cfg* original) {
    struct fixup_graph* fg = malloc(sizeof(struct fixup_graph));
    
    // Add artificial single entry
    fg->new_entry_index = add_vertex(fg, ARTIFICIAL_ENTRY);
    
    // Add artificial single exit  
    fg->new_exit_index = add_vertex(fg, ARTIFICIAL_EXIT);
    
    // Transform original graph between these
    // ...
    
    return fg;
}
