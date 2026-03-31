// In a graph printing function
for (each node n in graph) {
  fprintf(file, "Node %d: ", n);
  if (n == ENTRY_BLOCK)
    fputs("ENTRY", file);
  else if (n == ENTRY_BLOCK + 1)
    fputs("ENTRY''", file);
  // ... other special cases
  else
    fprintf(file, "%d", n);  // Regular node
  // Print node details...
}
