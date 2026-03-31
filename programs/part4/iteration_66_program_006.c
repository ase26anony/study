/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test_reload
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex nested structures to create addressing complexity */
struct Inner {
    int data[8];
    int offset;
};

struct Middle {
    struct Inner inner[4];
    long base;
};

struct Outer {
    struct Middle middle[3];
    int indices[5];
    volatile int *volatile_ptr;
};

/* Global volatile variables to prevent optimization */
volatile int global_index = 0;
volatile int global_offset = 0;

/* Function to force address computation before call (RELOAD_FOR_OPERAND_ADDRESS) */
void __attribute__((noinline)) use_complex_address(struct Inner *addr) {
    asm volatile("" : : "r"(addr) : "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
int __attribute__((noinline)) test_input_address(struct Outer *outer, int idx1, int idx2, int idx3) {
    int result = 0;
    
    /* Complex addressing that requires input address reload */
    for (int i = 0; i < 4; i++) {
        /* Multiple indices in address computation */
        int val = outer->middle[idx1].inner[idx2].data[idx3 + i];
        
        /* Inline asm with memory input constraint and complex address */
        asm volatile(
            "addl %%eax, %0\n\t"
            : "+m"(outer->middle[idx1].inner[idx2].data[idx3 + i])
            : "a"(val)
            : "memory"
        );
        
        result += outer->middle[idx1].inner[idx2].data[idx3 + i];
    }
    
    return result;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
int __attribute__((noinline)) test_output_address(struct Outer *outer, int base_idx, int offset) {
    int sum = 0;
    
    /* Complex output addressing */
    for (int i = 0; i < 3; i++) {
        int idx = (base_idx + i) & 3;
        
        /* Inline asm with memory output constraint and computed address */
        asm volatile(
            "movl $0x%0, %1\n\t"
            : "=m"(outer->middle[i].inner[idx].data[offset])
            : "i"(i * 100 + 42)
            : "memory"
        );
        
        sum += outer->middle[i].inner[idx].data[offset];
    }
    
    return sum;
}

/* Test RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS together */
int __attribute__((noinline)) test_mixed_address(struct Outer *outer, int start, int count) {
    int total = 0;
    
    for (int i = 0; i < count; i++) {
        int input_idx = (start + i) % 3;
        int output_idx = (start + i + 1) % 3;
        int data_idx = (i * 2) % 8;
        
        /* Read from complex address (input) */
        int input_val = outer->middle[input_idx].inner[data_idx % 4].data[data_idx];
        
        /* Write to different complex address (output) */
        asm volatile(
            "movl %%eax, %0\n\t"
            : "=m"(outer->middle[output_idx].inner[(data_idx + 1) % 4].data[(data_idx + 2) % 8])
            : "a"(input_val * 2)
            : "memory"
        );
        
        total += input_val;
    }
    
    return total;
}

/* Test RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
void __attribute__((noinline)) test_operand_address(struct Outer *outer, int idx) {
    /* Force address computation for function argument */
    use_complex_address(&outer->middle[idx % 3].inner[(idx + 1) % 4]);
    
    /* Multiple address computations in sequence */
    use_complex_address(&outer->middle[(idx + 1) % 3].inner[idx % 4]);
    
    /* Address computation with shift */
    int shifted_idx = idx << 2;
    use_complex_address(&outer->middle[shifted_idx % 3].inner[(shifted_idx + 1) % 4]);
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
int __attribute__((noinline)) test_addr_of_addr(struct Outer *outer, volatile int *indices) {
    int result = 0;
    
    /* Create address-of-address scenarios */
    for (int i = 0; i < 4; i++) {
        int *addr1 = &outer->middle[i % 3].inner[indices[i] % 4].data[0];
        int *addr2 = &outer->middle[(i + 1) % 3].inner[indices[i + 1] % 4].data[4];
        
        /* Operations on addresses */
        asm volatile(
            "movl (%1), %%eax\n\t"
            "addl %%eax, (%2)\n\t"
            : : "r"(addr1), "r"(addr2)
            : "eax", "memory"
        );
        
        result += *addr1 + *addr2;
    }
    
    return result;
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
int __attribute__((noinline)) test_other_address(struct Outer *outer, int *dynamic_indices) {
    int sum = 0;
    
    /* Unpredictable addressing patterns */
    for (int i = 0; i < 5; i++) {
        int idx = dynamic_indices[i];
        
        /* Complex address with multiple components */
        int val = outer->middle[idx % 3].inner[(idx * 2) % 4].data[(idx * 3) % 8];
        
        /* Memory barrier to force reloads */
        asm volatile("" : : : "memory");
        
        /* Modify using inline asm with multiple constraints */
        asm volatile(
            "imull $0x%0, %%eax\n\t"
            : "+a"(val)
            : "m"(outer->middle[(idx + 1) % 3].inner[(idx * 2 + 1) % 4].offset)
            : "memory"
        );
        
        sum += val;
    }
    
    return sum;
}

/* Main driver that exercises all test functions */
int main(void) {
    /* Initialize test data structure */
    struct Outer outer;
    int indices[10];
    int dynamic_indices[5];
    
    /* Initialize with pattern */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                outer.middle[i].inner[j].data[k] = i * 100 + j * 10 + k;
            }
            outer.middle[i].inner[j].offset = j * 5;
        }
        outer.middle[i].base = i * 1000L;
    }
    
    for (int i = 0; i < 10; i++) {
        indices[i] = (i * 7) % 4;
        if (i < 5) dynamic_indices[i] = (i * 11) % 3;
    }
    
    /* Call test functions to trigger different reload types */
    int checksum = 0;
    
    checksum += test_input_address(&outer, 1, 2, 3);
    checksum += test_output_address(&outer, 2, 4);
    checksum += test_mixed_address(&outer, 0, 3);
    
    test_operand_address(&outer, 2);
    
    checksum += test_addr_of_addr(&outer, indices);
    checksum += test_other_address(&outer, dynamic_indices);
    
    /* Additional complex scenario combining everything */
    for (int iter = 0; iter < 2; iter++) {
        for (int i = 0; i < 2; i++) {
            int idx = (iter * 3 + i) % 3;
            
            /* Nested addressing with inline asm */
            asm volatile(
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                "movl %2, %%ebx\n\t"
                "subl %%ebx, %3\n\t"
                : "+m"(outer.middle[idx].inner[0].data[0]),
                  "+m"(outer.middle[(idx + 1) % 3].inner[1].data[1])
                : "m"(outer.middle[(idx + 2) % 3].inner[2].data[2]),
                  "m"(outer.middle[idx].inner[3].data[3])
                : "eax", "ebx", "memory"
            );
        }
    }
    
    /* Compute final checksum from structure */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                checksum += outer.middle[i].inner[j].data[k];
            }
            checksum += outer.middle[i].inner[j].offset;
        }
        checksum += (int)outer.middle[i].base;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
