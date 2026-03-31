/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Complex data structures to force address computations */
struct Inner {
    int data[8];
    int extra;
};

struct Outer {
    struct Inner arrays[4];
    int base;
    volatile int* volatile_ptr;
};

/* Global variables to prevent optimization */
volatile int global_index = 0;
struct Outer global_struct;

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void __attribute__((noinline)) 
use_complex_address(struct Inner* ptr) {
    asm volatile("" : : "r"(ptr) : "memory");
}

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
int __attribute__((noinline))
test_input_address(struct Outer* outer, int idx1, int idx2, int idx3) {
    int result = 0;
    
    /* Complex addressing: outer->arrays[idx1].data[idx2 + idx3] */
    /* This forces address computation with multiple registers */
    asm volatile(
        "movl (%[addr]), %[res]\n\t"
        : [res] "=r"(result)
        : [addr] "r"(&outer->arrays[idx1].data[idx2 + idx3])
        : "memory"
    );
    
    return result;
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
void __attribute__((noinline))
test_output_address(struct Outer* outer, int idx1, int idx2, int value) {
    /* Complex output addressing */
    asm volatile(
        "movl %[val], (%[addr])\n\t"
        : 
        : [val] "r"(value), 
          [addr] "r"(&outer->arrays[idx1].data[idx2 << 1])
        : "memory"
    );
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void __attribute__((noinline))
test_mixed_addresses(struct Outer* outer, int* input, int* output, 
                     int idx, int scale) {
    /* Mixed input/output with complex addressing */
    int temp;
    
    /* Input with complex address computation */
    asm volatile(
        "movl (%[in_addr]), %[tmp]\n\t"
        : [tmp] "=r"(temp)
        : [in_addr] "r"(&outer->arrays[idx].data[(*input) * scale])
        : "memory"
    );
    
    /* Output with different complex address */
    asm volatile(
        "movl %[tmp], (%[out_addr])\n\t"
        : 
        : [tmp] "r"(temp + 1),
          [out_addr] "r"(&outer->arrays[idx + 1].data[(*output) >> 2])
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OPADDR_ADDR */
void __attribute__((noinline))
test_operand_address_chaining(int* base, int offset1, int offset2) {
    /* Nested address computation */
    int* addr1 = base + offset1;
    int* addr2 = addr1 + (offset2 << 2);
    
    /* Use both addresses in inline asm */
    asm volatile(
        "movl (%[a1]), %%eax\n\t"
        "addl %%eax, (%[a2])\n\t"
        : 
        : [a1] "r"(addr1), [a2] "r"(addr2)
        : "eax", "memory"
    );
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS */
int __attribute__((noinline))
test_other_address(struct Outer* outer, volatile int* indices, int count) {
    int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Volatile index forces reload each iteration */
        int idx = indices[i];
        
        /* Complex addressing in loop */
        sum += outer->arrays[idx % 4].data[(idx * 3) % 8];
        
        /* Inline asm with memory clobber to force spills */
        asm volatile("" : : "r"(idx), "m"(*outer) : "memory");
    }
    
    return sum;
}

/* Function to force RELOAD_OTHER type */
void __attribute__((noinline))
test_other_reloads(struct Outer* outer, int* results, int n) {
    /* Multiple memory operations with register pressure */
    register int r1 asm("r10");
    register int r2 asm("r11");
    register int r3 asm("r12");
    
    for (int i = 0; i < n; i++) {
        r1 = outer->arrays[i % 4].data[0];
        r2 = outer->arrays[(i + 1) % 4].data[1];
        r3 = outer->arrays[(i + 2) % 4].data[2];
        
        /* Complex asm with multiple constraints */
        asm volatile(
            "imull %[v1], %[v2]\n\t"
            "addl %[v2], %[v3]\n\t"
            "movl %[v3], (%[dst])\n\t"
            : 
            : [v1] "r"(r1), [v2] "r"(r2), [v3] "r"(r3),
              [dst] "r"(&results[i])
            : "memory"
        );
    }
}

/* Main driver function */
int main() {
    /* Initialize test data */
    struct Outer test_struct;
    int indices[10];
    int results[20];
    int input_val = 5, output_val = 10;
    
    /* Initialize structure */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            test_struct.arrays[i].data[j] = i * 10 + j;
        }
        test_struct.arrays[i].extra = i * 100;
    }
    
    test_struct.base = 1000;
    test_struct.volatile_ptr = &global_index;
    
    /* Initialize indices */
    for (int i = 0; i < 10; i++) {
        indices[i] = (i * 7) % 4;
    }
    
    /* Call test functions to trigger different reload types */
    
    /* 1. RELOAD_FOR_INPUT_ADDRESS */
    int sum1 = test_input_address(&test_struct, 1, 2, 3);
    
    /* 2. RELOAD_FOR_OUTPUT_ADDRESS */
    test_output_address(&test_struct, 2, 1, 999);
    
    /* 3. Mixed addresses for INPADDR/OUTADDR */
    int in_val = 2, out_val = 3;
    test_mixed_addresses(&test_struct, &in_val, &out_val, 0, 4);
    
    /* 4. RELOAD_FOR_OPERAND_ADDRESS */
    use_complex_address(&test_struct.arrays[2]);
    
    /* 5. RELOAD_FOR_OPADDR_ADDR */
    int base_array[100];
    for (int i = 0; i < 100; i++) base_array[i] = i;
    test_operand_address_chaining(base_array, 10, 5);
    
    /* 6. RELOAD_FOR_OTHER_ADDRESS */
    volatile int volatile_indices[5] = {1, 3, 2, 0, 1};
    int sum2 = test_other_address(&test_struct, volatile_indices, 5);
    
    /* 7. RELOAD_OTHER */
    test_other_reloads(&test_struct, results, 10);
    
    /* Compute checksum to ensure all code runs */
    int checksum = sum1 + sum2;
    for (int i = 0; i < 10; i++) {
        checksum += results[i];
        checksum += test_struct.arrays[i % 4].data[i % 8];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed - reload patterns generated at compile time\n");
    
    return 0;
}
