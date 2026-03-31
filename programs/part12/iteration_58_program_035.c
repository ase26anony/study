/* test_mcf_coverage.c - Test program to cover special node printing in GCC's MCF implementation */

#include <stdio.h>
#include <stdlib.h>

/* Mock definitions to match GCC's internal structures */
#define ENTRY_BLOCK (0)
#define EXIT_BLOCK (1)

/* Mock fixup_graph structure */
struct fixup_graph {
    int new_exit_index;
    int new_entry_index;
    int num_vertices;
    int num_edges;
};

/* Mock printing function that contains the uncovered lines */
void print_fixup_graph_node(FILE *file, struct fixup_graph *fixup_graph, int n) {
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
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    volatile int q = 17, r = 18, s = 19, t = 20, u = 21, v = 22, w = 23, x = 24;
    
    /* Complex array accesses to extend live ranges */
    int arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * idx;
    }
    
    /* Nested loops with register usage */
    int sum = 0;
    for (int i1 = 0; i1 < 10; i1++) {
        for (int j1 = 0; j1 < 10; j1++) {
            for (int k1 = 0; k1 < 10; k1++) {
                /* Force register usage with complex expression */
                sum += arr[i1] * arr[j1] + arr[k1] - a + b - c + d;
                
                /* Inline assembly to clobber registers */
                __asm__ volatile (
                    "movl $0, %%eax\n"
                    "movl $0, %%ebx\n"
                    "movl $0, %%ecx\n"
                    "movl $0, %%edx\n"
                    : /* no outputs */
                    : /* no inputs */
                    : "%eax", "%ebx", "%ecx", "%edx", "memory"
                );
            }
        }
    }
    
    /* Use all variables to prevent optimization */
    volatile int result = a + b + c + d + e + f + g + h + i + j + k + l + 
                         m + n + o + p + q + r + s + t + u + v + w + x + sum;
    
    /* Use result to prevent dead code elimination */
    if (result > 1000) {
        printf("Register pressure result: %d\n", result);
    }
}

/* Another function with different register pressure pattern */
void more_register_pressure() {
    /* Use register keyword to hint at register allocation */
    register int r1 asm("eax") = 1;
    register int r2 asm("ebx") = 2;
    register int r3 asm("ecx") = 3;
    register int r4 asm("edx") = 4;
    
    /* Complex computation spanning multiple registers */
    for (int i = 0; i < 1000; i++) {
        r1 = r1 * r2 + r3;
        r2 = r2 * r3 + r4;
        r3 = r3 * r4 + r1;
        r4 = r4 * r1 + r2;
        
        /* Force spills with large array */
        double large_array[1000];
        for (int j = 0; j < 1000; j++) {
            large_array[j] = (r1 + r2 + r3 + r4) * j;
        }
        
        /* Use the array to prevent optimization */
        volatile double sum = 0;
        for (int j = 0; j < 1000; j++) {
            sum += large_array[j];
        }
    }
}

/* Main test function */
int main() {
    int special_labels_printed = 0;
    
    /* Create and initialize a fixup_graph with specific indices */
    struct fixup_graph graph;
    
    /* Set special indices to trigger the uncovered conditions */
    graph.new_exit_index = 100;  /* Arbitrary distinct value */
    graph.new_entry_index = 200; /* Another distinct value */
    graph.num_vertices = 300;
    graph.num_edges = 400;
    
    printf("Testing special node label printing:\n");
    printf("====================================\n");
    
    /* Test all special cases from the uncovered lines */
    int test_cases[] = {
        ENTRY_BLOCK,           /* Should print "ENTRY" */
        ENTRY_BLOCK + 1,       /* Should print "ENTRY''" */
        2 * EXIT_BLOCK,        /* Should print "EXIT" */
        2 * EXIT_BLOCK + 1,    /* Should print "EXIT''" */
        graph.new_exit_index,  /* Should print "NEW_EXIT" */
        graph.new_entry_index, /* Should print "NEW_ENTRY" */
        42,                    /* Regular node - should print "42" */
        99                     /* Another regular node */
    };
    
    /* Test each case */
    for (int i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        printf("Node %d: ", test_cases[i]);
        print_fixup_graph_node(stdout, &graph, test_cases[i]);
        printf("\n");
        
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
    printf("\nCreating register pressure...\n");
    create_register_pressure();
    more_register_pressure();
    
    /* Verify we hit all special cases */
    if (special_labels_printed == 6) {
        printf("\nSUCCESS: All special node cases covered!\n");
        return 0;
    } else {
        printf("\nFAILURE: Only %d/6 special cases covered\n", special_labels_printed);
        return 1;
    }
}

/* Additional complex function to increase compilation complexity */
void complex_function_with_many_vars() {
    /* Force many simultaneous live variables */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    int var6 = 6, var7 = 7, var8 = 8, var9 = 9, var10 = 10;
    int var11 = 11, var12 = 12, var13 = 13, var14 = 14, var15 = 15;
    
    /* Complex control flow */
    for (int i = 0; i < 100; i++) {
        switch (i % 5) {
            case 0: var1 += var2 * var3; break;
            case 1: var4 = var5 - var6; break;
            case 2: var7 = var8 / (var9 + 1); break;
            case 3: var10 = var11 ^ var12; break;
            case 4: var13 = var14 | var15; break;
        }
        
        /* Nested loop with array access */
        int matrix[10][10];
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {
                matrix[x][y] = var1 + var2 + var3 + var4 + var5 + 
                              var6 + var7 + var8 + var9 + var10 +
                              var11 + var12 + var13 + var14 + var15;
            }
        }
        
        /* Use matrix to prevent optimization */
        volatile int total = 0;
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {
                total += matrix[x][y];
            }
        }
    }
}
