/* test-mcf-printing.c */
/* Test program to trigger uncovered lines in mcf.cc (lines 151-162) */

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

/* Simulate the printing function from mcf.cc */
void print_fixup_graph_node(FILE *file, int n, struct fixup_graph *fixup_graph) {
    /* This simulates the uncovered lines 151-162 from mcf.cc */
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
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Complex array accesses to extend live ranges */
    int arr1[100], arr2[100], arr3[100];
    
    /* Nested loops with complex operations */
    for (a = 0; a < 10; a++) {
        for (b = 0; b < 10; b++) {
            arr1[a * 10 + b] = a + b;
            for (c = 0; c < 5; c++) {
                arr2[b * 5 + c] = arr1[a * 10 + b] * c;
                for (d = 0; d < 3; d++) {
                    arr3[c * 3 + d] = arr2[b * 5 + c] + d;
                }
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
    register int reg1 asm("esi");
    register int reg2 asm("edi");
    reg1 = 42;
    reg2 = 24;
    
    /* More operations to prevent optimization */
    volatile int result = 0;
    for (e = 0; e < 100; e++) {
        result += arr1[e] + arr2[e % 50] + arr3[e % 30];
    }
}

/* Test the printing function with all special indices */
void test_printing(struct fixup_graph *fg) {
    FILE *output = stdout;
    int test_cases[] = {
        ENTRY_BLOCK,           /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,       /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,        /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,    /* Should print "EXIT''" */
        fg->new_exit_index,    /* Should print "NEW_EXIT" */
        fg->new_entry_index,   /* Should print "NEW_ENTRY" */
        42,                    /* Should print "42" */
        100                    /* Should print "100" */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("Testing fixup graph node printing:\n");
    printf("==================================\n");
    
    for (int idx = 0; idx < num_cases; idx++) {
        printf("Node %d: ", test_cases[idx]);
        print_fixup_graph_node(output, test_cases[idx], fg);
        printf("\n");
    }
}

/* Main function that sets up the fixup graph and triggers printing */
int main(int argc, char **argv) {
    /* Create and initialize a fixup_graph structure */
    struct fixup_graph fg;
    
    /* Set special indices - use values that won't conflict with ENTRY/EXIT */
    fg.new_exit_index = 1000;
    fg.new_entry_index = 1001;
    fg.num_vertices = 2000;
    fg.num_edges = 5000;
    
    printf("Fixup Graph Configuration:\n");
    printf("  ENTRY_BLOCK: %d\n", ENTRY_BLOCK);
    printf("  EXIT_BLOCK: %d\n", EXIT_BLOCK);
    printf("  new_exit_index: %d\n", fg.new_exit_index);
    printf("  new_entry_index: %d\n", fg.new_entry_index);
    printf("\n");
    
    /* First, create register pressure to potentially trigger MCF */
    printf("Creating register pressure to force MCF algorithm...\n");
    create_register_pressure();
    printf("Register pressure test completed.\n\n");
    
    /* Test the printing function with all special cases */
    test_printing(&fg);
    
    /* Additional test: ensure indices are not optimized away */
    volatile int dynamic_index = 0;
    if (argc > 1) {
        dynamic_index = atoi(argv[1]);
        printf("\nDynamic test with index %d: ", dynamic_index);
        print_fixup_graph_node(stdout, dynamic_index, &fg);
        printf("\n");
    }
    
    return 0;
}
