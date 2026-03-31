/* sel-sched-dump-trigger.c
 * 
 * This program is specifically designed to trigger GCC's selective scheduler
 * debug dumping logic (sel-sched-dump.cc lines 159-163) by creating complex
 * scheduling scenarios that force the scheduler to emit detailed RTL dumps.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

/* ==================== FUNCTION ATTRIBUTES FOR SCHEDULING CONTROL ==================== */

/* Hot function with scheduling pressure optimization */
__attribute__((hot, optimize("O3", "sched-pressure"))) 
static float hot_vector_loop(float* restrict a, float* restrict b, float* restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b[i] depends on previous a[i] calculation */
        float temp = a[i] * 2.5f;
        
        /* WAR hazard: reusing temp variable */
        temp = temp + b[i];
        
        /* WAW hazard: multiple writes to c[i] */
        c[i] = temp * 1.5f;
        c[i] = c[i] - a[i];  /* Second write to c[i] */
        
        /* Pointer chasing pattern */
        float* ptr = &c[i];
        *ptr = *ptr + sinf(*ptr);
        
        sum += c[i];
        
        /* Inline assembly barrier splitting scheduling region */
        asm volatile("" ::: "memory");
    }
    return sum;
}

/* Cold function with noinline to create scheduling boundaries */
__attribute__((cold, noinline))
static double cold_complex_flow(int* arr, int n) {
    double result = 0.0;
    int* ptr = arr;
    
    /* Nested loops with mixed dependencies */
    for (int i = 0; i < n; i++) {
        int val = *ptr++;
        
        /* Complex control flow with multiple early exits */
        if (val < 0) {
            /* Early exit path */
            result += 1.0 / (1.0 - val);
            continue;
        }
        
        /* Switch statement with sparse cases */
        switch (val % 7) {
            case 0:
                result += sqrt(val);
                break;
            case 1:
                /* Conditional move mixed with computation */
                result += (val > 100) ? log(val) : exp(val);
                break;
            case 3:  /* Note: case 2 is intentionally missing */
                result += val * 0.5;
                /* Fall through */
            case 4:
                result += cos(val);
                break;
            default:
                result += val * val;
        }
        
        /* Another scheduling barrier with register clobber */
        asm volatile("" ::: "r0", "r1", "r2", "r3", "memory");
    }
    return result;
}

/* Function with forced unrolling and vectorization */
__attribute__((optimize("O3")))
static void simd_unrolled_ops(int* restrict dst, 
                              const int* restrict src1, 
                              const int* restrict src2, 
                              int size) {
    int i;
    
    /* Manual unrolling with pragma hint */
    #pragma GCC unroll 4
    for (i = 0; i < size; i++) {
        /* Mixed operations creating various hazards */
        int t1 = src1[i] + i;
        int t2 = src2[i] * 2;
        
        /* WAW hazard on dst[i] */
        dst[i] = t1 - t2;
        dst[i] = dst[i] & 0xFF;  /* Second write */
        
        /* RAW: dependency chain */
        t1 = dst[i] * 3;
        t2 = t1 + 7;
        dst[i] = t2 >> 1;
        
        /* Memory barrier every 8 iterations */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

/* Function with computed goto for complex control flow */
__attribute__((noinline))
static int computed_goto_pattern(int x) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3, 
        &&case_4, &&case_default
    };
    
    int index = x % 6;
    int result = 0;
    
    goto *jump_table[index];
    
case_0:
    result = x * 2;
    /* Fall through */
case_1:
    result += x / 3;
    goto end;
    
case_2:
    result = x & 0xF;
    /* Inline asm with specific constraints */
    asm volatile("mov %0, %0" : "+r"(result) ::);
    goto end;
    
case_3:
    result = x | 0xFF;
    goto end;
    
case_4:
    result = x ^ 0xAA;
    goto end;
    
case_default:
    result = ~x;
    
end:
    return result;
}

/* Outer loop pipelining stress test */
__attribute__((optimize("O3")))
static double outer_loop_pipelining(int n) {
    double matrix[64][64];
    double sum = 0.0;
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = (i * j) / 64.0;
        }
    }
    
    /* Nested loops with mixed fp/int operations */
    for (int iter = 0; iter < n; iter++) {
        #pragma GCC unroll 2
        for (int i = 1; i < 63; i++) {
            for (int j = 1; j < 63; j++) {
                /* Stencil computation with multiple dependencies */
                double north = matrix[i-1][j];
                double south = matrix[i+1][j];
                double east = matrix[i][j+1];
                double west = matrix[i][j-1];
                
                /* Long dependency chain */
                double avg = (north + south + east + west) / 4.0;
                double diff = matrix[i][j] - avg;
                double smoothed = matrix[i][j] - diff * 0.1;
                
                /* Conditional update */
                matrix[i][j] = (diff > 0) ? smoothed : avg;
                
                sum += matrix[i][j];
            }
        }
        
        /* Scheduling barrier between outer loop iterations */
        asm volatile("" ::: "r4", "r5", "r6", "r7", "memory");
    }
    
    return sum;
}

/* ==================== MAIN EXECUTION FLOW ==================== */

int main(void) {
    const int SIZE = 1024;
    float fa[SIZE], fb[SIZE], fc[SIZE];
    int ia[SIZE], ib[SIZE], ic[SIZE];
    double total = 0.0;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE; i++) {
        fa[i] = i * 0.1f;
        fb[i] = sinf(i * 0.05f);
        ia[i] = i * 3;
        ib[i] = i * 7;
    }
    
    printf("Starting selective scheduling stress tests...\n");
    
    /* Test 1: Hot vector loop with mixed operations */
    total += hot_vector_loop(fa, fb, fc, SIZE);
    
    /* Test 2: Cold function with complex control flow */
    total += cold_complex_flow(ia, SIZE);
    
    /* Test 3: SIMD and unrolled operations */
    simd_unrolled_ops(ic, ia, ib, SIZE);
    for (int i = 0; i < SIZE; i++) {
        total += ic[i];
    }
    
    /* Test 4: Computed goto pattern */
    for (int i = 0; i < 100; i++) {
        total += computed_goto_pattern(i);
    }
    
    /* Test 5: Outer loop pipelining */
    total += outer_loop_pipelining(10);
    
    /* Additional mixed workload */
    for (int i = 0; i < SIZE; i++) {
        /* Mix of operations in main */
        double x = fa[i];
        double y = fb[i];
        
        /* Conditional move */
        double z = (x > y) ? x * y : x / (y + 1.0);
        
        /* Inline asm barrier */
        asm volatile("" : "+r"(z) :: "memory");
        
        total += z + sin(x) * cos(y);
    }
    
    printf("Total result: %f\n", total);
    printf("(This value is meaningless - program designed for compiler coverage)\n");
    
    return (total > 0) ? 0 : 1;
}
