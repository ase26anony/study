/* ddg_test.c - Complex dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP(var) asm volatile("" : : "r"(var))
#define MEM_BARRIER() asm volatile("" ::: "memory")

/* Global to force memory dependencies */
int global_array[SIZE];
int global_accum = 0;

/* Kernel 1: Memory aliasing and pointer arithmetic dependencies */
NOOPT static int kernel1_memory_aliasing(int *arr1, int *arr2, int n) {
    int sum = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    int *p1, *p2, *p3;
    
    /* Complex pointer setup to create aliasing possibilities */
    p1 = arr1;
    p2 = arr2;
    p3 = &arr1[n/2];  /* Potential alias */
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if-else chain */
        if (i & 1) {
            /* True dependency chain (RAW) */
            tmp1 = *p1 + i;
            tmp2 = tmp1 * 2;
            tmp3 = tmp2 - tmp1;
            
            /* Anti-dependency (WAR) - read before write */
            int read_before_write = *p3;
            *p3 = tmp3 + read_before_write;
            
            /* Memory dependency with potential aliasing */
            *p1 = tmp3;
            sum += *p2;  /* p2 may alias with p1 */
            
            /* Output dependency (WAW) */
            tmp4 = tmp3 % 17;
            tmp4 = tmp4 * 3;  /* Overwrites tmp4 */
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = *p2 - i;
            tmp2 = tmp1 / 3;
            
            /* Nested loop for additional complexity */
            for (int j = 0; j < 3; ++j) {
                tmp2 += j;
                /* Control dependency inside nested loop */
                if (tmp2 & 1) {
                    tmp3 = tmp2 * 7;
                } else {
                    tmp3 = tmp2 * 11;
                }
            }
            
            /* Memory operations with different latencies */
            *p2 = tmp3;
            tmp5 = *p1 + *p2;  /* Could be high latency due to aliasing */
            
            /* Complex arithmetic chain */
            sum = (sum * 13 + tmp5) % 997;
        }
        
        /* Loop-carried dependency with distance > 0 */
        if (i > 0) {
            /* Use value from previous iteration */
            arr1[i] = arr1[i-1] + sum;
        }
        
        /* Pointer increment with wrap-around */
        p1 = &arr1[(i + 1) % (n/4)];
        p2 = &arr2[(i * 3) % (n/4)];
        p3 = &arr1[(i * 5) % (n/2)];
        
        /* Prevent dead code elimination */
        KEEP(sum);
        KEEP(tmp1);
        KEEP(tmp5);
    }
    
    return sum;
}

/* Kernel 2: Arithmetic chains with varying latencies and control flow */
NOOPT static int kernel2_arithmetic_chains(int *arr, int n) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, j = 9, k = 10;
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple parallel dependency chains */
        
        /* Chain 1: Integer arithmetic with varying latencies */
        a = arr[i] + b;
        b = a * c;          /* Multiplication often has higher latency */
        c = b / (a + 1);    /* Division has very high latency */
        d = c % 19;         /* Modulo also high latency */
        
        /* Chain 2: Mix of operations */
        e = e + arr[i & 255];
        f = e - f;
        g = f * g;
        h = g >> 2;
        
        /* Chain 3: Conditional updates creating control dependencies */
        if ((i ^ arr[i]) & 1) {
            j = j * 3 + 1;
            k = k - arr[i % 256];
        } else {
            j = j / 2;
            k = k + arr[(i + 128) % 256];
        }
        
        /* Cross-chain dependencies */
        if (i % 3 == 0) {
            a = a + j;
            b = b - k;
        } else if (i % 3 == 1) {
            c = c * j;
            d = d ^ k;      /* Bitwise operation, different data type */
        }
        
        /* Loop-carried output dependencies (WAW) */
        result = a + b;
        result = result + c + d;  /* Overwrites result */
        result = result * e;
        result = result / (f + 1);
        
        /* Anti-dependency (WAR) pattern */
        int temp = arr[i];
        arr[i] = result;
        result = temp + result;
        
        /* Force all variables to stay live */
        KEEP(a); KEEP(b); KEEP(c); KEEP(d); KEEP(e);
        KEEP(f); KEEP(g); KEEP(h); KEEP(j); KEEP(k);
        MEM_BARRIER();
    }
    
    return result;
}

/* Kernel 3: Complex control flow with nested loops */
NOOPT static int kernel3_control_flow(int *arr1, int *arr2, int n) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Outer switch creates multiple basic blocks */
        switch (i % 4) {
            case 0:
                /* Memory-intensive block */
                x = arr1[i] * arr2[i];
                y = arr1[(i + 1) % n] + arr2[(i + 2) % n];
                z = x * y;
                
                /* Inner loop with carried dependency */
                for (int j = 0; j < 2; ++j) {
                    z = z + arr1[(i + j) % n];
                    acc1 = acc1 ^ z;  /* Bitwise dependency */
                }
                break;
                
            case 1:
                /* Arithmetic chain block */
                x = x + i;
                y = y * x;
                z = z - y;
                
                /* Conditional with anti-dependency */
                int old_z = z;
                z = acc2 + old_z;
                acc2 = old_z * 2;
                break;
                
            case 2:
                /* Mixed operations */
                x = arr1[i] % 17;
                y = arr2[i] / 3;
                z = (x * y) & 0xFF;  /* Bitwise AND */
                
                /* Output dependency chain */
                acc3 = x + y;
                acc3 = acc3 + z;
                acc3 = acc3 * 2;
                break;
                
            case 3:
                /* Complex dependency across iterations */
                x = acc1 + acc2;
                y = acc3 - x;
                z = (z << 3) | (x & 7);  /* Shift and bitwise */
                
                /* Memory dependency with potential aliasing */
                arr1[(i + 3) % n] = z;
                x = arr1[i] + arr1[(i + 3) % n];  /* Read what we just wrote */
                break;
        }
        
        /* Loop-carried true dependency */
        if (i > 1) {
            acc1 = acc1 + arr1[i-2];
            acc2 = acc2 * arr2[i-1];
        }
        
        /* Cross-iteration anti-dependency */
        int tmp = arr2[i];
        arr2[i] = acc3;
        acc3 = tmp + acc3;
        
        KEEP(x); KEEP(y); KEEP(z);
        KEEP(acc1); KEEP(acc2); KEEP(acc3);
    }
    
    return acc1 + acc2 + acc3;
}

/* Simple PRNG to avoid library dependencies */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    int data1[SIZE];
    int data2[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; ++i) {
        data1[i] = simple_rand(&seed) % 1000;
        data2[i] = simple_rand(&seed) % 1000;
        global_array[i] = data1[i];
    }
    
    /* Call kernels with complex dependency patterns */
    int result1 = kernel1_memory_aliasing(data1, data2, ITERATIONS);
    int result2 = kernel2_arithmetic_chains(data1, ITERATIONS);
    int result3 = kernel3_control_flow(data1, data2, ITERATIONS);
    
    /* Combine results to prevent elimination */
    int final_result = result1 + result2 + result3;
    
    /* Force result to be observable */
    sink = final_result;
    asm volatile("" : : "r"(final_result));
    
    return final_result & 255;  /* Return non-zero to indicate execution */
}
