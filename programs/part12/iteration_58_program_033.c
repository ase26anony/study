/* test-mcf-print.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -I. test-mcf-print.c -o test-mcf-print */

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

/* Function to create register pressure and potentially trigger MCF */
void create_register_pressure() {
    /* Many local variables to force register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile int k = 11, l = 12, m = 13, n = 14, o = 15;
    volatile int p = 16, q = 17, r = 18, s = 19, t = 20;
    
    /* Complex array accesses to increase live ranges */
    int arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * idx;
    }
    
    /* Nested loops */
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                arr[x * 10 + y] += arr[y * 10 + z] - arr[z * 10 + x];
            }
        }
    }
    
    /* Inline assembly with clobbered registers */
    asm volatile (
        "movl %0, %%eax\n\t"
        "movl %1, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0"
        : "+r" (a)
        : "r" (b)
        : "%eax", "%ebx"
    );
    
    /* Use register keyword to hint at register allocation */
    register int reg1 asm("esi") = a + b;
    register int reg2 asm("edi") = c + d;
    
    /* More computation to ensure values are used */
    volatile int result = reg1 * reg2 + e * f - g / h + i % j;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(result));
}

/* Test all special node indices */
void test_special_nodes(struct fixup_graph *graph) {
    int test_cases[] = {
        ENTRY_BLOCK,            /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,        /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,         /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,     /* Should print "EXIT''" */
        graph->new_exit_index,  /* Should print "NEW_EXIT" */
        graph->new_entry_index, /* Should print "NEW_ENTRY" */
        42,                     /* Should print "42" */
        100                     /* Should print "100" */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int special_labels_printed = 0;
    
    printf("Testing special node indices:\n");
    printf("=============================\n");
    
    for (int i = 0; i < num_cases; i++) {
        printf("Node %d: ", test_cases[i]);
        print_fixup_graph_node(stdout, test_cases[i], graph);
        printf("\n");
        
        /* Count how many times we hit the special labels */
        if (test_cases[i] == ENTRY_BLOCK ||
            test_cases[i] == ENTRY_BLOCK + 1 ||
            test_cases[i] == 2 * EXIT_BLOCK ||
            test_cases[i] == 2 * EXIT_BLOCK + 1 ||
            test_cases[i] == graph->new_exit_index ||
            test_cases[i] == graph->new_entry_index) {
            special_labels_printed++;
        }
    }
    
    printf("\nSpecial labels printed: %d/%d\n", 
           special_labels_printed, num_cases);
}

/* Main test function */
int main() {
    /* Create and initialize a fixup_graph with specific indices */
    struct fixup_graph graph;
    
    /* Set special indices to distinct values that don't conflict with
       ENTRY_BLOCK or EXIT_BLOCK constants */
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
    
    /* First, create register pressure to potentially trigger GCC's MCF */
    printf("Creating register pressure...\n");
    create_register_pressure();
    printf("Register pressure test completed.\n\n");
    
    /* Test the printing function with all special node indices */
    test_special_nodes(&graph);
    
    /* Additional test: verify the indices don't accidentally overlap */
    printf("\nVerifying index uniqueness:\n");
    printf("ENTRY_BLOCK (%d) != new_exit_index (%d): %s\n",
           ENTRY_BLOCK, graph.new_exit_index,
           ENTRY_BLOCK != graph.new_exit_index ? "OK" : "ERROR");
    printf("ENTRY_BLOCK (%d) != new_entry_index (%d): %s\n",
           ENTRY_BLOCK, graph.new_entry_index,
           ENTRY_BLOCK != graph.new_entry_index ? "OK" : "ERROR");
    printf("EXIT_BLOCK (%d) != new_exit_index (%d): %s\n",
           EXIT_BLOCK, graph.new_exit_index,
           EXIT_BLOCK != graph.new_exit_index ? "OK" : "ERROR");
    
    return 0;
}
