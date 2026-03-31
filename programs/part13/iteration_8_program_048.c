/* ddg_test.c - Complex dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing with mixed dependencies */
NOINLINE static void kernel_mem_aliasing(int *arr1, int *arr2, int n) {
    int *p1, *p2;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Create pointer aliasing possibilities */
        p1 = &arr1[i];
        p2 = &arr2[i % (SIZE/2)];
        
        /* True dependency (RAW) chain */
        tmp1 = *p1 + i;           /* Read from memory */
        tmp2 = tmp1 * 2;          /* Use tmp1 */
        tmp3 = tmp2 / 3;          /* Higher latency division */
        
        /* Anti-dependency (WAR) pattern */
        int old_val = *p2;        /* Read before write */
        *p2 = tmp3 + i;           /* Write to same location */
        acc1 += old_val;          /* Use the read value */
        
        /* Output dependency (WAW) with control flow */
        if (i & 1) {
            tmp4 = tmp3 % 7;      /* Modulo - higher latency */
        } else {
            tmp4 = tmp2 - 5;      /* Different assignment to same var */
        }
        
        /* Another WAW in nested loop */
        for (int j = 0; j < 3; ++j) {
            tmp5 = tmp4 + j;      /* Multiple assignments to tmp5 */
            tmp5 = tmp5 * (j + 1); /* WAW within inner loop */
            acc2 += tmp5;
        }
        
        /* Memory dependency with possible aliasing */
        arr1[(i + 1) % SIZE] = acc2;
        acc3 += arr2[i % (SIZE/2)]; /* Read from potentially aliased location */
        
        /* Control-dependent computation */
        if (tmp3 > 100) {
            tmp4 = tmp4 / 2;
            acc1 += tmp4;
        } else {
            tmp4 = tmp4 * 3;
            acc2 += tmp4;
        }
    }
    
    /* Force values to be live */
    KEEP_ALIVE(acc1);
    KEEP_ALIVE(acc2);
    KEEP_ALIVE(acc3);
    global_acc += acc1 + acc2 + acc3;
}

/* Kernel 2: Arithmetic chains with complex control flow */
NOINLINE static void kernel_arithmetic_chains(int *arr, int n) {
    int a, b, c, d, e, f, g, h;
    int x1, x2, x3, x4, x5, x6, x7, x8;
    int acc = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Long true dependency chain */
        a = arr[i] + 1;
        b = a * i;
        c = b - arr[i];
        d = c / (arr[i] + 2);
        e = d % 13;               /* Higher latency modulo */
        f = e << 2;
        g = f | 0xFF;
        h = g & 0x3F;
        
        /* Parallel chains that converge */
        x1 = arr[(i + 1) % SIZE];
        x2 = x1 * x1;
        x3 = x2 + i;
        x4 = x3 - x1;
        
        /* Control flow creates different basic blocks */
        if (h > x4) {
            x5 = h * 2;
            x6 = x5 + x4;
            arr[i] = x6;          /* Memory write */
        } else if (h < x4 / 2) {
            x5 = h / 2;
            x6 = x4 - x5;
            arr[(i + 2) % SIZE] = x6; /* Different memory location */
        } else {
            x5 = h + x4;
            x6 = x5 * 3;
            /* WAW on x7 */
            x7 = x6;
            x7 = x5;              /* Output dependency */
        }
        
        /* Anti-dependency with the same variable */
        int temp = x6;            /* Read x6 */
        x6 = x5 + 1;              /* Write x6 */
        acc += temp;              /* Use the read value */
        
        /* Nested loop with carried dependencies */
        for (int j = 0; j < 2; ++j) {
            static int carry = 0; /* Loop-carried dependency */
            int local = x6 + j + carry;
            carry = local % 100;
            acc += carry;
        }
        
        /* Complex expression with multiple uses */
        x8 = (x5 * x6) / (arr[i] + 1);
        acc += x8;
        
        /* Volatile asm to prevent elimination */
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), 
                     "r"(e), "r"(f), "r"(g), "r"(h));
    }
    
    KEEP_ALIVE(acc);
    global_acc += acc;
}

/* Kernel 3: Mixed dependencies with pointer chasing */
NOINLINE static void kernel_pointer_chasing(int *base, int n) {
    int *ptr1 = base;
    int *ptr2 = base + SIZE/4;
    int *ptr3 = base + SIZE/2;
    
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int acc = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Pointer arithmetic creating aliasing */
        int idx1 = i % (SIZE/4);
        int idx2 = (i * 7) % (SIZE/4);
        
        /* Memory dependencies with possible aliasing */
        v1 = ptr1[idx1];
        v2 = ptr2[idx2];
        
        /* True dependency chain */
        v3 = v1 + v2;
        v4 = v3 * v1;
        v5 = v4 - v2;
        v6 = v5 / (v1 + 1);
        
        /* Write to memory creating WAR */
        int old_v2 = v2;          /* Read before write */
        ptr2[idx2] = v6 + i;
        acc += old_v2;            /* Anti-dependency use */
        
        /* Multiple writes to same memory (WAW) */
        if (v3 & 1) {
            ptr1[idx1] = v4;
        } else {
            ptr1[idx1] = v5;      /* WAW on ptr1[idx1] */
        }
        
        /* Complex control flow with nested loops */
        for (int j = 0; j < ((i % 3) + 1); ++j) {
            v7 = ptr3[(idx1 + j) % (SIZE/2)];
            v8 = v7 * j;
            
            /* Inner loop with carried dependency */
            for (int k = 0; k < 2; ++k) {
                static int inner_carry = 0;
                v9 = v8 + k + inner_carry;
                inner_carry = v9 % 50;
                acc += inner_carry;
            }
            
            v10 = v8 % 17;        /* Higher latency */
            ptr3[(idx1 + j) % (SIZE/2)] = v10;
        }
        
        /* Converging dependency paths */
        int sum = v4 + v6 + v8;
        acc += sum;
        
        /* Force all variables to be live */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                     "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                     "r"(v9), "r"(v10));
    }
    
    KEEP_ALIVE(acc);
    global_acc += acc;
}

/* Simple PRNG to avoid library calls */
static uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    int data[SIZE];
    uint32_t seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (int)(lcg(&seed) % 1000);
    }
    
    /* Call kernels with different access patterns */
    kernel_mem_aliasing(data, data + SIZE/2, ITERS);
    kernel_arithmetic_chains(data, ITERS);
    kernel_pointer_chasing(data, ITERS);
    
    /* Use result to prevent dead code elimination */
    sink = global_acc;
    
    return global_acc != 0;
}
