/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force complex addressing */
typedef struct {
    int data[8];
    int* ptr_array[4];
} InnerStruct;

typedef struct {
    InnerStruct inner[4];
    long long big_array[16];
    volatile int* volatile_index;
} OuterStruct;

/* Global volatile variables to prevent optimization */
volatile int g_index1 = 0;
volatile int g_index2 = 0;
volatile int g_base_offset = 100;

/* Function to test RELOAD_FOR_INPUT_ADDRESS */
int test_input_address(OuterStruct* os, int idx1, int idx2) {
    int result = 0;
    
    /* Complex addressing that requires input address reload */
    for (int i = 0; i < 4; i++) {
        /* Force address computation with multiple components */
        int complex_idx = (idx1 << 2) + (idx2 >> 1) + i * g_base_offset;
        
        /* Inline asm with memory input using complex address */
        asm volatile (
            "movl %[mem], %%eax\n\t"
            "addl %%eax, %[res]\n\t"
            : [res] "+r" (result)
            : [mem] "m" (os->inner[i].data[complex_idx % 8])
            : "eax", "memory"
        );
    }
    
    return result;
}

/* Function to test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(OuterStruct* os, int* offsets, int count) {
    /* Force output to memory with complex addressing */
    for (int i = 0; i < count; i++) {
        int offset = offsets[i] + g_index1;
        
        /* Inline asm with memory output to complex address */
        asm volatile (
            "movl %[val], %%eax\n\t"
            "movl %%eax, %[mem]\n\t"
            : [mem] "=m" (os->big_array[offset % 16])
            : [val] "ri" (i * 100)
            : "eax", "memory"
        );
    }
}

/* Function to test RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT_ADDRESS together */
int test_mixed_io(OuterStruct* os1, OuterStruct* os2, int idx) {
    int temp = 0;
    
    /* Mixed input/output with complex addressing */
    for (int i = 0; i < 3; i++) {
        int addr_component = (idx * i) + g_index2;
        
        /* Read from complex address in os1 */
        asm volatile (
            "movl %[in_mem], %%ebx\n\t"
            "movl %%ebx, %[temp]\n\t"
            : [temp] "=r" (temp)
            : [in_mem] "m" (os1->inner[i].data[addr_component % 8])
            : "ebx", "memory"
        );
        
        /* Write to complex address in os2 */
        asm volatile (
            "movl %[val], %%ecx\n\t"
            "movl %%ecx, %[out_mem]\n\t"
            : [out_mem] "=m" (os2->inner[i].data[(addr_component + 1) % 8])
            : [val] "r" (temp + i)
            : "ecx", "memory"
        );
    }
    
    return temp;
}

/* Function to test RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(InnerStruct** ptr_array, int count) {
    /* Force address computation before function-like asm */
    for (int i = 0; i < count; i++) {
        int complex_offset = i * 2 + g_index1;
        
        /* Complex address as operand */
        asm volatile (
            "call *%[addr]\n\t"
            "nop\n\t"
            : 
            : [addr] "r" (&ptr_array[i]->data[complex_offset % 8])
            : "memory"
        );
    }
}

/* Function to test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_addr_of_addr(OuterStruct* os, int* indices) {
    /* Create situations needing address-of-address reloads */
    for (int i = 0; i < 4; i++) {
        volatile int** addr_ptr;
        int idx = indices[i] + g_index2;
        
        /* Get address of a memory location (inpaddr) */
        asm volatile (
            "leaq %[base], %[ptr]\n\t"
            : [ptr] "=r" (addr_ptr)
            : [base] "m" (os->inner[idx % 4].data)
            : "memory"
        );
        
        /* Use that address for output (outaddr) */
        asm volatile (
            "movq %[val], (%[ptr])\n\t"
            : 
            : [val] "ri" ((long long)addr_ptr), [ptr] "r" (&os->inner[i].ptr_array[idx % 4])
            : "memory"
        );
    }
}

/* Function to test RELOAD_FOR_OTHER_ADDRESS */
int test_other_address(OuterStruct* os, int idx) {
    int result = 0;
    
    /* Complex addressing that doesn't fit other categories */
    for (int i = 0; i < 8; i++) {
        /* Very complex address computation */
        int addr = ((idx << i) | (i << idx)) + g_base_offset + g_index1 - g_index2;
        
        /* Memory access with shifting offset */
        asm volatile (
            "movl (%[base], %[offset], 4), %%edx\n\t"
            "addl %%edx, %[res]\n\t"
            : [res] "+r" (result)
            : [base] "r" (os->big_array), [offset] "r" (addr % 12)
            : "edx", "memory"
        );
    }
    
    return result;
}

/* Main driver function */
int main() {
    /* Initialize test data */
    OuterStruct os1, os2;
    InnerStruct* ptr_array[8];
    int indices[8];
    int offsets[8];
    
    /* Initialize structures */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            os1.inner[i].data[j] = i * 100 + j;
            os2.inner[i].data[j] = i * 200 + j;
        }
        for (int j = 0; j < 4; j++) {
            os1.inner[i].ptr_array[j] = &os1.inner[(i + j) % 4].data[0];
            os2.inner[i].ptr_array[j] = &os2.inner[(i + j) % 4].data[0];
        }
    }
    
    for (int i = 0; i < 16; i++) {
        os1.big_array[i] = i * 50;
        os2.big_array[i] = i * 75;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = &os1.inner[i % 4];
        indices[i] = i * 3;
        offsets[i] = i * 2;
    }
    
    os1.volatile_index = &g_index1;
    os2.volatile_index = &g_index2;
    
    /* Run tests to trigger different reload types */
    int checksum = 0;
    
    /* Test 1: Input address reloads */
    checksum += test_input_address(&os1, 1, 2);
    
    /* Test 2: Output address reloads */
    test_output_address(&os2, offsets, 8);
    
    /* Test 3: Mixed input/output */
    checksum += test_mixed_io(&os1, &os2, 3);
    
    /* Test 4: Operand address reloads */
    test_operand_address(ptr_array, 8);
    
    /* Test 5: Address-of-address reloads */
    test_addr_of_addr(&os1, indices);
    
    /* Test 6: Other address reloads */
    checksum += test_other_address(&os2, 4);
    
    /* Use results to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    printf("Sample values: %lld, %d\n", 
           os2.big_array[5], 
           os1.inner[2].data[3]);
    
    return 0;
}
