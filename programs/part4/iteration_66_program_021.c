/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
struct Inner {
    int data[8];
    int extra;
};

struct Outer {
    struct Inner arrays[4];
    int base;
    long offset;
};

/* Volatile variables to prevent optimization */
volatile int global_index = 0;
volatile int *volatile global_ptr = NULL;

/* Test function for RELOAD_FOR_INPUT_ADDRESS */
int test_input_address(struct Outer *outer, int idx1, int idx2, int idx3) {
    int sum = 0;
    
    /* Complex addressing: outer->arrays[idx1].data[idx2 + idx3] */
    /* This forces address computation with multiple registers */
    asm volatile (
        "movl (%[addr]), %[sum]\n\t"
        : [sum] "=r" (sum)
        : [addr] "m" (outer->arrays[idx1].data[idx2 + idx3])
        : "memory"
    );
    
    /* More complex addressing with shift */
    asm volatile (
        ""
        :: "m" (outer->arrays[(idx1 << 1) + idx2].data[idx3])
        : "memory"
    );
    
    return sum;
}

/* Test function for RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct Outer *outer, int idx1, int idx2, int value) {
    /* Complex output addressing */
    asm volatile (
        "movl %[val], (%[addr])\n\t"
        : "=m" (outer->arrays[idx1].data[idx2 * 2])
        : [val] "r" (value), [addr] "r" (&outer->arrays[idx1].data[idx2 * 2])
        : "memory"
    );
    
    /* Output with base+index+displacement */
    asm volatile (
        ""
        : "=m" (outer->arrays[0].data[idx1 + idx2 + outer->base])
        : "r" (value)
        : "memory"
    );
}

/* Test function for RELOAD_FOR_INPUT and mixed types */
int test_mixed_reloads(struct Outer *outer, int *indices, int count) {
    int total = 0;
    
    for (int i = 0; i < count; i++) {
        int idx1 = indices[i];
        int idx2 = indices[(i + 1) % count];
        
        /* Mixed input/output with complex addressing */
        int temp;
        asm volatile (
            "movl (%[in]), %[temp]\n\t"
            "addl %[temp], %[total]\n\t"
            "movl %[temp], (%[out])\n\t"
            : [total] "+r" (total), [temp] "=r" (temp),
              "=m" (outer->arrays[idx1 % 4].data[idx2 % 8])
            : [in] "m" (outer->arrays[idx2 % 4].data[idx1 % 8]),
              [out] "r" (&outer->arrays[idx1 % 4].data[idx2 % 8])
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile (
            ""
            :: "m" (outer->arrays[idx1].data[outer->base + idx2]),
               "m" (outer->base)
            : "memory"
        );
    }
    
    return total;
}

/* Test function for RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address_types(int **ptr_array, int *offsets, int n) {
    for (int i = 0; i < n; i++) {
        /* Complex indirect addressing */
        asm volatile (
            "movl (%[base], %[idx], 4), %%eax\n\t"
            "addl %%eax, (%[dest])\n\t"
            : "=m" (**(ptr_array + i))
            : [base] "r" (ptr_array[i]), 
              [idx] "r" (offsets[i]),
              [dest] "r" (ptr_array[(i + 1) % n])
            : "eax", "memory"
        );
        
        /* Multiple memory operands with different addressing */
        asm volatile (
            ""
            :: "m" (ptr_array[offsets[i] % n][offsets[(i + 1) % n] % 16]),
               "m" (offsets[i * 2 % n])
            : "memory"
        );
    }
}

/* Helper to force RELOAD_FOR_OPERAND_ADDRESS */
void use_complex_address(struct Inner *inner, int offset) {
    /* Taking address of complex expression */
    asm volatile (
        "leal (%[base], %[off], 4), %%eax\n\t"
        : 
        : [base] "r" (inner), [off] "r" (offset)
        : "eax", "memory"
    );
}

/* Test function for RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_addr_of_addr(struct Outer *outer, int idx) {
    struct Inner *temp;
    
    /* Address of address computation */
    asm volatile (
        "leal (%[base], %[idx], 8), %[temp]\n\t"
        : [temp] "=r" (temp)
        : [base] "r" (outer->arrays), [idx] "r" (idx)
        : "memory"
    );
    
    /* Use the computed address */
    asm volatile (
        "movl (%[addr]), %%eax\n\t"
        : 
        : [addr] "m" (*temp)
        : "eax", "memory"
    );
    
    /* Output address of address */
    struct Inner **ptr_ptr;
    asm volatile (
        "leal (%[base], %[idx], 4), %[ptr]\n\t"
        : [ptr] "=r" (ptr_ptr)
        : [base] "r" (&outer->arrays), [idx] "r" (idx)
        : "memory"
    );
}

/* Main driver that exercises all test functions */
int main(void) {
    /* Initialize test data */
    struct Outer outer_data;
    int indices[] = {1, 3, 5, 7, 2, 4, 6, 0};
    int *ptr_array[8];
    int offsets[8];
    
    /* Initialize structures */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            outer_data.arrays[i].data[j] = i * 10 + j;
        }
        outer_data.arrays[i].extra = i * 100;
    }
    outer_data.base = 2;
    outer_data.offset = 4;
    
    /* Initialize pointer array */
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = malloc(16 * sizeof(int));
        for (int j = 0; j < 16; j++) {
            ptr_array[i][j] = i * 20 + j;
        }
        offsets[i] = i * 3;
    }
    
    /* Call test functions to trigger different reload types */
    int sum1 = test_input_address(&outer_data, 1, 2, 3);
    test_output_address(&outer_data, 2, 1, 99);
    int sum2 = test_mixed_reloads(&outer_data, indices, 8);
    test_other_address_types(ptr_array, offsets, 8);
    test_addr_of_addr(&outer_data, 1);
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS through function call */
    use_complex_address(&outer_data.arrays[0], outer_data.base);
    
    /* Compute checksum to prevent optimization */
    int checksum = sum1 + sum2;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += outer_data.arrays[i].data[j];
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(ptr_array[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
