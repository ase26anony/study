/* ddg_test.c - Complex dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP(var) asm volatile("" : : "r"(var))
#define MEM_BARRIER() asm volatile("" : : : "memory")

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing and pointer arithmetic dependencies */
NOOPT static void kernel_mem_aliasing(int *arr1, int *arr2, int n) {
    int i, acc = 0;
    int *p1, *p2, *p3;
    
    for (i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if-else */
        if (i & 1) {
            /* True dependency chain with memory */
            p1 = &arr1[i];
            p2 = &arr2[i % 32];
            p3 = &arr1[(i + 1) % n];
            
            /* RAW: Read after write dependency */
            int val1 = *p1;          /* Read 1 */
            *p2 = val1 + i;          /* Write 1 (WAR with next read if p2 == p3) */
            int val2 = *p3;          /* Read 2 (potential anti-dependency) */
            
            /* Output dependency (WAW) */
            int tmp = val1 * 2;
            tmp = val2 + tmp;        /* WAW on tmp */
            
            /* Complex arithmetic chain with varying latency ops */
            int div_result = (tmp != 0) ? (val1 % (tmp | 1)) : 0;  /* Higher latency */
            int mult_result = div_result * 3;
            
            acc += mult_result + (val2 & 0xFF);
        } else {
            /* Different path with its own dependencies */
            p1 = &arr1[(i * 3) % n];
            p2 = &arr2[(i * 7) % 32];
            
            /* Anti-dependency pattern (WAR) */
            int read_first = *p1;
            *p1 = read_first ^ i;    /* WAR: Write after read to same location */
            
            /* Output dependency chain */
            int compute = read_first + *p2;
            compute = compute - i;   /* WAW */
            compute = compute | 0x1; /* WAW continuation */
            
            /* Memory dependency with potential aliasing */
            *p2 = compute;
            int verify = *p1;        /* MEM_DEP: Read after write, p1 may alias p2? */
            
            acc += verify * 2;
        }
        
        /* Loop-carried dependency */
        if (i > 0) {
            /* Cross-iteration true dependency */
            static int carry = 0;
            acc += carry;            /* RAW across iterations */
            carry = acc & 0xFF;
        }
        
        /* Volatile to prevent elimination */
        KEEP(acc);
    }
    
    global_acc += acc;
    sink = acc;
}

/* Kernel 2: Arithmetic dependency chains with control flow */
NOOPT static void kernel_arithmetic_chains(int *arr, int n) {
    int i, a, b, c, d, e, f, g, h;
    
    /* Initialize many live variables */
    a = arr[0];
    b = arr[1];
    c = arr[2];
    d = arr[3];
    e = arr[4];
    f = arr[5];
    g = arr[6];
    h = arr[7];
    
    for (i = 0; i < n; ++i) {
        /* Nested loop to create more basic blocks */
        int j;
        for (j = 0; j < 3; ++j) {
            /* Long true dependency chain */
            int t1 = a + b;      /* Stage 1 */
            int t2 = t1 * c;     /* Stage 2 (RAW on t1) */
            int t3 = t2 - d;     /* Stage 3 (RAW on t2) */
            int t4 = t3 / (e | 1); /* Stage 4 with higher latency (RAW on t3) */
            int t5 = t4 ^ f;     /* Stage 5 (RAW on t4) */
            
            /* Parallel chain that merges */
            int u1 = g * h;
            int u2 = u1 + i;
            int u3 = u2 << 2;
            
            /* Control flow creates separate basic blocks */
            if (t5 > u3) {
                /* Output dependencies in this path */
                int tmp = t5;
                tmp = tmp + u3;     /* WAW */
                tmp = tmp % 256;    /* WAW */
                a = tmp;
            } else {
                int tmp = u3;
                tmp = tmp - t5;     /* WAW */
                tmp = tmp & 0xFF;   /* WAW */
                b = tmp;
            }
            
            /* Anti-dependency pattern */
            int old_c = c;          /* Read c */
            c = old_c + j;          /* Write c (WAR) */
            
            /* Update other variables with dependencies */
            d = d ^ t5;             /* RAW on t5 */
            e = e + u3;             /* RAW on u3 */
            
            /* Memory operation with potential dependency */
            arr[(i + j) % 32] = f;
            f = arr[(i * j) % 32];  /* MEM_DEP potential */
            
            KEEP(t5);
            KEEP(u3);
        }
        
        /* Loop-carried output dependencies */
        static int loop_carry = 0;
        g = h + loop_carry;         /* RAW across iterations */
        loop_carry = g;
        
        h = h ^ arr[i % 8];
        
        /* Prevent everything from being optimized away */
        MEM_BARRIER();
    }
    
    /* Consume all values */
    int result = a + b + c + d + e + f + g + h;
    global_acc += result;
    sink = result;
}

/* Kernel 3: Complex control flow with mixed dependencies */
NOOPT static void kernel_control_flow(int *arr1, int *arr2, int n) {
    int i, sum = 0;
    
    for (i = 0; i < n; ++i) {
        /* Multiple conditionally executed blocks */
        int branch_var = arr1[i];
        
        if (branch_var & 0x01) {
            /* Block A: Memory intensive */
            int *ptr1 = &arr1[(i + 1) % n];
            int *ptr2 = &arr2[i % 32];
            
            /* True memory dependency */
            int read1 = *ptr1;
            *ptr2 = read1 + i;
            
            /* Anti-dependency if ptr1 == ptr2 */
            int read2 = *ptr1;
            
            sum += read1 * read2;
        }
        
        if (branch_var & 0x02) {
            /* Block B: Arithmetic chain */
            int x = arr2[i % 32];
            int y = x * 2;      /* RAW */
            int z = y + i;      /* RAW */
            int w = z % 17;     /* RAW with higher latency */
            
            /* Output dependency */
            int out = w;
            out = out ^ x;      /* WAW */
            
            sum += out;
        }
        
        if (branch_var & 0x04) {
            /* Block C: Nested control flow */
            for (int k = 0; k < 2; k++) {
                int idx = (i + k) % n;
                int val = arr1[idx];
                
                /* Write with later read in same iteration */
                arr2[(idx + 1) % 32] = val + k;
                int check = arr1[idx];  /* WAR if arr1 == arr2 */
                
                sum += check << k;
            }
        }
        
        /* Loop-carried dependency with distance > 0 */
        static int prev[4] = {0};
        int carry_idx = i % 4;
        sum += prev[carry_idx];         /* RAW across iterations */
        prev[carry_idx] = sum & 0xFF;
        
        /* Volatile to preserve operations */
        KEEP(sum);
    }
    
    global_acc += sum;
    sink = sum;
}

/* Simple PRNG to avoid library dependencies */
static uint32_t lcg = 123456789;
static uint32_t rand_lcg(void) {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

int main(void) {
    /* Initialize data with pseudo-random values */
    int data[SIZE];
    int data2[32];
    
    for (int i = 0; i < SIZE; i++) {
        data[i] = (int)(rand_lcg() & 0x7FFF);
    }
    for (int i = 0; i < 32; i++) {
        data2[i] = (int)(rand_lcg() & 0x7FFF);
    }
    
    /* Call kernels with complex dependency patterns */
    kernel_mem_aliasing(data, data2, ITERS);
    kernel_arithmetic_chains(data, ITERS);
    kernel_control_flow(data, data2, ITERS);
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : : "r"(global_acc));
    
    return global_acc & 0xFF;
}
