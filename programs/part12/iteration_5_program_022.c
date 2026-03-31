// Example: Creating a transformed CFG with artificial entry/exit
struct fixup_graph fg;
fg.new_entry_index = add_artificial_entry(original_cfg);
fg.new_exit_index = add_artificial_exit(original_cfg);
fg.num_vertices = count_vertices(transformed_cfg);
