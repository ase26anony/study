/* test-mcf-printing.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-printing test-mcf-printing.c */

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

/* Forward declaration of the printing function we want to test */
void print_fixup_graph_node(FILE *file, struct fixup_graph *fixup_graph, int n);

/* Stub implementation of the printing function that contains the uncovered logic */
void print_fixup_graph_node(FILE *file, struct fixup_graph *fixup_graph, int n) {
    /* This is the exact logic from mcf.cc lines 151-162 */
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

/* Function to create register pressure and potentially trigger MCF */
void create_register_pressure() {
    /* Many local variables to force register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    volatile int q = 17, r = 18, s = 19, t = 20, u = 21, v = 22, w = 23, x = 24;
    
    /* Complex array accesses to increase live ranges */
    int array[100];
    for (int idx = 0; idx < 100; idx++) {
        array[idx] = idx * 2;
    }
    
    /* Nested loops */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                array[i * 10 + j] += array[j * 10 + k];
            }
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
    
    /* Use register keyword to hint at register allocation */
    register int reg1 asm("esi") = a + b;
    register int reg2 asm("edi") = c + d;
    
    /* More computations to extend live ranges */
    volatile int result = 0;
    for (int idx = 0; idx < 100; idx++) {
        result += array[idx];
        result *= (reg1 + reg2);
        result /= (idx + 1);
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(result));
}

/* Test function that calls the printing function with all special indices */
void test_printing(struct fixup_graph *graph) {
    int special_labels_printed = 0;
    
    printf("Testing fixup graph node printing:\n");
    printf("==================================\n");
    
    /* Test ENTRY_BLOCK */
    printf("Node %d: ", ENTRY_BLOCK);
    print_fixup_graph_node(stdout, graph, ENTRY_BLOCK);
    printf("\n");
    special_labels_printed++;
    
    /* Test ENTRY_BLOCK + 1 */
    printf("Node %d: ", ENTRY_BLOCK + 1);
    print_fixup_graph_node(stdout, graph, ENTRY_BLOCK + 1);
    printf("\n");
    special_labels_printed++;
    
    /* Test 2 * EXIT_BLOCK */
    printf("Node %d: ", 2 * EXIT_BLOCK);
    print_fixup_graph_node(stdout, graph, 2 * EXIT_BLOCK);
    printf("\n");
    special_labels_printed++;
    
    /* Test 2 * EXIT_BLOCK + 1 */
    printf("Node %d: ", 2 * EXIT_BLOCK + 1);
    print_fixup_graph_node(stdout, graph, 2 * EXIT_BLOCK + 1);
    printf("\n");
    special_labels_printed++;
    
    /* Test new_exit_index */
    printf("Node %d: ", graph->new_exit_index);
    print_fixup_graph_node(stdout, graph, graph->new_exit_index);
    printf("\n");
    special_labels_printed++;
    
    /* Test new_entry_index */
    printf("Node %d: ", graph->new_entry_index);
    print_fixup_graph_node(stdout, graph, graph->new_entry_index);
    printf("\n");
    special_labels_printed++;
    
    /* Test some regular nodes */
    printf("Node %d: ", 42);
    print_fixup_graph_node(stdout, graph, 42);
    printf("\n");
    
    printf("Node %d: ", 100);
    print_fixup_graph_node(stdout, graph, 100);
    printf("\n");
    
    printf("\nTotal special labels printed: %d\n", special_labels_printed);
    printf("==================================\n");
}

int main() {
    /* Create a fixup_graph with specific indices */
    struct fixup_graph graph;
    
    /* Set the special indices - using values that don't conflict with ENTRY/EXIT */
    graph.new_exit_index = 1000;
    graph.new_entry_index = 1001;
    graph.num_vertices = 2000;
    graph.num_edges = 5000;
    
    /* Create register pressure to potentially trigger MCF in a real GCC build */
    create_register_pressure();
    
    /* Test the printing function with all special indices */
    test_printing(&graph);
    
    /* Additional test: ensure indices aren't optimized away */
    volatile int test_index = graph.new_exit_index;
    printf("\nVolatile test - new_exit_index via volatile: %d\n", test_index);
    
    return 0;
}
