/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture that supports hardware loops */
#ifdef __ARM_ARCH
#warning "Compiling for ARM hardware loops"
#elif defined(__riscv)
#warning "Compiling for RISC-V hardware loops"
#endif

/* Prevent inlining to preserve CFG structure */
__attribute__((noinline,noipa))
void test_loop_patterns(int N, int M, int K) {
    volatile int prevent_dead_code = 0;
    int array1[1000], array2[1000], array3[1000];
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Initialize arrays with volatile to prevent constant propagation */
    volatile int init_val = 7;
    for (int i = 0; i < 1000; i++) {
        array1[i] = init_val + i;
        array2[i] = init_val - i;
        array3[i] = init_val * i;
    }
    
    /* ============================================
       PATTERN 1: Two adjacent disjoint loops
       Should NOT trigger bitmap intersection
       ============================================ */
    
    /* First countable loop - simple for loop */
    for (int i = 0; i < N; i++) {
        /* Simple body to keep it hardware-loop eligible */
        array1[i % 1000] += i;
        sum1 += array1[i % 1000];
        /* Prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Second countable loop - adjacent but disjoint */
    for (int j = 0; j < M; j++) {
        array2[j % 1000] *= 2;
        sum2 += array2[j % 1000];
        asm volatile("" : : : "memory");
    }
    
    /* ============================================
       PATTERN 2: Perfectly nested loops  
       Should trigger: loop->loops.safe_push(other)
       ============================================ */
    
    /* Outer loop */
    for (int outer = 0; outer < K; outer++) {
        /* Inner loop - completely contained within outer */
        for (int inner = 0; inner < 10; inner++) {
            array3[(outer * 10 + inner) % 1000] += outer + inner;
            sum3 += array3[(outer * 10 + inner) % 1000];
            asm volatile("" : : : "memory");
        }
    }
    
    /* ============================================
       PATTERN 3: Partially overlapping loops
       Should trigger: other->loops.safe_push(loop)
       ============================================ */
    
    /* Loop A - do-while style for CFG variation */
    int a = 0;
    do {
        array1[a % 1000] -= 3;
        sum1 += array1[a % 1000];
        
        /* Conditional that creates shared basic blocks */
        if (a % 3 == 0) {
            /* This block will be shared with Loop B */
            array2[a % 1000] += 5;
            sum2 += array2[a % 1000];
            asm volatile("" : : : "memory");
            
            /* CRITICAL: Jump to label inside Loop B */
            if (a < N/2) {
                goto shared_block;
            }
        }
        
        a++;
    } while (a < N);
    
    /* Loop B - partially overlaps with Loop A */
    int b = N/2;
    while (b < N + M) {
        /* Shared block label - creates CFG intersection */
        shared_block:
        array3[b % 1000] *= 3;
        sum3 += array3[b % 1000];
        asm volatile("" : : : "memory");
        
        /* More conditional complexity */
        if (b % 4 == 0) {
            array1[b % 1000] += 7;
            sum1 += array1[b % 1000];
        }
        
        b++;
    }
    
    /* ============================================
       PATTERN 4: Complex adjacency with goto
       Creates more CFG edges for analysis
       ============================================ */
    
    int x = 0;
    start_loop_x:
    if (x >= K) goto end_loop_x;
    
    array1[x % 1000] = array2[x % 1000] + array3[x % 1000];
    sum1 += array1[x % 1000];
    
    /* Conditional goto that jumps into another loop's region */
    if (x == K/2) {
        int y = 0;
        /* This creates partial overlap */
        while (y < 5) {
            array2[y % 1000] += x;
            sum2 += array2[y % 1000];
            y++;
        }
    }
    
    x++;
    goto start_loop_x;
    end_loop_x:
    
    /* Use results to prevent dead code elimination */
    prevent_dead_code = sum1 + sum2 + sum3;
    asm volatile("" : "+r" (prevent_dead_code) : : "memory");
}

/* Main function with volatile parameters to prevent constant folding */
int main() {
    /* Use volatile to make loop bounds runtime values */
    volatile int N = 100;
    volatile int M = 50;
    volatile int K = 30;
    
    /* Call the test function multiple times to ensure execution */
    int total = 0;
    for (int iter = 0; iter < 3; iter++) {
        test_loop_patterns(N + iter, M + iter, K + iter);
        total += N + M + K + iter;
    }
    
    /* Print something to guarantee execution */
    printf("Test completed. Total iterations: %d\n", total);
    
    return 0;
}
