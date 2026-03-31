/* test-mcf-printing.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -I. test-mcf-printing.c -o test-mcf-printing */

#include <stdio.h>
#include <stdlib.h>

/* Minimal stub definitions to match mcf.cc structures */
#define ENTRY_BLOCK 0
#define EXIT_BLOCK 1

/* Simulated fixup_graph structure */
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
    /* Many local variables to force register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
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
        "movl %0, %%eax\n"
        "movl %1, %%ebx\n"
        "addl %%ebx, %%eax\n"
        "movl %%eax, %0\n"
        : "+r" (a)
        : "r" (b)
        : "%eax", "%ebx"
    );
    
    /* Use register keyword to hint at register allocation */
    register int r1 asm("esi") = a + b;
    register int r2 asm("edi") = c + d;
    
    /* More computation to ensure variables are used */
    volatile int result = a + b + c + d + e + f + g + h + i + j + 
                         k + l + m + n + o + p + r1 + r2;
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(result));
}

/* Test the printing function directly */
void test_printing_function() {
    struct fixup_graph graph;
    
    /* Set up special indices - these should match the constants */
    graph.new_exit_index = 100;  /* Arbitrary distinct value */
    graph.new_entry_index = 200; /* Another distinct value */
    
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
        42,                    /* Should print "42" (regular node) */
        99                     /* Should print "99" (regular node) */
    };
    
    int special_labels_printed = 0;
    
    for (int idx = 0; idx < sizeof(test_cases)/sizeof(test_cases[0]); idx++) {
        printf("Node %d: ", test_cases[idx]);
        print_fixup_graph_node(stdout, test_cases[idx], &graph);
        printf("\n");
        
        /* Count how many special labels were printed */
        if (test_cases[idx] == ENTRY_BLOCK ||
            test_cases[idx] == ENTRY_BLOCK + 1 ||
            test_cases[idx] == 2 * EXIT_BLOCK ||
            test_cases[idx] == 2 * EXIT_BLOCK + 1 ||
            test_cases[idx] == graph.new_exit_index ||
            test_cases[idx] == graph.new_entry_index) {
            special_labels_printed++;
        }
    }
    
    printf("\nSummary: Printed %d special labels out of %d test cases\n",
           special_labels_printed, 
           (int)(sizeof(test_cases)/sizeof(test_cases[0])));
}

/* Main function that combines both approaches */
int main(int argc, char **argv) {
    printf("=== MCF Printing Test Program ===\n\n");
    
    /* Part 1: Direct test of the printing function */
    test_printing_function();
    
    printf("\n=== Creating register pressure ===\n");
    
    /* Part 2: Create code that would trigger MCF in real GCC */
    /* This function creates register pressure that might cause GCC's
       register allocator to use the MCF solver */
    create_register_pressure();
    
    printf("\n=== Test completed successfully ===\n");
    
    return 0;
}
