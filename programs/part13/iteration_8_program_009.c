/* ddg_test.c - Complex dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP(var) asm volatile("" : : "r"(var))
#define MEM_BARRIER() asm volatile("" ::: "memory")

static int global_accumulator;

/* Kernel 1: Memory aliasing and pointer arithmetic dependencies */
NOOPT static int kernel_memory_aliasing(int* arr1, int* arr2, int n) {
    int sum = 0;
    int* p1 = arr1;
    int* p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if/else */
        if (i & 1) {
            /* True dependency (RAW) with memory */
            int temp = *p1;
            KEEP(temp);
            
            /* Anti-dependency (WAR) */
            *p1 = temp + i;
            
            /* Complex pointer arithmetic that may alias */
            p2 = arr1 + (i % 16);
            
            /* Output dependency (WAW) on sum */
            sum = sum + *p2;
            sum = sum * 2;  // Another WAW
            
            /* Memory barrier to prevent reordering */
            MEM_BARRIER();
        } else {
            /* Different dependency pattern in else branch */
            int tmp1 = arr2[i % 32];
            int tmp2 = tmp1 * 3;  // RAW
            
            /* WAR on arr1 */
            arr1[i % 16] = tmp2 + i;
            
            /* Chain of dependencies */
            for (int j = 0; j < 4; ++j) {
                tmp1 = tmp1 + j;  // WAW on tmp1
                tmp2 = tmp2 - tmp1;  // RAW from tmp1
                KEEP(tmp2);
            }
            
            sum += tmp2;
        }
        
        /* Loop-carried dependency */
        static int lc = 0;
        lc = lc + sum;  // Loop-carried WAW
        KEEP(lc);
        
        /* Pointer update with potential aliasing */
        p1 = &arr1[(i + 1) % 32];
    }
    
    return sum;
}

/* Kernel 2: Arithmetic chains with varying latencies and control flow */
NOOPT static int kernel_arithmetic_chains(int* data, int n) {
    int a = 0, b = 1, c = 2, d = 3, e = 4, f = 5;
    int g = 6, h = 7, j = 8, k = 9, l = 10;
    
    for (int i = 0; i < n; ++i) {
        /* Long arithmetic chain with mixed operations */
        a = b + data[i % SIZE];  // RAW from b, memory
        b = c * a;               // RAW from c and a
        c = d - b;               // RAW from d and b
        
        /* High-latency operation (simulated with multiple steps) */
        d = e % (data[(i + 1) % SIZE] | 1);  // Avoid division by zero
        e = f / ((d & 0xFF) + 1);           // Integer division
        
        /* Nested loop creating inner basic blocks */
        int inner_sum = 0;
        for (int m = 0; m < 3; ++m) {
            f = g + m;          // WAW on f
            g = h * f;          // RAW from h and f
            h = j - g;          // RAW from j and g
            
            /* Conditional inside nested loop */
            if ((m + i) & 1) {
                j = k + h;      // RAW from k and h
                k = l * j;      // RAW from l and j
            } else {
                l = j + k;      // RAW from j and k
                j = l - 1;      // RAW from l
            }
            
            inner_sum += f + g + h + j + k + l;
        }
        
        /* Output dependencies */
        int tmp = a + b;
        tmp = c + d;      // WAW on tmp
        tmp = e + f;      // Another WAW
        
        /* Anti-dependencies */
        int read_before_write = b;
        b = tmp + i;      // WAR on b
        
        /* Use all variables to keep them live */
        KEEP(a); KEEP(b); KEEP(c); KEEP(d);
        KEEP(e); KEEP(f); KEEP(g); KEEP(h);
        KEEP(j); KEEP(k); KEEP(l);
        KEEP(inner_sum);
        KEEP(read_before_write);
        
        /* Loop-carried dependency chain */
        static int chain = 0;
        chain = chain * 31 + tmp;  // WAW loop-carried
    }
    
    return a + b + c + d + e + f + g + h + j + k + l;
}

/* Kernel 3: Complex control flow with mixed dependencies */
NOOPT static int kernel_control_flow(int* arr, int n) {
    int x = 0, y = 0, z = 0;
    int* ptr1 = arr;
    int* ptr2 = arr + SIZE/2;
    
    for (int i = 0; i < n; ++i) {
        /* Multi-way branching */
        switch (i % 4) {
            case 0: {
                /* Memory dependencies with potential aliasing */
                int val = *ptr1;
                *ptr2 = val + x;  // RAW from val, WAR on *ptr2
                
                /* Arithmetic chain */
                x = y + z;
                y = x * 2;        // RAW from x
                z = y / 3;        // RAW from y
                
                ptr1 = &arr[(i + 5) % SIZE];
                break;
            }
            case 1: {
                /* Output dependencies */
                int temp = arr[i % SIZE];
                temp = temp + 1;   // WAW
                temp = temp * 2;   // WAW
                
                /* Anti-dependency */
                int read_x = x;
                x = temp + read_x;  // WAR on x
                
                /* True dependency across iterations */
                static int cross_iter = 0;
                y = cross_iter + i;
                cross_iter = y;     // Loop-carried WAW
                break;
            }
            case 2: {
                /* Nested loops with dependencies */
                for (int j = 0; j < 2; ++j) {
                    z = x + y + j;  // RAW from x, y
                    x = z - 1;      // RAW from z
                    
                    /* Memory operation */
                    arr[(i + j) % SIZE] = z;
                }
                break;
            }
            case 3: {
                /* Complex expression with many intermediates */
                int t1 = *ptr1;
                int t2 = *ptr2;
                int t3 = t1 + t2;   // RAW from t1, t2
                int t4 = t3 * x;    // RAW from t3, x
                int t5 = t4 / (y | 1); // RAW from t4, y
                
                /* All become inputs to next */
                x = t1 + t5;        // WAW on x
                y = t2 + t4;        // WAW on y
                z = t3 + t5;        // WAW on z
                
                ptr2 = &arr[(i + 3) % SIZE];
                break;
            }
        }
        
        /* Ensure all values are used */
        KEEP(x); KEEP(y); KEEP(z);
        KEEP(ptr1); KEEP(ptr2);
        
        /* Update pointers with wrap-around */
        ptr1 = &arr[(i * 7) % SIZE];
    }
    
    return x + y + z;
}

/* Simple PRNG to avoid library dependencies */
static uint32_t simple_rand(uint32_t* seed) {
    *seed = *seed * 1103515245 + 12345;
    return *seed;
}

int main(void) {
    int data[SIZE];
    uint32_t seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (int)(simple_rand(&seed) % 1000);
    }
    
    /* Call kernels with different access patterns */
    int result = 0;
    
    result += kernel_memory_aliasing(data, data + SIZE/2, ITERATIONS);
    KEEP(result);
    
    result += kernel_arithmetic_chains(data, ITERATIONS);
    KEEP(result);
    
    result += kernel_control_flow(data, ITERATIONS);
    KEEP(result);
    
    /* Force result to be observable */
    sink = result;
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
