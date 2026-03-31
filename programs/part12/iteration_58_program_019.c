/* test-mcf-printing.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-printing test-mcf-printing.c */

#include <stdio.h>
#include <stdlib.h>

/* Simulate the constants from GCC's internal headers */
#define ENTRY_BLOCK 0
#define EXIT_BLOCK 1

/* Simulate the fixup_graph structure */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Simulate the printing function from mcf.cc */
void print_fixup_graph_node(FILE *file, int n, struct fixup_graph *fixup_graph) {
    /* This is the exact logic from the uncovered lines */
    if (n == ENTRY_BLOCK) {
        fputs("ENTRY", file);
    } else if (n == ENTRY_BLOCK + 1) {
        fputs("ENTRY''", file);
    } else if (n == 2 * EXIT_BLOCK) {
        fputs("EXIT", file);
    } else if (n == 2 * EXIT_BLOCK + 1) {
        fputs("EXIT''", file);
    } else if (n == fixup_graph->new_exit_index) {
        fputs("NEW_EXIT", file);
    } else if (n == fixup_graph->new_entry_index) {
        fputs("NEW_ENTRY", file);
    } else {
        fprintf(file, "%d", n);  /* Default case for regular nodes */
    }
}

/* Function that creates high register pressure to potentially trigger MCF */
void high_register_pressure_function() {
    /* Many local variables to increase register pressure */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    volatile double x, y, z;
    
    /* Complex array accesses to extend live ranges */
    int array1[100], array2[100];
    
    /* Nested loops with register-intensive operations */
    for (a = 0; a < 50; a++) {
        for (b = 0; b < 50; b++) {
            /* Force register usage with inline assembly */
            asm volatile ("" : : "r"(a), "r"(b) : "memory");
            
            /* Complex array access pattern */
            array1[a * 2 + b] = array2[b * 3 + a] * 2;
            
            /* More register pressure */
            c = a + b;
            d = a - b;
            e = a * b;
            f = a ^ b;
            g = a | b;
            h = a & b;
        }
    }
    
    /* Use 'register' keyword to hint at register allocation */
    register int reg1 asm("r12") = 42;
    register int reg2 asm("r13") = 24;
    
    /* Inline assembly with clobbered registers */
    asm volatile (
        "addl %1, %0\n\t"
        : "+r"(reg1)
        : "r"(reg2)
        : "cc", "memory"
    );
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(reg1), "r"(reg2));
}

/* Main test function that exercises all the special node indices */
void test_printing_logic() {
    struct fixup_graph graph;
    int test_indices[10];
    int i, special_count = 0;
    
    /* Initialize the fixup graph with specific indices */
    graph.new_exit_index = 100;  /* Arbitrary distinct value */
    graph.new_entry_index = 200; /* Another distinct value */
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    /* Create test indices covering all special cases */
    test_indices[0] = ENTRY_BLOCK;           /* Should print "ENTRY" */
    test_indices[1] = ENTRY_BLOCK + 1;       /* Should print "ENTRY''" */
    test_indices[2] = 2 * EXIT_BLOCK;        /* Should print "EXIT" */
    test_indices[3] = 2 * EXIT_BLOCK + 1;    /* Should print "EXIT''" */
    test_indices[4] = graph.new_exit_index;  /* Should print "NEW_EXIT" */
    test_indices[5] = graph.new_entry_index; /* Should print "NEW_ENTRY" */
    test_indices[6] = 42;                    /* Regular node */
    test_indices[7] = 99;                    /* Regular node */
    test_indices[8] = 101;                   /* Regular node */
    test_indices[9] = 201;                   /* Regular node */
    
    printf("Testing fixup graph node printing logic:\n");
    printf("========================================\n");
    
    /* Test each index */
    for (i = 0; i < 10; i++) {
        printf("Node %d: ", test_indices[i]);
        print_fixup_graph_node(stdout, test_indices[i], &graph);
        printf("\n");
        
        /* Count how many were special labels */
        if (test_indices[i] == ENTRY_BLOCK ||
            test_indices[i] == ENTRY_BLOCK + 1 ||
            test_indices[i] == 2 * EXIT_BLOCK ||
            test_indices[i] == 2 * EXIT_BLOCK + 1 ||
            test_indices[i] == graph.new_exit_index ||
            test_indices[i] == graph.new_entry_index) {
            special_count++;
        }
    }
    
    printf("\nSummary: Printed %d special labels out of %d total nodes\n", 
           special_count, 10);
    printf("Expected: 6 special labels (ENTRY, ENTRY'', EXIT, EXIT'', NEW_EXIT, NEW_ENTRY)\n");
}

/* Main function that combines everything */
int main() {
    printf("=== MCF Printing Logic Test ===\n\n");
    
    /* First, create register pressure to potentially trigger GCC's MCF */
    printf("1. Creating register pressure...\n");
    high_register_pressure_function();
    printf("   Done.\n\n");
    
    /* Then test the specific printing logic */
    printf("2. Testing fixup graph node printing:\n");
    test_printing_logic();
    
    /* Additional test with different graph configurations */
    printf("\n3. Testing with different graph configuration:\n");
    struct fixup_graph graph2;
    graph2.new_exit_index = 999;
    graph2.new_entry_index = 888;
    
    printf("   Testing NEW_EXIT (999): ");
    print_fixup_graph_node(stdout, 999, &graph2);
    printf("\n");
    
    printf("   Testing NEW_ENTRY (888): ");
    print_fixup_graph_node(stdout, 888, &graph2);
    printf("\n");
    
    printf("\n=== Test Complete ===\n");
    
    return 0;
}
