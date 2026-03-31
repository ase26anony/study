/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_index1 = 1;
volatile int g_index2 = 2;
volatile int g_index3 = 3;
volatile int g_base_offset = 100;

/* Complex data structures to force address computations */
struct InnerStruct {
    int data[8];
    int* ptr_array[4];
};

struct OuterStruct {
    struct InnerStruct inner[4];
    int matrix[4][8];
    volatile int* volatile_ptr;
};

/* Global test structures */
struct OuterStruct g_outer[4];
int g_global_array[256];
volatile int* g_volatile_ptr = g_global_array;

/* Function to force address computation before call */
void __attribute__((noinline)) 
use_complex_address(struct InnerStruct* addr) {
    asm volatile("" : : "r"(addr) : "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
int __attribute__((noinline,optimize("O2")))
test_input_address(void) {
    int result = 0;
    volatile int idx1 = g_index1;
    volatile int idx2 = g_index2;
    
    /* Complex addressing that should require input address reload */
    for (int i = 0; i < 4; i++) {
        /* Force input address computation with multiple registers */
        asm volatile(
            "movl (%[base], %[idx1], 4), %%eax\n\t"
            "addl %%eax, %[res]\n\t"
            : [res] "+r" (result)
            : [base] "r" (g_global_array), 
              [idx1] "r" (idx1 + idx2 + i * 8)
            : "eax", "memory"
        );
    }
    
    /* Nested structure access requiring address reload */
    int temp = g_outer[idx1].inner[idx2].data[idx1 + idx2];
    result += temp;
    
    return result;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
int __attribute__((noinline,optimize("O2")))
test_output_address(void) {
    volatile int idx = g_index3;
    volatile int offset = g_base_offset;
    
    /* Force output to complex address */
    for (int i = 0; i < 4; i++) {
        int computed_idx = (idx << 2) + i * 3;
        
        /* Output to memory with complex address computation */
        asm volatile(
            "movl %[val], (%[base], %[idx], 4)\n\t"
            : "=m" (g_global_array[computed_idx])
            : [base] "r" (g_global_array),
              [idx] "r" (computed_idx),
              [val] "r" (i * 100)
            : "memory"
        );
    }
    
    /* Mixed input/output with different addressing */
    int* volatile ptr = &g_global_array[offset];
    asm volatile(
        "movl (%[in]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%[out])\n\t"
        : "=m" (*ptr)
        : [in] "r" (&g_global_array[idx * 16]),
          [out] "r" (ptr)
        : "eax", "memory"
    );
    
    return g_global_array[idx * 16];
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
int __attribute__((noinline,optimize("O2")))
test_inpaddr_outaddr(void) {
    volatile int idx1 = g_index1;
    volatile int idx2 = g_index2;
    int result = 0;
    
    /* Complex addressing with both input and output address computations */
    for (int i = 0; i < 3; i++) {
        int* in_addr = &g_outer[idx1].matrix[idx2][i * 2];
        int* out_addr = &g_outer[i].inner[idx1].data[idx2];
        
        asm volatile(
            "movl (%[in]), %%eax\n\t"
            "imull $2, %%eax\n\t"
            "movl %%eax, (%[out])\n\t"
            : "=m" (*out_addr)
            : [in] "r" (in_addr),
              [out] "r" (out_addr)
            : "eax", "memory"
        );
        
        result += *out_addr;
    }
    
    return result;
}

/* Test RELOAD_FOR_OPERAND_ADDRESS */
int __attribute__((noinline,optimize("O2")))
test_operand_address(void) {
    volatile int idx = g_index3;
    
    /* Force operand address computation for function call */
    for (int i = 0; i < 2; i++) {
        /* Complex address expression as function argument */
        use_complex_address(&g_outer[idx + i].inner[(idx * i) % 4]);
        
        /* Another complex address computation */
        int* addr = &g_global_array[(idx << i) + (i * 16)];
        asm volatile("" : : "r"(addr) : "memory");
    }
    
    return idx;
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
int __attribute__((noinline,optimize("O2")))
test_other_address(void) {
    volatile int idx1 = g_index1;
    volatile int idx2 = g_index2;
    int result = 0;
    
    /* Multiple memory accesses with different address computations */
    for (int i = 0; i < 4; i++) {
        /* Complex addressing in loop */
        int addr1 = (idx1 + i) * 8;
        int addr2 = (idx2 * i) * 4;
        
        /* Multiple memory operations forcing various reloads */
        asm volatile(
            "movl %[array1], %%eax\n\t"
            "addl %[array2], %%eax\n\t"
            "movl %%eax, %[result]\n\t"
            : [result] "=m" (result)
            : [array1] "m" (g_global_array[addr1]),
              [array2] "m" (g_global_array[addr2])
            : "eax", "memory"
        );
        
        /* Additional address computation */
        int* complex_ptr = &g_outer[i].inner[idx1].data[idx2];
        asm volatile(
            "addl $1, (%[ptr])\n\t"
            : : [ptr] "r" (complex_ptr)
            : "memory"
        );
    }
    
    return result;
}

/* Test mixed reload types in single function */
int __attribute__((noinline,optimize("O3")))
test_mixed_reloads(void) {
    volatile int idx = g_index1;
    volatile int offset = g_base_offset;
    int sum = 0;
    
    /* Mix input and output addressing with register pressure */
    for (int i = 0; i < 8; i++) {
        /* Create register pressure */
        int r1 = idx + i;
        int r2 = offset - i;
        int r3 = idx * i;
        int r4 = offset / (i + 1);
        int r5 = r1 ^ r2;
        int r6 = r3 | r4;
        
        /* Complex input address */
        int in_val;
        asm volatile(
            "movl (%[base], %[idx], 4), %%eax\n\t"
            : "=a" (in_val)
            : [base] "r" (g_global_array),
              [idx] "r" (r1 + r2 + r5)
            : "memory"
        );
        
        /* Complex output address */
        int out_idx = (r3 + r4 + r6) % 128;
        asm volatile(
            "movl %%eax, (%[base], %[idx], 4)\n\t"
            : "=m" (g_global_array[out_idx])
            : [base] "r" (g_global_array),
              [idx] "r" (out_idx),
              "a" (in_val + 1)
            : "memory"
        );
        
        sum += g_global_array[out_idx];
    }
    
    return sum;
}

/* Initialize test data */
void init_test_data(void) {
    for (int i = 0; i < 256; i++) {
        g_global_array[i] = i;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                g_outer[i].inner[j].data[k] = i * 100 + j * 10 + k;
            }
            for (int k = 0; k < 4; k++) {
                g_outer[i].inner[j].ptr_array[k] = &g_global_array[(i + j + k) * 4];
            }
        }
        
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                g_outer[i].matrix[j][k] = i * 64 + j * 8 + k;
            }
        }
        
        g_outer[i].volatile_ptr = &g_global_array[i * 16];
    }
}

int main(void) {
    init_test_data();
    
    printf("Testing various reload patterns...\n");
    
    int result = 0;
    
    /* Call each test function to trigger different reload types */
    result += test_input_address();
    printf("test_input_address: %d\n", result);
    
    result += test_output_address();
    printf("test_output_address: %d\n", result);
    
    result += test_inpaddr_outaddr();
    printf("test_inpaddr_outaddr: %d\n", result);
    
    result += test_operand_address();
    printf("test_operand_address: %d\n", result);
    
    result += test_other_address();
    printf("test_other_address: %d\n", result);
    
    result += test_mixed_reloads();
    printf("test_mixed_reloads: %d\n", result);
    
    printf("Final checksum: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result > 1000000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
