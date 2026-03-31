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
    int base_array[32];
    volatile int index;
};

/* Global variables to increase register pressure */
volatile int global_index1 = 0;
volatile int global_index2 = 0;
struct outer* volatile global_struct_ptr = NULL;

/* Function to trigger RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct outer* data, int idx1, int idx2, int idx3) {
    /* Complex addressing: data->inner_struct[idx1].member_array[idx2 + idx3] */
    int val;
    
    /* Force address computation before inline asm */
    asm volatile (
        "/* Input address reload test */\n\t"
        : "=r"(val)
        : "m"(data->inner_struct[idx1].member_array[idx2 + idx3]),
          "r"(idx1), "r"(idx2), "r"(idx3)
        : "memory"
    );
    
    /* Multiple complex address computations in sequence */
    for (int i = 0; i < 4; i++) {
        int offset = (idx1 << i) + idx2;
        asm volatile (
            ""
            : 
            : "m"(data->base_array[offset]),
              "r"(offset)
            : "memory"
        );
    }
}

/* Function to trigger RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct outer* data, int idx1, int idx2, int idx3) {
    /* Complex output addressing */
    int temp = idx1 * idx2;
    
    /* Force output address reload */
    asm volatile (
        "/* Output address reload test */\n\t"
        : "=m"(data->inner_struct[idx1].member_array[idx2 + idx3])
        : "r"(temp), "r"(idx1), "r"(idx2), "r"(idx3)
        : "memory"
    );
    
    /* Multiple output addresses with different computations */
    for (int i = 0; i < 3; i++) {
        int complex_idx = (idx1 << 2) + (idx2 * i) + idx3;
        asm volatile (
            ""
            : "=m"(data->base_array[complex_idx])
            : "r"(complex_idx)
            : "memory"
        );
    }
}

/* Function to trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
void helper_func(int* addr1, volatile int* addr2, int** addr3) {
    /* Force address computations before call */
    asm volatile (
        "/* Operand address usage */\n\t"
        : 
        : "r"(addr1), "r"(addr2), "r"(addr3)
        : "memory"
    );
}

void test_operand_address(struct outer* data, int idx1, int idx2) {
    /* Complex address expressions as function arguments */
    helper_func(
        &data->inner_struct[idx1].member_array[idx2],
        &data->base_array[(idx1 << 3) + idx2],
        (int**)&data->inner_struct[idx1].volatile_ptr
    );
    
    /* Multiple calls with different address computations */
    for (int i = 0; i < 2; i++) {
        int complex_offset = (idx1 * i) + (idx2 << 1);
        helper_func(
            &data->base_array[complex_offset],
            &data->inner_struct[i].member_array[idx1],
            (int**)&data->inner_struct[idx2].volatile_ptr
        );
    }
}

/* Function to trigger RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_mixed_address_reloads(struct outer* data, int idx1, int idx2, int idx3) {
    /* Mixed input and output with complex addressing */
    int temp1, temp2;
    
    /* Input address for computation, output address for result */
    asm volatile (
        "/* Mixed address reloads */\n\t"
        : "=m"(data->inner_struct[idx1].member_array[idx2]), 
          "=r"(temp1), "=r"(temp2)
        : "m"(data->base_array[idx3]),
          "r"(idx1), "r"(idx2), "r"(idx3)
        : "memory"
    );
    
    /* Chain of address-dependent operations */
    for (int i = 0; i < 4; i++) {
        int in_idx = (idx1 + i) * 2;
        int out_idx = (idx2 << i) + idx3;
        
        asm volatile (
            ""
            : "=m"(data->base_array[out_idx]), "=r"(temp1)
            : "m"(data->inner_struct[i].member_array[in_idx]),
              "r"(in_idx), "r"(out_idx)
            : "memory"
        );
    }
}

/* Function to trigger RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(struct outer* data, int idx1, int idx2) {
    /* Unusual address computation pattern */
    int* volatile ptr_array[4];
    int temp;
    
    /* Setup pointers with complex addresses */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &data->inner_struct[i].member_array[(idx1 + i) * idx2];
    }
    
    /* Use all pointers in a way that forces address reloads */
    for (int i = 0; i < 4; i++) {
        asm volatile (
            "/* Other address reload */\n\t"
            : "=r"(temp)
            : "m"(*ptr_array[i]), "r"(ptr_array[i])
            : "memory"
        );
    }
    
    /* Additional complex addressing */
    int (*complex_ptr)[8] = &data->inner_struct[idx1].member_array;
    asm volatile (
        ""
        : 
        : "m"((*complex_ptr)[idx2]), "r"(complex_ptr), "r"(idx2)
        : "memory"
    );
}

/* Function to trigger RELOAD_OTHER */
void test_other_reload(struct outer* data, int idx1, int idx2, int idx3) {
    /* Multiple memory operations with register pressure */
    register int r1 asm("r10") = idx1;
    register int r2 asm("r11") = idx2;
    register int r3 asm("r12") = idx3;
    
    /* Force spilling of all registers */
    asm volatile (
        "/* Force register spilling */\n\t"
        : "+r"(r1), "+r"(r2), "+r"(r3)
        : "m"(data->inner_struct[0].member_array[0]),
          "m"(data->inner_struct[1].member_array[1]),
          "m"(data->inner_struct[2].member_array[2])
        : "memory"
    );
    
    /* Use all registers in address computations */
    asm volatile (
        ""
        : "=m"(data->base_array[r1 + r2]),
          "=m"(data->base_array[r2 + r3]),
          "=m"(data->base_array[r3 + r1])
        : "r"(r1), "r"(r2), "r"(r3)
        : "memory"
    );
}

/* Main driver function */
int main() {
    /* Allocate and initialize test data */
    struct outer* data = (struct outer*)malloc(sizeof(struct outer));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            data->inner_struct[i].member_array[j] = i * 100 + j;
        }
        data->inner_struct[i].volatile_ptr = &data->base_array[i];
    }
    
    for (int i = 0; i < 32; i++) {
        data->base_array[i] = i * 10;
    }
    
    data->index = 0;
    global_struct_ptr = data;
    
    /* Run tests with various parameters to trigger different reload types */
    int checksum = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 2; k++) {
                /* Update volatile indices to prevent optimization */
                global_index1 = i;
                global_index2 = j;
                
                /* Call test functions */
                test_input_address(data, i, j, k);
                test_output_address(data, i, j, k);
                test_operand_address(data, i, j);
                test_mixed_address_reloads(data, i, j, k);
                test_other_address(data, i, j);
                test_other_reload(data, i, j, k);
                
                /* Compute checksum to ensure code isn't optimized away */
                checksum += data->inner_struct[i].member_array[j] + 
                           data->base_array[(i << 2) + j];
            }
        }
    }
    
    printf("Test completed. Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(data);
    
    return 0;
}
