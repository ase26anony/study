/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test_reload
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

/* Global variables to prevent optimization */
volatile int global_index = 0;
volatile int* volatile global_ptr = NULL;
struct outer_struct global_nested;

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void __attribute__((noinline)) 
use_complex_address(struct inner_struct* addr) {
    asm volatile("" : : "r"(addr) : "memory");
}

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
void __attribute__((noinline))
test_input_address(struct outer_struct* nested, int idx1, int idx2) {
    /* Complex addressing that requires input address reload */
    int val;
    asm volatile(
        "movl (%[addr]), %[val]\n\t"
        : [val] "=r"(val)
        : [addr] "m"(nested->inner[idx1].member_array[idx2])
        : "memory"
    );
    
    /* Another complex input address */
    asm volatile(
        ""
        :
        : "m"(nested->outer_array[(idx1 << 2) + idx2]),
          "m"(nested->inner[idx2].member_array[idx1])
        : "memory"
    );
    
    global_ptr = &val;
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
void __attribute__((noinline))
test_output_address(struct outer_struct* nested, int idx1, int idx2, int value) {
    /* Complex output addressing */
    asm volatile(
        "movl %[val], (%[addr])\n\t"
        : [addr] "=m"(nested->inner[idx1].member_array[idx2])
        : [val] "r"(value)
        : "memory"
    );
    
    /* Multiple output addresses with computation */
    int offset = (idx1 * 3 + idx2) & 0xF;
    asm volatile(
        "movl %[val], (%[addr])\n\t"
        : [addr] "=m"(nested->outer_array[offset])
        : [val] "r"(value + 1)
        : "memory"
    );
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void __attribute__((noinline))
test_mixed_addresses(struct outer_struct* nested, int* results, int count) {
    for (int i = 0; i < count; i++) {
        int idx1 = (i * 7) & 0x3;
        int idx2 = (i * 13) & 0x7;
        
        /* Mixed input/output with complex addressing */
        int temp;
        asm volatile(
            "movl (%[in_addr]), %[temp]\n\t"
            "addl $1, %[temp]\n\t"
            "movl %[temp], (%[out_addr])\n\t"
            : [temp] "=&r"(temp),
              [out_addr] "=m"(results[(idx1 << 3) + idx2])
            : [in_addr] "m"(nested->inner[idx1].member_array[idx2]),
              "m"(nested->outer_array[idx2])
            : "memory"
        );
        
        /* Force address of address reload */
        volatile int** ptr_to_ptr = &nested->inner[idx1].volatile_ptr;
        asm volatile("" : : "m"(ptr_to_ptr), "m"(*ptr_to_ptr) : "memory");
    }
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void __attribute__((noinline))
test_other_addresses(struct outer_struct* nested, int idx) {
    /* Complex address computation in loop */
    for (int i = 0; i < 4; i++) {
        /* Force other address reloads */
        struct inner_struct* current = &nested->inner[(idx + i) & 0x3];
        
        /* Use in inline asm with multiple constraints */
        asm volatile(
            "leal (%[base], %[index], 4), %%eax\n\t"
            "movl (%%eax), %%ebx\n\t"
            : 
            : [base] "r"(current->member_array),
              [index] "r"(i),
              "m"(current->member_array[i])
            : "eax", "ebx", "memory"
        );
        
        /* Force operand address reload through function call */
        use_complex_address(&nested->inner[i]);
    }
}

/* Function with multiple reload types in sequence */
int __attribute__((noinline))
test_comprehensive(struct outer_struct* nested, int* results, int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int idx1 = (i + global_index) & 0x3;
        int idx2 = (i * 3) & 0x7;
        
        /* Test input address reload */
        test_input_address(nested, idx1, idx2);
        
        /* Test output address reload */
        test_output_address(nested, idx1, idx2, i);
        
        /* Get value through complex addressing */
        int val = nested->inner[idx1].member_array[idx2];
        int val2 = nested->outer_array[(idx1 << 2) + idx2];
        
        /* Mixed test */
        test_mixed_addresses(nested, results, 2);
        
        /* Use values to prevent optimization */
        sum += val + val2;
        
        /* Update volatile to force reloads */
        global_index = (global_index + 1) & 0x3;
    }
    
    /* Test other address types */
    test_other_addresses(nested, iterations & 0x3);
    
    return sum;
}

/* Helper to initialize test data */
void init_test_data(struct outer_struct* nested, int* array, int size) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            nested->inner[i].member_array[j] = i * 100 + j;
        }
        nested->inner[i].volatile_ptr = &global_index;
    }
    
    for (int i = 0; i < 16; i++) {
        nested->outer_array[i] = i * 10;
    }
    
    for (int i = 0; i < size; i++) {
        array[i] = 0;
    }
    
    nested->index = 0;
}

int main() {
    /* Allocate on heap to force more complex addressing */
    struct outer_struct* nested = (struct outer_struct*)malloc(sizeof(struct outer_struct));
    int* results = (int*)malloc(100 * sizeof(int));
    
    if (!nested || !results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize test data */
    init_test_data(nested, results, 100);
    
    /* Set global pointer */
    global_ptr = &nested->index;
    global_nested = *nested;
    
    /* Run comprehensive test */
    int sum = test_comprehensive(nested, results, 8);
    
    /* Additional tests targeting specific reload types */
    
    /* Force RELOAD_FOR_OPADDR_ADDR */
    for (int i = 0; i < 4; i++) {
        struct inner_struct* addr = &nested->inner[i];
        /* Complex address of address computation */
        asm volatile(
            ""
            : 
            : "m"(addr->volatile_ptr),
              "r"(&addr->member_array[0])
            : "memory"
        );
    }
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS with function arguments */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            /* Complex address expression as function argument */
            use_complex_address(&nested->inner[(i + j) & 0x3]);
        }
    }
    
    /* Compute checksum */
    int checksum = sum;
    for (int i = 0; i < 100; i++) {
        checksum += results[i];
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += nested->inner[i].member_array[j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(results);
    free(nested);
    
    return 0;
}
