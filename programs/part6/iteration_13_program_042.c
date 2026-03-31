/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline))
int process_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    int i;
    
    /* Loop-carried state variables */
    int state = 0x12345678;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    /* Many temporary variables to create register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Volatile modifiers to prevent optimization */
    volatile int mod1 = 3, mod2 = 7, mod3 = 13;
    
    for (i = 0; i < N; i++) {
        /* 1. Loop-carried dependency: state depends on previous iteration */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* 2. Multiple independent arithmetic operations */
        tmp1 = i * mod1;
        tmp2 = i + mod2;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = tmp1 & tmp2;
        tmp5 = tmp1 | tmp2;
        tmp6 = tmp3 * tmp4;
        tmp7 = tmp5 - tmp6;
        tmp8 = tmp7 << 2;
        tmp9 = tmp8 >> 1;
        tmp10 = tmp9 % (mod3 + 1);
        
        /* 3. More operations using loop counter and state */
        tmp11 = (i * state) & 0xFF;
        tmp12 = (state >> 8) & 0xFF;
        tmp13 = tmp11 * tmp12;
        tmp14 = tmp13 + i;
        tmp15 = tmp14 ^ state;
        
        /* 4. Conditional memory access with loop-variant index */
        if (state & 1) {
            /* True loop-carried dependency: sum accumulates across iterations */
            sum += data[(state + i) % size];
        } else {
            /* Alternative path with different operations */
            sum -= data[(tmp15 + i) % size] & 0x7F;
        }
        
        /* 5. Multiple accumulators with loop-carried dependencies */
        acc1 = acc1 + tmp1;      /* Distance-1 dependency */
        acc2 = acc2 ^ tmp3;      /* Distance-1 dependency */
        acc3 = acc3 * (tmp5 + 1); /* Distance-1 dependency */
        acc4 = (acc4 << 1) | (tmp7 & 1); /* Distance-1 dependency */
        
        /* 6. Additional arithmetic to increase instruction count */
        tmp1 = tmp1 + tmp2;
        tmp3 = tmp3 * tmp4;
        tmp5 = tmp5 / (tmp6 ? tmp6 : 1);
        tmp7 = tmp7 - tmp8;
        tmp9 = tmp9 ^ tmp10;
        
        /* 7. Nested control flow with loop-variant condition */
        if (i & 3) {
            if (state > 0) {
                tmp11 = tmp11 * 2;
            } else {
                tmp11 = tmp11 / 2;
            }
        } else {
            tmp11 = tmp11 + 1;
        }
        
        /* 8. More operations to keep variables live */
        tmp12 = tmp12 + tmp13;
        tmp14 = tmp14 - tmp15;
        
        /* 9. Array access with complex indexing */
        int idx = (i * 17 + state) % size;
        if (idx >= 0 && idx < size) {
            acc1 += data[idx] * mod1;
        }
        
        /* 10. Final combination of temporaries */
        sum += (tmp1 + tmp3 + tmp5 + tmp7 + tmp9 + tmp11 + tmp12 + tmp14) & 0xFF;
    }
    
    /* Combine all results to prevent dead code elimination */
    return sum + acc1 + acc2 + acc3 + acc4 + state;
}

int main() {
    int i;
    int data[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Process the loop multiple times to ensure execution */
    int total = 0;
    volatile int iterations = 3;
    
    for (i = 0; i < iterations; i++) {
        total += process_loop(data, ARRAY_SIZE);
    }
    
    printf("Result: %d\n", total);
    
    /* Additional loop with different characteristics */
    {
        volatile int M = 300;
        int x = 0, y = 1, z = 2;
        int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        
        for (i = 0; i < M; i++) {
            /* Complex dependency chain */
            a = b + i;
            b = c * a;
            c = d ^ b;
            d = e | c;
            e = f & d;
            f = a - e;
            
            /* Memory access with loop-carried dependency */
            x = y + data[i % ARRAY_SIZE];
            y = z * x;
            z = x ^ y;
            
            /* Conditional update */
            if ((i + x) & 1) {
                a += data[(i + y) % ARRAY_SIZE];
            }
        }
        
        printf("Secondary result: %d\n", a + b + c + d + e + f + x + y + z);
    }
    
    return 0;
}
