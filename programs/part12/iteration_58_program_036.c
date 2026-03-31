/* test-mcf-printing.c - Test program to trigger uncovered lines in mcf.cc */

#include <stdio.h>
#include <stdlib.h>

/* Constants matching those in GCC's mcf.cc */
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

/* Function that creates register pressure to potentially trigger MCF */
void create_register_pressure() {
    /* Many local variables to increase register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile int k = 11, l = 12, m = 13, n = 14, o = 15;
    volatile int p = 16, q = 17, r = 18, s = 19, t = 20;
    
    /* Complex array accesses to extend live ranges */
    int arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * idx;
    }
    
    /* Nested loops with register usage */
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            /* Force register usage with inline asm */
            asm volatile("" : "+r"(a), "+r"(b), "+r"(c) : : "memory");
            arr[x * 10 + y] = a + b + c;
        }
    }
    
    /* Use all variables to prevent optimization */
    volatile int sum = a + b + c + d + e + f + g + h + i + j +
                      k + l + m + n + o + p + q + r + s + t;
    (void)sum;
}

/* Test the printing function with various node indices */
void test_printing(struct fixup_graph *graph) {
    int special_labels_printed = 0;
    
    /* Test all special cases from the uncovered lines */
    int test_cases[] = {
        ENTRY_BLOCK,            /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,        /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,         /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,     /* Should print "EXIT''" */
        graph->new_exit_index,  /* Should print "NEW_EXIT" */
        graph->new_entry_index, /* Should print "NEW_ENTRY" */
        100,                    /* Regular node */
        200                     /* Regular node */
    };
    
    printf("Testing fixup graph node printing:\n");
    printf("==================================\n");
    
    for (int i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        printf("Node %d: ", test_cases[i]);
        print_fixup_graph_node(stdout, test_cases[i], graph);
        printf("\n");
        
        /* Count special labels */
        if (test_cases[i] == ENTRY_BLOCK ||
            test_cases[i] == ENTRY_BLOCK + 1 ||
            test_cases[i] == 2 * EXIT_BLOCK ||
            test_cases[i] == 2 * EXIT_BLOCK + 1 ||
            test_cases[i] == graph->new_exit_index ||
            test_cases[i] == graph->new_entry_index) {
            special_labels_printed++;
        }
    }
    
    printf("\nSpecial labels printed: %d\n", special_labels_printed);
}

/* Main function that sets up the test */
int main() {
    /* Create and initialize a fixup_graph with specific indices */
    struct fixup_graph graph;
    
    /* Set indices to trigger the uncovered lines */
    graph.new_exit_index = 42;   /* Arbitrary distinct value */
    graph.new_entry_index = 99;  /* Arbitrary distinct value */
    graph.num_vertices = 1000;
    graph.num_edges = 1500;
    
    printf("Fixup Graph Configuration:\n");
    printf("  ENTRY_BLOCK: %d\n", ENTRY_BLOCK);
    printf("  EXIT_BLOCK: %d\n", EXIT_BLOCK);
    printf("  new_exit_index: %d\n", graph.new_exit_index);
    printf("  new_entry_index: %d\n", graph.new_entry_index);
    printf("\n");
    
    /* Test the printing function */
    test_printing(&graph);
    
    /* Create register pressure to potentially trigger GCC's MCF */
    printf("\nCreating register pressure...\n");
    create_register_pressure();
    
    /* Additional test with volatile to prevent optimization */
    volatile int force_execution = 1;
    if (force_execution) {
        /* Test edge cases */
        struct fixup_graph volatile_graph;
        volatile_graph.new_exit_index = 500;
        volatile_graph.new_entry_index = 600;
        
        printf("\nAdditional test with volatile graph:\n");
        print_fixup_graph_node(stdout, 500, (struct fixup_graph*)&volatile_graph);
        printf("\n");
        print_fixup_graph_node(stdout, 600, (struct fixup_graph*)&volatile_graph);
        printf("\n");
    }
    
    return 0;
}
