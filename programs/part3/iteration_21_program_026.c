// In a graph dump/print function
void print_node(FILE *file, int n, fixup_graph_t *fixup_graph) {
  if (n == ENTRY_BLOCK)
    fputs("ENTRY", file);
  else if (n == ENTRY_BLOCK + 1)
    fputs("ENTRY''", file);
  // ... rest of conditions
  else
    fprintf(file, "%d", n);  // Print numeric index for regular blocks
}
