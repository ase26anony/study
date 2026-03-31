/* test_hw_doloop.c
 * Designed to trigger uncovered bitmap intersection logic in hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_doloop.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture support for hardware loops */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(int N, int *results) {
    volatile int limit = N;  /* Prevent constant propagation */
    int i, j, k;
    
    /* Array to ensure side effects */
    volatile int arr[100] = {0};
    
    /* LOOP 1: Simple countable loop - will create its own basic blocks */
    for (i = 0; i < limit; i++) {
        arr[i % 100] += i;
        results[0] += arr[i % 100];
    }
    
    /* LOOP 2: Adjacent but disjoint loop - shares no basic blocks with Loop 1 */
    for (j = 0; j < limit / 2; j++) {
        arr[(j * 2) % 100] *= 2;
        results[1] += arr[(j * 2) % 100];
    }
    
    /* LOOP 3: Perfectly nested inside conditional - creates hierarchical relationship */
    int counter = 0;
    do {
        /* Inner loop that will be perfectly contained within outer do-while */
        for (k = 0; k < 5; k++) {
            arr[(counter + k) % 100] -= k;
            results[2] += arr[(counter + k) % 100];
        }
        counter++;
    } while (counter < limit / 3);
    
    /* LOOP 4 and LOOP 5: Partially overlapping loops using goto */
    /* This creates the partial overlap scenario for bitmap_intersect_compl_p */
    
    /* Start of LOOP 4 */
    int x = 0;
    int y = 0;
    
loop4_start:
    if (x >= limit / 4) goto loop4_end;
    
    arr[x % 100] = x * x;
    results[3] += arr[x % 100];
    
    /* Conditional that may jump into LOOP 5 */
    if (x % 3 == 0) {
        /* Jump to a label inside LOOP 5's body */
        goto loop5_middle;
    }
    
    x++;
    goto loop4_start;
    
loop4_end:
    /* Continue to LOOP 5 */
    
    /* LOOP 5: Partially overlaps with LOOP 4 via goto */
    y = 0;
loop5_start:
    if (y >= limit / 4) goto loop5_end;
    
    arr[(y + 50) % 100] = y * y;
    results[4] += arr[(y + 50) % 100];
    
loop5_middle:  /* Shared label - creates CFG overlap */
    /* This basic block belongs to BOTH loops when entered from LOOP 4 */
    arr[(y + 25) % 100] += y;
    results[5] += arr[(y + 25) % 100];
    
    y++;
    goto loop5_start;
    
loop5_end:
    
    /* LOOP 6: Another loop with complex control flow for more bitmap analysis */
    int z = 0;
    int w = 0;
    
    while (z < limit / 5) {
        /* Nested if creates additional basic blocks */
        if (z % 2 == 0) {
            for (w = 0; w < 3; w++) {
                arr[(z + w) % 100] |= 1;
                results[6] += arr[(z + w) % 100];
            }
        } else {
            arr[z % 100] &= 0xFF;
            results[7] += arr[z % 100];
        }
        
        /* Another potential overlap point */
        if (z == limit / 10) {
            /* Small inner loop that might be analyzed separately */
            int t;
            for (t = 0; t < 2; t++) {
                arr[99] = t;
                results[8] += arr[99];
            }
        }
        
        z++;
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile("" ::: "memory");
}

/* Main function to drive execution */
int main() {
    const int N = 1000;
    int results[10] = {0};
    int *array = (int*)malloc(N * sizeof(int));
    
    if (!array) return 1;
    
    /* Initialize with random-ish data */
    for (int i = 0; i < N; i++) {
        array[i] = (i * 13) % 97;
    }
    
    /* Call the function with all the loop patterns */
    test_loop_patterns(N, results);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += results[i];
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    printf("Total checksum: %d\n", checksum);
    
    free(array);
    return 0;
}
