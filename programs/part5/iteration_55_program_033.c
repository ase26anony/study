/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force register pressure by using many variables */
#define FORCE_REGISTER_PRESSURE \
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9; \
    volatile int w0, w1, w2, w3, w4, w5, w6, w7, w8, w9; \
    volatile int x0, x1, x2, x3, x4, x5, x6, x7, x8, x9; \
    volatile int y0, y1, y2, y3, y4, y5, y6, y7, y8, y9; \
    volatile int z0, z1, z2, z3, z4, z5, z6, z7, z8, z9;

/* Complex structure for address computations */
struct nested {
    int data[8];
    struct nested *next;
    volatile int flags;
};

/* Test 1: Complex array addressing with multiple index computations */
__attribute__((noinline))
static int test_complex_addressing(int *base, int size) {
    FORCE_REGISTER_PRESSURE
    
    /* Multi-dimensional array access pattern */
    int array3d[4][4][4];
    int sum = 0;
    
    /* Manual unrolling to increase register pressure */
    #pragma GCC unroll 4
    for (int i = 0; i < 4; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS: address of array3d[i] needs computation */
        int (*layer)[4] = array3d[i];
        
        #pragma GCC unroll 4
        for (int j = 0; j < 4; j++) {
            /* RELOAD_FOR_INPADDR_ADDRESS: address of layer[j] */
            int *row = layer[j];
            
            #pragma GCC unroll 4
            for (int k = 0; k < 4; k++) {
                /* Complex addressing with multiple terms */
                /* RELOAD_FOR_INPUT: row[k] as input */
                /* RELOAD_FOR_OPERAND_ADDRESS: &row[k] computation */
                int idx = (i * 16 + j * 4 + k) % size;
                int val1 = row[k];
                int val2 = base[idx];
                
                /* Force address reloads through pointer arithmetic */
                int *addr1 = &row[k] + (val1 & 0x3);
                int *addr2 = base + idx + (val2 & 0x3);
                
                /* Use both values to prevent optimization */
                sum += *addr1 + *addr2 + v0 + w0;
                
                /* Volatile writes to force spills */
                v0 = val1;
                w0 = val2;
            }
        }
    }
    
    return sum;
}

/* Test 2: Inline assembly with multiple outputs and complex constraints */
__attribute__((noinline))
static int test_asm_reloads(struct nested *ptr, int count) {
    FORCE_REGISTER_PRESSURE
    
    int result = 0;
    struct nested *current = ptr;
    
    /* Loop with pointer chasing - causes address reloads */
    for (int i = 0; i < count && current != NULL; i++) {
        /* RELOAD_FOR_OUTPUT_ADDRESS: output to memory with complex address */
        int temp1, temp2;
        
        /* Complex inline asm with multiple outputs and memory constraints */
        __asm__ volatile (
            /* Output to memory with addressing */
            "movl %%eax, %[out1]\n\t"
            "movl %%ebx, %[out2]\n\t"
            /* Input from memory with complex addressing */
            : [out1] "=m" (current->data[i % 8]),
              [out2] "=m" (current->data[(i + 1) % 8])
            /* Inputs that need registers */
            : "a" (i), 
              "b" (result),
              /* Memory input with complex address */
              "m" (*(volatile int *)((char *)current + i * 4))
            : "memory", "ecx", "edx"
        );
        
        /* RELOAD_FOR_OUTADDR_ADDRESS: address computation for next */
        struct nested **next_ptr = &current->next;
        
        /* Force address computation through multiple steps */
        volatile int offset = (i * sizeof(struct nested)) % 64;
        char *byte_ptr = (char *)next_ptr;
        struct nested *next = *(struct nested **)(byte_ptr + offset);
        
        /* Complex expression that uses many temporaries */
        result += current->data[0] + current->data[1] +
                  current->data[2] + current->data[3] +
                  x0 + y0 + z0;
        
        current = next;
        
        /* Volatile accesses to prevent optimization */
        x0 = result;
        y0 = i;
        z0 = (int)current;
    }
    
    return result;
}

/* Test 3: Mixed operand types and addressing modes */
__attribute__((noinline))
static int test_mixed_operands(volatile int *mem, int iterations) {
    FORCE_REGISTER_PRESSURE
    
    /* Local arrays for additional pressure */
    int local1[16], local2[16], local3[16];
    int sum = 0;
    
    /* Initialize locals */
    for (int i = 0; i < 16; i++) {
        local1[i] = i * 2;
        local2[i] = i * 3;
        local3[i] = i * 5;
    }
    
    /* Complex loop with mixed addressing */
    for (int i = 0; i < iterations; i++) {
        /* RELOAD_FOR_OPADDR_ADDR: address of pointer itself */
        volatile int **mem_ptr = (volatile int **)&mem;
        
        /* Multiple levels of indirection */
        volatile int *indirect1 = *mem_ptr + (i & 0xF);
        volatile int *indirect2 = indirect1 + local1[i & 0xF];
        volatile int *indirect3 = indirect2 + local2[i & 0xF];
        
        /* Complex expression with many intermediate values */
        int val1 = *indirect1;
        int val2 = *indirect2;
        int val3 = *indirect3;
        
        /* RELOAD_FOR_OTHER_ADDRESS: obscure address computation */
        int *weird_addr = (int *)((uintptr_t)indirect3 ^ (val1 << 2));
        
        /* Use inline asm with alternative constraints */
        int out1, out2;
        __asm__ volatile (
            "addl %%ecx, %%eax\n\t"
            "subl %%edx, %%ebx\n\t"
            : "=&a" (out1), "=&b" (out2)
            : "c" (val1), "d" (val2),
              "m" (*weird_addr),  /* Memory operand needing address reload */
              "m" (local3[i & 0xF])
            : "cc"
        );
        
        /* More complex computations */
        sum += out1 * out2 + val3 + v0 + w0 + x0 + y0;
        
        /* Update volatiles to force memory traffic */
        v0 = val1;
        w0 = val2;
        x0 = val3;
        y0 = out1;
        
        /* RELOAD_OTHER: through target-specific builtin */
        #ifdef __i386__
        /* Use builtin that might need special handling */
        int popcnt = __builtin_popcount(val1);
        sum += popcnt;
        #endif
    }
    
    return sum;
}

/* Test 4: Nested function calls with register clobbering */
__attribute__((noinline, regparm(3)))
static int nested_callee(int a, int b, int c, int d, int e, int f) {
    /* Force many parameters to be passed in memory */
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6;
}

__attribute__((noinline))
static int test_call_reloads(void) {
    FORCE_REGISTER_PRESSURE
    
    int total = 0;
    
    /* Call with many arguments - forces reloads around calls */
    for (int i = 0; i < 10; i++) {
        /* Many live variables across call */
        int a = v0 + i;
        int b = w0 + i * 2;
        int c = x0 + i * 3;
        int d = y0 + i * 4;
        int e = z0 + i * 5;
        int f = (v0 + w0 + x0) * i;
        
        /* Call with register parameters */
        int result = nested_callee(a, b, c, d, e, f);
        
        /* Use result immediately to prevent optimization */
        total += result + a + b + c;
        
        /* Update volatiles */
        v0 = a;
        w0 = b;
        x0 = result;
    }
    
    return total;
}

/* Main driver */
int main(void) {
    /* Initialize test data */
    int data_array[64];
    for (int i = 0; i < 64; i++) {
        data_array[i] = i * 7 + 1;
    }
    
    /* Set up linked structure */
    struct nested nodes[4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            nodes[i].data[j] = i * 8 + j;
        }
        nodes[i].next = (i < 3) ? &nodes[i + 1] : NULL;
        nodes[i].flags = i;
    }
    
    volatile int mem_area[128];
    for (int i = 0; i < 128; i++) {
        mem_area[i] = i * 11 - 3;
    }
    
    /* Run all tests to trigger different reload patterns */
    int sum = 0;
    
    sum += test_complex_addressing(data_array, 64);
    sum += test_asm_reloads(nodes, 4);
    sum += test_mixed_operands(mem_area, 8);
    sum += test_call_reloads();
    
    /* Use result to prevent dead code elimination */
    volatile int *output = (volatile int *)malloc(sizeof(int));
    *output = sum;
    
    return (*output > 0) ? 0 : 1;
}
