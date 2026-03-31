/* This program is designed to generate specific RTL patterns that will
   exercise the uncovered lines in GCC's resource.cc file (lines 282-290).
   It creates ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM_P
   patterns through various C constructs. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_data = 0xDEADBEEF;

/* Global arrays and structs for memory operand patterns */
int g_array[256];
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
} g_bitfield;

/* Union for SUBREG patterns */
union MixedSizeUnion {
    int64_t full;
    int32_t halves[2];
    int16_t quarters[4];
    int8_t bytes[8];
} g_union;

/* Struct with array for complex memory addressing */
struct ComplexMemStruct {
    int preamble[10];
    int data[100];
    int postamble[10];
} g_cmem;

/* ========== ZERO_EXTRACT patterns ========== */
/* Method 1: Direct bitfield extraction from volatile */
unsigned int zero_extract_volatile(void) {
    /* This should generate ZERO_EXTRACT: extract bits 8-15 */
    return (g_volatile_data >> 8) & 0xFF;
}

/* Method 2: Bitfield struct member access */
unsigned int zero_extract_bitfield(void) {
    /* Taking address and accessing bitfield may generate ZERO_EXTRACT */
    struct BitFieldStruct local = g_bitfield;
    return local.mid8;  /* Access specific bit range */
}

/* Method 3: More complex extraction with arithmetic */
unsigned int zero_extract_complex(volatile unsigned int *p) {
    /* Multiple extractions in one expression */
    unsigned int temp = *p;
    return ((temp >> 4) & 0xF) | ((temp >> 20) & 0xF0);
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Method 1: Writing to low part of larger variable */
void strict_low_part_write32(void) {
    volatile uint32_t *p = (volatile uint32_t*)&g_volatile_data;
    /* Write only to low 16 bits */
    *p = (*p & ~0xFFFF) | 0x1234;
}

/* Method 2: Using smaller type assignment */
void strict_low_part_char(void) {
    volatile uint32_t val = g_volatile_data;
    uint8_t low_byte = 0xAB;
    /* This may generate STRICT_LOW_PART when assigning to part of register */
    val = (val & ~0xFF) | low_byte;
    g_volatile_data = val;
}

/* Method 3: Pointer cast to smaller type */
void strict_low_part_pointer_cast(void) {
    uint32_t x = g_volatile_data;
    /* Direct assignment to part of variable */
    *(uint16_t*)&x = 0x5678;
    g_volatile_data = x;
}

/* ========== SUBREG patterns ========== */
/* Method 1: Union-based type punning */
int32_t subreg_union_access(void) {
    /* Access different parts of the same storage */
    g_union.full = 0x0123456789ABCDEFULL;
    return g_union.halves[g_volatile_flag & 1];  /* Access 32-bit part of 64-bit */
}

/* Method 2: Mixed-size operations */
int64_t subreg_mixed_ops(int32_t a, int16_t b) {
    /* Operations mixing different sizes */
    int64_t result = a;          /* Sign extension may involve SUBREG */
    result += b;                 /* Different size operands */
    return result;
}

/* Method 3: Array access with different types */
void subreg_array_conversion(void) {
    int32_t buffer[4];
    /* Access as different size */
    int16_t *half_ptr = (int16_t*)buffer;
    half_ptr[1] = 0x1234;        /* Write 16-bit to 32-bit aligned location */
}

/* ========== Complex MEM_P patterns ========== */
/* Method 1: Array indexing with complex addressing */
int mem_complex_index(int i, int j) {
    /* Non-trivial address calculation */
    return g_array[(i * 3 + j * 7) & 0xFF];
}

/* Method 2: Struct with offset calculation */
int mem_struct_offset(struct ComplexMemStruct *s, int index) {
    /* Address calculation involving struct field */
    return s->data[index * 2 + 5];
}

/* Method 3: Pointer arithmetic in loop */
int mem_pointer_arithmetic(int *base, int n) {
    int sum = 0;
    int *ptr = base;
    /* Complex addressing in loop */
    for (int i = 0; i < n; i++) {
        sum += ptr[i * 2];       /* Strided access */
    }
    return sum;
}

/* ========== Combined function with control flow ========== */
/* This function combines multiple patterns in a single basic block
   with conditional execution to create complex RTL */
unsigned int combined_patterns(int mode) {
    unsigned int result = 0;
    volatile int flag = g_volatile_flag;
    
    /* ZERO_EXTRACT pattern */
    if (flag & 0x1) {
        result |= zero_extract_volatile();
    }
    
    /* STRICT_LOW_PART pattern */
    if (flag & 0x2) {
        strict_low_part_char();
        result += g_volatile_data & 0xFF;
    }
    
    /* SUBREG pattern */
    if (flag & 0x4) {
        result ^= subreg_union_access();
    }
    
    /* Complex MEM pattern */
    if (flag & 0x8) {
        result += mem_complex_index(result & 0xF, (result >> 4) & 0xF);
    }
    
    /* Mixed operations to prevent optimization */
    switch (mode & 0x3) {
        case 0:
            result = (result >> 8) & 0xFF;  /* Another ZERO_EXTRACT */
            break;
        case 1:
            *(uint16_t*)&result = 0x1234;   /* STRICT_LOW_PART */
            break;
        case 2:
            result = ((int16_t)result) * 2; /* SUBREG from truncation */
            break;
        case 3:
            result = g_array[result % 256]; /* MEM access */
            break;
    }
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    unsigned int final_result = 0;
    
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    g_bitfield.low8 = 0x12;
    g_bitfield.mid8 = 0x34;
    g_bitfield.high16 = 0x5678;
    
    for (int i = 0; i < 100; i++) {
        g_cmem.data[i] = i * 2;
    }
    
    /* Execute pattern functions in loops to increase coverage chance */
    for (int i = 0; i < 100; i++) {
        g_volatile_flag = i;  /* Change control flow each iteration */
        
        /* Call individual pattern functions */
        final_result ^= zero_extract_complex(&g_volatile_data);
        final_result += zero_extract_bitfield();
        
        strict_low_part_write32();
        strict_low_part_pointer_cast();
        
        final_result ^= subreg_mixed_ops(final_result, i);
        subreg_array_conversion();
        
        final_result += mem_struct_offset(&g_cmem, i % 50);
        final_result += mem_pointer_arithmetic(g_array, 10);
        
        /* Combined function with all patterns */
        final_result = combined_patterns(i);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: 0x%08X\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);  /* Ensure non-negative return */
}
