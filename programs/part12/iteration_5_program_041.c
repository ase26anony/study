struct fixup_graph fixup;
fixup.num_vertices = cfg_num_vertices + 2;  // +2 for new entry/exit
fixup.new_entry_index = cfg_num_vertices;
fixup.new_exit_index = cfg_num_vertices + 1;
