/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that might eliminate reloads */
#define NOOPT __attribute__((optimize("O0")))
#define VOLATILE_MEMORY asm volatile("" ::: "memory")

/* Complex data structures to force address computations */
struct inner {
    int data[8];
    volatile int* volatile_ptr;
};

struct outer {
    struct inner arrays[4];
    int base_index;
    volatile int dynamic_offset;
};

/* Global variables to increase register pressure */
volatile int global_index = 0;
volatile int* global_base = NULL;
struct outer global_struct;

/* Test RELOAD_FOR_INPUT_ADDRESS */
NOOPT void test_input_address(struct outer* s, int idx1, int idx2) {
    /* Complex addressing: array[idx1].arrays[idx2].data[idx1+idx2] */
    int result;
    
    /* Force address computation into registers */
    asm volatile(
        "movl %[idx1], %%eax\n\t"
        "movl %[idx2], %%ebx\n\t"
        "addl %%eax, %%ebx\n\t"
        "shll $2, %%ebx\n\t"
        "movl %[base], %%ecx\n\t"
        "movl (%%ecx,%%eax,8), %%edx\n\t"
        "movl (%%edx,%%ebx), %[res]\n\t"
        : [res] "=r" (result)
        : [base] "r" (&s->arrays[0].data[0]),
          [idx1] "r" (idx1),
          [idx2] "r" (idx2)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    VOLATILE_MEMORY;
    global_index = result;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
NOOPT void test_output_address(struct outer* s, int idx, int value) {
    /* Complex output addressing: s->arrays[idx].volatile_ptr[offset] = value */
    int offset = s->dynamic_offset;
    
    asm volatile(
        "movl %[idx], %%eax\n\t"
        "movl %[offset], %%ebx\n\t"
        "shll $2, %%ebx\n\t"
        "movl %[base], %%ecx\n\t"
        "movl (%%ecx,%%eax,8), %%edx\n\t"
        "movl %[val], (%%edx,%%ebx)\n\t"
        :
        : [base] "r" (&s->arrays[0].volatile_ptr),
          [idx] "r" (idx),
          [offset] "r" (offset),
          [val] "r" (value)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    VOLATILE_MEMORY;
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and mixed types */
NOOPT void test_mixed_address(struct outer* s, int idx1, int idx2, int idx3) {
    /* Multiple complex addresses in one asm statement */
    int input_val, output_val;
    
    /* Force different addressing modes */
    asm volatile(
        /* Input address computation 1 */
        "movl %[idx1], %%eax\n\t"
        "movl %[idx2], %%ebx\n\t"
        "leal (%%eax,%%ebx,2), %%ecx\n\t"
        "movl %[base1], %%edx\n\t"
        "movl (%%edx,%%ecx,4), %%esi\n\t"
        
        /* Output address computation */
        "movl %[idx3], %%edi\n\t"
        "movl %[base2], %%ecx\n\t"
        "movl %%esi, (%%ecx,%%edi,4)\n\t"
        
        /* Input address computation 2 (different pattern) */
        "movl %[idx2], %%eax\n\t"
        "shll $3, %%eax\n\t"
        "addl %[idx3], %%eax\n\t"
        "movl %[base3], %%edx\n\t"
        "movl (%%edx,%%eax,4), %%ebx\n\t"
        
        : "=m" (s->arrays[idx3].data[0]), 
          [out] "=r" (output_val)
        : [base1] "r" (&s->arrays[0].data[0]),
          [base2] "r" (&s->arrays[0].data[4]),
          [base3] "r" (&s->arrays[2].data[0]),
          [idx1] "r" (idx1),
          [idx2] "r" (idx2),
          [idx3] "r" (idx3),
          "m" (s->arrays[idx1].data[idx2])
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    VOLATILE_MEMORY;
}

/* Test RELOAD_FOR_OPERAND_ADDRESS */
NOOPT void test_operand_address(struct inner* arr, int count) {
    /* Force address computation before function-like asm */
    for (int i = 0; i < count; i++) {
        /* Complex address as operand */
        asm volatile(
            "addl $1, %[addr]\n\t"
            : 
            : [addr] "m" (arr[(i * 3) % 4].data[(i * 2) % 8])
            : "memory"
        );
    }
    VOLATILE_MEMORY;
}

/* Test RELOAD_FOR_OTHER_ADDRESS */
NOOPT void test_other_address(struct outer* s, int* indices, int n) {
    /* Multiple interdependent address computations */
    for (int i = 0; i < n; i++) {
        int idx = indices[i] + s->base_index;
        
        /* Chain of address computations */
        asm volatile(
            "movl %[idx], %%eax\n\t"
            "movl %[base], %%ebx\n\t"
            "leal (%%ebx,%%eax,8), %%ecx\n\t"
            "movl (%%ecx), %%edx\n\t"
            "addl %%edx, %[sum]\n\t"
            : [sum] "+m" (s->dynamic_offset)
            : [base] "r" (&s->arrays[0]),
              [idx] "r" (idx)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
    }
    VOLATILE_MEMORY;
}

/* Main driver that creates register pressure */
NOOPT int main() {
    /* Initialize test structures */
    struct outer test_struct;
    int indices[] = {1, 3, 0, 2, 1, 3};
    
    /* Allocate and initialize memory */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            test_struct.arrays[i].data[j] = i * 10 + j;
        }
        test_struct.arrays[i].volatile_ptr = 
            (volatile int*)malloc(16 * sizeof(int));
        for (int j = 0; j < 16; j++) {
            test_struct.arrays[i].volatile_ptr[j] = i * 20 + j;
        }
    }
    
    test_struct.base_index = 5;
    test_struct.dynamic_offset = 8;
    
    global_base = (volatile int*)&test_struct;
    
    /* Create register pressure with many live values */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int keep_alive[10];
    
    /* Call test functions with complex parameters */
    test_input_address(&test_struct, a, b);
    keep_alive[0] = global_index;
    
    test_output_address(&test_struct, c, d);
    keep_alive[1] = test_struct.dynamic_offset;
    
    test_mixed_address(&test_struct, a, b, c);
    keep_alive[2] = test_struct.arrays[c].data[0];
    
    test_operand_address(&test_struct.arrays[0], 4);
    keep_alive[3] = test_struct.arrays[0].data[0];
    
    test_other_address(&test_struct, indices, 6);
    keep_alive[4] = test_struct.dynamic_offset;
    
    /* Force all values to be used to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 5; i++) {
        checksum += keep_alive[i];
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free((void*)test_struct.arrays[i].volatile_ptr);
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
