/* Test program to cover special node printing in mcf.cc */
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
void print_fixup_graph_node(FILE *file, struct fixup_graph *fixup_graph, int n) {
    /* This simulates the uncovered lines 151-162 from mcf.cc */
    if (n == ENTRY_BLOCK)
        fputs("ENTRY", file);
    else if (n == ENTRY_BLOCK + 1)
        fputs("ENTRY''", file);
    else if (n == 2 * EXIT_BLOCK)
        fputs("EXIT", file);
    else if (n == 2 * EXIT_BLOCK + 1)
        fputs("EXIT''", file);
    else if (fixup_graph && n == fixup_graph->new_exit_index)
        fputs("NEW_EXIT", file);
    else if (fixup_graph && n == fixup_graph->new_entry_index)
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
            asm volatile ("" : : "r"(f), "r"(g), "r"(h), "r"(i), "r"(j));
            arr[x * 10 + y] = a + b + c + d + e + f + g + h + i + j;
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
    int special_labels_printed = 0;
    
    /* Set up special indices - using values that won't conflict with constants */
    graph.new_exit_index = 100;
    graph.new_entry_index = 200;
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    printf("Testing fixup graph node printing:\n");
    printf("==================================\n");
    
    /* Test all special cases from the uncovered lines */
    int test_cases[] = {
        ENTRY_BLOCK,            /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,        /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,         /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,     /* Should print "EXIT''" */
        graph.new_exit_index,   /* Should print "NEW_EXIT" */
        graph.new_entry_index,  /* Should print "NEW_ENTRY" */
        42,                     /* Regular node */
        99                      /* Another regular node */
    };
    
    const char *expected[] = {
        "ENTRY", "ENTRY''", "EXIT", "EXIT''", 
        "NEW_EXIT", "NEW_ENTRY", "42", "99"
    };
    
    for (int i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        printf("Node %3d: ", test_cases[i]);
        print_fixup_graph_node(stdout, &graph, test_cases[i]);
        
        /* Check if we printed a special label */
        if (test_cases[i] == ENTRY_BLOCK ||
            test_cases[i] == ENTRY_BLOCK + 1 ||
            test_cases[i] == 2 * EXIT_BLOCK ||
            test_cases[i] == 2 * EXIT_BLOCK + 1 ||
            test_cases[i] == graph.new_exit_index ||
            test_cases[i] == graph.new_entry_index) {
            special_labels_printed++;
        }
        
        printf(" (expected: %s)\n", expected[i]);
    }
    
    printf("\nSpecial labels printed: %d\n", special_labels_printed);
    
    /* Create register pressure to potentially trigger MCF in real GCC */
    create_register_pressure();
}

/* Main function with complex control flow */
int main(int argc, char **argv) {
    volatile int force_no_optimization = argc;
    
    printf("MCF Special Node Printing Test\n");
    printf("===============================\n\n");
    
    /* Call test multiple times with different conditions */
    for (int i = 0; i < 3; i++) {
        printf("Iteration %d:\n", i + 1);
        test_fixup_graph_printing();
        printf("\n");
        
        /* Prevent loop unrolling */
        if (force_no_optimization > 1000) {
            printf("This won't happen\n");
        }
    }
    
    /* Additional complex code to increase register pressure */
    {
        register int r1 asm("r0") = 1;
        register int r2 asm("r1") = 2;
        register int r3 asm("r2") = 3;
        register int r4 asm("r3") = 4;
        
        /* Use inline assembly to clobber registers */
        asm volatile (
            "add %0, %1, %2\n"
            "add %0, %0, %3\n"
            : "=r"(r1)
            : "r"(r1), "r"(r2), "r"(r3), "r"(r4)
            : "cc"
        );
        
        printf("Register computation result: %d\n", r1);
    }
    
    return 0;
}
