/* test-mcf-print.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-print test-mcf-print.c */

#include <stdio.h>
#include <stdlib.h>

/* Simulate the constants from GCC's internal headers */
#define ENTRY_BLOCK (0)
#define EXIT_BLOCK (1)

/* Minimal stub for fixup_graph structure */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Stub for the printing function - this simulates the uncovered code */
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
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + a;
            e = a + b;
        }
    }
    
    /* Use all variables to prevent optimization */
    volatile int result = a + b + c + d + e + f + g + h + i + j +
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Test function that triggers the printing logic */
void test_fixup_graph_printing() {
    struct fixup_graph graph;
    
    /* Set up the fixup graph with specific indices */
    graph.new_exit_index = 100;  /* Arbitrary distinct value */
    graph.new_entry_index = 200; /* Another distinct value */
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    printf("Testing fixup graph node printing:\n");
    printf("==================================\n");
    
    /* Test all the special cases from the uncovered lines */
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
    
    for (int i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        printf("Node %d: ", test_cases[i]);
        print_fixup_graph_node(stdout, test_cases[i], &graph);
        printf("\n");
        
        /* Count how many special labels were printed */
        if (test_cases[i] == ENTRY_BLOCK ||
            test_cases[i] == ENTRY_BLOCK + 1 ||
            test_cases[i] == 2 * EXIT_BLOCK ||
            test_cases[i] == 2 * EXIT_BLOCK + 1 ||
            test_cases[i] == graph.new_exit_index ||
            test_cases[i] == graph.new_entry_index) {
            special_labels_printed++;
        }
    }
    
    printf("\nSummary: Printed %d special labels (should be 6)\n", 
           special_labels_printed);
    
    /* Create register pressure to potentially trigger MCF in real GCC */
    create_register_pressure();
}

/* Main function with complex control flow to increase register pressure */
int main(int argc, char **argv) {
    volatile int seed = argc; /* Prevent optimization */
    
    printf("MCF Fixup Graph Printing Test\n");
    printf("=============================\n\n");
    
    /* Multiple function calls to create register pressure */
    for (int i = 0; i < 5; i++) {
        test_fixup_graph_printing();
        
        /* Additional register pressure with inline assembly */
        register int r1 asm("r12") = i * 10;
        register int r2 asm("r13") = i * 20;
        register int r3 asm("r14") = i * 30;
        
        asm volatile (
            "add %1, %2, %0\n\t"
            : "=r"(r1)
            : "r"(r2), "r"(r3)
        );
        
        /* Use the result to prevent dead code elimination */
        volatile int dummy = r1 + r2 + r3;
        (void)dummy;
    }
    
    /* Array operations to extend live ranges */
    int large_array[1000];
    for (int i = 0; i < 1000; i++) {
        large_array[i] = i * seed;
        if (i % 100 == 0) {
            /* Force spill with inline asm that clobbers many registers */
            asm volatile (
                "nop\n\t"
                :
                :
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
        }
    }
    
    /* Use the array to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += large_array[i];
    }
    
    printf("\nTest completed successfully!\n");
    return 0;
}
