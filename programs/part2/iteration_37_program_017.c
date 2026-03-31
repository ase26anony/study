/* test_hw_doloop.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force architecture that supports hardware loops */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#elif __riscv
__attribute__((target("arch=rv64gc_zba")))
#endif
__attribute__((noinline, hot))
void test_loop_patterns(int N, int M, int *result1, int *result2, int *result3) {
    volatile int limit1 = N;  /* Prevent constant propagation */
    volatile int limit2 = M;
    volatile int limit3 = N/2;
    
    int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop A: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < limit1; i++) {
        /* Simple body with side effect */
        sum1 += i * 2;
        result1[i] = sum1;
        
        /* Conditional that creates separate basic blocks */
        if (i % 3 == 0) {
            /* This creates a basic block inside Loop A */
            sum1 -= i;
            asm volatile("" : : : "memory");  /* Prevent optimization */
        }
    }
    
    /* Loop B: Adjacent loop, disjoint from Loop A (no shared blocks) */
    /* This should make bitmap_intersect_p return false initially */
    for (j = 0; j < limit2; j++) {
        sum2 += j * 3;
        result2[j] = sum2;
        
        /* Create multiple basic blocks within this loop */
        if (j % 4 == 0) {
            sum2 += 100;
            asm volatile("" : : : "memory");
        } else {
            sum2 -= 50;
            asm volatile("" : : : "memory");
        }
    }
    
    /* Loop C: Perfectly nested inside a conditional structure */
    /* This creates hierarchical relationship where Loop C is subset of outer structure */
    if (limit3 > 0) {
        k = 0;
        do {
            sum3 += k * 5;
            result3[k] = sum3;
            
            /* Loop D: Perfectly nested inside Loop C */
            /* This should trigger: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap) */
            /* So loop->loops.safe_push(other) for Loop C containing Loop D */
            {
                int m;
                for (m = 0; m < 5; m++) {
                    sum3 += m;
                    asm volatile("" : : : "memory");
                    
                    /* Create another level of basic blocks */
                    if (m % 2 == 0) {
                        result3[k] += m;
                    }
                }
            }
            
            k++;
        } while (k < limit3);
    }
    
    /* Loop E and F: Partially overlapping loops with goto creating shared blocks */
    /* This should trigger the else if path: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap) */
    /* So other->loops.safe_push(loop) */
    {
        int x = 0;
        int y = 0;
        int shared_counter = 0;
        
        /* Loop E */
        for (x = 0; x < limit1/2; x++) {
            shared_counter += x;
            result1[x] += shared_counter;
            
            /* Conditional jump that can go to Loop F's body */
            if (x == limit1/4) {
                /* Jump to label inside Loop F */
                goto overlap_point;
            }
            
            /* Normal continuation of Loop E */
            asm volatile("" : : : "memory");
        }
        
        /* Loop F - partially overlaps with Loop E via goto */
        for (y = 0; y < limit2/2; y++) {
            shared_counter -= y;
            result2[y] += shared_counter;
            
            /* Label that Loop E can jump to */
            overlap_point:
            asm volatile("" : : : "memory");
            
            /* Create more basic blocks in this loop */
            if (y % 3 == 0) {
                shared_counter += 1000;
                goto continue_loop_f;  /* Forward goto within same loop */
            }
            
            shared_counter -= 500;
            
            continue_loop_f:
            /* Empty statement for label */
            ;
        }
    }
    
    /* Final volatile store to ensure all loops have side effects */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
}

/* Helper to compute checksum */
int compute_checksum(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum ^= arr[i];  /* Use XOR to create non-linear checksum */
    }
    return sum;
}

int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int N = 100;
    volatile int M = 80;
    
    /* Allocate arrays for results */
    int *result1 = (int*)malloc(N * sizeof(int));
    int *result2 = (int*)malloc(M * sizeof(int));
    int *result3 = (int*)malloc((N/2) * sizeof(int));
    
    if (!result1 || !result2 || !result3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) result1[i] = i;
    for (int i = 0; i < M; i++) result2[i] = i * 2;
    for (int i = 0; i < N/2; i++) result3[i] = i * 3;
    
    /* Call the function with all the loop patterns */
    test_loop_patterns(N, M, result1, result2, result3);
    
    /* Compute and print checksums to ensure execution */
    int checksum1 = compute_checksum(result1, N);
    int checksum2 = compute_checksum(result2, M);
    int checksum3 = compute_checksum(result3, N/2);
    
    printf("Checksums: %d, %d, %d\n", checksum1, checksum2, checksum3);
    
    /* Free memory */
    free(result1);
    free(result2);
    free(result3);
    
    return 0;
}
