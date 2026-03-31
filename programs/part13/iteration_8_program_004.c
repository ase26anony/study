/* Complex dependency pattern generator for DDG edge creation coverage */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing and pointer arithmetic dependencies */
NOINLINE static void kernel_memory_aliasing(int* arr, int n, int* result) {
    int* p1 = arr;
    int* p2 = arr + n/2;
    int* p3 = arr + n/4;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (int i = 0; i < n; ++i) {
        /* True dependency chain (RAW) */
        tmp1 = *p1 + i;
        tmp2 = tmp1 * 2;
        tmp3 = tmp2 - *p2;
        
        /* Anti-dependency (WAR) - read before write to same location */
        int read_before_write = *p3;
        *p3 = tmp3 + read_before_write;
        
        /* Output dependency (WAW) */
        tmp4 = acc1 + tmp2;
        tmp4 = acc2 + tmp3;  // Overwrites tmp4
        
        /* Memory aliasing with potential dependencies */
        if (i & 1) {
            *p1 = tmp4 + *p2;
            acc1 += *p1;  // Creates MEM_DEP edge
        } else {
            *p2 = tmp4 - *p1;
            acc2 += *p2;
        }
        
        /* Complex control flow with dependencies */
        if (i % 3 == 0) {
            tmp1 = acc3 * 3;
            acc3 = tmp1 + *p3;
        } else if (i % 3 == 1) {
            tmp2 = acc3 / 2;
            acc3 = tmp2 - *p1;
        } else {
            tmp3 = acc3 % 17;
            acc3 = tmp3 + *p2;
        }
        
        /* Pointer movement with wrap-around */
        p1 = arr + ((i * 7) % n);
        p2 = arr + ((i * 13) % n);
        p3 = arr + ((i * 19) % n);
        
        KEEP_ALIVE(tmp1);
        KEEP_ALIVE(tmp2);
        KEEP_ALIVE(tmp3);
        KEEP_ALIVE(tmp4);
    }
    
    *result = acc1 + acc2 + acc3;
    global_acc += *result;
}

/* Kernel 2: Arithmetic chains with varying latencies and control dependencies */
NOINLINE static void kernel_arithmetic_chains(int* arr, int n, int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, j = 9, k = 10;
    
    int chain1, chain2, chain3, chain4;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple parallel dependency chains */
        
        /* Chain 1: High latency operations (division/modulo) */
        chain1 = arr[i] % 13;
        chain1 = chain1 / (arr[i % 16] + 1);
        chain1 = chain1 * 7;
        a = chain1 + b;
        
        /* Chain 2: Medium latency with anti-dependencies */
        int temp = c;
        c = d * 3;
        d = temp + i;  // WAR: temp read before c written
        
        /* Chain 3: Nested control flow with dependencies */
        if (i & 4) {
            chain2 = e * f;
            e = chain2 - g;
        } else {
            chain2 = h / (j + 1);
            h = chain2 + k;
        }
        
        /* Chain 4: Loop-carried output dependencies */
        static int persistent = 0;
        int old_persistent = persistent;
        persistent = old_persistent + i;
        chain3 = persistent * 2;
        
        /* Inter-chain dependencies */
        if (i % 5 == 0) {
            chain4 = a + c;
            f = chain4 * e;
        } else if (i % 5 == 1) {
            chain4 = b - d;
            g = chain4 / (h + 1);
        } else if (i % 5 == 2) {
            chain4 = e * j;
            j = chain4 % 19;
        } else if (i % 5 == 3) {
            chain4 = f + k;
            k = chain4 - a;
        } else {
            chain4 = g * h;
            b = chain4 + c;
        }
        
        /* Nested loop to create more basic blocks */
        for (int j = 0; j < 3; ++j) {
            int inner_tmp = chain1 + j;
            chain2 = inner_tmp * chain3;
            if (j & 1) {
                chain4 = chain2 - chain1;
            } else {
                chain4 = chain2 + chain1;
            }
            KEEP_ALIVE(inner_tmp);
        }
        
        /* Volatile operations to prevent elimination */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
        asm volatile("" : "+r"(e), "+r"(f), "+r"(g), "+r"(h));
        asm volatile("" : "+r"(j), "+r"(k));
    }
    
    *result = a + b + c + d + e + f + g + h + j + k;
    global_acc += *result;
}

/* Kernel 3: Complex control flow with mixed dependencies */
NOINLINE static void kernel_control_flow(int* arr, int n, int* result) {
    int x = 0, y = 0, z = 0;
    int w1 = 0, w2 = 0, w3 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Switch-like control flow with data dependencies */
        switch (i % 7) {
            case 0:
                x = arr[i] + y;
                y = x * 2;
                break;
            case 1:
                z = y - arr[i];
                x = z / 3;
                break;
            case 2:
                w1 = x + z;
                w2 = w1 * y;
                w3 = w2 - x;
                break;
            case 3:
                /* Output dependency chain */
                w1 = arr[i % 32];
                w1 = w1 + i;
                w1 = w1 * w2;
                break;
            case 4:
                /* Anti-dependency pattern */
                int read_val = w3;
                w3 = read_val + arr[i];
                y = read_val * 2;
                break;
            case 5:
                /* Memory dependency with pointer */
                int* ptr = arr + (i % 64);
                *ptr = x + y;
                z = *ptr - w1;
                break;
            case 6:
                /* Complex expression with multiple uses */
                w2 = (x * y) + (z / 2) - (w3 % 5);
                x = w2 + arr[i];
                y = w2 - arr[i];
                break;
        }
        
        /* Loop-carried dependency with distance > 0 */
        static int carry[4] = {0};
        int idx = i % 4;
        int old_carry = carry[idx];
        carry[idx] = old_carry + x + y;
        
        /* Conditional with nested loop */
        if (i % 11 == 0) {
            for (int j = 0; j < 2; ++j) {
                int inner_acc = 0;
                for (int k = 0; k < 2; ++k) {
                    inner_acc += arr[(i + j + k) % n];
                }
                w3 += inner_acc;
                KEEP_ALIVE(inner_acc);
            }
        }
        
        /* Force register pressure with many live variables */
        int t1 = x, t2 = y, t3 = z;
        int t4 = w1, t5 = w2, t6 = w3;
        int t7 = t1 + t2, t8 = t3 - t4, t9 = t5 * t6;
        int t10 = t7 / (t8 + 1), t11 = t9 % 17, t12 = t10 + t11;
        
        KEEP_ALIVE(t1); KEEP_ALIVE(t2); KEEP_ALIVE(t3);
        KEEP_ALIVE(t4); KEEP_ALIVE(t5); KEEP_ALIVE(t6);
        KEEP_ALIVE(t7); KEEP_ALIVE(t8); KEEP_ALIVE(t9);
        KEEP_ALIVE(t10); KEEP_ALIVE(t11); KEEP_ALIVE(t12);
    }
    
    *result = x + y + z + w1 + w2 + w3;
    global_acc += *result;
}

/* Simple PRNG to avoid library dependencies */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main() {
    int data[SIZE];
    int result1, result2, result3;
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 100;
    }
    
    /* Execute all kernels with complex dependency patterns */
    kernel_memory_aliasing(data, ITERATIONS, &result1);
    kernel_arithmetic_chains(data, ITERATIONS, &result2);
    kernel_control_flow(data, ITERATIONS, &result3);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2 + result3 + global_acc;
    
    /* Volatile sink to ensure all computation is kept */
    sink = final_result;
    
    return final_result & 0xFF;
}
