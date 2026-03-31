/* test-mcf-printing.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-printing test-mcf-printing.c */

#include <stdio.h>
#include <stdlib.h>

/* Simulate GCC internal constants and structures */
#define ENTRY_BLOCK (0)
#define EXIT_BLOCK (1)

/* Simulated fixup_graph structure from mcf.h */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Simulated printing function - this is what we're testing */
void print_fixup_graph_node(FILE *file, int n, struct fixup_graph *fixup_graph) {
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
    /* Create many local variables to force register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    /* Complex array accesses to extend live ranges */
    int arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * idx;
    }
    
    /* Nested loops with register usage */
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            /* Force register usage with inline asm */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            arr[x * 10 + y] = a + b + c + d + e + f + g + h;
        }
    }
    
    /* Use all variables to prevent optimization */
    volatile int result = a + b + c + d + e + f + g + h + 
                         i + j + k + l + m + n + o + p;
    (void)result;
}

/* Test all special node indices */
void test_special_nodes(struct fixup_graph *fg) {
    int test_cases[] = {
        ENTRY_BLOCK,            /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,        /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,         /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,     /* Should print "EXIT''" */
        fg->new_exit_index,     /* Should print "NEW_EXIT" */
        fg->new_entry_index,    /* Should print "NEW_ENTRY" */
        42,                     /* Should print "42" (regular node) */
        100                     /* Should print "100" (regular node) */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("Testing special node printing:\n");
    printf("==============================\n");
    
    for (int i = 0; i < num_cases; i++) {
        printf("Node %d: ", test_cases[i]);
        print_fixup_graph_node(stdout, test_cases[i], fg);
        printf("\n");
    }
}

/* Main test driver */
int main() {
    /* Create a simulated fixup_graph with special indices */
    struct fixup_graph fg;
    
    /* Set special indices - using values that won't conflict with ENTRY/EXIT */
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
    
    /* Test the printing function with all special cases */
    test_special_nodes(&fg);
    
    /* Create register pressure to potentially trigger actual MCF */
    printf("\nCreating register pressure...\n");
    create_register_pressure();
    
    /* Additional test: verify edge cases */
    printf("\nEdge case tests:\n");
    printf("================\n");
    
    /* Test with modified indices */
    struct fixup_graph fg2;
    fg2.new_exit_index = 2 * EXIT_BLOCK;      /* Same as EXIT */
    fg2.new_entry_index = ENTRY_BLOCK + 1;    /* Same as ENTRY'' */
    
    printf("Testing with overlapping indices:\n");
    printf("Node %d (new_exit_index == 2*EXIT_BLOCK): ", fg2.new_exit_index);
    print_fixup_graph_node(stdout, fg2.new_exit_index, &fg2);
    printf("\n");
    
    printf("Node %d (new_entry_index == ENTRY_BLOCK+1): ", fg2.new_entry_index);
    print_fixup_graph_node(stdout, fg2.new_entry_index, &fg2);
    printf("\n");
    
    /* Count how many special labels were printed in first test */
    printf("\nSummary: All special node printing conditions tested.\n");
    
    return 0;
}
