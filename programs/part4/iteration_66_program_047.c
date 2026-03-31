/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
struct inner_struct {
    int member_array[8];
    volatile int* volatile_ptr;
};

struct outer_struct {
    struct inner_struct inner[4];
    int outer_array[16];
    volatile int index;
};

/* Global volatile variables to prevent optimization */
volatile int global_index = 0;
volatile int* volatile global_ptr = NULL;

/* Test function for RELOAD_FOR_INPUT_ADDRESS */
int test_input_address(struct outer_struct* os, int idx1, int idx2) {
    int result = 0;
    
    /* Complex addressing that requires input address reload */
    for (int i = 0; i < 4; i++) {
        /* Force address computation with multiple registers */
        int addr_idx = (idx1 + i) * 2 + idx2;
        
        /* Inline asm with memory input constraint and complex address */
        asm volatile (
            "addl %%ebx, %%eax\n\t"
            "movl (%%rax), %%ecx\n\t"
            : "=c" (result)
            : "a" (&os->inner[i].member_array[0]), 
              "b" (addr_idx * sizeof(int)),
              "m" (os->inner[i].member_array[addr_idx])  /* Memory input */
            : "memory"
        );
    }
    
    return result;
}

/* Test function for RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct outer_struct* os, int* indices, int count) {
    /* Force output address reloads with computed addresses */
    for (int i = 0; i < count; i++) {
        int offset = indices[i] * 3 + i;
        
        /* Inline asm with memory output constraint at computed address */
        asm volatile (
            "movl %%eax, (%%rbx)\n\t"
            : "=m" (os->outer_array[offset])  /* Memory output */
            : "a" (i * 100), 
              "b" (&os->outer_array[offset])
            : "memory"
        );
    }
}

/* Test function for RELOAD_FOR_INPADDR_ADDRESS */
int test_inpaddr_address(struct outer_struct* os, volatile int* idx_ptr) {
    int sum = 0;
    
    /* Force input address address reloads */
    for (int i = 0; i < 3; i++) {
        int complex_idx = (*idx_ptr << 2) + i;
        
        /* Nested addressing requiring multiple reloads */
        asm volatile (
            "movl (%%rax, %%rbx, 4), %%ecx\n\t"
            "addl %%ecx, %%edx\n\t"
            : "+d" (sum)
            : "a" (&os->inner[0].member_array[0]),
              "b" (complex_idx),
              "m" (*(&os->inner[0].member_array[complex_idx]))  /* Indirect memory */
            : "ecx", "memory"
        );
    }
    
    return sum;
}

/* Test function for RELOAD_FOR_OUTADDR_ADDRESS */
void test_outaddr_address(struct outer_struct* os, int base_idx) {
    /* Force output address address reloads */
    for (int i = 0; i < 2; i++) {
        int* target = &os->inner[i].member_array[base_idx + i * 2];
        
        /* Complex output addressing */
        asm volatile (
            "leal (%%rax, %%rbx, 2), %%ecx\n\t"
            "movl %%ecx, (%%rdx)\n\t"
            : "=m" (*target)  /* Memory output through computed pointer */
            : "a" (i), "b" (base_idx), "d" (target)
            : "ecx", "memory"
        );
    }
}

/* Test function for RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(struct inner_struct* is, int idx) {
    /* Force operand address reload by taking address of complex expression */
    volatile int* addr = &is->member_array[(idx << 1) + 3];
    
    /* Use in inline asm */
    asm volatile (
        "movl $0x12345678, %0\n\t"
        : "=m" (*addr)
        :
        : "memory"
    );
}

/* Test function for RELOAD_FOR_OPADDR_ADDR */
int test_opaddr_addr(struct outer_struct* os, int idx1, int idx2) {
    int result = 0;
    
    /* Force other operand address reloads */
    int* volatile ptr1 = &os->outer_array[idx1];
    int* volatile ptr2 = &os->inner[0].member_array[idx2];
    
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl (%2), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (result)
        : "r" (ptr1), "r" (ptr2), "m" (*ptr1), "m" (*ptr2)
        : "eax", "memory"
    );
    
    return result;
}

/* Test function for RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(struct outer_struct* os, int* indices, int n) {
    /* Mixed addressing modes to trigger other address reloads */
    for (int i = 0; i < n; i++) {
        /* Complex address computation that changes each iteration */
        int offset = (indices[i] * i) % 8;
        
        /* Multiple memory operands with different addressing */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m" (os->inner[i % 4].member_array[offset])
            : "m" (os->outer_array[indices[i] % 16]),
              "m" (os->inner[(i + 1) % 4].member_array[(offset + 1) % 8])
            : "eax", "memory"
        );
    }
}

/* Test function for RELOAD_OTHER */
int test_other_reload(struct outer_struct* os) {
    int total = 0;
    
    /* Complex pattern that might trigger RELOAD_OTHER */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            /* Volatile pointer chasing */
            volatile int* ptr = &os->inner[i].member_array[j];
            
            asm volatile (
                "movl (%1), %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+r" (total)
                : "r" (ptr)
                : "eax", "memory"
            );
            
            /* Modify through pointer */
            *ptr = total + i + j;
        }
    }
    
    return total;
}

/* Main driver function */
int main() {
    /* Initialize test data structures */
    struct outer_struct os;
    int indices[] = {1, 3, 5, 7, 9, 11, 13, 15};
    
    /* Initialize arrays */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            os.inner[i].member_array[j] = i * 100 + j;
        }
        os.inner[i].volatile_ptr = &global_index;
    }
    
    for (int i = 0; i < 16; i++) {
        os.outer_array[i] = i * 10;
    }
    
    os.index = 2;
    
    /* Call test functions to trigger different reload types */
    int result1 = test_input_address(&os, 1, 2);
    test_output_address(&os, indices, 8);
    int result2 = test_inpaddr_address(&os, &global_index);
    test_outaddr_address(&os, 3);
    test_operand_address(&os.inner[1], 2);
    int result3 = test_opaddr_addr(&os, 4, 5);
    test_other_address(&os, indices, 8);
    int result4 = test_other_reload(&os);
    
    /* Compute checksum to ensure code isn't optimized away */
    int checksum = result1 + result2 + result3 + result4;
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += os.inner[i].member_array[j];
        }
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += os.outer_array[i];
    }
    
    printf("Reload test checksum: %d\n", checksum);
    
    return 0;
}
