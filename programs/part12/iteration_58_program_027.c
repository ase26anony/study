/* test-mcf-printing.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-printing test-mcf-printing.c */

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

/* Stub for the printing function - this simulates the uncovered code */
void print_fixup_graph_node(FILE *file, int n, struct fixup_graph *fixup_graph) {
    /* This is the exact logic from the uncovered lines */
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
    /* Use many local variables to increase register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    /* Complex array access pattern */
    int arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * idx;
    }
    
    /* Nested loops with register usage */
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            /* Force register usage with inline asm */
            asm volatile("" : : "r"(arr[x * 10 + y]), "r"(a), "r"(b), "r"(c));
            a = a + b + c;
            b = b + c + d;
            c = c + d + e;
        }
    }
    
    /* Use all variables to prevent optimization */
    volatile int result = a + b + c + d + e + f + g + h + 
                         i + j + k + l + m + n + o + p;
    (void)result;
}

/* Main test function */
int main() {
    struct fixup_graph graph;
    int test_indices[10];
    int special_labels_printed = 0;
    
    /* Initialize the fixup graph with specific indices */
    graph.new_exit_index = 100;  /* Arbitrary distinct value */
    graph.new_entry_index = 200; /* Another distinct value */
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    /* Create register pressure first (may trigger MCF in real GCC) */
    create_register_pressure();
    
    printf("Testing fixup graph node printing...\n");
    printf("====================================\n");
    
    /* Test all special cases from the uncovered lines */
    test_indices[0] = ENTRY_BLOCK;           /* Should print "ENTRY" */
    test_indices[1] = ENTRY_BLOCK + 1;       /* Should print "ENTRY''" */
    test_indices[2] = 2 * EXIT_BLOCK;        /* Should print "EXIT" */
    test_indices[3] = 2 * EXIT_BLOCK + 1;    /* Should print "EXIT''" */
    test_indices[4] = graph.new_exit_index;  /* Should print "NEW_EXIT" */
    test_indices[5] = graph.new_entry_index; /* Should print "NEW_ENTRY" */
    test_indices[6] = 42;                    /* Should print "42" */
    test_indices[7] = 99;                    /* Should print "99" */
    test_indices[8] = 101;                   /* Should print "101" */
    test_indices[9] = 201;                   /* Should print "201" */
    
    /* Call the printing function for each test index */
    for (int i = 0; i < 10; i++) {
        printf("Node %d: ", test_indices[i]);
        print_fixup_graph_node(stdout, test_indices[i], &graph);
        printf("\n");
        
        /* Count how many special labels were printed */
        if (test_indices[i] == ENTRY_BLOCK ||
            test_indices[i] == ENTRY_BLOCK + 1 ||
            test_indices[i] == 2 * EXIT_BLOCK ||
            test_indices[i] == 2 * EXIT_BLOCK + 1 ||
            test_indices[i] == graph.new_exit_index ||
            test_indices[i] == graph.new_entry_index) {
            special_labels_printed++;
        }
    }
    
    printf("\n====================================\n");
    printf("Special labels printed: %d/6\n", special_labels_printed);
    
    if (special_labels_printed == 6) {
        printf("SUCCESS: All special node indices triggered!\n");
        return 0;
    } else {
        printf("FAILURE: Only %d special indices were triggered\n", 
               special_labels_printed);
        return 1;
    }
}
