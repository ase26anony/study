/* test_hw_doloop.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
/* Mark as hot to encourage hardware loop optimization */
__attribute__((hot, noinline))
void test_loop_patterns(int N, int *results) {
    volatile int limit = N;  /* Prevent constant propagation */
    int i, j, k, m;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop (disjoint from others initially) */
    for (i = 0; i < limit; i++) {
        results[i] = i * 2;
        sum1 += results[i];
    }
    
    /* Loop 2: Perfectly nested inside Loop 3 */
    /* This should create bitmap where Loop2 ⊆ Loop3 */
    for (j = 0; j < limit/2; j++) {
        /* Loop 3: Outer loop containing Loop 2 */
        for (k = 0; k < limit/3; k++) {
            results[j + k] += j * k;
            sum2 += results[j + k];
            
            /* Force side effect to prevent dead code elimination */
            asm volatile("" : : "r"(sum2) : "memory");
        }
    }
    
    /* Loop 4: Do-while loop (different structure) */
    m = 0;
    do {
        results[m % N] -= m;
        sum3 += results[m % N];
        m++;
    } while (m < limit);
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* Create CFG where blocks intersect but neither is subset of the other */
    int x = 0, y = 0;
    int flag = 0;
    
    /* Loop 5 */
    for (x = 0; x < limit; x++) {
        results[x] += x;
        
        /* Conditional that creates branch to Loop 6's block */
        if (x % 3 == 0 && flag == 0) {
            /* This goto creates partial overlap with Loop 6 */
            goto overlap_point;
        }
        
        results[x] *= 2;
        continue;
        
    overlap_point:
        /* Loop 6: Starts here, creating partial overlap */
        for (y = x; y < limit && y < x + 5; y++) {
            results[y] += y * 3;
            
            /* Jump back to Loop 5's continuation */
            if (y == x + 2) {
                flag = 1;
                goto continue_loop5;
            }
        }
        
    continue_loop5:
        /* Back to Loop 5's normal flow */
        results[x] /= 2;
    }
    
    /* Loop 7: Another loop with complex entry/exit */
    int z = 0;
    int start = limit / 4;
    
    /* Create entry block shared with previous loops */
    if (sum1 > 0) {
        /* Shared entry block */
        for (z = start; z < limit; z++) {
            /* Loop 8: Nested inside Loop 7 but with early exit */
            int w = 0;
            for (w = 0; w < 3; w++) {
                results[z] += w;
                if (results[z] > 1000) {
                    /* Early exit creates different block bitmap */
                    goto early_exit;
                }
            }
            
            if (z % 2 == 0) {
                /* Additional block in Loop 7 but not in Loop 8 */
                results[z] -= z;
            }
        }
    }
    
early_exit:
    /* Final computation using all sums */
    results[0] = sum1 + sum2 + sum3;
}

/* Helper function to create more loop nesting depth */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
__attribute__((noinline))
void nested_helper(int *arr, int n, int depth) {
    if (depth <= 0) return;
    
    /* Create multiple nested loops at different depths */
    for (int a = 0; a < n; a++) {
        for (int b = 0; b < depth; b++) {
            arr[a] += a * b;
            for (int c = 0; c < 2; c++) {
                arr[b] -= c;
            }
        }
    }
}

int main() {
    const int N = 100;
    int *array = (int*)malloc(N * sizeof(int));
    
    /* Initialize with volatile to prevent compile-time optimization */
    volatile int init_val = 42;
    for (int i = 0; i < N; i++) {
        array[i] = i + init_val;
    }
    
    /* Call the function with carefully constructed loops */
    test_loop_patterns(N, array);
    
    /* Call helper to create additional nested loops */
    nested_helper(array, N/2, 3);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += array[i];
        checksum ^= (checksum << 3);
    }
    
    printf("Result checksum: %d\n", checksum);
    
    /* Additional loops in main to increase CFG complexity */
    int temp = 0;
    
    /* Adjacent loops with shared variables */
    for (int i = 0; i < 50; i++) {
        temp += array[i % N];
    }
    
    for (int i = 50; i < 100; i++) {
        temp -= array[i % N];
    }
    
    /* Loop with multiple entry points (complex CFG) */
    int idx = 0;
entry_point_1:
    while (idx < 30) {
        array[idx] += temp;
        idx++;
        if (idx == 15) {
            goto entry_point_2;
        }
    }
    goto loop_end;
    
entry_point_2:
    while (idx < 30) {
        array[idx] -= temp;
        idx++;
    }
    
loop_end:
    printf("Final temp: %d\n", temp);
    
    free(array);
    return 0;
}
