/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(int n, int *results) {
    volatile int N = n;  /* Prevent constant propagation */
    volatile int M = n / 2;
    volatile int K = n / 3;
    
    int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < N; i++) {
        sum1 += i * 2;
        results[i] = sum1;
    }
    
    /* Loop 2: Adjacent but disjoint loop - should have no block intersection */
    for (j = 0; j < M; j++) {
        sum2 += j * 3;
        results[N + j] = sum2;
    }
    
    /* Loop 3: Perfectly nested inside conditional - creates subset relationship */
    if (N > 10) {
        for (k = 0; k < K; k++) {
            sum3 += k * 4;
            results[N + M + k] = sum3;
        }
    }
    
    /* Loop 4: Do-while loop for CFG variation */
    int l = 0;
    do {
        sum1 += l * 5;
        results[l] += sum1;
        l++;
    } while (l < N/2);
    
    /* Loop 5: Creates partial overlap with Loop 6 via goto */
    int m = 0;
    int flag = 0;
    
    /* Start of overlapping region */
overlap_start:
    if (m < N) {
        sum2 += m * 6;
        results[m] += sum2;
        
        /* Conditional that creates partial overlap */
        if (flag == 0 && m > N/2) {
            flag = 1;
            goto overlap_middle;  /* Jump into Loop 6's region */
        }
        m++;
        goto overlap_start;
    }
    
    /* Loop 6: Partially overlaps with Loop 5 */
    int p = 0;
overlap_middle:
    while (p < M) {
        sum3 += p * 7;
        results[N - p - 1] += sum3;
        
        /* Jump back to Loop 5's region to create intersection */
        if (p == M/2 && flag == 1) {
            flag = 2;
            goto overlap_start;
        }
        p++;
    }
    
    /* Loop 7: Another loop that might be analyzed */
    for (int q = 0; q < K; q++) {
        /* Inline assembly to prevent optimization */
        asm volatile("" : "+r"(q) : : "memory");
        results[q] ^= sum1;
    }
    
    /* Final side effect to prevent dead code elimination */
    results[0] = sum1 + sum2 + sum3;
}

/* Secondary function with more complex nesting */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void nested_loops_complex(int n, int *arr) {
    volatile int limit = n;
    int a, b, c;
    
    /* Outer loop - will contain inner loops */
    for (a = 0; a < limit; a++) {
        arr[a] = a;
        
        /* Inner loop 1 - perfectly nested */
        for (b = 0; b < a; b++) {
            arr[a] += b;
            
            /* Innermost loop - triple nesting */
            for (c = 0; c < b; c++) {
                arr[a] -= c;
            }
        }
        
        /* Another inner loop at same level - creates sibling relationship */
        for (b = limit - 1; b > a; b--) {
            arr[b] += arr[a];
        }
    }
    
    /* Loop with early exit that might create interesting CFG */
    int d = 0;
    while (d < limit) {
        if (arr[d] > 1000) {
            break;  /* Creates additional basic blocks */
        }
        arr[d] *= 2;
        d++;
    }
}

int main() {
    const int SIZE = 100;
    int *results = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = i;
    }
    
    /* Call function with carefully constructed loops */
    test_loop_patterns(SIZE, results);
    
    /* Call second function for more coverage opportunities */
    nested_loops_complex(SIZE / 2, results + SIZE / 2);
    
    /* Compute checksum to ensure execution */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(results);
    return 0;
}
