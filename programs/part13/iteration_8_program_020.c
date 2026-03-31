/* Complex DDG edge creation stress test
 * Designed to trigger ddg_edge allocation and initialization
 * in GCC's DDG builder (ddg.cc lines 749-757)
 */

#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP(var) asm volatile("" : : "r"(var))
#define MEM_BARRIER() asm volatile("" ::: "memory")

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Heavy memory aliasing with mixed dependencies */
NOOPT static int kernel1(int *arr1, int *arr2, int n) {
    int sum = 0;
    int tmp1, tmp2, tmp3, tmp4;
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    
    /* Force pointer aliasing - arr2 may alias arr1 */
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if/else */
        if (i & 1) {
            /* True dependency chain (RAW) */
            tmp1 = *p1 + i;      /* Read from memory */
            tmp2 = tmp1 * 2;     /* RAW on tmp1 */
            tmp3 = tmp2 / 3;     /* RAW on tmp2 - higher latency */
            
            /* Anti-dependency (WAR) */
            a = tmp3;           /* Read tmp3 */
            tmp3 = i * 7;       /* Write tmp3 - WAR with previous read */
            
            /* Memory anti-dependency */
            *p1 = a + tmp3;     /* Write to memory that was read earlier */
            
            p1 = &arr1[(i + 1) % SIZE];
        } else {
            /* Output dependency (WAW) */
            tmp4 = *p2;         /* Read */
            tmp4 = tmp4 + 5;    /* WAW - overwrite tmp4 */
            
            /* Another true dependency chain */
            b = tmp4 % 13;      /* High latency modulo */
            c = b + 7;
            d = c * c;
            
            /* Memory true dependency across iterations */
            *p2 = d + arr1[i % SIZE];  /* Write */
            
            p2 = &arr2[(i + 2) % SIZE];
        }
        
        /* Complex nested loop to create more basic blocks */
        for (int j = 0; j < 3; ++j) {
            /* Mix of dependencies in nested scope */
            e = (e + j) * 2;
            f = e - 1;
            
            if (j & 1) {
                /* Control-dependent computation */
                sum += f;
            } else {
                sum -= f;
            }
        }
        
        /* Cross-iteration dependency (loop-carried) */
        a = b + c;      /* Uses values from current iteration */
        b = a - d;      /* Will be used next iteration */
        
        /* Volatile to prevent elimination */
        KEEP(sum);
        KEEP(a);
        KEEP(b);
    }
    
    MEM_BARRIER();
    return sum;
}

/* Kernel 2: Arithmetic chains with varying latencies and control flow */
NOOPT static int kernel2(int *arr, int n) {
    int x = 0, y = 0, z = 0, w = 0;
    int acc = 0;
    
    /* Multiple live variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    
    for (int i = 0; i < n; ++i) {
        /* Long true dependency chain */
        x = arr[i % SIZE] + v1;
        y = x * v2 - v3;        /* RAW on x */
        z = y / (v4 + 1);       /* RAW on y, higher latency division */
        w = z % (v5 | 1);       /* RAW on z, high latency modulo */
        
        /* Parallel chains that converge */
        v1 = v1 + v6;
        v2 = v2 - v7;
        v3 = v3 * 2;
        
        /* Conditional with anti-dependencies */
        if (w > 100) {
            int t = v8;         /* Read v8 */
            v8 = t + w;         /* Write v8 - WAR */
            acc += v8;
        } else if (w < 50) {
            int t = v7;         /* Read v7 */
            v7 = t - w;         /* Write v7 - WAR */
            acc -= v7;
        } else {
            /* Output dependencies */
            v6 = i;             /* Write v6 */
            v6 = w * 2;         /* WAW on v6 */
            acc ^= v6;
        }
        
        /* Loop-carried dependencies with distance > 0 */
        static int carry = 0;
        int old_carry = carry;  /* Read from previous iteration */
        carry = w + i;          /* Write for next iteration */
        
        /* Complex expression with many operands */
        v4 = (old_carry + v1 + v2 + v3 + v4 + v5) / 7;
        
        /* Nested loop with its own dependencies */
        for (int k = 0; k < 2; ++k) {
            int inner = v1 + k;
            v5 = (v5 + inner) & 0xFF;
            KEEP(inner);
        }
        
        /* Force all variables to stay live */
        KEEP(x); KEEP(y); KEEP(z); KEEP(w);
        KEEP(v1); KEEP(v2); KEEP(v3); KEEP(v4);
        KEEP(v5); KEEP(v6); KEEP(v7); KEEP(v8);
    }
    
    MEM_BARRIER();
    return acc + x + y + z + w;
}

/* Kernel 3: Pointer chasing with complex aliasing */
NOOPT static int kernel3(int *base, int n) {
    int *ptr1 = base;
    int *ptr2 = base + SIZE/2;
    int *ptr3 = base + SIZE/4;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int t1, t2, t3, t4;
    
    for (int i = 0; i < n; ++i) {
        /* Memory dependencies with potential aliasing */
        t1 = *ptr1;                 /* Read from ptr1 */
        *ptr2 = t1 + i;             /* Write to ptr2 - no alias with ptr1? */
        t2 = *ptr3;                 /* Read from ptr3 - may alias? */
        
        /* Pointer arithmetic that could cause aliasing */
        ptr1 = base + ((i * 17) % SIZE);
        ptr2 = base + ((i * 13) % SIZE);
        ptr3 = base + ((i * 11) % SIZE);
        
        /* Cross-iteration memory dependencies */
        static int buffer[10];
        buffer[i % 10] = t1 + t2;
        t3 = buffer[(i + 1) % 10];  /* Read from previous iteration */
        
        /* Multiple dependency types mixed */
        if (t3 > 0) {
            t4 = t3 * 2;            /* RAW on t3 */
            sum1 += t4;
            
            /* Anti-dependency in memory */
            int temp = buffer[0];   /* Read */
            buffer[0] = t4;         /* Write - WAR */
            sum2 ^= temp;
        } else {
            t4 = -t3;               /* RAW on t3 */
            sum1 -= t4;
            
            /* Output dependency */
            buffer[1] = i;          /* Write */
            buffer[1] = t4;         /* WAW */
        }
        
        /* Complex control flow within loop */
        switch (i % 4) {
            case 0:
                sum3 = (sum3 + t1) * 3;
                break;
            case 1:
                sum3 = (sum3 - t2) / 2;
                break;
            case 2:
                sum3 = (sum3 ^ t3) | 1;
                break;
            case 3:
                sum3 = (sum3 * t4) % 100;
                break;
        }
        
        /* Dependency across switch cases */
        t1 = sum3 + 1;
        KEEP(t1);
        
        /* Memory barrier to prevent reordering */
        MEM_BARRIER();
    }
    
    return sum1 + sum2 + sum3;
}

/* Simple PRNG to avoid library dependencies */
static int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int main() {
    int data[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call kernels with different access patterns */
    int result = 0;
    
    /* Kernel 1: Two arrays that may alias */
    result += kernel1(data, data + SIZE/2, ITERS);
    
    /* Kernel 2: Single array with arithmetic */
    result += kernel2(data, ITERS);
    
    /* Kernel 3: Pointer chasing within same array */
    result += kernel3(data, ITERS);
    
    /* Use result to prevent dead code elimination */
    sink = result;
    KEEP(result);
    
    return result & 0xFF;
}
