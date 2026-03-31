/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 *
 * Compilation recommendations:
 *   - For general RTL expansion: gcc -O1 -fdump-rtl-all test_resource_coverage.c
 *   - For scheduling passes: gcc -O2 -fschedule-insns test_resource_coverage.c
 *   - For BPF target: gcc -target bpf -O2 -fdump-rtl-all test_resource_coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t field1 : 5;   /* Volatile to prevent optimization */
    volatile uint32_t field2 : 12;
    volatile uint32_t field3 : 8;
    volatile uint32_t field4 : 7;
};

/* Global array with volatile elements to force memory operations */
static volatile int global_array[32];

/* Function to create data dependencies and complex addressing */
static int compute_index(int i, int j) {
    /* Complex enough to prevent simple addressing optimization */
    return (i * 7 + j * 3) & 0x1F;
}

int main(void) {
    struct bitfield_struct bf = {0};
    volatile uint32_t accumulator = 0;
    volatile int temp_result = 0;
    
    /* Local array with volatile pointer to encourage complex MEM addresses */
    volatile int local_array[64];
    volatile int *ptr = local_array;
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 2;
    }
    for (int i = 0; i < 64; i++) {
        local_array[i] = i;
    }
    
    /* Main loop to generate the target RTL patterns */
    for (int i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may generate SUBREG in RTL */
        uint32_t base = i * 3 + 7;
        
        /* 2. Bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf.field1 = (base >> 0) & 0x1F;      /* 5 bits */
        bf.field2 = (base >> 5) & 0xFFF;     /* 12 bits */
        bf.field3 = (base >> 17) & 0xFF;     /* 8 bits */
        
        /* 3. Mixed bitwise operations on the same location */
        /* This creates a pattern where bits are extracted and modified */
        bf.field4 ^= (i & 0x7F);             /* 7 bits - XOR assignment */
        
        /* 4. Complex memory addressing with pointer arithmetic */
        /* This should generate MEM with non-trivial address expressions */
        int idx = compute_index(i, bf.field1);
        
        /* Array access with computed index - complex addressing mode */
        temp_result = global_array[idx];
        
        /* Pointer arithmetic followed by dereference */
        ptr += (bf.field2 & 0x3);           /* Small offset based on bit-field */
        accumulator += *ptr;
        ptr = local_array;                  /* Reset pointer */
        
        /* 5. Additional bit manipulation on memory location */
        /* This may generate ZERO_EXTRACT for bit-field in memory */
        if (i & 1) {
            /* Mask specific bits of a variable */
            uint32_t mask = 0x00FF00FF;
            accumulator = (accumulator & ~mask) | (temp_result & mask);
        }
        
        /* 6. Access structure through volatile pointer */
        volatile struct bitfield_struct *bf_ptr = &bf;
        accumulator += bf_ptr->field3;
    }
    
    /* Use results to prevent optimization */
    printf("Result: %u\n", accumulator);
    
    /* Additional test: direct bit-field manipulation in loop */
    for (int i = 0; i < 50; i++) {
        /* These assignments on volatile bit-fields are strong candidates
         * for generating ZERO_EXTRACT/STRICT_LOW_PART in RTL */
        bf.field1 = i & 0x1F;
        bf.field2 = (bf.field2 + 1) & 0xFFF;
        
        /* Complex memory address: array + bit-field value */
        int complex_idx = (bf.field1 + bf.field2) & 0x1F;
        accumulator += global_array[complex_idx];
    }
    
    printf("Final checksum: %u\n", accumulator);
    return 0;
}
