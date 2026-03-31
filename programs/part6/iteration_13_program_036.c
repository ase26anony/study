/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms -fdump-rtl-sched1 modulo-sched-test.c -o modulo-sched-test
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
    int state = 123456789;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    
    /* Many temporary variables to create register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Modifiers to prevent optimization */
    volatile int mod1 = 7, mod2 = 13, mod3 = 31;
    
    for (i = 0; i < N; i++) {
        /* 1. Loop-carried dependency: state depends on previous iteration */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* 2. Multiple independent arithmetic operations */
        tmp1 = i * mod1;
        tmp2 = i + mod2;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = tmp1 * tmp2;
        tmp5 = tmp3 & tmp4;
        tmp6 = tmp5 | state;
        tmp7 = tmp6 << 2;
        tmp8 = tmp7 >> 1;
        tmp9 = tmp8 * 3;
        tmp10 = tmp9 / 5;
        tmp11 = tmp10 % 17;
        tmp12 = tmp11 + i;
        tmp13 = tmp12 * mod3;
        tmp14 = tmp13 - tmp1;
        tmp15 = tmp14 ^ tmp5;
        
        /* 3. Memory access with variable indexing */
        int idx = (state + i) % size;
        sum += data[idx] * mod1;
        
        /* 4. Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Complex memory access pattern */
            int alt_idx = (idx * 13 + 7) % size;
            acc1 += data[alt_idx] * tmp3;
            
            /* More arithmetic in conditional path */
            tmp1 = tmp1 * 2 + 1;
            tmp2 = tmp2 | 0x5555;
        } else {
            /* Alternative path with different operations */
            acc2 += data[(idx + 1) % size] * tmp4;
            tmp3 = tmp3 ^ 0xAAAA;
            tmp4 = tmp4 & 0x3333;
        }
        
        /* 5. Multiple accumulators with loop-carried dependencies */
        acc3 = acc3 + tmp5;          /* Simple accumulation */
        acc4 = acc4 ^ tmp6;          /* XOR accumulation */
        acc5 = (acc5 * 3 + tmp7) % 1000;  /* Mixed operation accumulation */
        
        /* 6. Additional arithmetic to increase instruction count */
        tmp8 = (tmp8 + i) * 11;
        tmp9 = tmp9 - (i % 19);
        tmp10 = tmp10 | (tmp1 & tmp2);
        tmp11 = tmp11 ^ (tmp3 | tmp4);
        tmp12 = tmp12 + tmp5 * tmp6;
        tmp13 = tmp13 - tmp7 / (mod2 + 1);
        tmp14 = tmp14 & (tmp8 ^ tmp9);
        tmp15 = tmp15 | (tmp10 & tmp11);
        
        /* 7. Another conditional with different condition */
        if ((i & 3) == 0) {
            acc1 = acc1 + tmp12;
            tmp13 = tmp13 * 2;
        } else if ((i & 3) == 1) {
            acc2 = acc2 - tmp13;
            tmp14 = tmp14 >> 1;
        } else {
            acc3 = acc3 ^ tmp14;
            tmp15 = tmp15 << 1;
        }
        
        /* 8. Cross-iteration dependency through array */
        if (i > 0) {
            int prev_idx = (i - 1) % size;
            acc4 += data[prev_idx] * tmp15;
        }
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = sum + acc1 + acc2 + acc3 + acc4 + acc5;
    result = result ^ state;  /* Include final state */
    
    return result;
}

/* Another loop with different characteristics */
__attribute__((noinline))
int process_loop2(int *data, int size) {
    volatile int M = 300;
    int i, j;
    
    int total = 0;
    int carry = 1;  /* Loop-carried dependency */
    
    /* Even more temporaries for additional pressure */
    int a, b, c, d, e, f, g, h, p, q, r, s, t, u, v, w, x, y, z;
    
    for (i = 0; i < M; i++) {
        /* Strong loop-carried dependency chain */
        carry = (carry * 6364136223846793005ULL + 1442695040888963407ULL) & 0xFFFFFFFF;
        
        /* Complex address calculation */
        int base = (carry + i) % size;
        
        /* Many parallel computations */
        a = data[base] * 3;
        b = data[(base + 1) % size] * 5;
        c = data[(base + 2) % size] * 7;
        d = a + b;
        e = b - c;
        f = c * a;
        g = d & e;
        h = e | f;
        p = g ^ h;
        q = p << (i % 4);
        r = q >> 1;
        s = r + i;
        t = s * carry;
        u = t % 97;
        v = u ^ 0x1234;
        w = v & 0xABCD;
        x = w | 0x5678;
        y = x * 11;
        z = y / 3;
        
        /* Nested if-else chain */
        if (carry & 0x1) {
            total += a + d + g;
        } else if (carry & 0x2) {
            total += b + e + h;
        } else if (carry & 0x4) {
            total += c + f + p;
        } else {
            total += q + r + s;
        }
        
        /* Conditional with side effects */
        if ((total & 0xFF) > 128) {
            t = t * 2 + 1;
            u = u ^ t;
        }
        
        /* Update multiple state variables */
        a = b + c;
        b = c + d;
        c = d + e;
        d = e + f;
        
        /* Another loop-carried dependency */
        total = (total + z) & 0x7FFFFFFF;
    }
    
    return total + carry;
}

int main() {
    /* Initialize with random data */
    int data[ARRAY_SIZE];
    int i;
    
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Process with first loop */
    int result1 = process_loop(data, ARRAY_SIZE);
    
    /* Process with second loop */
    int result2 = process_loop2(data, ARRAY_SIZE);
    
    /* Combine and print results */
    int final_result = result1 + result2;
    printf("Result: %d\n", final_result);
    
    /* Use result to prevent optimization */
    volatile int sink = final_result;
    
    return 0;
}
