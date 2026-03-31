/* This program is designed to trigger specific RTL patterns in GCC's resource
   tracking pass (resource.cc lines 282-290). It creates operations that should
   generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions
   in the RTL representation during optimization passes. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global variables for memory operations */
volatile unsigned int global_bitfield = 0xABCD1234;
int global_array[256];
struct ComplexStruct {
    int32_t full;
    int16_t parts[4];
    unsigned int bits : 8;
    unsigned int more_bits : 8;
} cs;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Pattern 1: Bit-field extraction using shift and mask */
int extract_bits_shift(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT for the bit range */
    return (*p >> 8) & 0xFF;  /* Extract bits 8-15 */
}

/* Pattern 2: Bit-field struct member access */
unsigned int extract_bitfield_member(struct ComplexStruct *s) {
    /* Accessing bit-field members often creates ZERO_EXTRACT */
    unsigned int result = s->bits;
    result += s->more_bits;
    return result;
}

/* Pattern 3: Multiple extractions in a loop */
int extract_multiple_ranges(volatile unsigned int *p, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* Different extraction ranges to encourage ZERO_EXTRACT */
        sum += (*p >> (i * 4)) & 0xF;
        sum += (*p >> 16) & 0xFFFF;
    }
    return sum;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Pattern 1: Writing to low part of a larger variable */
void set_low_byte_direct(volatile unsigned int *p, unsigned char v) {
    /* This pattern may generate STRICT_LOW_PART for byte write */
    *p = (*p & ~0xFF) | v;
}

/* Pattern 2: Cast and assignment to partial type */
int32_t set_low_part_via_cast(int32_t *dest, int16_t value) {
    /* Writing 16-bit value to 32-bit location */
    *(int16_t*)dest = value;
    return *dest;
}

/* Pattern 3: Inline assembly simulating low-part write (x86 specific) */
#ifdef __x86_64__
void set_low_part_asm(volatile uint32_t *p, uint16_t v) {
    /* Assembly that writes only to lower 16 bits */
    asm volatile (
        "movw %w1, %0\n\t"
        : "=m" (*p)
        : "r" (v)
        : "memory"
    );
}
#endif

/* ==================== SUBREG patterns ==================== */

/* Pattern 1: Union for type aliasing */
union TypeAlias {
    int64_t full;
    int32_t halves[2];
    int16_t quarters[4];
};

int32_t subreg_via_union(union TypeAlias *u, int index) {
    /* Accessing smaller parts of larger type */
    u->quarters[index] = 0x1234;
    return u->halves[0];
}

/* Pattern 2: Pointer casting between different sizes */
int32_t subreg_via_cast(int64_t *src) {
    /* Casting pointer to access part of larger type */
    int32_t result = *(int32_t*)src;
    *(int16_t*)((char*)src + 2) = 0x5678;
    return result;
}

/* Pattern 3: Array access with mixed types */
int16_t subreg_array_access(int64_t *array, int index) {
    /* Accessing 16-bit portion of 64-bit element */
    return ((int16_t*)array)[index * 4 + 1];
}

/* ==================== Complex MEM patterns ==================== */

/* Pattern 1: Complex addressing with multiple indices */
int mem_complex_addressing(int *base, int idx1, int idx2, int idx3) {
    /* Non-trivial address calculation */
    return base[(idx1 * idx2) + (idx3 << 2) + 7];
}

/* Pattern 2: Struct with array and computed offset */
int mem_struct_array(struct ComplexStruct *arr, int i, int j) {
    /* Multiple levels of indirection */
    return arr[i].parts[j] + arr[j].full;
}

/* Pattern 3: Pointer arithmetic in loop */
int mem_pointer_arithmetic(int *data, int size) {
    int sum = 0;
    int *end = data + size;
    while (data < end) {
        sum += *data;
        data += 2;  /* Skip every other element */
    }
    return sum;
}

/* Pattern 4: Multi-dimensional array access */
int mem_multi_dim(int matrix[10][10], int i, int j) {
    /* This creates non-simple memory addresses */
    return matrix[i][j] + matrix[j][i];
}

/* ==================== Combined function ==================== */

/* Function that combines multiple patterns in complex control flow */
int combined_operations(int seed) {
    int result = seed;
    union TypeAlias u;
    u.full = seed;
    
    /* Volatile condition to create complex CFG */
    if (v_flag1) {
        /* ZERO_EXTRACT pattern */
        result ^= extract_bits_shift(&global_bitfield);
        
        /* STRICT_LOW_PART pattern */
        set_low_byte_direct((volatile unsigned int*)&result, 
                           (unsigned char)(result & 0xFF));
    }
    
    /* Loop with volatile condition */
    while (v_counter < 3) {
        /* SUBREG pattern */
        result += subreg_via_union(&u, v_counter % 4);
        
        /* Complex MEM pattern */
        result += mem_complex_addressing(global_array, 
                                        result & 0xF, 
                                        (result >> 4) & 0xF,
                                        v_counter);
        
        v_counter++;
    }
    
    /* Another volatile condition */
    if (v_flag2) {
        /* More MEM patterns */
        result += mem_pointer_arithmetic(global_array, 64);
    }
    
    return result;
}

/* ==================== Main function ==================== */

int main(void) {
    int final_result = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    cs.full = 0xDEADBEEF;
    cs.bits = 0xAB;
    cs.more_bits = 0xCD;
    for (int i = 0; i < 4; i++) {
        cs.parts[i] = i * 0x1111;
    }
    
    /* Execute combined operations multiple times */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Change volatile flags to vary control flow */
        v_flag1 = iteration % 3;
        v_flag2 = iteration % 5;
        v_counter = iteration % 4;
        
        /* Call functions that generate target RTL patterns */
        
        /* 1. ZERO_EXTRACT patterns */
        final_result += extract_bits_shift(&global_bitfield);
        final_result += extract_bitfield_member(&cs);
        final_result += extract_multiple_ranges(&global_bitfield, 4);
        
        /* 2. STRICT_LOW_PART patterns */
        set_low_byte_direct(&global_bitfield, (unsigned char)iteration);
        final_result += set_low_part_via_cast((int32_t*)&global_bitfield, 
                                             (int16_t)iteration);
        #ifdef __x86_64__
        set_low_part_asm((volatile uint32_t*)&global_bitfield, 
                        (uint16_t)iteration);
        #endif
        
        /* 3. SUBREG patterns */
        union TypeAlias u;
        u.full = final_result;
        final_result += subreg_via_union(&u, iteration % 4);
        final_result += subreg_via_cast((int64_t*)&global_array[iteration]);
        final_result += subreg_array_access((int64_t*)global_array, iteration);
        
        /* 4. Complex MEM patterns */
        final_result += mem_complex_addressing(global_array, 
                                              iteration, 
                                              iteration * 2, 
                                              iteration + 1);
        final_result += mem_struct_array(&cs, iteration % 2, (iteration + 1) % 2);
        final_result += mem_pointer_arithmetic(global_array, 128);
        
        int matrix[10][10];
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                matrix[i][j] = i * 10 + j;
            }
        }
        final_result += mem_multi_dim(matrix, iteration % 10, (iteration * 7) % 10);
        
        /* 5. Combined operations */
        final_result += combined_operations(iteration);
        
        /* Modify global_bitfield to create new patterns */
        global_bitfield ^= (final_result << 16) | (final_result & 0xFFFF);
    }
    
    /* Use result to prevent optimization */
    printf("Final result: %d\n", final_result);
    
    /* Additional printf to ensure all code paths are considered live */
    printf("Bitfield value: 0x%08X\n", global_bitfield);
    printf("Struct bits: %u, more_bits: %u\n", cs.bits, cs.more_bits);
    
    return final_result != 0 ? 0 : 1;
}
