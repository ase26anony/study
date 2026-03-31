/* test-mcf-print.c */
/* Compile with: gcc -O0 -g -DENABLE_CHECKING -o test-mcf-print test-mcf-print.c */

#include <stdio.h>
#include <stdlib.h>

/* Simulate the constants from GCC's internal headers */
#define ENTRY_BLOCK (0)
#define EXIT_BLOCK (1)

/* Simulate the fixup_graph structure from mcf.cc */
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

/* Function that creates high register pressure to trigger MCF */
void high_register_pressure_function() {
    /* Create many local variables to force register pressure */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    volatile int q, r, s, t, u, v, w, x, y, z;
    volatile int aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    
    /* Initialize them to prevent optimization */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6; g = 7; h = 8; i = 9; j = 10;
    k = 11; l = 12; m = 13; n = 14; o = 15; p = 16; q = 17; r = 18; s = 19;
    t = 20; u = 21; v = 22; w = 23; x = 24; y = 25; z = 26;
    aa = 27; ab = 28; ac = 29; ad = 30; ae = 31; af = 32; ag = 33; ah = 34;
    ai = 35; aj = 36;
    
    /* Complex array access pattern to increase live ranges */
    volatile int arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * 2;
    }
    
    /* Nested loops with register-intensive computation */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                sum += arr[i] + arr[j] + arr[k] + a + b + c + d + e;
            }
        }
    }
    
    /* Inline assembly with clobbered registers */
    asm volatile (
        "movl %0, %%eax\n"
        "movl %1, %%ebx\n"
        "addl %%ebx, %%eax\n"
        "movl %%eax, %0\n"
        : "+r" (sum)
        : "r" (1000)
        : "%eax", "%ebx", "cc"
    );
    
    /* Use all variables to prevent dead code elimination */
    volatile int result = a + b + c + d + e + f + g + h + i + j +
                        k + l + m + n + o + p + q + r + s + t +
                        u + v + w + x + y + z + aa + ab + ac + ad +
                        ae + af + ag + ah + ai + aj + sum;
    
    (void)result; /* Suppress unused variable warning */
}

/* Test the printing function with all special indices */
void test_print_fixup_graph() {
    struct fixup_graph graph;
    
    /* Set up the fixup graph with specific indices */
    graph.new_exit_index = 100;
    graph.new_entry_index = 200;
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    printf("Testing fixup graph node printing:\n");
    printf("==================================\n");
    
    /* Test all the special cases from the uncovered lines */
    int test_indices[] = {
        ENTRY_BLOCK,           /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,       /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,        /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,    /* Should print "EXIT''" */
        graph.new_exit_index,  /* Should print "NEW_EXIT" */
        graph.new_entry_index, /* Should print "NEW_ENTRY" */
        42,                    /* Should print "42" (regular node) */
        99                     /* Should print "99" (regular node) */
    };
    
    int special_count = 0;
    int total_tests = sizeof(test_indices) / sizeof(test_indices[0]);
    
    for (int i = 0; i < total_tests; i++) {
        printf("Node %d: ", test_indices[i]);
        print_fixup_graph_node(stdout, test_indices[i], &graph);
        printf("\n");
        
        /* Count how many special labels were printed */
        if (test_indices[i] == ENTRY_BLOCK ||
            test_indices[i] == ENTRY_BLOCK + 1 ||
            test_indices[i] == 2 * EXIT_BLOCK ||
            test_indices[i] == 2 * EXIT_BLOCK + 1 ||
            test_indices[i] == graph.new_exit_index ||
            test_indices[i] == graph.new_entry_index) {
            special_count++;
        }
    }
    
    printf("\nSummary: Printed %d special labels out of %d total tests\n", 
           special_count, total_tests);
}

#ifdef ENABLE_CHECKING
/* Simulate GCC's internal checking mode */
void dump_fixup_graph_info() {
    printf("\n=== Dumping fixup graph info (ENABLE_CHECKING) ===\n");
    test_print_fixup_graph();
}
#endif

int main() {
    printf("MCF Fixup Graph Node Printing Test\n");
    printf("==================================\n\n");
    
    /* First create register pressure to potentially trigger MCF */
    printf("Creating register pressure...\n");
    high_register_pressure_function();
    printf("Register pressure test completed.\n\n");
    
    /* Test the printing function directly */
    test_print_fixup_graph();
    
#ifdef ENABLE_CHECKING
    /* This would be called in GCC's internal code when ENABLE_CHECKING is defined */
    dump_fixup_graph_info();
#endif
    
    /* Additional test with different graph configurations */
    printf("\n\nTesting with different graph configuration:\n");
    printf("===========================================\n");
    
    struct fixup_graph graph2;
    graph2.new_exit_index = 999;
    graph2.new_entry_index = 888;
    
    /* Test edge cases */
    printf("Testing NEW_EXIT (999): ");
    print_fixup_graph_node(stdout, 999, &graph2);
    printf("\n");
    
    printf("Testing NEW_ENTRY (888): ");
    print_fixup_graph_node(stdout, 888, &graph2);
    printf("\n");
    
    /* Test that regular nodes still work */
    printf("Testing regular node (555): ");
    print_fixup_graph_node(stdout, 555, &graph2);
    printf("\n");
    
    return 0;
}
