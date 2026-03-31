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
    int base_value;
    volatile int index;
};

/* Global variables to increase register pressure */
volatile int global_index1 = 0;
volatile int global_index2 = 0;
struct outer_struct* volatile global_struct_ptr = NULL;

/* Function to trigger RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct outer_struct* s, int idx1, int idx2, int idx3) {
    /* Complex addressing: s->inner[idx1].member_array[idx2 + idx3] */
    int val;
    
    /* Force address computation with multiple register values */
    asm volatile (
        "movl %[array], %[val]\n\t"
        : [val] "=r" (val)
        : [array] "m" (s->inner[idx1].member_array[idx2 + idx3])
        : "memory"
    );
    
    /* Use the value to prevent optimization */
    global_index1 = val;
}

/* Function to trigger RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct outer_struct* s, int* results, int idx) {
    /* Complex output addressing with shift */
    int offset = (idx << 2) + s->base_value;
    
    /* Force output address reload */
    asm volatile (
        "movl $42, %[output]\n\t"
        : [output] "=m" (results[offset])
        : 
        : "memory"
    );
    
    /* Another with more complex computation */
    int complex_offset = (global_index1 * 3 + idx) % 16;
    asm volatile (
        "movl $99, %[output]\n\t"
        : [output] "=m" (s->inner[idx].member_array[complex_offset])
        :
        : "memory"
    );
}

/* Function to trigger RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(struct outer_struct* s, int* in_array, int* out_array) {
    volatile int idx = global_index2;
    
    /* Mixed input/output with address computations */
    for (int i = 0; i < 4; i++) {
        int in_idx = (idx + i * 3) % 8;
        int out_idx = (idx * 2 + i) % 8;
        
        /* Complex input address */
        int input_val;
        asm volatile (
            "movl %[input], %[val]\n\t"
            : [val] "=r" (input_val)
            : [input] "m" (in_array[in_idx + s->inner[i].member_array[0]])
            : "memory"
        );
        
        /* Complex output address based on input */
        asm volatile (
            "movl %[val], %[output]\n\t"
            : [output] "=m" (out_array[out_idx + input_val % 4])
            : [val] "r" (input_val)
            : "memory"
        );
    }
}

/* Function to trigger RELOAD_FOR_OPERAND_ADDRESS */
void helper_function(int* addr1, int* addr2, int* addr3) {
    /* Force address computations before call */
    *addr1 = *addr2 + *addr3;
}

void test_operand_address(struct outer_struct* s, int idx) {
    /* Complex address expressions as function arguments */
    helper_function(
        &s->inner[idx].member_array[global_index1],
        &s->inner[(idx + 1) % 4].member_array[global_index2],
        &s->inner[(idx + 2) % 4].member_array[(global_index1 + global_index2) % 8]
    );
}

/* Function to trigger RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address_types(int** ptr_array, int size) {
    volatile int idx = 0;
    
    /* Complex addressing in loop with multiple uses */
    for (int i = 0; i < size; i++) {
        /* Force address reload for pointer arithmetic */
        int* current = ptr_array[i] + (idx << 1);
        
        /* Use in inline asm with memory constraint */
        asm volatile (
            "addl $1, %[mem]\n\t"
            : [mem] "+m" (*current)
            : 
            : "memory"
        );
        
        /* Update index with complex computation */
        idx = (idx * 7 + i) % 16;
        
        /* Another memory operation with different addressing */
        int offset = (i * 3 + idx) % 8;
        asm volatile (
            "subl $2, %[mem2]\n\t"
            : [mem2] "+m" (ptr_array[offset][idx])
            :
            : "memory"
        );
    }
}

/* Function to trigger RELOAD_FOR_OPADDR_ADDR */
void test_opaddr_addr(struct outer_struct* s) {
    /* Take address of a complex expression */
    int* addr1 = &s->inner[global_index1 % 4].member_array[global_index2 % 8];
    int* addr2 = &s->inner[(global_index1 + 1) % 4].member_array[(global_index2 + 2) % 8];
    
    /* Use addresses in computations */
    int diff = addr2 - addr1;
    
    /* Force address reload through inline asm */
    asm volatile (
        "movl %[addr], %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        : 
        : [addr] "r" (addr1 + diff / 2)
        : "eax", "ebx", "memory"
    );
}

/* Main driver function */
int main() {
    /* Initialize test data */
    struct outer_struct test_struct;
    int results[64];
    int* ptr_array[16];
    
    /* Initialize structure */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            test_struct.inner[i].member_array[j] = i * 8 + j;
        }
        test_struct.inner[i].volatile_ptr = &global_index1;
    }
    test_struct.base_value = 100;
    test_struct.index = 0;
    
    /* Initialize pointer array */
    for (int i = 0; i < 16; i++) {
        ptr_array[i] = malloc(32 * sizeof(int));
        for (int j = 0; j < 32; j++) {
            ptr_array[i][j] = i * 32 + j;
        }
    }
    
    /* Initialize results array */
    for (int i = 0; i < 64; i++) {
        results[i] = 0;
    }
    
    /* Set global pointers */
    global_struct_ptr = &test_struct;
    
    /* Run tests to trigger different reload types */
    
    /* Test 1: Input address reloads */
    for (int i = 0; i < 10; i++) {
        test_input_address(&test_struct, i % 4, (i * 2) % 8, (i * 3) % 8);
    }
    
    /* Test 2: Output address reloads */
    for (int i = 0; i < 8; i++) {
        test_output_address(&test_struct, results, i % 4);
    }
    
    /* Test 3: Input/Output address reloads */
    int in_array[32];
    int out_array[32];
    for (int i = 0; i < 32; i++) {
        in_array[i] = i;
        out_array[i] = 0;
    }
    test_inpaddr_outaddr(&test_struct, in_array, out_array);
    
    /* Test 4: Operand address reloads */
    for (int i = 0; i < 5; i++) {
        test_operand_address(&test_struct, i);
    }
    
    /* Test 5: Other address types */
    test_other_address_types(ptr_array, 16);
    
    /* Test 6: Opaddr address reloads */
    test_opaddr_addr(&test_struct);
    
    /* Compute checksum to ensure all code executes */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += results[i];
    }
    for (int i = 0; i < 32; i++) {
        checksum += out_array[i];
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += test_struct.inner[i].member_array[j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    for (int i = 0; i < 16; i++) {
        free(ptr_array[i]);
    }
    
    return 0;
}
