/* test_resource_tracking.c
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking pass during optimization, specifically targeting
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global arrays and structs for memory operand patterns */
int g_array[256];
unsigned long long g_large_buffer[128];

/* Struct with bit-fields for ZERO_EXTRACT */
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
} g_bitfield;

/* Union for SUBREG patterns */
union MixedSizeUnion {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
} g_union;

/* ==================== ZERO_EXTRACT Patterns ==================== */

/* Function 1: Bit-field extraction using struct member access
 * Should generate ZERO_EXTRACT when accessing specific bit ranges */
unsigned int extract_from_bitfield(struct BitFieldStruct *bf) {
    /* Multiple extractions to increase chances */
    unsigned int val1 = bf->low8;        /* Likely ZERO_EXTRACT */
    unsigned int val2 = bf->mid8;        /* Likely ZERO_EXTRACT */
    unsigned int val3 = bf->high16;      /* Likely ZERO_EXTRACT */
    
    /* Combine with volatile to prevent optimization */
    if (g_volatile_flag) {
        return val1 + val2 + val3;
    }
    return val1;
}

/* Function 2: Manual bit extraction using shifts and masks
 * Should generate ZERO_EXTRACT in RTL */
unsigned int extract_bits_manual(volatile unsigned int *p) {
    /* Multiple extraction patterns */
    unsigned int val = *p;
    unsigned int bits8_15 = (val >> 8) & 0xFF;      /* Potential ZERO_EXTRACT */
    unsigned int bits16_23 = (val >> 16) & 0xFF;    /* Potential ZERO_EXTRACT */
    unsigned int bits0_7 = val & 0xFF;              /* Potential ZERO_EXTRACT */
    
    return bits8_15 + bits16_23 + bits0_7;
}

/* ==================== STRICT_LOW_PART Patterns ==================== */

/* Function 3: Writing to low part of a larger variable
 * Should generate STRICT_LOW_PART when writing partial values */
void write_low_part(volatile unsigned int *dest, unsigned char value) {
    /* Multiple low-part write patterns */
    *dest = (*dest & ~0xFF) | value;                /* Potential STRICT_LOW_PART */
    
    /* Another pattern using type punning */
    uint32_t *p32 = (uint32_t *)dest;
    uint16_t *p16 = (uint16_t *)p32;
    p16[0] = value * 2;                             /* Potential STRICT_LOW_PART */
}

/* Function 4: Using smaller types to write to parts of larger types */
void write_with_small_types(void) {
    /* Write to parts of the union */
    g_union.halves[0] = g_volatile_counter & 0xFFFF;  /* Potential STRICT_LOW_PART */
    g_union.bytes[2] = (g_volatile_counter >> 8) & 0xFF; /* Potential STRICT_LOW_PART */
}

/* ==================== SUBREG Patterns ==================== */

/* Function 5: Accessing parts of larger data through smaller views
 * Should generate SUBREG RTL patterns */
int32_t subreg_access_patterns(void) {
    int32_t result = 0;
    
    /* Access through union members */
    result += g_union.halves[0];        /* Likely SUBREG access */
    result += g_union.bytes[1];         /* Likely SUBREG access */
    
    /* Type punning with pointers */
    int64_t large_val = g_volatile_counter * 3LL;
    int32_t *p32 = (int32_t *)&large_val;
    result += p32[0];                   /* Potential SUBREG pattern */
    result += p32[1];                   /* Potential SUBREG pattern */
    
    return result;
}

/* Function 6: Mixed-size operations */
int mixed_size_operations(int32_t a, int16_t b, int8_t c) {
    /* Operations that require mode changes */
    int32_t temp = b;                   /* Potential SUBREG for promotion */
    temp += c;                          /* Potential SUBREG for promotion */
    temp *= a;
    
    /* Cast between different sizes */
    int16_t *p16 = (int16_t *)&a;
    temp += p16[0];                     /* Potential SUBREG pattern */
    temp += p16[1];                     /* Potential SUBREG pattern */
    
    return temp;
}

/* ==================== Complex MEM Patterns ==================== */

/* Function 7: Complex memory addressing modes
 * Should generate MEM with non-trivial addressing */
int complex_memory_access(int *base, int idx1, int idx2, int idx3) {
    /* Multiple addressing modes */
    int val1 = base[idx1 + idx2 * 4];               /* Complex MEM address */
    int val2 = base[(idx1 << 2) + idx3];            /* Complex MEM address */
    int val3 = *(base + idx1 + (idx2 & 0x0F));      /* Complex MEM address */
    
    /* Struct with array access */
    struct {
        int arr[64];
        int padding[16];
    } local_struct;
    
    for (int i = 0; i < 8; i++) {
        local_struct.arr[i * 2] = base[i];          /* Complex MEM with scaling */
    }
    
    return val1 + val2 + val3 + local_struct.arr[4];
}

/* Function 8: Pointer arithmetic with multiple indices */
int pointer_arithmetic_pattern(int *arr, volatile int *offsets) {
    int sum = 0;
    
    /* Complex addressing in loop */
    for (int i = 0; i < 8; i++) {
        /* Multiple complex addressing modes */
        sum += arr[offsets[i] + i * 3];             /* Complex MEM */
        sum += *(arr + (offsets[i] << 1) + 5);      /* Complex MEM */
    }
    
    return sum;
}

/* ==================== Main Orchestration Function ==================== */

/* Function that combines all patterns with control flow */
int combine_patterns(void) {
    int result = 0;
    
    /* Initialize data */
    g_bitfield.low8 = 0xAB;
    g_bitfield.mid8 = 0xCD;
    g_bitfield.high16 = 0x1234;
    
    g_union.full = 0;
    
    /* Use volatile to create unpredictable control flow */
    for (int i = 0; i < (g_volatile_flag & 0x3); i++) {
        /* ZERO_EXTRACT patterns */
        result += extract_from_bitfield(&g_bitfield);
        
        volatile unsigned int extract_src = 0x89ABCDEF;
        result += extract_bits_manual(&extract_src);
        
        /* STRICT_LOW_PART patterns */
        volatile unsigned int write_target = 0xFFFFFFFF;
        write_low_part(&write_target, 0x55);
        result += write_target & 0xFF;
        
        write_with_small_types();
        result += g_union.full & 0xFFFF;
        
        /* SUBREG patterns */
        result += subreg_access_patterns();
        result += mixed_size_operations(result, 0x1234, 0x56);
        
        /* Complex MEM patterns */
        int offsets[] = {1, 3, 5, 7, 9, 11, 13, 15};
        volatile int *v_offsets = offsets;
        result += complex_memory_access(g_array, i, i*2, i*3);
        result += pointer_arithmetic_pattern(g_array, v_offsets);
        
        g_volatile_counter++;
    }
    
    return result;
}

/* ==================== Main Function ==================== */

int main(void) {
    /* Initialize global array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    /* Initialize large buffer */
    for (int i = 0; i < 128; i++) {
        g_large_buffer[i] = (unsigned long long)i << 32 | i;
    }
    
    /* Run the pattern combination multiple times */
    int final_result = 0;
    
    for (int iteration = 0; iteration < 3; iteration++) {
        g_volatile_flag = iteration + 1;
        final_result += combine_patterns();
        
        /* Modify some data between iterations */
        g_bitfield.low8 = (g_bitfield.low8 + 0x11) & 0xFF;
        g_union.full = final_result;
    }
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", final_result);
    
    /* Additional volatile operations to ensure all code paths are considered */
    if (g_volatile_counter > 0) {
        volatile int check = final_result;
        return check % 256;
    }
    
    return 0;
}
