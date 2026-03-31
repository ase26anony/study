// Could be refactored to use a switch or lookup table:
const char* get_special_block_name(int n, fixup_graph_t* fg) {
    if (n == ENTRY_BLOCK) return "ENTRY";
    if (n == ENTRY_BLOCK + 1) return "ENTRY''";
    if (n == 2 * EXIT_BLOCK) return "EXIT";
    if (n == 2 * EXIT_BLOCK + 1) return "EXIT''";
    if (n == fg->new_exit_index) return "NEW_EXIT";
    if (n == fg->new_entry_index) return "NEW_ENTRY";
    return NULL; // Not a special block
}
