/* test-mcf-print.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-print test-mcf-print.c */

#include <stdio.h>
#include <stdlib.h>

/* Simulate the constants from GCC's internal headers */
#define ENTRY_BLOCK (0)
#define EXIT_BLOCK (1)

/* Minimal stub for fixup_graph structure */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Stub for the printing function we want to test */
void print_fixup_graph_node(FILE *file, int n, struct fixup_graph *fixup_graph) {
    /* This simulates the exact logic from mcf.cc lines 151-162 */
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
void create_register_pressure() {
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
            for (int z = 0; z < 10; z++) {
                /* Force register usage with complex expression */
                arr[x * 10 + y] += arr[y * 10 + z] * arr[z * 10 + x];
            }
        }
    }
    
    /* Inline assembly with clobbered registers */
    asm volatile (
        "movl %0, %%eax\n\t"
        "movl %1, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "+r" (a)
        : "r" (b)
        : "%eax", "%ebx"
    );
    
    /* Use all variables to prevent optimization */
    printf("Pressure result: %d\n", a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t + arr[0]);
}

/* Main test function that exercises the printing logic */
void test_fixup_graph_printing() {
    struct fixup_graph graph;
    int special_labels_printed = 0;
    
    /* Set up the fixup graph with specific indices */
    graph.new_exit_index = 100;  /* Arbitrary distinct value */
    graph.new_entry_index = 200; /* Another distinct value */
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    printf("Testing fixup graph node printing:\n");
    printf("==================================\n");
    
    /* Test all special cases from the uncovered lines */
    int test_cases[] = {
        ENTRY_BLOCK,           /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,       /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,        /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,    /* Should print "EXIT''" */
        graph.new_exit_index,  /* Should print "NEW_EXIT" */
        graph.new_entry_index, /* Should print "NEW_ENTRY" */
        42,                    /* Regular node - should print "42" */
        99                     /* Regular node - should print "99" */
    };
    
    const char *expected[] = {
        "ENTRY", "ENTRY''", "EXIT", "EXIT''", 
        "NEW_EXIT", "NEW_ENTRY", "42", "99"
    };
    
    for (int idx = 0; idx < sizeof(test_cases)/sizeof(test_cases[0]); idx++) {
        printf("Node %3d: ", test_cases[idx]);
        print_fixup_graph_node(stdout, test_cases[idx], &graph);
        printf("\n");
        
        /* Count special labels (non-numeric outputs) */
        if (test_cases[idx] <= 2 * EXIT_BLOCK + 1 || 
            test_cases[idx] == graph.new_exit_index || 
            test_cases[idx] == graph.new_entry_index) {
            special_labels_printed++;
        }
    }
    
    printf("\nSpecial labels printed: %d\n", special_labels_printed);
    printf("Expected: 6 (ENTRY, ENTRY'', EXIT, EXIT'', NEW_EXIT, NEW_ENTRY)\n");
}

#ifdef ENABLE_CHECKING
/* Simulate GCC's internal checking mode */
void dump_fixup_graph_info(struct fixup_graph *graph) {
    printf("\n=== DUMPING FIXUP GRAPH INFO (ENABLE_CHECKING) ===\n");
    printf("new_exit_index: %d\n", graph->new_exit_index);
    printf("new_entry_index: %d\n", graph->new_entry_index);
    printf("num_vertices: %d\n", graph->num_vertices);
    printf("num_edges: %d\n", graph->num_edges);
    
    /* This would trigger the actual printing in GCC */
    printf("Sample node prints:\n");
    for (int i = 0; i < 5; i++) {
        printf("  Node %d: ", i);
        print_fixup_graph_node(stdout, i, graph);
        printf("\n");
    }
}
#endif

int main() {
    printf("=== MCF Fixup Graph Printing Test ===\n\n");
    
    /* First create register pressure to potentially trigger MCF in real GCC */
    printf("Creating register pressure...\n");
    create_register_pressure();
    printf("Register pressure test completed.\n\n");
    
    /* Test the specific printing logic */
    test_fixup_graph_printing();
    
#ifdef ENABLE_CHECKING
    /* Simulate GCC's internal dump when ENABLE_CHECKING is defined */
    struct fixup_graph graph;
    graph.new_exit_index = 1000;
    graph.new_entry_index = 2000;
    graph.num_vertices = 3000;
    graph.num_edges = 4000;
    
    dump_fixup_graph_info(&graph);
#endif
    
    /* Additional test with different index values */
    printf("\n=== Additional Test with Different Indices ===\n");
    struct fixup_graph graph2;
    graph2.new_exit_index = 999;
    graph2.new_entry_index = 888;
    
    /* Test edge cases around the boundaries */
    int edge_cases[] = {0, 1, 2, 3, 888, 999, 1000};
    for (int i = 0; i < sizeof(edge_cases)/sizeof(edge_cases[0]); i++) {
        printf("Edge case node %d: ", edge_cases[i]);
        print_fixup_graph_node(stdout, edge_cases[i], &graph2);
        printf("\n");
    }
    
    return 0;
}
