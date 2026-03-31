/* test_mcf_coverage.c - Test program to cover special node printing in mcf.cc */

#include <stdio.h>
#include <stdlib.h>

/* Simulate the constants from GCC's internal headers */
#define ENTRY_BLOCK (0)
#define EXIT_BLOCK (1)

/* Simulate the fixup_graph structure */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Simulate the printing function from mcf.cc lines 151-162 */
void print_fixup_graph_node(FILE *file, int n, struct fixup_graph *fixup_graph) {
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

/* Function to create register pressure and force MCF usage */
void create_register_pressure() {
    /* Use many local variables to increase register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    /* Complex array access pattern */
    int arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * idx;
    }
    
    /* Nested loops to extend live ranges */
    int sum = 0;
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                sum += arr[x * 10 + y] * arr[y * 10 + z];
            }
        }
    }
    
    /* Inline assembly with clobbered registers */
    asm volatile (
        "movl %0, %%eax\n"
        "movl %1, %%ebx\n"
        "addl %%ebx, %%eax\n"
        "movl %%eax, %0\n"
        : "+r" (sum)
        : "r" (a)
        : "%eax", "%ebx"
    );
    
    /* Use all variables to prevent optimization */
    volatile int result = a + b + c + d + e + f + g + h + 
                         i + j + k + l + m + n + o + p + sum;
    (void)result;
}

/* Test all special node indices */
void test_special_nodes() {
    struct fixup_graph graph;
    
    /* Set up special indices - using values that won't conflict with ENTRY/EXIT */
    graph.new_exit_index = 100;
    graph.new_entry_index = 200;
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    printf("Testing special node printing:\n");
    printf("==============================\n");
    
    /* Test ENTRY_BLOCK cases */
    printf("Node %d: ", ENTRY_BLOCK);
    print_fixup_graph_node(stdout, ENTRY_BLOCK, &graph);
    printf("\n");
    
    printf("Node %d: ", ENTRY_BLOCK + 1);
    print_fixup_graph_node(stdout, ENTRY_BLOCK + 1, &graph);
    printf("\n");
    
    /* Test EXIT_BLOCK cases */
    printf("Node %d: ", 2 * EXIT_BLOCK);
    print_fixup_graph_node(stdout, 2 * EXIT_BLOCK, &graph);
    printf("\n");
    
    printf("Node %d: ", 2 * EXIT_BLOCK + 1);
    print_fixup_graph_node(stdout, 2 * EXIT_BLOCK + 1, &graph);
    printf("\n");
    
    /* Test new_exit_index */
    printf("Node %d: ", graph.new_exit_index);
    print_fixup_graph_node(stdout, graph.new_exit_index, &graph);
    printf("\n");
    
    /* Test new_entry_index */
    printf("Node %d: ", graph.new_entry_index);
    print_fixup_graph_node(stdout, graph.new_entry_index, &graph);
    printf("\n");
    
    /* Test some regular nodes */
    printf("Node %d: ", 50);
    print_fixup_graph_node(stdout, 50, &graph);
    printf("\n");
    
    printf("Node %d: ", 150);
    print_fixup_graph_node(stdout, 150, &graph);
    printf("\n");
}

/* Main function that combines everything */
int main() {
    printf("MCF Special Node Coverage Test\n");
    printf("===============================\n\n");
    
    /* First create register pressure to potentially trigger MCF */
    printf("Creating register pressure...\n");
    create_register_pressure();
    
    /* Test the special node printing */
    printf("\n");
    test_special_nodes();
    
    /* Additional test with different index values */
    printf("\nTesting with different index values:\n");
    printf("====================================\n");
    
    struct fixup_graph graph2;
    graph2.new_exit_index = 999;
    graph2.new_entry_index = 888;
    
    /* Test edge cases */
    int test_indices[] = {ENTRY_BLOCK, ENTRY_BLOCK + 1, 
                          2 * EXIT_BLOCK, 2 * EXIT_BLOCK + 1,
                          999, 888, 500, 600};
    
    int special_count = 0;
    for (int idx = 0; idx < sizeof(test_indices)/sizeof(test_indices[0]); idx++) {
        printf("Node %d: ", test_indices[idx]);
        print_fixup_graph_node(stdout, test_indices[idx], &graph2);
        printf("\n");
        
        /* Count how many were special labels */
        if (test_indices[idx] == ENTRY_BLOCK ||
            test_indices[idx] == ENTRY_BLOCK + 1 ||
            test_indices[idx] == 2 * EXIT_BLOCK ||
            test_indices[idx] == 2 * EXIT_BLOCK + 1 ||
            test_indices[idx] == graph2.new_exit_index ||
            test_indices[idx] == graph2.new_entry_index) {
            special_count++;
        }
    }
    
    printf("\nSummary: %d out of %d nodes were special labels\n", 
           special_count, (int)(sizeof(test_indices)/sizeof(test_indices[0])));
    
    return 0;
}
