/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
typedef struct {
    int data[8];
    struct {
        int x;
        int y[4];
    } nested;
} ComplexStruct;

typedef struct {
    ComplexStruct* array[16];
    int offsets[32];
} Container;

/* Volatile variables to prevent optimization */
volatile int g_index1 = 3;
volatile int g_index2 = 7;
volatile int g_offset = 12;

/* Global test data */
ComplexStruct g_struct_array[32];
Container g_container;
int g_output_buffer[256];

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(void) {
    volatile int idx1 = g_index1;
    volatile int idx2 = g_index2;
    volatile int off = g_offset;
    
    /* Complex addressing that requires input address reload */
    int result;
    asm volatile (
        "movl %[array], %%eax\n\t"
        "movl %[idx1], %%ebx\n\t"
        "movl %[idx2], %%ecx\n\t"
        "addl %%ebx, %%ecx\n\t"
        "shll $2, %%ecx\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl (%%eax), %[res]"
        : [res] "=r" (result)
        : [array] "m" (g_struct_array),
          [idx1] "m" (idx1),
          [idx2] "m" (idx2)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* Use result to prevent dead code elimination */
    g_output_buffer[0] = result;
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(void) {
    volatile int base_idx = g_index1 * 4;
    volatile int offset = g_offset;
    
    /* Complex output addressing */
    int value = 0x12345678;
    asm volatile (
        "movl %[value], %%eax\n\t"
        "movl %[buffer], %%ebx\n\t"
        "movl %[base], %%ecx\n\t"
        "movl %[offset], %%edx\n\t"
        "leal (%%ebx,%%ecx,4), %%ebx\n\t"
        "addl %%edx, %%ecx\n\t"
        "movl %%eax, (%%ebx,%%ecx,4)"
        : 
        : [value] "r" (value),
          [buffer] "m" (g_output_buffer),
          [base] "m" (base_idx),
          [offset] "m" (offset)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS */
void test_inpaddr_address(void) {
    volatile int idx = g_index1;
    
    /* Addressing with pointer indirection */
    ComplexStruct* ptr = &g_struct_array[idx];
    
    asm volatile (
        "movl %[ptr], %%eax\n\t"
        "movl 16(%%eax), %%ebx\n\t"
        "movl %%ebx, %[out]"
        : [out] "=m" (g_output_buffer[4])
        : [ptr] "m" (ptr)
        : "eax", "ebx", "memory"
    );
}

/* Function to force RELOAD_FOR_OUTADDR_ADDRESS */
void test_outaddr_address(void) {
    volatile int idx = g_index2;
    
    /* Complex output with address computation */
    int* dest = &g_output_buffer[idx * 2];
    
    asm volatile (
        "movl $0xDEADBEEF, %%eax\n\t"
        "movl %[dest], %%ebx\n\t"
        "movl %%eax, (%%ebx)"
        : 
        : [dest] "m" (dest)
        : "eax", "ebx", "memory"
    );
}

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(ComplexStruct* cs, int idx1, int idx2) {
    /* Complex address passed as function argument */
    int val = cs[idx1].nested.y[idx2];
    g_output_buffer[8] = val;
}

/* Function to force RELOAD_FOR_OPADDR_ADDR */
void test_opaddr_addr(void) {
    volatile int idx = g_index1;
    
    /* Address of address computation */
    int** ptr_ptr = (int**)&g_output_buffer[16];
    
    asm volatile (
        "movl %[idx], %%eax\n\t"
        "leal g_struct_array(,%%eax,8), %%ebx\n\t"
        "movl %%ebx, %[ptr]"
        : [ptr] "=m" (*ptr_ptr)
        : [idx] "m" (idx)
        : "eax", "ebx", "memory"
    );
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(void) {
    volatile int idx1 = g_index1;
    volatile int idx2 = g_index2;
    
    /* Mixed addressing modes in loop */
    for (int i = 0; i < 4; i++) {
        int* addr1 = &g_struct_array[idx1 + i].data[idx2];
        int* addr2 = &g_output_buffer[idx2 * i];
        
        asm volatile (
            "movl (%[addr1]), %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, (%[addr2])"
            : 
            : [addr1] "r" (addr1),
              [addr2] "r" (addr2)
            : "eax", "memory"
        );
    }
}

/* Function to force RELOAD_OTHER */
void test_other_reload(void) {
    volatile int idx = g_index1;
    
    /* Multiple constraints forcing various reloads */
    int temp1, temp2;
    
    asm volatile (
        "movl %[idx], %%eax\n\t"
        "movl g_struct_array(,%%eax,8), %%ebx\n\t"
        "movl %%ebx, %[t1]\n\t"
        "leal g_output_buffer(,%%eax,4), %%ecx\n\t"
        "movl (%%ecx), %[t2]"
        : [t1] "=m" (temp1),
          [t2] "=m" (temp2)
        : [idx] "m" (idx)
        : "eax", "ebx", "ecx", "memory"
    );
    
    g_output_buffer[20] = temp1 + temp2;
}

/* Mixed test combining multiple reload types */
void test_mixed_reloads(void) {
    volatile int idx = g_index1;
    volatile int offset = g_offset;
    
    /* Complex expression with multiple address computations */
    int value = g_struct_array[idx].nested.y[offset % 4];
    
    /* Force output address reload */
    g_output_buffer[idx * 3 + 2] = value;
    
    /* Inline asm with mixed constraints */
    asm volatile (
        "movl %[val], %%eax\n\t"
        "movl %[idx], %%ebx\n\t"
        "movl %[offset], %%ecx\n\t"
        "leal g_output_buffer(,%%ebx,4), %%edx\n\t"
        "addl %%ecx, %%edx\n\t"
        "movl %%eax, (%%edx)"
        : 
        : [val] "m" (value),
          [idx] "m" (idx),
          [offset] "m" (offset)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
}

/* Initialize test data */
void init_test_data(void) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) {
            g_struct_array[i].data[j] = i * 100 + j;
        }
        for (int j = 0; j < 4; j++) {
            g_struct_array[i].nested.y[j] = i * 50 + j * 10;
        }
        g_struct_array[i].nested.x = i * 25;
    }
    
    for (int i = 0; i < 256; i++) {
        g_output_buffer[i] = 0;
    }
}

/* Compute checksum to ensure all code executes */
int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += g_output_buffer[i];
        sum &= 0xFFFF; /* Keep it manageable */
    }
    return sum;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    init_test_data();
    
    /* Execute all test functions to trigger different reload types */
    test_input_address();
    test_output_address();
    test_inpaddr_address();
    test_outaddr_address();
    test_operand_address(&g_struct_array[0], g_index1, g_index2 % 4);
    test_opaddr_addr();
    test_other_address();
    test_other_reload();
    test_mixed_reloads();
    
    /* Additional complex scenario */
    for (int i = 0; i < 8; i++) {
        volatile int idx = i;
        int* ptr = &g_struct_array[idx].data[idx % 8];
        
        asm volatile (
            "movl (%[ptr]), %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, g_output_buffer(,%[idx],4)"
            : 
            : [ptr] "r" (ptr),
              [idx] "r" (idx)
            : "eax", "memory"
        );
    }
    
    int checksum = compute_checksum();
    printf("Test completed. Checksum: %d\n", checksum);
    printf("If checksum != 0, code executed successfully.\n");
    
    return 0;
}
