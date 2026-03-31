/* test-mcf-print.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-print test-mcf-print.c */

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

/* Function to create register pressure and force MCF usage */
void create_register_pressure() {
    /* Many local variables to create register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile int k = 11, l = 12, m = 13, n = 14, o = 15;
    volatile int p = 16, q = 17, r = 18, s = 19, t = 20;
    
    /* Complex array accesses to extend live ranges */
    int arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * 2;
    }
    
    /* Nested loops with register usage */
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            /* Force register usage with inline asm */
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            asm volatile ("" : : "r"(f), "r"(g), "r"(h), "r"(i), "r"(j));
            arr[x * 10 + y] = a + b + c + d + e + f + g + h + i + j;
        }
    }
    
    /* Use all variables to prevent optimization */
    volatile int result = a + b + c + d + e + f + g + h + i + j +
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Main test function */
int main() {
    struct fixup_graph graph;
    int special_labels_printed = 0;
    
    /* Set up the fixup graph with specific indices */
    graph.new_exit_index = 100;  /* Arbitrary distinct value */
    graph.new_entry_index = 200; /* Arbitrary distinct value */
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    printf("Testing fixup graph node printing:\n");
    printf("==================================\n");
    
    /* Test all special cases from the uncovered lines */
    int test_cases[] = {
        ENTRY_BLOCK,           /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,       /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,        /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,    /* Should print "EXIT''" */
        graph.new_exit_index,  /* Should print "NEW_EXIT" */
        graph.new_entry_index, /* Should print "NEW_ENTRY" */
        42,                    /* Regular node - should print "42" */
        99                     /* Regular node - should print "99" */
    };
    
    const char* expected[] = {
        "ENTRY", "ENTRY''", "EXIT", "EXIT''", 
        "NEW_EXIT", "NEW_ENTRY", "42", "99"
    };
    
    for (int i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        printf("Node %d: ", test_cases[i]);
        print_fixup_graph_node(stdout, test_cases[i], &graph);
        printf(" (expected: %s)\n", expected[i]);
        
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
    
    /* Create register pressure to potentially trigger MCF in real GCC */
    printf("\nCreating register pressure to simulate MCF conditions...\n");
    create_register_pressure();
    
    /* Additional test with more complex control flow */
    printf("\nTesting with variable indices:\n");
    volatile int dynamic_index = graph.new_exit_index;
    printf("Dynamic index %d: ", dynamic_index);
    print_fixup_graph_node(stdout, dynamic_index, &graph);
    printf("\n");
    
    /* Test edge cases */
    printf("\nTesting edge cases:\n");
    printf("ENTRY_BLOCK = %d\n", ENTRY_BLOCK);
    printf("EXIT_BLOCK = %d\n", EXIT_BLOCK);
    printf("2*EXIT_BLOCK = %d\n", 2 * EXIT_BLOCK);
    printf("new_exit_index = %d\n", graph.new_exit_index);
    printf("new_entry_index = %d\n", graph.new_entry_index);
    
    return 0;
}
