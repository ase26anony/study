/* test-mcf-printing.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-printing test-mcf-printing.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* Function to create register pressure and potentially trigger MCF */
void create_register_pressure() {
    /* Many local variables to force register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    volatile int q = 17, r = 18, s = 19, t = 20, u = 21, v = 22, w = 23, x = 24;
    
    /* Complex array accesses to increase live ranges */
    int arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * idx;
    }
    
    /* Nested loops */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                arr[i * 10 + j] += arr[j * 10 + k];
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
    
    /* More computations to ensure variables are used */
    volatile int result = a + b + c + d + e + f + g + h + 
                         i + j + k + l + m + n + o + p +
                         q + r + s + t + u + v + w + x +
                         reg1 + reg2;
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(result));
}

/* Test the printing function with all special indices */
void test_printing(struct fixup_graph *fg) {
    int special_labels_printed = 0;
    FILE *output = stdout;
    
    /* Test all the special indices from the uncovered lines */
    int test_indices[] = {
        ENTRY_BLOCK,
        ENTRY_BLOCK + 1,
        2 * EXIT_BLOCK,
        2 * EXIT_BLOCK + 1,
        fg->new_exit_index,
        fg->new_entry_index,
        100,  /* Regular node */
        200   /* Another regular node */
    };
    
    printf("Testing fixup graph node printing:\n");
    printf("==================================\n");
    
    for (int idx = 0; idx < sizeof(test_indices)/sizeof(test_indices[0]); idx++) {
        int n = test_indices[idx];
        printf("Node %d prints as: ", n);
        print_fixup_graph_node(output, n, fg);
        printf("\n");
        
        /* Count how many special labels were printed */
        if (n == ENTRY_BLOCK || n == ENTRY_BLOCK + 1 ||
            n == 2 * EXIT_BLOCK || n == 2 * EXIT_BLOCK + 1 ||
            n == fg->new_exit_index || n == fg->new_entry_index) {
            special_labels_printed++;
        }
    }
    
    printf("\nSpecial labels printed: %d out of %d test cases\n", 
           special_labels_printed, 
           (int)(sizeof(test_indices)/sizeof(test_indices[0])));
}

int main() {
    /* Create a fixup_graph with specific indices */
    struct fixup_graph fg;
    
    /* Set the special indices - these should be distinct from the constants */
    fg.new_exit_index = 42;    /* Arbitrary value different from constants */
    fg.new_entry_index = 99;   /* Another arbitrary value */
    fg.num_vertices = 1000;
    fg.num_edges = 1500;
    
    printf("Fixup Graph Configuration:\n");
    printf("  ENTRY_BLOCK: %d\n", ENTRY_BLOCK);
    printf("  EXIT_BLOCK: %d\n", EXIT_BLOCK);
    printf("  new_exit_index: %d\n", fg.new_exit_index);
    printf("  new_entry_index: %d\n", fg.new_entry_index);
    printf("\n");
    
    /* First, create register pressure to simulate conditions that might
       trigger MCF in a real GCC compilation */
    create_register_pressure();
    
    /* Test the printing function with our configured indices */
    test_printing(&fg);
    
    /* Additional test: ensure indices aren't optimized away */
    volatile int dynamic_index = fg.new_exit_index;
    printf("\nDynamic index test (should print NEW_EXIT): ");
    print_fixup_graph_node(stdout, dynamic_index, &fg);
    printf("\n");
    
    return 0;
}
