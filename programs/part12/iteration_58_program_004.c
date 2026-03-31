/* test-mcf-print.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-print test-mcf-print.c */

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
    /* This is the exact logic from lines 151-162 of mcf.cc */
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
    /* Many local variables to create register pressure */
    register int r0 asm("r0") = 1;
    volatile int v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6;
    volatile int v6 = 7, v7 = 8, v8 = 9, v9 = 10, v10 = 11;
    volatile int v11 = 12, v12 = 13, v13 = 14, v14 = 15, v15 = 16;
    
    /* Complex array access pattern to extend live ranges */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * i;
    }
    
    /* Nested loops with register usage */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            v1 = array[i] + array[j] + r0;
            /* Inline assembly to clobber registers */
            asm volatile (
                "add %0, %1, %2"
                : "=r" (v2)
                : "r" (v1), "r" (v3)
                : "cc"
            );
        }
    }
    
    /* Use all variables to prevent optimization */
    printf("Pressure result: %d\n", v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10);
}

/* Test all the special node indices */
void test_special_nodes(struct fixup_graph *fg) {
    int test_cases[] = {
        ENTRY_BLOCK,            /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,        /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,         /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,     /* Should print "EXIT''" */
        fg->new_exit_index,     /* Should print "NEW_EXIT" */
        fg->new_entry_index,    /* Should print "NEW_ENTRY" */
        42,                     /* Should print "42" */
        100                     /* Should print "100" */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int special_count = 0;
    
    printf("Testing special node printing:\n");
    printf("==============================\n");
    
    for (int i = 0; i < num_cases; i++) {
        printf("Node %d: ", test_cases[i]);
        print_fixup_graph_node(stdout, test_cases[i], fg);
        printf("\n");
        
        /* Count how many were special labels */
        if (test_cases[i] == ENTRY_BLOCK ||
            test_cases[i] == ENTRY_BLOCK + 1 ||
            test_cases[i] == 2 * EXIT_BLOCK ||
            test_cases[i] == 2 * EXIT_BLOCK + 1 ||
            test_cases[i] == fg->new_exit_index ||
            test_cases[i] == fg->new_entry_index) {
            special_count++;
        }
    }
    
    printf("\nTotal special nodes printed: %d/%d\n", special_count, num_cases);
}

int main() {
    /* Create a fixup_graph with specific indices */
    struct fixup_graph fg;
    
    /* Set the special indices - using values that won't conflict with
       ENTRY_BLOCK (0) and EXIT_BLOCK (1) */
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
    
    /* First create register pressure to potentially trigger MCF */
    printf("Creating register pressure...\n");
    create_register_pressure();
    printf("\n");
    
    /* Test the printing function directly */
    test_special_nodes(&fg);
    
    /* Additional test: ensure indices aren't optimized away */
    volatile int dynamic_index = 0;
    printf("\nDynamic test (should print 'ENTRY'): ");
    print_fixup_graph_node(stdout, dynamic_index, &fg);  /* ENTRY_BLOCK = 0 */
    printf("\n");
    
    return 0;
}
