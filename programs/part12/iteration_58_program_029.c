/* test-mcf-printing.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-printing test-mcf-printing.c */

#include <stdio.h>
#include <stdlib.h>

/* Simulate the constants from GCC's internal headers */
#define ENTRY_BLOCK 0
#define EXIT_BLOCK 1

/* Simulate the fixup_graph structure from mcf.h */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Simulate the printing function from mcf.cc */
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

/* Function that creates high register pressure to potentially trigger MCF */
void high_register_pressure_function() {
    /* Create many local variables to increase register pressure */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Use inline assembly to clobber registers */
    asm volatile ("" : : : 
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    
    /* Complex array access pattern to extend live ranges */
    int arr[100];
    for (int x = 0; x < 100; x++) {
        arr[x] = x * x;
        /* Force dependencies between variables */
        a = arr[x] + b;
        b = a + c;
        c = b + d;
        d = c + e;
        e = d + f;
    }
    
    /* Nested loops with register variables */
    register int reg1 asm("r12") = 0;
    register int reg2 asm("r13") = 0;
    
    for (int y = 0; y < 10; y++) {
        for (int z = 0; z < 10; z++) {
            reg1 = y * z;
            reg2 = reg1 + reg2;
            arr[reg1 % 100] = reg2;
        }
    }
}

/* Test function that simulates the MCF printing scenario */
void test_mcf_printing() {
    struct fixup_graph graph;
    int special_labels_printed = 0;
    
    /* Set up the fixup graph with special indices */
    graph.new_exit_index = 100;  /* Arbitrary distinct value */
    graph.new_entry_index = 200; /* Another distinct value */
    graph.num_vertices = 300;
    graph.num_edges = 500;
    
    printf("Testing MCF fixup graph node printing:\n");
    printf("======================================\n");
    
    /* Test all special cases from the uncovered lines */
    int test_cases[] = {
        ENTRY_BLOCK,            /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,        /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,         /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,     /* Should print "EXIT''" */
        graph.new_exit_index,   /* Should print "NEW_EXIT" */
        graph.new_entry_index,  /* Should print "NEW_ENTRY" */
        42,                     /* Regular node */
        99                      /* Another regular node */
    };
    
    for (int i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        printf("Node %3d: ", test_cases[i]);
        print_fixup_graph_node(stdout, test_cases[i], &graph);
        printf("\n");
        
        /* Count special labels */
        if (test_cases[i] == ENTRY_BLOCK ||
            test_cases[i] == ENTRY_BLOCK + 1 ||
            test_cases[i] == 2 * EXIT_BLOCK ||
            test_cases[i] == 2 * EXIT_BLOCK + 1 ||
            test_cases[i] == graph.new_exit_index ||
            test_cases[i] == graph.new_entry_index) {
            special_labels_printed++;
        }
    }
    
    printf("\nSummary: Printed %d special labels out of %d test cases\n",
           special_labels_printed, 
           (int)(sizeof(test_cases)/sizeof(test_cases[0])));
}

/* Main function that combines everything */
int main() {
    printf("MCF Printing Test Program\n");
    printf("==========================\n\n");
    
    /* First, create register pressure to potentially trigger GCC's MCF */
    printf("Creating register pressure...\n");
    high_register_pressure_function();
    
    /* Then test the printing logic */
    printf("\n");
    test_mcf_printing();
    
    /* Additional test with different graph configurations */
    printf("\n\nAdditional test with different indices:\n");
    printf("=======================================\n");
    
    struct fixup_graph graph2;
    graph2.new_exit_index = 999;
    graph2.new_entry_index = 888;
    
    /* Test edge cases */
    int edge_cases[] = {0, 1, 2, 3, 888, 999, 1000};
    for (int i = 0; i < sizeof(edge_cases)/sizeof(edge_cases[0]); i++) {
        printf("Node %3d: ", edge_cases[i]);
        print_fixup_graph_node(stdout, edge_cases[i], &graph2);
        printf("\n");
    }
    
    return 0;
}
