/* test-mcf-printing.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-printing test-mcf-printing.c */

#include <stdio.h>
#include <stdlib.h>

/* Simulate the constants from GCC's internal headers */
#define ENTRY_BLOCK (0)
#define EXIT_BLOCK (1)

/* Simulate the fixup_graph structure from mcf.h */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Simulate the printing function from mcf.cc */
void print_fixup_graph_node(FILE *file, int n, struct fixup_graph *fixup_graph) {
    /* This simulates the uncovered lines 151-162 from mcf.cc */
    if (n == ENTRY_BLOCK) {
        fputs("ENTRY", file);
        fprintf(file, " [n=%d]", n);
    } else if (n == ENTRY_BLOCK + 1) {
        fputs("ENTRY''", file);
        fprintf(file, " [n=%d]", n);
    } else if (n == 2 * EXIT_BLOCK) {
        fputs("EXIT", file);
        fprintf(file, " [n=%d]", n);
    } else if (n == 2 * EXIT_BLOCK + 1) {
        fputs("EXIT''", file);
        fprintf(file, " [n=%d]", n);
    } else if (fixup_graph && n == fixup_graph->new_exit_index) {
        fputs("NEW_EXIT", file);
        fprintf(file, " [n=%d]", n);
    } else if (fixup_graph && n == fixup_graph->new_entry_index) {
        fputs("NEW_ENTRY", file);
        fprintf(file, " [n=%d]", n);
    } else {
        fprintf(file, "NODE_%d", n);
    }
}

/* Function to create register pressure and potentially trigger MCF */
void create_register_pressure() {
    /* Create many local variables to increase register pressure */
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
    int sum = 0;
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            sum += arr[x * 10 + y] + a + b + c;
        }
    }
    
    /* Inline assembly to create artificial register pressure */
    asm volatile (
        "movl %0, %%eax\n"
        "movl %1, %%ebx\n"
        "addl %%ebx, %%eax\n"
        "movl %%eax, %2\n"
        : "=r"(sum)
        : "r"(a), "r"(b)
        : "%eax", "%ebx"
    );
    
    printf("Register pressure function result: %d\n", sum);
}

/* Test the printing function with all special node indices */
void test_printing() {
    struct fixup_graph graph;
    int special_labels_printed = 0;
    
    /* Set up the fixup graph with specific indices */
    graph.new_exit_index = 100;
    graph.new_entry_index = 200;
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    printf("Testing fixup graph node printing:\n");
    printf("==================================\n");
    
    /* Test all special cases */
    int test_cases[] = {
        ENTRY_BLOCK,           /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,       /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,        /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,    /* Should print "EXIT''" */
        graph.new_exit_index,  /* Should print "NEW_EXIT" */
        graph.new_entry_index, /* Should print "NEW_ENTRY" */
        42,                    /* Should print "NODE_42" */
        99                     /* Should print "NODE_99" */
    };
    
    for (int idx = 0; idx < sizeof(test_cases)/sizeof(test_cases[0]); idx++) {
        int n = test_cases[idx];
        printf("Node %3d: ", n);
        print_fixup_graph_node(stdout, n, &graph);
        printf("\n");
        
        /* Count how many special labels were printed */
        if (n == ENTRY_BLOCK || n == ENTRY_BLOCK + 1 ||
            n == 2 * EXIT_BLOCK || n == 2 * EXIT_BLOCK + 1 ||
            n == graph.new_exit_index || n == graph.new_entry_index) {
            special_labels_printed++;
        }
    }
    
    printf("\nSummary: Printed %d special labels out of %d test cases\n",
           special_labels_printed, 
           (int)(sizeof(test_cases)/sizeof(test_cases[0])));
}

/* Main function that simulates MCF scenario */
int main() {
    printf("=== MCF Printing Test Program ===\n\n");
    
    /* First create register pressure to potentially trigger MCF */
    printf("Creating register pressure...\n");
    create_register_pressure();
    printf("\n");
    
    /* Test the printing function directly */
    test_printing();
    
    /* Additional test with different graph configurations */
    printf("\n=== Additional Tests ===\n");
    
    struct fixup_graph graph2;
    graph2.new_exit_index = 999;
    graph2.new_entry_index = 888;
    
    printf("Testing with new_exit_index=%d: ", graph2.new_exit_index);
    print_fixup_graph_node(stdout, graph2.new_exit_index, &graph2);
    printf("\n");
    
    printf("Testing with new_entry_index=%d: ", graph2.new_entry_index);
    print_fixup_graph_node(stdout, graph2.new_entry_index, &graph2);
    printf("\n");
    
    /* Edge case: What if indices overlap with ENTRY/EXIT? */
    printf("\n=== Edge Case Tests ===\n");
    struct fixup_graph graph3;
    graph3.new_exit_index = ENTRY_BLOCK;  /* Overlap with ENTRY */
    graph3.new_entry_index = 2 * EXIT_BLOCK;  /* Overlap with EXIT */
    
    printf("Testing overlap new_exit_index=ENTRY_BLOCK (%d): ", graph3.new_exit_index);
    print_fixup_graph_node(stdout, graph3.new_exit_index, &graph3);
    printf(" (should print ENTRY, not NEW_EXIT due to order of checks)\n");
    
    printf("Testing overlap new_entry_index=2*EXIT_BLOCK (%d): ", graph3.new_entry_index);
    print_fixup_graph_node(stdout, graph3.new_entry_index, &graph3);
    printf(" (should print EXIT, not NEW_ENTRY due to order of checks)\n");
    
    return 0;
}
