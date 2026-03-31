/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_index1 = 100;
volatile int g_index2 = 200;
volatile int g_base_offset = 300;

/* Complex nested structure to force address computations */
struct Inner {
    int data[8];
    int* ptr_array[4];
};

struct Middle {
    struct Inner inner[4];
    long long padding[2];
};

struct Outer {
    struct Middle middle[3];
    int index_array[16];
    volatile int* volatile_ptr;
};

/* Global test structures */
struct Outer g_outer_struct;
int g_global_array[1024];
volatile int* g_volatile_ptr = &g_global_array[0];

/* Function to force address computation before call - triggers RELOAD_FOR_OPERAND_ADDRESS */
void __attribute__((noinline)) use_complex_address(struct Inner* inner_ptr, int offset) {
    asm volatile("" : : "r"(inner_ptr), "r"(offset) : "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
int __attribute__((noinline)) test_input_address(void) {
    int result = 0;
    volatile int local_index = g_index1;
    
    /* Complex addressing that requires input address reload */
    for (int i = 0; i < 4; i++) {
        int idx = local_index + i * g_index2;
        
        /* Inline asm with complex memory input address */
        asm volatile(
            "movl %[mem], %%eax\n\t"
            "addl %%eax, %[res]\n\t"
            : [res] "+r" (result)
            : [mem] "m" (g_global_array[(idx << 1) + g_base_offset])
            : "eax", "memory"
        );
        
        /* Another complex access pattern */
        asm volatile(
            "movl %[mem2], %%ebx\n\t"
            "subl %%ebx, %[res]\n\t"
            : [res] "+r" (result)
            : [mem2] "m" (g_outer_struct.middle[i % 3].inner[(local_index + i) % 4].data[(idx >> 2) % 8])
            : "ebx", "memory"
        );
    }
    
    return result;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void __attribute__((noinline)) test_output_address(int seed) {
    volatile int offset = seed;
    
    for (int i = 0; i < 8; i++) {
        int complex_offset = (offset + i * 17) & 0x3FF;
        
        /* Inline asm with complex memory output address */
        asm volatile(
            "movl %[val], %%ecx\n\t"
            "movl %%ecx, %[out]\n\t"
            : [out] "=m" (g_global_array[(complex_offset << 2) + i + g_base_offset])
            : [val] "r" (i + seed)
            : "ecx", "memory"
        );
        
        /* Mixed input/output with different addressing */
        int temp;
        asm volatile(
            "movl %[in], %%edx\n\t"
            "leal (%%edx, %%edx, 2), %%edx\n\t"
            "movl %%edx, %[temp]\n\t"
            : [temp] "=r" (temp)
            : [in] "m" (g_outer_struct.index_array[complex_offset % 16])
            : "edx", "memory"
        );
        
        /* Store result with another complex address */
        asm volatile(
            "movl %[temp], %[store]\n\t"
            : [store] "=m" (g_outer_struct.middle[i % 3].inner[0].data[complex_offset % 8])
            : [temp] "r" (temp)
            : "memory"
        );
    }
}

/* Test RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
int __attribute__((noinline)) test_input_and_inpaddr(void) {
    int sum = 0;
    volatile int idx1 = g_index1;
    volatile int idx2 = g_index2;
    
    /* Force multiple register usage in address computation */
    for (int i = 0; i < 6; i++) {
        int* volatile ptr = &g_global_array[0];
        
        /* Complex address with multiple components */
        asm volatile(
            "movl (%[base], %[idx1], 4), %%esi\n\t"
            "addl %%esi, %[sum]\n\t"
            "movl 4(%[base], %[idx2], 2), %%edi\n\t"
            "subl %%edi, %[sum]\n\t"
            : [sum] "+r" (sum)
            : [base] "r" (ptr), [idx1] "r" (idx1 + i), [idx2] "r" (idx2 - i)
            : "esi", "edi", "memory"
        );
        
        idx1 += 3;
        idx2 -= 2;
    }
    
    return sum;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void __attribute__((noinline)) test_output_and_outaddr(int start_val) {
    volatile int base_idx = start_val;
    
    /* Multiple output addresses with complex computations */
    for (int i = 0; i < 5; i++) {
        int offset1 = (base_idx + i * 7) % 256;
        int offset2 = (base_idx - i * 3 + 128) % 256;
        
        /* Two different output memory locations with complex addresses */
        asm volatile(
            "movl %[val1], %%eax\n\t"
            "movl %%eax, (%[base1], %[off1], 4)\n\t"
            "movl %[val2], %%ebx\n\t"
            "movl %%ebx, 8(%[base2], %[off2], 2)\n\t"
            :
            : [val1] "r" (i + 100), [base1] "r" (g_global_array), [off1] "r" (offset1),
              [val2] "r" (i + 200), [base2] "r" (&g_outer_struct), [off2] "r" (offset2)
            : "eax", "ebx", "memory"
        );
    }
}

/* Test RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
void __attribute__((noinline)) test_operand_addresses(void) {
    volatile int idx = g_index1;
    
    /* Force address computation for function arguments */
    for (int i = 0; i < 4; i++) {
        /* Complex address expression as function argument */
        use_complex_address(
            &g_outer_struct.middle[idx % 3].inner[(idx + i) % 4],
            g_outer_struct.index_array[(idx * i) % 16]
        );
        
        /* Another complex address computation */
        int* complex_ptr = &g_outer_struct.middle[i].inner[0].data[
            (g_outer_struct.index_array[idx % 16] + i * 3) % 8
        ];
        
        asm volatile("" : : "r"(complex_ptr) : "memory");
        
        idx += 11;
    }
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
int __attribute__((noinline)) test_other_addresses(void) {
    int total = 0;
    volatile int* volatile base_ptr = g_volatile_ptr;
    
    /* Mixed operations that don't fit clean categories */
    for (int i = 0; i < 8; i++) {
        /* Complex addressing in multiple contexts */
        int temp1, temp2;
        
        /* First asm with memory input */
        asm volatile(
            "movl (%[ptr], %[scale], 4), %%ecx\n\t"
            "movl %%ecx, %[t1]\n\t"
            : [t1] "=r" (temp1)
            : [ptr] "r" (base_ptr), [scale] "r" (i * 3 + g_index1)
            : "ecx", "memory"
        );
        
        /* Second asm with different addressing */
        asm volatile(
            "movl %[idx], %%edx\n\t"
            "imull $37, %%edx, %%edx\n\t"
            "movl %%edx, %[t2]\n\t"
            : [t2] "=r" (temp2)
            : [idx] "m" (g_outer_struct.index_array[i])
            : "edx", "memory"
        );
        
        /* Third asm combining results */
        asm volatile(
            "addl %[a], %[b]\n\t"
            "movl %[b], %[total]\n\t"
            : [total] "+r" (total)
            : [a] "r" (temp1), [b] "r" (temp2)
            : "memory"
        );
        
        /* Force spill/reload of base pointer */
        base_ptr = g_volatile_ptr + ((i * 13) % 32);
    }
    
    return total;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Initialize test data */
    for (int i = 0; i < 1024; i++) {
        g_global_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 16; i++) {
        g_outer_struct.index_array[i] = i * 5 - 2;
    }
    
    /* Run all tests to trigger different reload types */
    checksum += test_input_address();
    
    test_output_address(checksum);
    
    checksum += test_input_and_inpaddr();
    
    test_output_and_outaddr(checksum % 256);
    
    test_operand_addresses();
    
    checksum += test_other_addresses();
    
    /* Final computation to ensure all code is used */
    volatile int final_result = checksum;
    for (int i = 0; i < 100; i++) {
        final_result = (final_result * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Reload test checksum: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
