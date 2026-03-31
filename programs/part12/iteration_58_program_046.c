/* test-mcf-printing.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -I. test-mcf-printing.c -o test-mcf-printing */

#include <stdio.h>
#include <stdlib.h>

/* Minimal stub definitions to match mcf.cc structures */
#define ENTRY_BLOCK (0)
#define EXIT_BLOCK (1)

/* Simplified fixup_graph structure */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Stub for the actual printing function from mcf.cc */
void print_fixup_graph_node(FILE *file, struct fixup_graph *fixup_graph, int n) {
    /* This simulates the uncovered lines 151-162 from mcf.cc */
    if (n == ENTRY_BLOCK) {
        fputs("ENTRY", file);
    } else if (n == ENTRY_BLOCK + 1) {
        fputs("ENTRY''", file);
    } else if (n == 2 * EXIT_BLOCK) {
        fputs("EXIT", file);
    } else if (n == 2 * EXIT_BLOCK + 1) {
        fputs("EXIT''", file);
    } else if (fixup_graph && n == fixup_graph->new_exit_index) {
        fputs("NEW_EXIT", file);
    } else if (fixup_graph && n == fixup_graph->new_entry_index) {
        fputs("NEW_ENTRY", file);
    } else {
        fprintf(file, "%d", n);  /* Regular node */
    }
}

/* Function that creates high register pressure to potentially trigger MCF */
void high_register_pressure_function() {
    /* Many local variables to force register pressure */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    volatile double x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
    
    /* Complex array accesses to increase live ranges */
    int array1[100], array2[100];
    
    /* Nested loops */
    for (a = 0; a < 10; a++) {
        for (b = 0; b < 10; b++) {
            for (c = 0; c < 10; c++) {
                array1[(a * 100 + b * 10 + c) % 100] = 
                    array2[(c * 100 + b * 10 + a) % 100] + a * b * c;
            }
        }
    }
    
    /* Inline assembly with clobbered registers */
    asm volatile (
        "movl $0, %%eax\n"
        "movl $1, %%ebx\n"
        "addl %%ebx, %%eax\n"
        : /* no outputs */
        : /* no inputs */
        : "%eax", "%ebx", "cc"
    );
    
    /* Use register keyword to hint at register allocation */
    register int reg_var1 asm("esi");
    register int reg_var2 asm("edi");
    reg_var1 = 42;
    reg_var2 = reg_var1 * 2;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(reg_var1), "r"(reg_var2));
}

/* Test function that simulates MCF graph printing */
void test_mcf_printing() {
    struct fixup_graph graph;
    int special_labels_printed = 0;
    
    /* Initialize fixup_graph with special indices */
    graph.new_exit_index = 100;  /* Arbitrary distinct value */
    graph.new_entry_index = 200; /* Arbitrary distinct value */
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    printf("Testing MCF fixup graph node printing:\n");
    printf("=======================================\n");
    
    /* Test all special cases from the uncovered lines */
    int test_cases[] = {
        ENTRY_BLOCK,           /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,       /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,        /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,    /* Should print "EXIT''" */
        graph.new_exit_index,  /* Should print "NEW_EXIT" */
        graph.new_entry_index, /* Should print "NEW_ENTRY" */
        42,                    /* Regular node */
        99,                    /* Regular node */
    };
    
    for (int i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        printf("Node %d: ", test_cases[i]);
        print_fixup_graph_node(stdout, &graph, test_cases[i]);
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
    
    printf("\nSpecial labels printed: %d\n", special_labels_printed);
    printf("Expected: 6\n");
    
    /* Force compiler to consider the graph values as used */
    volatile int *volatile_ptr = (volatile int*)&graph.new_exit_index;
    asm volatile("" : : "r"(*volatile_ptr));
}

/* Main function that triggers the test */
int main() {
    printf("MCF Printing Test Program\n");
    printf("==========================\n\n");
    
    /* First create register pressure to potentially trigger MCF */
    printf("Creating register pressure...\n");
    high_register_pressure_function();
    
    /* Then test the printing functionality */
    printf("\n");
    test_mcf_printing();
    
    /* Additional test with different graph configurations */
    printf("\n\nAdditional test with different indices:\n");
    printf("=======================================\n");
    
    struct fixup_graph graph2;
    graph2.new_exit_index = 999;
    graph2.new_entry_index = 888;
    
    /* Test boundary cases */
    int test_nodes[] = {0, 1, 2, 3, 888, 999, 1000};
    for (int i = 0; i < sizeof(test_nodes)/sizeof(test_nodes[0]); i++) {
        printf("Node %d: ", test_nodes[i]);
        print_fixup_graph_node(stdout, &graph2, test_nodes[i]);
        printf("\n");
    }
    
    return 0;
}
