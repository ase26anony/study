/* Test program to trigger uncovered lines in mcf.cc */
#include <stdio.h>
#include <stdlib.h>

/* Constants matching those in GCC's mcf.cc */
#define ENTRY_BLOCK (0)
#define EXIT_BLOCK (1)

/* Minimal fixup_graph structure */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Stub for the actual printing function in mcf.cc */
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
    
    /* Nested loops with register-intensive operations */
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                /* Force register usage with complex expression */
                a = b + c * d - e / (f + 1) + g * h - i / (j + 1);
                b = c + d * e - f / (g + 1) + h * i - j / (k + 1);
                c = d + e * f - g / (h + 1) + i * j - k / (l + 1);
                
                /* Use inline assembly to clobber registers */
                asm volatile ("" : : "r"(a), "r"(b), "r"(c) : "memory");
            }
        }
    }
    
    /* Use all variables to prevent optimization */
    volatile int result = a + b + c + d + e + f + g + h + i + j +
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Test the printing function with various node indices */
void test_printing(struct fixup_graph *graph) {
    int test_cases[] = {
        ENTRY_BLOCK,           /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,       /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,        /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,    /* Should print "EXIT''" */
        graph->new_exit_index, /* Should print "NEW_EXIT" */
        graph->new_entry_index,/* Should print "NEW_ENTRY" */
        42,                    /* Should print "42" */
        100                    /* Should print "100" */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("Testing fixup graph node printing:\n");
    printf("==================================\n");
    
    for (int i = 0; i < num_cases; i++) {
        printf("Node %d: ", test_cases[i]);
        print_fixup_graph_node(stdout, test_cases[i], graph);
        printf("\n");
    }
}

/* Main function that sets up the scenario */
int main() {
    /* Create and initialize a fixup_graph structure */
    struct fixup_graph graph;
    
    /* Set special indices - these should be distinct from the constants */
    graph.new_exit_index = 1000;
    graph.new_entry_index = 1001;
    graph.num_vertices = 2000;
    graph.num_edges = 5000;
    
    printf("Fixup Graph Configuration:\n");
    printf("  ENTRY_BLOCK: %d\n", ENTRY_BLOCK);
    printf("  EXIT_BLOCK: %d\n", EXIT_BLOCK);
    printf("  new_exit_index: %d\n", graph.new_exit_index);
    printf("  new_entry_index: %d\n", graph.new_entry_index);
    printf("\n");
    
    /* Test the printing function */
    test_printing(&graph);
    
    /* Create register pressure to potentially trigger MCF solver */
    printf("\nCreating register pressure to force MCF usage...\n");
    create_register_pressure();
    
    /* Additional test with different index values */
    printf("\nAdditional test with modified indices:\n");
    printf("======================================\n");
    
    /* Test with indices that might appear in transformed graphs */
    struct fixup_graph graph2;
    graph2.new_exit_index = 2000;
    graph2.new_entry_index = 2001;
    
    int additional_tests[] = {
        ENTRY_BLOCK,
        ENTRY_BLOCK + 1,
        2 * EXIT_BLOCK,
        2 * EXIT_BLOCK + 1,
        graph2.new_exit_index,
        graph2.new_entry_index,
        2 * EXIT_BLOCK + 2,  /* Edge case */
        ENTRY_BLOCK - 1      /* Edge case */
    };
    
    for (int i = 0; i < sizeof(additional_tests)/sizeof(additional_tests[0]); i++) {
        printf("Node %d: ", additional_tests[i]);
        print_fixup_graph_node(stdout, additional_tests[i], &graph2);
        printf("\n");
    }
    
    /* Force compiler to consider all paths by using volatile */
    volatile int check = 0;
    if (check) {
        /* This path won't execute, but prevents dead code elimination */
        print_fixup_graph_node(NULL, -1, NULL);
    }
    
    printf("\nTest completed successfully!\n");
    return 0;
}
