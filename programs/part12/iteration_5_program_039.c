// Example: Creating a transformed CFG with artificial entry/exit
struct fixup_graph fg;
fg.new_entry_index = add_artificial_entry(cfg);
fg.new_exit_index = add_artificial_exit(cfg);
fg.num_vertices = cfg->vertex_count + 2;  // Original + entry/exit

// The fixup graph now has guaranteed single entry/exit points
// which simplifies many algorithms
