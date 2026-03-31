/* ddg_test.c - Complex loop-carried dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Global accumulator to keep variables live */
static int global_acc = 0;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP_ALIVE(v) asm volatile("" : : "r"(v) : "memory")

/* Kernel 1: Memory-heavy with pointer aliasing and WAR/WAW dependencies */
NOOPT static void kernel_memory(int* data, int n, int stride) {
    int* p1 = data;
    int* p2 = data + (n / 2);
    int* p3 = data + (n / 4);
    
    int tmp1 = 0, tmp2 = 0, tmp3 = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple basic blocks created by if-else chain */
        if (i & 1) {
            /* True dependency chain with memory */
            tmp1 = *p1 + i;
            acc1 = tmp1 * 2;
            *p1 = acc1;  /* WAW: overwriting *p1 */
            
            /* Anti-dependency: read before write to same location */
            tmp2 = *p2;
            *p2 = tmp2 + tmp1;  /* WAR: tmp2 read, then *p2 written */
            
            /* Pointer aliasing - p3 may alias with p1 or p2 */
            tmp3 = *p3 % 17;  /* Higher latency modulo */
            *p3 = tmp3 + (i % 5);
            
            p1++;
            p2 += stride;
            p3 += (stride % 3) + 1;
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = *p2 - *p1;
            acc2 = tmp1 / 3;  /* Higher latency division */
            
            /* Output dependency on tmp1 */
            tmp1 = *p3 + acc2;  /* WAW: tmp1 reassigned */
            
            /* Memory anti-dependency chain */
            int old_val = *p1;
            *p1 = old_val ^ tmp1;
            
            /* Complex addressing to create more edges */
            p1 += stride;
            p2++;
            p3 += stride;
        }
        
        /* Nested loop to create more basic blocks */
        for (int j = 0; j < 3; j++) {
            if ((i + j) & 2) {
                /* Cross-iteration dependency */
                acc3 += tmp1 * j;
                tmp2 = acc3 & 0xFF;
            } else {
                tmp3 = tmp2 + j;
            }
        }
        
        /* Force memory dependencies across iterations */
        if (i > 0) {
            data[i] = data[i-1] + tmp3;  /* Loop-carried true dependency */
        }
        
        KEEP_ALIVE(tmp1);
        KEEP_ALIVE(tmp2);
        KEEP_ALIVE(tmp3);
    }
    
    global_acc += acc1 + acc2 + acc3;
    sink = global_acc;
}

/* Kernel 2: Arithmetic-heavy with long dependency chains and control flow */
NOOPT static void kernel_arithmetic(int* data, int n, int seed) {
    int a = seed, b = seed * 2, c = seed * 3;
    int d = 0, e = 0, f = 0;
    int g = 0, h = 0, j = 0;
    
    for (int i = 0; i < n; i++) {
        /* Long true dependency chain spanning multiple operations */
        a = b + data[i];
        b = c - a;
        c = a * b;
        d = c % 19;  /* Higher latency modulo */
        e = d / 7;   /* Higher latency division */
        f = e ^ b;
        
        /* Multiple basic blocks with switch */
        switch (i & 3) {
            case 0:
                g = a + f;
                h = g << 2;
                break;
            case 1:
                g = b - f;
                h = g >> 1;
                break;
            case 2:
                g = c * f;
                h = g & 0xFFFF;
                break;
            default:
                g = d ^ f;
                h = g | 0xFF;
                break;
        }
        
        /* Anti-dependencies */
        int old_g = g;      /* Read g */
        g = h + old_g;      /* Write g - WAR */
        
        int old_h = h;      /* Read h */
        h = old_h * 2;      /* Write h - WAW if h was written earlier */
        
        /* Output dependencies */
        j = old_g + i;
        j = old_h - j;      /* WAW on j */
        
        /* Loop-carried dependency with distance > 0 */
        if (i >= 2) {
            data[i] = data[i-2] + j;  /* Distance 2 dependency */
        }
        
        /* Complex condition creating control dependencies */
        if ((a + b + c) > 1000) {
            f = f % 13;
        } else if ((d + e) < 500) {
            f = f * 3;
        } else {
            f = f ^ 0xABCD;
        }
        
        KEEP_ALIVE(a); KEEP_ALIVE(b); KEEP_ALIVE(c);
        KEEP_ALIVE(d); KEEP_ALIVE(e); KEEP_ALIVE(f);
        KEEP_ALIVE(g); KEEP_ALIVE(h); KEEP_ALIVE(j);
    }
    
    global_acc += a + b + c + d + e + f + g + h + j;
    sink = global_acc;
}

/* Kernel 3: Mixed dependencies with nested loops and function calls */
NOOPT static int helper_compute(int x, int y) {
    return (x * y) + (x % 11) - (y & 7);
}

NOOPT static void kernel_mixed(int* data, int n, int offset) {
    int vec[8] = {0};
    int matrix[4][4] = {{0}};
    
    for (int i = 0; i < n; i++) {
        /* Initialize multiple live variables */
        int idx = i & 7;
        int row = i & 3;
        int col = (i >> 2) & 3;
        
        /* Memory dependencies with array accesses */
        int old_vec = vec[idx];                 /* Read */
        vec[idx] = data[i + offset] + old_vec;  /* Write - WAR */
        
        /* Matrix operations with output dependencies */
        int old_cell = matrix[row][col];        /* Read */
        matrix[row][col] = old_cell * 2;        /* Write - WAW potential */
        
        /* Function call creates additional dependencies */
        int computed = helper_compute(old_vec, old_cell);
        
        /* Nested loop with carried dependencies */
        int inner_acc = 0;
        for (int k = 0; k < 4; k++) {
            if (k & 1) {
                inner_acc += matrix[k][col];
            } else {
                inner_acc -= vec[k];
            }
            
            /* Cross-iteration dependency in inner loop */
            if (k > 0) {
                matrix[(row + k) & 3][col] += inner_acc;
            }
        }
        
        /* Complex if-else chain creating many basic blocks */
        if (computed > 100) {
            data[i] = vec[idx] * 3;
            if (inner_acc < 50) {
                vec[idx] = computed / 5;
            } else {
                vec[idx] = computed % 17;
            }
        } else if (computed < -100) {
            data[i] = vec[idx] / 3;
            vec[idx] = computed * 2;
        } else {
            data[i] = vec[idx] + computed;
            if ((i & 15) == 0) {
                vec[idx] = 0;  /* Periodic reset - WAW */
            }
        }
        
        /* Pointer chasing to create memory dependencies */
        int* ptr1 = &data[i];
        int* ptr2 = &data[(i * 13 + 7) % n];
        
        int val1 = *ptr1;
        *ptr2 = val1 + i;  /* May alias with future ptr1 */
        
        KEEP_ALIVE(old_vec);
        KEEP_ALIVE(old_cell);
        KEEP_ALIVE(computed);
        KEEP_ALIVE(inner_acc);
    }
    
    /* Accumulate results from all arrays */
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += vec[i];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            sum += matrix[i][j];
    
    global_acc += sum;
    sink = global_acc;
}

/* Simple PRNG to avoid library calls */
static uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    int data[SIZE];
    uint32_t seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (int)(lcg(&seed) % 1000);
    }
    
    /* Call kernels with different dependency patterns */
    kernel_memory(data, ITERATIONS, 2);
    kernel_arithmetic(data + 100, ITERATIONS / 2, 123);
    kernel_mixed(data + 200, ITERATIONS / 4, 50);
    
    /* Final volatile store to prevent dead code elimination */
    asm volatile("" : : "r"(global_acc) : "memory");
    
    return global_acc & 0xFF;
}
