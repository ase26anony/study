/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
struct inner {
    int member_array[8];
    volatile int* volatile_ptr;
};

struct outer {
    struct inner inner_struct[4];
    int base_array[16];
    volatile int index;
};

/* Global volatile variables to prevent optimization */
volatile int g_index1 = 1;
volatile int g_index2 = 2;
volatile int* g_ptr = NULL;

/* Function to test RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct outer* data, int idx1, int idx2) {
    /* Complex addressing: array[(index << 2) + struct.member] */
    int val;
    
    /* Force input address reload with multiple register values */
    asm volatile (
        "movl %[result], %[val]\n\t"
        : [val] "=r" (val)
        : [result] "m" (data->inner_struct[idx1].member_array[(idx2 << 1) + g_index1])
        : "memory"
    );
    
    /* Another complex input address computation */
    asm volatile (
        ""
        :
        : "m" (data->base_array[idx1 * 3 + idx2 * 2]),
          "m" (data->inner_struct[g_index2].member_array[idx1])
        : "memory"
    );
    
    /* Use the value to prevent dead code elimination */
    data->index = val;
}

/* Function to test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct outer* data, int offset1, int offset2) {
    /* Complex output addressing with multiple computations */
    int temp = offset1 + offset2;
    
    /* Force output address reload */
    asm volatile (
        "movl %[src], %%eax\n\t"
        "movl %%eax, %[dest]\n\t"
        : [dest] "=m" (data->inner_struct[offset1].member_array[temp])
        : [src] "r" (offset2)
        : "eax", "memory"
    );
    
    /* Nested output addressing */
    asm volatile (
        "movl $42, %[out]\n\t"
        : [out] "=m" (data->base_array[(offset1 << 2) | (offset2 & 3)])
        :
        : "memory"
    );
}

/* Function to test RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(struct outer* data, int idx) {
    /* Taking address of complex expression forces operand address reload */
    volatile int* addr1 = &data->inner_struct[idx].member_array[idx * 2];
    volatile int* addr2 = &data->base_array[(idx << 1) + g_index1];
    
    /* Use addresses in inline asm to force reloads */
    asm volatile (
        ""
        :
        : "r" (addr1), "r" (addr2)
        : "memory"
    );
    
    /* Function call with complex address argument */
    helper_function(&data->inner_struct[g_index2].member_array[idx * 3]);
}

/* Helper function to force address computation before call */
void helper_function(volatile int* ptr) {
    *ptr = *ptr + 1;
}

/* Function to test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(struct outer* data, int i, int j) {
    int temp;
    
    /* Mixed input/output with complex addressing */
    asm volatile (
        "movl %[in], %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=m" (data->inner_struct[i].member_array[j])
        : [in] "m" (data->base_array[i + j]),
          [idx] "r" (j)
        : "eax", "memory"
    );
    
    /* Multiple memory operands with different addressing */
    asm volatile (
        "movl (%[addr1]), %%ebx\n\t"
        "addl (%[addr2]), %%ebx\n\t"
        "movl %%ebx, %[result]\n\t"
        : [result] "=m" (data->base_array[i])
        : [addr1] "r" (&data->inner_struct[i].member_array[j]),
          [addr2] "r" (&data->inner_struct[j].member_array[i])
        : "ebx", "memory"
    );
}

/* Function to test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address(struct outer* data, int idx) {
    /* Complex addressing in loop to force various reload types */
    for (int i = 0; i < 4; i++) {
        volatile int* volatile_ptr = &data->inner_struct[i].member_array[idx];
        
        /* Force other address reloads */
        asm volatile (
            "movl (%[base], %[index], 4), %%ecx\n\t"
            "addl %%ecx, %[accum]\n\t"
            : [accum] "+m" (data->index)
            : [base] "r" (data->base_array),
              [index] "r" (i + idx)
            : "ecx", "memory"
        );
        
        /* Use volatile pointer to prevent optimization */
        *volatile_ptr = *volatile_ptr + i;
    }
    
    /* Force RELOAD_OTHER type with unusual pattern */
    asm volatile (
        ""
        : "=m" (data->inner_struct[0].member_array[0]),
          "=m" (data->inner_struct[1].member_array[1]),
          "=m" (data->inner_struct[2].member_array[2])
        : "r" (idx), "r" (g_ptr)
        : "memory"
    );
}

/* Mixed test combining multiple reload types */
void test_mixed_reloads(struct outer* data, int iterations) {
    for (int i = 0; i < iterations; i++) {
        int idx1 = (i * 3) % 4;
        int idx2 = (i * 5) % 8;
        
        /* Alternate between different test patterns */
        if (i % 3 == 0) {
            test_input_address(data, idx1, idx2);
        } else if (i % 3 == 1) {
            test_output_address(data, idx1, idx2);
        } else {
            test_inpaddr_outaddr(data, idx1, idx2);
        }
        
        /* Force operand address reload periodically */
        if (i % 5 == 0) {
            test_operand_address(data, idx1);
        }
    }
    
    /* Final other address test */
    test_other_address(data, iterations % 4);
}

/* Main driver function */
int main() {
    /* Initialize test data */
    struct outer* data = (struct outer*)malloc(sizeof(struct outer));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            data->inner_struct[i].member_array[j] = i * 8 + j;
        }
        data->inner_struct[i].volatile_ptr = &data->index;
    }
    
    for (int i = 0; i < 16; i++) {
        data->base_array[i] = i * 2;
    }
    
    data->index = 0;
    g_ptr = &data->index;
    
    printf("Starting reload coverage tests...\n");
    
    /* Run tests to trigger different reload types */
    test_input_address(data, g_index1, g_index2);
    test_output_address(data, 2, 3);
    test_operand_address(data, 1);
    test_inpaddr_outaddr(data, 0, 2);
    test_other_address(data, 3);
    
    /* Comprehensive mixed test */
    test_mixed_reloads(data, 10);
    
    /* Compute checksum to ensure all computations are used */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += data->inner_struct[i].member_array[j];
        }
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += data->base_array[i];
    }
    
    checksum += data->index;
    
    printf("Checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    free(data);
    return 0;
}
