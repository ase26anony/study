/* test-mcf-special-nodes.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -I. test-mcf-special-nodes.c -o test-mcf-special-nodes */

/* Minimal stub definitions to simulate the GCC MCF environment */
#include <stdio.h>
#include <stdlib.h>

/* Constants matching those in GCC's mcf.cc */
#define ENTRY_BLOCK 0
#define EXIT_BLOCK 1

/* Simulated fixup_graph structure */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Stub for the actual printing function from mcf.cc */
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

/* Function that creates high register pressure to trigger MCF */
void high_register_pressure_function(void) {
    /* Many local variables to increase register pressure */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    volatile int q, r, s, t, u, v, w, x, y, z;
    volatile int aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    
    /* Complex array accesses to extend live ranges */
    int array[100];
    for (a = 0; a < 100; a++) {
        array[a] = a * 2;
    }
    
    /* Nested loops with register usage */
    for (b = 0; b < 10; b++) {
        for (c = 0; c < 10; c++) {
            for (d = 0; d < 10; d++) {
                /* Force register usage with volatile operations */
                e = array[b] + array[c];
                f = array[d] * e;
                g = f - array[b];
                
                /* Inline assembly to clobber registers */
                __asm__ volatile (
                    "movl $0, %%eax\n"
                    "movl $0, %%ebx\n"
                    "movl $0, %%ecx\n"
                    "movl $0, %%edx\n"
                    : /* no outputs */
                    : /* no inputs */
                    : "%eax", "%ebx", "%ecx", "%edx", "memory"
                );
            }
        }
    }
    
    /* Use register keyword to hint at register allocation */
    register int reg1 asm("eax");
    register int reg2 asm("ebx");
    reg1 = 42;
    reg2 = reg1 * 2;
    
    /* More complex operations */
    for (h = 0; h < 50; h++) {
        i = array[h % 20];
        j = array[(h + 1) % 20];
        k = i + j;
        l = k * h;
        m = l >> 2;
        
        /* Another inline assembly to force spills */
        __asm__ volatile (
            "addl $1, %0\n"
            "subl $1, %1\n"
            : "+r" (m), "+r" (l)
            :
            : "cc"
        );
    }
}

/* Test function that exercises all special node conditions */
void test_special_nodes(void) {
    struct fixup_graph graph;
    int test_cases[10];
    int i, special_count = 0;
    
    /* Initialize fixup_graph with special indices */
    graph.new_exit_index = 100;  /* Arbitrary distinct value */
    graph.new_entry_index = 200; /* Another distinct value */
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    /* Create test cases covering all special conditions */
    test_cases[0] = ENTRY_BLOCK;           /* Should print "ENTRY" */
    test_cases[1] = ENTRY_BLOCK + 1;       /* Should print "ENTRY''" */
    test_cases[2] = 2 * EXIT_BLOCK;        /* Should print "EXIT" */
    test_cases[3] = 2 * EXIT_BLOCK + 1;    /* Should print "EXIT''" */
    test_cases[4] = graph.new_exit_index;  /* Should print "NEW_EXIT" */
    test_cases[5] = graph.new_entry_index; /* Should print "NEW_ENTRY" */
    test_cases[6] = 42;                    /* Should print "42" */
    test_cases[7] = 99;                    /* Should print "99" */
    test_cases[8] = graph.new_exit_index - 1; /* Should print the number */
    test_cases[9] = graph.new_entry_index + 1; /* Should print the number */
    
    printf("Testing special node printing logic:\n");
    printf("====================================\n");
    
    /* Test each case */
    for (i = 0; i < 10; i++) {
        printf("Node %d: ", test_cases[i]);
        print_fixup_graph_node(stdout, test_cases[i], &graph);
        printf("\n");
        
        /* Count how many were special labels */
        if (test_cases[i] == ENTRY_BLOCK ||
            test_cases[i] == ENTRY_BLOCK + 1 ||
            test_cases[i] == 2 * EXIT_BLOCK ||
            test_cases[i] == 2 * EXIT_BLOCK + 1 ||
            test_cases[i] == graph.new_exit_index ||
            test_cases[i] == graph.new_entry_index) {
            special_count++;
        }
    }
    
    printf("\nSummary: %d special nodes printed out of 10 test cases\n", special_count);
    
    /* Force the compiler to keep the graph structure */
    volatile int dummy = graph.num_vertices + graph.num_edges;
    (void)dummy; /* Suppress unused variable warning */
}

/* Main function that combines everything */
int main(void) {
    printf("MCF Special Nodes Test Program\n");
    printf("===============================\n\n");
    
    /* First create register pressure to potentially trigger MCF */
    printf("Creating register pressure...\n");
    high_register_pressure_function();
    
    /* Then test the special node printing logic */
    printf("\n");
    test_special_nodes();
    
    /* Additional test with different graph configurations */
    printf("\n\nAdditional test with different indices:\n");
    printf("=======================================\n");
    
    struct fixup_graph graph2;
    graph2.new_exit_index = 999;
    graph2.new_entry_index = 888;
    
    /* Test boundary conditions */
    int test_indices[] = {0, 1, 2, 3, 999, 888, 1000};
    for (int i = 0; i < 7; i++) {
        printf("Testing index %d: ", test_indices[i]);
        print_fixup_graph_node(stdout, test_indices[i], &graph2);
        printf("\n");
    }
    
    return 0;
}
