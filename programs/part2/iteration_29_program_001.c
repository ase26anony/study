/* test_hw_loops.c - Test program for hardware loop optimization coverage */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * Three perfectly nested loops - creates clear parent-child relationships
     * Outer loop (i) contains middle loop (j) which contains inner loop (k)
     */
    for (int i = 0; i < 10; ++i) {
        /* Basic block A - start of outer loop body */
        g_array[i] = i * 2;
        
        for (int j = 0; j < 8; ++j) {
            /* Basic block B - start of middle loop body */
            local_sink = i + j;
            
            for (int k = 0; k < 6; ++k) {
                /* Basic block C - innermost loop body */
                g_sink = i * j * k;
                g_array[k] += g_sink;
            }
            
            /* Basic block D - middle loop continuation */
            g_array[j] = local_sink;
        }
        
        /* 
         * PARTIALLY OVERLAPPING LOOP - shares basic blocks with outer loop
         * This loop starts inside the outer loop's body but isn't perfectly nested
         * It will intersect bitmap with outer loop but not be a subset
         */
        for (int m = 0; m < 5; ++m) {
            /* Basic block E - overlapping loop body */
            g_sink = i * m;
            g_array[m + 10] = g_sink;
            
            /* This creates intersection with outer loop's blocks */
            if (m % 2 == 0) {
                /* Basic block F - conditional inside overlapping loop */
                local_sink += g_array[i];  /* Uses outer loop variable i */
            }
        }
        
        /* Basic block G - outer loop continuation */
        g_array[i + 20] = local_sink;
    }
    
    /* 
     * SEPARATE NON-INTERSECTING LOOP 
     * This loop doesn't share any basic blocks with the nested structure above
     * Will fail bitmap_intersect_p check and be skipped
     */
    for (int n = 0; n < 7; ++n) {
        /* Basic block H - completely separate loop */
        volatile int temp = n * 3;
        g_array[n + 30] = temp;
        
        /* Small inner loop to create another nesting level */
        for (int p = 0; p < 3; ++p) {
            /* Basic block I - inner to separate loop */
            g_sink = n * p;
        }
    }
    
    /* Additional loop with different structure for variety */
    int counter = 0;
    while (counter < 4) {
        /* Basic block J - while loop body */
        g_array[counter + 40] = counter * counter;
        counter++;
    }
}

/* 
 * Additional function with different loop pattern
 * Ensures multiple loop candidates exist in compilation unit
 */
__attribute__((noinline, noipa))
void auxiliary_loops(void) {
    volatile int aux_sink = 0;
    
    /* Loop with post-increment in body */
    for (int x = 0; x < 12; x++) {
        aux_sink = x;
        for (int y = x; y < 10; y++) {
            g_array[y] += aux_sink;
        }
    }
}

int main(void) {
    printf("Starting hardware loop test...\n");
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 100; i++) {
        g_array[i] = i % 37;
    }
    
    /* Call the test function multiple times to ensure execution */
    test_nested_loops();
    auxiliary_loops();
    
    /* Create observable side effect */
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        sum += g_array[i];
    }
    
    printf("Result: %d (g_sink = %d)\n", sum, g_sink);
    printf("Test completed.\n");
    
    return 0;
}
