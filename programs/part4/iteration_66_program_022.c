/* reload_coverage.c - Test program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_index1 = 1;
volatile int g_index2 = 2;
volatile int g_index3 = 3;

/* Complex data structures to force address computations */
struct Inner {
    int data[8];
    int extra;
};

struct Outer {
    struct Inner arrays[4];
    int base;
    int offset;
};

/* Global test data */
struct Outer g_nested[16];
int g_global_array[256];
int g_output_buffer[128];

/* Helper to force address computation before call */
__attribute__((noinline))
void use_complex_address(int *addr) {
    asm volatile("" : : "r"(addr) : "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPUT */
void test_input_address(void) {
    int i, j, k;
    int result = 0;
    
    /* Force multiple registers for address computation */
    i = g_index1;
    j = g_index2;
    k = g_index3;
    
    /* Complex addressing that requires multiple reloads */
    for (int iter = 0; iter < 4; iter++) {
        /* Nested array access with shifting - forces address reloads */
        int val = g_nested[i + iter].arrays[j].data[k * 2];
        
        /* Inline asm with memory input using complex address */
        asm volatile(
            "addl %%ecx, %%eax\n\t"
            : "=a"(result)
            : "a"(result), 
              "m"(g_nested[(i << 1) + j].arrays[k].data[iter]),  /* Input with complex address */
              "c"(val)
            : "memory"
        );
    }
    
    /* Use result to prevent dead code elimination */
    g_output_buffer[0] = result;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(void) {
    int base_idx = g_index1;
    int offset = g_index2;
    
    /* Force output to memory with computed address */
    for (int i = 0; i < 8; i++) {
        /* Complex output address computation */
        int *output_ptr = &g_output_buffer[(base_idx << 2) + offset + i];
        
        /* Inline asm with memory output constraint */
        asm volatile(
            "movl %%eax, %0\n\t"
            : "=m"(*output_ptr)          /* Output to computed address */
            : "a"(i * 100 + base_idx)
            : "memory"
        );
    }
    
    /* Another pattern: output with index computation in constraint */
    int idx = g_index3;
    asm volatile(
        ""
        : "=m"(g_global_array[(idx << 3) + 5])  /* Complex output address */
        : 
        : "memory"
    );
}

/* Test RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
void test_operand_address(void) {
    int idx1 = g_index1;
    int idx2 = g_index2;
    
    /* Force address computation for function argument */
    for (int i = 0; i < 4; i++) {
        /* Complex address expression passed to function */
        use_complex_address(&g_nested[idx1 + i].arrays[idx2].data[i * 3]);
        
        /* Multiple address computations in same statement */
        int *addr1 = &g_global_array[(idx1 << 2) + idx2 + i];
        int *addr2 = &g_output_buffer[(idx2 << 1) + idx1 - i];
        
        /* Use both addresses */
        asm volatile(
            "movl (%1), %%eax\n\t"
            "addl %%eax, (%0)\n\t"
            : 
            : "r"(addr1), "r"(addr2)
            : "eax", "memory"
        );
    }
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_mixed_input_output_address(void) {
    volatile int temp;
    int idx = g_index1;
    
    /* Mixed input/output with address computations */
    for (int i = 0; i < 6; i++) {
        /* Input address reload */
        asm volatile(
            "movl %1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(temp)
            : "m"(g_nested[idx].arrays[i].data[(i << 1) + 3])  /* Complex input address */
            : "eax", "memory"
        );
        
        /* Output address reload */
        asm volatile(
            "movl %%eax, %0\n\t"
            : "=m"(g_global_array[(idx << 3) + i + 2])  /* Complex output address */
            : "a"(temp + i)
            : "memory"
        );
    }
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address(void) {
    int idx1 = g_index1;
    int idx2 = g_index2;
    int idx3 = g_index3;
    
    /* Multiple complex address computations in one asm */
    asm volatile(
        /* Complex address computations for multiple operands */
        "leal (%1, %2, 4), %%ecx\n\t"
        "movl (%%ecx), %%eax\n\t"
        "leal (%3, %4, 2), %%edx\n\t"
        "addl %%eax, (%%edx)\n\t"
        "leal (%5, %6, 8), %%esi\n\t"
        "movl %%esi, %0\n\t"
        : "=m"(g_output_buffer[0])      /* Output */
        : "r"(&g_global_array[0]),      /* Base */
          "r"(idx1),                    /* Index 1 */
          "r"(&g_output_buffer[0]),     /* Base 2 */
          "r"(idx2),                    /* Index 2 */
          "r"(&g_nested[0]),           /* Base 3 */
          "r"(idx3)                     /* Index 3 */
        : "eax", "ecx", "edx", "esi", "memory"
    );
    
    /* Another pattern with multiple memory clobbers */
    int complex_idx = (idx1 << 2) + (idx2 >> 1) - idx3;
    asm volatile(
        ""
        : 
        : "m"(g_global_array[complex_idx]), 
          "m"(g_nested[idx1].arrays[idx2].data[idx3])
        : "memory"
    );
}

/* Main driver */
int main(void) {
    int checksum = 0;
    
    /* Initialize test data */
    for (int i = 0; i < 16; i++) {
        g_nested[i].base = i * 100;
        g_nested[i].offset = i * 10;
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                g_nested[i].arrays[j].data[k] = i * 1000 + j * 100 + k;
            }
            g_nested[i].arrays[j].extra = i + j;
        }
    }
    
    for (int i = 0; i < 256; i++) {
        g_global_array[i] = i * 3;
    }
    
    for (int i = 0; i < 128; i++) {
        g_output_buffer[i] = 0;
    }
    
    /* Run all tests */
    test_input_address();
    test_output_address();
    test_operand_address();
    test_mixed_input_output_address();
    test_other_address();
    
    /* Compute checksum to ensure code executes */
    for (int i = 0; i < 128; i++) {
        checksum += g_output_buffer[i];
    }
    
    for (int i = 0; i < 256; i++) {
        checksum += g_global_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
