/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates code that
   should generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex
   MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global/volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;
volatile unsigned int g_bitfield_source = 0xDEADBEEF;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Pattern 1: Bit-field extraction using shift and mask */
unsigned int extract_bits_shift_mask(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT: extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Pattern 2: Bit-field struct member access */
struct BitFieldStruct {
    unsigned int low8: 8;
    unsigned int mid8: 8;
    unsigned int high16: 16;
};

unsigned int extract_bitfield_member(struct BitFieldStruct *bfs) {
    /* Taking address and accessing bit-field may generate ZERO_EXTRACT */
    unsigned int val = bfs->mid8;  /* bits 8-15 */
    return val;
}

/* Pattern 3: More complex bit extraction with variable position */
unsigned int extract_variable_bits(volatile unsigned int *p, int shift) {
    /* Variable shift may prevent optimization into simpler form */
    unsigned int mask = (1 << 8) - 1;  /* 8-bit mask */
    return (*p >> shift) & mask;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Pattern 1: Writing only low byte of a larger integer */
void set_low_byte_direct(volatile unsigned int *p, unsigned char v) {
    /* This may generate STRICT_LOW_PART: only modifying low 8 bits */
    *p = (*p & ~0xFF) | v;
}

/* Pattern 2: Cast and assignment to smaller type */
void set_low_part_via_cast(volatile uint32_t *p, uint16_t v) {
    /* Writing 16-bit value to 32-bit location */
    *(uint16_t*)p = v;
}

/* Pattern 3: Using char pointer to modify part of int */
void set_via_char_ptr(volatile unsigned int *p, unsigned char v) {
    unsigned char *cp = (unsigned char*)p;
    cp[1] = v;  /* Modify second byte only */
}

/* ==================== SUBREG patterns ==================== */

/* Pattern 1: Union for type punning */
union TypePunningUnion {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

int32_t access_via_subreg_union(union TypePunningUnion *u, int idx) {
    /* Accessing part of larger type through smaller type */
    u->halves[idx] = 0x1234;
    return u->full;
}

/* Pattern 2: Pointer casting between different sizes */
int32_t access_via_ptr_cast(volatile int64_t *large) {
    /* Access 64-bit as 32-bit */
    int32_t *small = (int32_t*)large;
    small[1] = 0xABCD;  /* Modify high 32 bits of 64-bit value */
    return *small;
}

/* Pattern 3: Array of different sized elements */
int32_t mixed_size_access(int16_t *arr, int idx) {
    /* Promote 16-bit to 32-bit */
    int32_t val = arr[idx];
    return val * 2;
}

/* ==================== Complex MEM patterns ==================== */

/* Pattern 1: Array access with complex index calculation */
struct ComplexMemStruct {
    int data[256];
    int padding[64];
};

int complex_array_index(struct ComplexMemStruct *s, 
                       volatile int idx1, 
                       volatile int idx2) {
    /* Complex addressing: base + (idx1 + idx2*8) * sizeof(int) */
    return s->data[idx1 + idx2 * 8];
}

/* Pattern 2: Pointer arithmetic with multiple variables */
int *pointer_arithmetic(int *base, volatile int a, volatile int b) {
    /* Complex address calculation */
    return &base[a + b * 16 + 32];
}

/* Pattern 3: Nested struct with array */
struct OuterStruct {
    struct {
        int matrix[10][10];
    } inner;
    int other;
};

int nested_struct_access(struct OuterStruct *os, 
                        volatile int i, 
                        volatile int j) {
    /* Multi-dimensional array access */
    return os->inner.matrix[i][j];
}

/* ==================== Combined patterns ==================== */

/* Function that combines multiple patterns */
unsigned int combined_operations(volatile unsigned int *mem, 
                                union TypePunningUnion *u,
                                struct BitFieldStruct *bfs) {
    unsigned int result = 0;
    
    /* Use volatile flag to create control flow */
    if (g_volatile_flag) {
        /* ZERO_EXTRACT pattern */
        result += extract_bits_shift_mask(mem);
        
        /* STRICT_LOW_PART pattern */
        set_low_byte_direct(mem, 0x42);
        
        /* SUBREG pattern */
        result += access_via_subreg_union(u, g_volatile_counter & 1);
        
        /* Complex MEM pattern */
        struct ComplexMemStruct cms;
        result += complex_array_index(&cms, 
                                     result & 0xF, 
                                     (result >> 4) & 0xF);
    } else {
        /* Alternative path with different patterns */
        result += extract_bitfield_member(bfs);
        set_via_char_ptr(mem, 0x99);
        result += access_via_ptr_cast((int64_t*)mem);
    }
    
    return result;
}

/* ==================== Main test driver ==================== */

int main(void) {
    unsigned int final_result = 0;
    volatile unsigned int memory_area[256];
    union TypePunningUnion pun_union;
    struct BitFieldStruct bitfield_struct;
    struct OuterStruct outer_struct;
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        memory_area[i] = i * 0x01010101;
    }
    
    pun_union.full = 0x12345678;
    bitfield_struct.low8 = 0xAA;
    bitfield_struct.mid8 = 0xBB;
    bitfield_struct.high16 = 0xCCDD;
    
    /* Create complex control flow with loops */
    for (g_volatile_counter = 0; 
         g_volatile_counter < 100; 
         g_volatile_counter++) {
        
        /* Vary the volatile flag */
        g_volatile_flag = (g_volatile_counter % 3) != 0;
        
        /* Test ZERO_EXTRACT patterns */
        final_result ^= extract_bits_shift_mask(&g_bitfield_source);
        final_result += extract_bitfield_member(&bitfield_struct);
        final_result ^= extract_variable_bits(&g_bitfield_source, 
                                             g_volatile_counter & 0x7);
        
        /* Test STRICT_LOW_PART patterns */
        set_low_byte_direct(&memory_area[g_volatile_counter % 256], 
                           final_result & 0xFF);
        set_low_part_via_cast((uint32_t*)&memory_area[64], 
                             final_result & 0xFFFF);
        set_via_char_ptr(&memory_area[128], final_result & 0xFF);
        
        /* Test SUBREG patterns */
        final_result += access_via_subreg_union(&pun_union, 
                                               g_volatile_counter & 1);
        final_result ^= access_via_ptr_cast((int64_t*)&memory_area[0]);
        final_result += mixed_size_access((int16_t*)&memory_area[32], 
                                         g_volatile_counter & 7);
        
        /* Test complex MEM patterns */
        struct ComplexMemStruct cms;
        final_result += complex_array_index(&cms, 
                                           final_result & 0xF, 
                                           (final_result >> 4) & 0xF);
        
        int *ptr = pointer_arithmetic((int*)memory_area, 
                                     final_result & 0x7, 
                                     (final_result >> 3) & 0x7);
        final_result += *ptr;
        
        final_result += nested_struct_access(&outer_struct, 
                                            final_result & 0x3, 
                                            (final_result >> 2) & 0x3);
        
        /* Combined operations */
        if (g_volatile_counter % 5 == 0) {
            final_result += combined_operations(&memory_area[g_volatile_counter % 128], 
                                               &pun_union, 
                                               &bitfield_struct);
        }
    }
    
    /* Ensure all operations have observable effect */
    printf("Final result: 0x%08X\n", final_result);
    
    /* Use result to prevent dead code elimination */
    return (final_result & 0xFF) == 0 ? 0 : 1;
}
