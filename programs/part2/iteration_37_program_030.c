/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __ARM_ARCH
#warning "Compiling for ARM architecture"
#else
/* Use attribute to target ARM if available */
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(volatile int N, int* results) {
    volatile int M = N / 2;
    volatile int K = N / 3;
    
    int i, j, k;
    
    /* Loop 1: Simple countable loop - will have its own basic blocks */
    for (i = 0; i < N; i++) {
        results[i] = i * 2;
        /* Simple side effect to prevent dead code elimination */
        asm volatile("" : "+r"(results[i]) : : "memory");
    }
    
    /* Loop 2: Adjacent but disjoint loop - no block intersection with Loop 1 */
    for (j = 0; j < M; j++) {
        results[j + N] = j * 3;
        asm volatile("" : "+r"(results[j + N]) : : "memory");
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* First, create outer loop context */
    int outer_acc = 0;
    for (k = 0; k < K; k++) {
        /* Loop 4: Outer loop */
        outer_acc += k;
        
        /* Loop 3: Inner loop - entirely contained within Loop 4's blocks */
        for (int l = 0; l < 5; l++) {
            results[k * 5 + l] += outer_acc + l;
            asm volatile("" : "+r"(results[k * 5 + l]) : : "memory");
        }
    }
    
    /* Loop 5 and Loop 6: Partially overlapping loops using goto */
    /* This creates CFG where neither loop is subset of the other */
    int x = 0;
    int y = 0;
    
    /* Loop 5 */
    for (x = 0; x < N; x++) {
        if (x % 3 == 0) {
            /* Jump into Loop 6's body, creating partial overlap */
            goto overlap_point;
        }
        results[x] += x * 7;
        asm volatile("" : "+r"(results[x]) : : "memory");
        
        if (x == N/2) continue;
        
overlap_point:
        /* Loop 6's body starts here */
        results[x + 100] = x * 11;
        asm volatile("" : "+r"(results[x + 100]) : : "memory");
        
        /* Loop 6: do-while for CFG variation */
        do {
            y++;
            results[y + 200] = y * 13;
            asm volatile("" : "+r"(results[y + 200]) : : "memory");
            if (y >= M) break;
        } while (1);
        
        /* Jump back to Loop 5 if needed */
        if (x < N - 1) {
            continue;
        }
    }
    
    /* Loop 7: Another loop that shares blocks with Loop 5 via conditional */
    int z = 0;
    while (z < K) {
        if (z % 2 == 0) {
            /* This block belongs to both Loop 5's CFG (via previous goto)
               and Loop 7's CFG */
            results[z + 300] = z * 17;
            asm volatile("" : "+r"(results[z + 300]) : : "memory");
            
            /* Early exit creates more complex CFG */
            if (results[z + 300] > 1000) {
                break;
            }
        }
        z++;
    }
}

/* Mark function as hot to encourage hardware loop optimization */
__attribute__((hot, noinline))
#ifdef __ARM_ARCH
#else
__attribute__((target("arch=armv8-a")))
#endif
void hot_loop_function(volatile int limit) {
    /* Allocate array with volatile elements to prevent optimizations */
    int* data = (int*)malloc(sizeof(int) * 1000);
    if (!data) return;
    
    /* Initialize with volatile reads */
    for (int i = 0; i < 1000; i++) {
        data[i] = i;
    }
    
    /* Call our complex loop pattern function */
    test_loop_patterns(limit, data);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += data[i];
    }
    
    /* Volatile write to ensure execution */
    volatile int final_sum = sum;
    (void)final_sum;
    
    free(data);
}

int main() {
    /* Use volatile to prevent constant propagation */
    volatile int N = 100;
    
    printf("Starting hardware loop test with N = %d\n", N);
    
    /* Call hot function multiple times to encourage optimization */
    for (int iter = 0; iter < 3; iter++) {
        hot_loop_function(N + iter);
    }
    
    printf("Test completed\n");
    
    /* Compute and print checksum to guarantee execution */
    volatile int checksum = 0;
    for (int i = 0; i < 1000; i++) {
        checksum += i * (i % 7);
    }
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
