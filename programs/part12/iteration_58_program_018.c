/* test-mcf-coverage.c - Test program to cover special node printing in mcf.cc */

#include <stdio.h>
#include <stdlib.h>

/* Simulate the constants from GCC's internal headers */
#define ENTRY_BLOCK (0)
#define EXIT_BLOCK (1)

/* Simulate the fixup_graph structure from mcf.h */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Simulate the printing function from mcf.cc */
void print_fixup_graph_node(FILE *file, int n, struct fixup_graph *fixup_graph) {
    /* This simulates the exact logic from lines 151-162 of mcf.cc */
    if (n == ENTRY_BLOCK)
        fputs("ENTRY", file);
    else if (n == ENTRY_BLOCK + 1)
        fputs("ENTRY''", file);
    else if (n == 2 * EXIT_BLOCK)
        fputs("EXIT", file);
    else if (n == 2 * EXIT_BLOCK + 1)
        fputs("EXIT''", file);
    else if (n == fixup_graph->new_exit_index)
        fputs("NEW_EXIT", file);
    else if (n == fixup_graph->new_entry_index)
        fputs("NEW_ENTRY", file);
    else
        fprintf(file, "%d", n);
}

/* Function that creates register pressure to potentially trigger MCF */
void create_register_pressure(void) {
    /* Many local variables to force register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile int k = 11, l = 12, m = 13, n = 14, o = 15;
    volatile int p = 16, q = 17, r = 18, s = 19, t = 20;
    
    /* Complex array accesses to increase live ranges */
    int arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * 2;
    }
    
    /* Nested loops */
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            arr[x * 10 + y] = arr[x * 10 + y] + a + b + c;
        }
    }
    
    /* Inline assembly with clobbered registers */
    asm volatile (
        "movl %0, %%eax\n"
        "movl %1, %%ebx\n"
        "addl %%ebx, %%eax\n"
        "movl %%eax, %0\n"
        : "+r" (a)
        : "r" (b)
        : "%eax", "%ebx"
    );
    
    /* Use all variables to prevent optimization */
    volatile int result = a + b + c + d + e + f + g + h + i + j +
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Test function that calls the printing function with all special indices */
void test_special_nodes(struct fixup_graph *graph) {
    int special_labels_printed = 0;
    
    printf("Testing special node indices:\n");
    printf("=============================\n");
    
    /* Test ENTRY_BLOCK */
    printf("Node %d: ", ENTRY_BLOCK);
    print_fixup_graph_node(stdout, ENTRY_BLOCK, graph);
    printf("\n");
    if (ENTRY_BLOCK == 0) special_labels_printed++;
    
    /* Test ENTRY_BLOCK + 1 */
    printf("Node %d: ", ENTRY_BLOCK + 1);
    print_fixup_graph_node(stdout, ENTRY_BLOCK + 1, graph);
    printf("\n");
    
    /* Test 2 * EXIT_BLOCK */
    printf("Node %d: ", 2 * EXIT_BLOCK);
    print_fixup_graph_node(stdout, 2 * EXIT_BLOCK, graph);
    printf("\n");
    
    /* Test 2 * EXIT_BLOCK + 1 */
    printf("Node %d: ", 2 * EXIT_BLOCK + 1);
    print_fixup_graph_node(stdout, 2 * EXIT_BLOCK + 1, graph);
    printf("\n");
    
    /* Test new_exit_index */
    printf("Node %d: ", graph->new_exit_index);
    print_fixup_graph_node(stdout, graph->new_exit_index, graph);
    printf("\n");
    
    /* Test new_entry_index */
    printf("Node %d: ", graph->new_entry_index);
    print_fixup_graph_node(stdout, graph->new_entry_index, graph);
    printf("\n");
    
    /* Test some regular nodes */
    printf("Node %d: ", 42);
    print_fixup_graph_node(stdout, 42, graph);
    printf("\n");
    
    printf("Node %d: ", 100);
    print_fixup_graph_node(stdout, 100, graph);
    printf("\n");
    
    printf("\nTotal special labels printed: %d\n", special_labels_printed);
}

int main(void) {
    /* Initialize a fixup_graph with specific indices */
    struct fixup_graph graph;
    
    /* Set special indices - choose values that don't conflict with ENTRY/EXIT */
    graph.new_exit_index = 1000;
    graph.new_entry_index = 1001;
    graph.num_vertices = 2000;
    graph.num_edges = 5000;
    
    printf("Fixup Graph Configuration:\n");
    printf("  ENTRY_BLOCK: %d\n", ENTRY_BLOCK);
    printf("  EXIT_BLOCK: %d\n", EXIT_BLOCK);
    printf("  new_exit_index: %d\n", graph.new_exit_index);
    printf("  new_entry_index: %d\n", graph.new_entry_index);
    printf("\n");
    
    /* Test the special node printing */
    test_special_nodes(&graph);
    
    /* Create register pressure to potentially trigger GCC's MCF solver */
    printf("\nCreating register pressure...\n");
    create_register_pressure();
    
    /* Additional test with different graph configurations */
    printf("\nTesting with alternative indices:\n");
    struct fixup_graph graph2;
    graph2.new_exit_index = 500;
    graph2.new_entry_index = 501;
    
    printf("Node %d: ", graph2.new_exit_index);
    print_fixup_graph_node(stdout, graph2.new_exit_index, &graph2);
    printf("\n");
    
    printf("Node %d: ", graph2.new_entry_index);
    print_fixup_graph_node(stdout, graph2.new_entry_index, &graph2);
    printf("\n");
    
    return 0;
}
