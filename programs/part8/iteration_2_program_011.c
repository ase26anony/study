// Printing a flow graph for debugging
void print_vertex_index(FILE *file, int n, fixup_graph *graph) {
    if (n == ENTRY_BLOCK)
        fputs("ENTRY", file);
    else if (n == ENTRY_BLOCK + 1)
        fputs("ENTRY''", file);
    // ... other cases
    else
        fprintf(file, "%d", n);  // Regular block index
}
