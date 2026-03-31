/* Complex dependency pattern generator for DDG edge coverage */
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
NOOPT static int kernel1_memory_aliasing(int* arr1, int* arr2, int n) {
    int sum = 0;
    int* p1 = arr1;
    int* p2 = arr2;
    
    /* Complex loop with multiple basic blocks */
    for (int i = 0; i < n; ++i) {
        /* Basic Block A: Memory RAW and WAR dependencies */
        int temp1 = *p1;           /* Read from p1 */
        *p1 = temp1 + i;           /* Write to p1 (WAR with previous read) */
        
        /* Pointer aliasing - p2 may alias with p1 */
        int temp2 = *p2;           /* Read from p2 (potential RAW with p1 write) */
        
        /* Basic Block B: Conditional with output dependencies */
        if (i & 1) {
            /* WAW dependency on temp1 */
            temp1 = temp2 * 3;     /* Reassign temp1 */
            *p2 = temp1 + 7;       /* Write to p2 */
        } else {
            /* Different computation path */
            temp1 = temp2 / 2;     /* Another assignment to temp1 (WAW) */
            *p2 = temp1 - 5;
        }
        
        /* Basic Block C: Arithmetic chain with true dependencies */
        int chain1 = temp1 + temp2;
        int chain2 = chain1 * (i + 1);  /* RAW on chain1 */
        int chain3 = chain2 % 17;       /* RAW on chain2, higher latency */
        int chain4 = chain3 / 2;        /* RAW on chain3, high latency */
        
        /* Anti-dependency: read before write to same variable */
        int old_chain = chain4;
        chain4 = old_chain + *p1;       /* WAR on chain4 */
        
        /* Output dependency across iterations simulated */
        static int persistent;
        int save = persistent;          /* Read persistent */
        persistent = chain4;             /* Write persistent (WAR) */
        
        sum += chain4 + save;
        
        /* Update pointers with potential aliasing */
        p1 = &arr1[(i + 1) % (SIZE/2)];
        p2 = &arr2[(i * 3) % (SIZE/2)];
        
        /* Memory barrier to prevent reordering */
        MEM_BARRIER();
    }
    
    KEEP(sum);
    return sum;
}

/* Kernel 2: Complex arithmetic and control flow dependencies */
NOOPT static int kernel2_arithmetic_chains(int* data, int n) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    
    for (int i = 0; i < n; ++i) {
        /* Start of dependency chain */
        t1 = data[i % SIZE];
        t2 = data[(i + 1) % SIZE];
        
        /* Multiple parallel chains that converge */
        if (i % 3 == 0) {
            t3 = t1 * t2;           /* RAW on t1, t2 */
            t4 = t3 + i;            /* RAW on t3 */
            t5 = t4 % 13;           /* RAW on t4, high latency */
        } else if (i % 3 == 1) {
            t3 = t1 / (t2 + 1);     /* RAW on t1, t2, high latency */
            t4 = t3 - i;            /* RAW on t3 */
            t5 = t4 * 7;            /* RAW on t4 */
        } else {
            t3 = t1 + t2;           /* RAW on t1, t2 */
            t4 = t3 & 0xFF;         /* RAW on t3 */
            t5 = t4 | 0x80;         /* RAW on t4 */
        }
        
        /* Converging point with output dependencies */
        int common = t5;
        
        /* Nested loop to create more complex CFG */
        for (int j = 0; j < 3; ++j) {
            /* Anti-dependency pattern */
            int old_common = common;
            common = old_common + j;    /* WAR on common */
            
            /* True dependency chain inside nested loop */
            t6 = common * 2;            /* RAW on common */
            t7 = t6 + data[j];          /* RAW on t6, memory */
            t8 = t7 - old_common;       /* RAW on t7, WAR on old_common */
            
            acc1 += t8;
        }
        
        /* Another independent chain with loop-carried dependency */
        static int carry = 0;
        t9 = carry + t5;                /* Loop-carried RAW on carry */
        t10 = t9 * 3;                   /* RAW on t9 */
        carry = t10 % 256;              /* Loop-carried write */
        
        /* Multiple accumulators to keep values live */
        acc2 += t3;
        acc3 += t4;
        acc4 += t5;
        
        /* Complex condition with side effects */
        if ((t10 + acc1) & 1) {
            data[i % SIZE] = acc2;      /* Memory write */
            acc2 = 0;                   /* Anti-dependency on acc2 */
        }
        
        KEEP(t10);
        MEM_BARRIER();
    }
    
    /* Combine all accumulators */
    int result = acc1 + acc2 + acc3 + acc4;
    KEEP(result);
    return result;
}

/* Kernel 3: Mixed dependencies with unpredictable control flow */
NOOPT static int kernel3_mixed_patterns(int* a, int* b, int n) {
    int x = 0, y = 0, z = 0;
    int* ptrs[2] = {a, b};
    
    for (int i = 0; i < n; ++i) {
        /* Select pointer with data-dependent index */
        int idx = (i * 167) % 2;
        int* current = ptrs[idx];
        int* other = ptrs[1 - idx];
        
        /* Memory dependencies with potential aliasing */
        int read1 = current[i % (SIZE/2)];
        int read2 = other[(i * 3) % (SIZE/2)];
        
        /* Long arithmetic chain */
        int val1 = read1 + read2;
        int val2 = val1 * (i + 1);      /* RAW on val1 */
        int val3 = val2 - read1;        /* RAW on val2, WAR on read1 */
        int val4 = val3 / 2;            /* RAW on val3, high latency */
        int val5 = val4 % 19;           /* RAW on val4, high latency */
        
        /* Switch-like control flow */
        switch (i % 4) {
            case 0:
                x = val5 + x;           /* Loop-carried RAW on x */
                current[i % (SIZE/2)] = x; /* Memory write */
                break;
            case 1:
                y = val5 - y;           /* Loop-carried RAW on y */
                /* Output dependency */
                val5 = y * 2;           /* WAW on val5 */
                break;
            case 2:
                z = val5 ^ z;           /* Loop-carried RAW on z */
                /* Anti-dependency */
                int old_z = z;
                z = old_z + 1;          /* WAR on z */
                break;
            default:
                /* Complex expression with all variables */
                x = (x + y + z) / 3;
                y = val5 * 2;
                z = x - y;
                break;
        }
        
        /* Cross-iteration memory dependency */
        static int shared[4] = {0};
        int slot = i % 4;
        int old_shared = shared[slot];  /* Read */
        shared[slot] = val5 + old_shared; /* Write (WAR) */
        
        /* Force multiple live values */
        KEEP(x); KEEP(y); KEEP(z);
        KEEP(val1); KEEP(val2); KEEP(val3); KEEP(val4); KEEP(val5);
        
        MEM_BARRIER();
    }
    
    return x + y + z;
}

/* Simple PRNG to initialize data without library calls */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main() {
    /* Initialize data with pseudo-random values */
    int data[SIZE];
    int seed = 42;
    
    for (int i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 1000;
    }
    
    /* Split data for different kernels */
    int* part1 = &data[0];
    int* part2 = &data[SIZE/2];
    
    /* Call kernels with complex dependency patterns */
    int result1 = kernel1_memory_aliasing(part1, part2, ITERATIONS);
    int result2 = kernel2_arithmetic_chains(data, ITERATIONS);
    int result3 = kernel3_mixed_patterns(part1, part2, ITERATIONS);
    
    /* Combine results and force output */
    int final_result = result1 + result2 + result3;
    
    /* Use volatile assembly to prevent dead code elimination */
    asm volatile("" : : "r"(final_result));
    
    /* Also use global accumulator to keep values live */
    global_accumulator += final_result;
    
    return global_accumulator != 0;
}
