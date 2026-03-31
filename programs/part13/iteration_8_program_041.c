/* ddg_test.c - Complex dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Global arrays to force memory dependencies */
int array1[SIZE];
int array2[SIZE];
int array3[SIZE];

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(v) asm volatile("" : : "r"(v) : "memory")

/* Kernel 1: Mixed RAW, WAR, WAW dependencies with control flow */
NOINLINE static int kernel1_memory_heavy(int *restrict a, int *restrict b, 
                                         int *restrict c, int n) {
    int sum = 0;
    int tmp1, tmp2, tmp3, tmp4;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Basic block 1: RAW dependencies */
        tmp1 = a[i] * 3;
        tmp2 = tmp1 + b[i];      /* RAW on tmp1 */
        tmp3 = tmp2 / 2;         /* RAW on tmp2 */
        
        /* Basic block 2: Control-dependent computations */
        if (i & 1) {
            /* WAR dependency: read tmp3 before overwriting */
            acc1 += tmp3;        /* WAR potential if tmp3 reused */
            tmp3 = c[i] * 7;     /* Overwrites tmp3 - WAW if in other path */
            acc2 ^= tmp3;        /* RAW on new tmp3 */
        } else {
            /* Different computation chain */
            tmp4 = b[i] - a[i];
            acc3 |= tmp4;        /* RAW on tmp4 */
            tmp3 = tmp4 * 11;    /* WAW on tmp3 across control flow */
        }
        
        /* Basic block 3: Memory aliasing patterns */
        int *p1 = &a[i];
        int *p2 = &b[(i * 17) % SIZE];  /* May alias with a */
        
        /* MEM_DEP: Potential true dependency through memory */
        *p1 = tmp3 + i;          /* Store */
        sum += *p2;              /* Load - could create MEM_DEP edge */
        
        /* Output dependency (WAW) on accumulator */
        if (i % 3 == 0) {
            acc4 = sum;          /* WAW on acc4 */
        } else {
            acc4 = acc1 + acc2;  /* Another WAW on acc4 */
        }
        
        /* Anti-dependency (WAR) through memory */
        int old_val = array1[i];
        array1[i] = acc4;        /* WAR: read old_val, then write array1[i] */
        acc1 ^= old_val;         /* Use old_val after store */
        
        /* Complex loop-carried dependency */
        static int lcd = 0;
        int carry = lcd;         /* Loop-carried RAW */
        lcd = (carry + tmp1) % 100;
        acc2 += carry;
        
        /* Prevent elimination */
        KEEP_ALIVE(tmp1);
        KEEP_ALIVE(tmp2);
        KEEP_ALIVE(tmp3);
    }
    
    return sum + acc1 + acc2 + acc3 + acc4;
}

/* Kernel 2: Arithmetic chains with varying latencies */
NOINLINE static int kernel2_arithmetic_chains(int *restrict a, int *restrict b, int n) {
    int x = 1, y = 2, z = 3, w = 4;
    int chain1, chain2, chain3;
    
    for (int i = 0; i < n; ++i) {
        /* Long arithmetic chain with high-latency ops */
        chain1 = a[i] % 13;      /* Higher latency operation */
        chain1 = chain1 * chain1 + i;  /* RAW */
        chain1 = chain1 / (b[i] + 1);  /* RAW, potentially higher latency */
        
        /* Parallel chain with different ops */
        chain2 = b[i] * 7;       /* Medium latency */
        chain2 = chain2 - a[i];  /* RAW */
        chain2 = chain2 ^ 0xABCD; /* RAW */
        
        /* Inter-chain dependencies */
        if (chain1 > chain2) {
            chain3 = chain1 - chain2;  /* RAW on both chains */
        } else {
            chain3 = chain2 - chain1;  /* RAW on both chains */
        }
        
        /* WAW patterns */
        x = chain1 + i;          /* WAW on x */
        y = chain2 * 2;          /* WAW on y */
        x = chain3 / 3;          /* WAW on x again */
        
        /* WAR through variables */
        int temp_x = x;          /* Read x */
        x = y + z;               /* Overwrite x - WAR */
        z = temp_x + w;          /* Use old x value */
        
        /* Memory dependencies with pointer arithmetic */
        int *ptr1 = &a[(i + 1) % SIZE];
        int *ptr2 = &b[(i * 3) % SIZE];
        
        *ptr1 = x + y;           /* Store */
        w = *ptr2 + z;           /* Load - potential MEM_DEP */
        
        /* Nested loop to create more basic blocks */
        for (int j = 0; j < 3; ++j) {
            if (j & 1) {
                chain1 += j;     /* RAW in nested scope */
            } else {
                chain2 -= j;     /* RAW in nested scope */
            }
        }
        
        KEEP_ALIVE(chain1);
        KEEP_ALIVE(chain2);
        KEEP_ALIVE(chain3);
    }
    
    return x + y + z + w;
}

/* Kernel 3: Complex control flow with dependencies */
NOINLINE static int kernel3_control_flow(int *restrict a, int *restrict b, int n) {
    int result = 0;
    int state = 0;
    
    for (int i = 0; i < n; ++i) {
        int val1 = a[i];
        int val2 = b[i];
        int computed;
        
        /* Multi-way branch creating many basic blocks */
        switch (i % 4) {
            case 0:
                computed = val1 * val2;      /* RAW on val1, val2 */
                state = computed + 1;        /* WAW on state */
                result ^= state;             /* RAW on state */
                break;
            case 1:
                computed = val1 - val2;      /* RAW */
                /* WAR: read state before overwrite */
                int old_state = state;       /* Read state */
                state = computed * 2;        /* Overwrite state */
                result += old_state;         /* Use old state */
                break;
            case 2:
                computed = val1 | val2;      /* RAW */
                /* Output dependency chain */
                state = computed;            /* WAW on state */
                state = state ^ 0xFF;        /* WAW on state */
                result |= state;             /* RAW on state */
                break;
            case 3:
                computed = val1 & val2;      /* RAW */
                /* Memory anti-dependency */
                array2[i] = computed;        /* Store */
                int loaded = array2[(i + 1) % SIZE]; /* Load - potential WAR in next iteration */
                state = loaded + i;          /* RAW on loaded */
                result &= state;             /* RAW on state */
                break;
        }
        
        /* Loop-carried dependency with distance > 0 */
        static int history[4] = {0};
        int idx = i % 4;
        int prev = history[idx];             /* Read from previous iteration */
        history[idx] = computed;             /* Write for next iteration */
        result += prev;                      /* Use value from previous iteration */
        
        /* Another level of conditional nesting */
        if (computed > 100) {
            for (int k = 0; k < 2; ++k) {
                int temp = result;
                result = temp + k;           /* WAW on result */
                state = result - temp;       /* RAW on result, WAR on temp */
            }
        } else if (computed < -100) {
            result = result * 2;             /* WAW on result */
            state = state / 2;               /* WAW on state */
        } else {
            result = result ^ state;         /* WAW on result, RAW on state */
        }
        
        KEEP_ALIVE(computed);
        KEEP_ALIVE(state);
    }
    
    return result;
}

/* Simple PRNG to initialize data without library calls */
static uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    uint32_t seed = 42;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        array1[i] = (int)(lcg(&seed) % 1000);
        array2[i] = (int)(lcg(&seed) % 1000);
        array3[i] = (int)(lcg(&seed) % 1000);
    }
    
    /* Call kernels with different dependency patterns */
    int result1 = kernel1_memory_heavy(array1, array2, array3, ITERS);
    int result2 = kernel2_arithmetic_chains(array1, array2, ITERS);
    int result3 = kernel3_control_flow(array2, array3, ITERS);
    
    /* Combine results and force output */
    int final_result = result1 + result2 + result3;
    
    /* Prevent dead code elimination */
    sink = final_result;
    asm volatile("" : : "r"(final_result) : "memory");
    
    return final_result & 0xFF;  /* Return non-zero to indicate execution */
}
