/* test_hw_loops.c - Designed to trigger specific uncovered lines in hw-doloop.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(int N) {
    volatile int limit = N;  /* Prevent constant propagation */
    int array1[1000], array2[1000], array3[1000];
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int i, j, k;
    
    /* Initialize arrays with volatile to prevent optimization */
    volatile int init = 1;
    for (int idx = 0; idx < 1000; idx++) {
        array1[idx] = idx + init;
        array2[idx] = idx * 2 + init;
        array3[idx] = idx * 3 + init;
    }
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < limit; i++) {
        /* Simple body with side effect */
        array1[i] += i * 2;
        sum1 += array1[i];
        
        /* This creates a basic block that will be in Loop 1's bitmap */
        if (i % 2 == 0) {
            /* Basic block A - part of Loop 1 */
            array1[i] *= 2;
        }
    }
    
    /* Loop 2: Adjacent but disjoint loop - should not intersect with Loop 1 */
    for (j = 0; j < limit / 2; j++) {
        array2[j] += j * 3;
        sum2 += array2[j];
        
        /* Different conditional pattern */
        if (j % 3 == 0) {
            array2[j] /= 2;
        }
    }
    
    /* Loop 3: Perfectly nested within Loop 4 */
    /* First, create outer loop (Loop 4) */
    for (k = 0; k < limit / 3; k++) {
        /* Loop 3: Inner loop - entirely contained within Loop 4 */
        /* This should trigger: loop->loops.safe_push(other) */
        for (int inner = 0; inner < 5; inner++) {
            array3[k] += inner;
            sum3 += array3[k];
        }
        
        /* Additional basic block in outer loop */
        if (k % 4 == 0) {
            array3[k] -= 10;
        }
    }
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* Create complex CFG with goto to achieve partial overlap */
    int x = 0;
    
    /* Loop 5 */
    while (x < limit) {
        /* Basic block B1 - part of Loop 5 */
        array1[x] += x;
        
        if (x % 5 == 0) {
            /* Basic block B2 - part of Loop 5 */
            array2[x] += x * 2;
            
            /* CRITICAL: Jump into Loop 6's body */
            /* This creates partial overlap between Loop 5 and Loop 6 */
            if (x < limit / 2) {
                goto loop6_entry;  /* Jump to label inside Loop 6 */
            }
        }
        
        /* Basic block B3 - part of Loop 5 */
        array3[x] += x * 3;
        x++;
        
        /* Skip the rest if we jumped */
        if (x >= limit) break;
        
        continue;
        
    loop6_entry:
        /* Loop 6: Starts here, partially overlapping with Loop 5 */
        /* This should trigger: other->loops.safe_push(loop) */
        int y = x;
        do {
            /* Basic block C1 - shared between Loop 5 and Loop 6 */
            array1[y] -= y;
            array2[y] -= y * 2;
            
            if (y % 7 == 0) {
                /* Basic block C2 - part of Loop 6 */
                array3[y] *= 2;
                
                /* Jump back to Loop 5 */
                goto loop5_continue;
            }
            
            y++;
        } while (y < limit && y < x + 10);
        
        /* If we didn't goto back, continue normally */
        x = y;
        continue;
        
    loop5_continue:
        /* Back in Loop 5, but we're also in Loop 6's region */
        x = y + 1;
        continue;
    }
    
    /* Loop 7: do-while loop for CFG variation */
    int z = 0;
    do {
        array1[z % 1000] += z % 17;
        z++;
    } while (z < limit / 4);
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
}

/* Main function to drive execution */
int main() {
    volatile int N = 100;  /* Use volatile to prevent compile-time evaluation */
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N);
    
    /* Additional calls with different sizes to explore more paths */
    test_loop_patterns(50);
    test_loop_patterns(200);
    
    printf("Hardware loop pattern test completed.\n");
    return 0;
}
