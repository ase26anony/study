/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates operations that
   should generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex
   MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global/volatile variables to prevent optimization */
volatile unsigned int g_counter = 0;
volatile int g_condition = 1;
volatile unsigned int g_bitfield_source = 0xDEADBEEF;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field extraction using shift and mask - may generate ZERO_EXTRACT */
unsigned int extract_bits_shift(volatile unsigned int *p) {
    /* Extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Struct with bit-field - taking address may create ZERO_EXTRACT */
struct BitFieldStruct {
    unsigned int low8:8;
    unsigned int mid8:8;
    unsigned int high16:16;
};

unsigned int extract_bitfield(struct BitFieldStruct *bfs) {
    /* Accessing bit-field members */
    unsigned int val = bfs->mid8;
    val |= (bfs->high16 << 16);
    return val;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Writing only low part of a variable */
void set_low_byte(volatile unsigned int *p, unsigned char v) {
    *p = (*p & ~0xFF) | v;
}

/* Cast and assignment to create partial write */
void write_low_half(int32_t *x) {
    /* Write to low 16 bits */
    *(int16_t*)x = 0x1234;
}

/* ==================== SUBREG patterns ==================== */

/* Union for type-punning - creates SUBREG accesses */
union TypePun {
    int32_t i32;
    int16_t i16[2];
    int8_t i8[4];
};

int32_t subreg_via_union(union TypePun *u) {
    u->i16[0] = 100;
    u->i16[1] = 200;
    return u->i32;
}

/* Mixed-size operations */
int64_t mixed_size_ops(int64_t ll) {
    int32_t i32 = *(int32_t*)&ll;
    int16_t i16 = *(int16_t*)((char*)&ll + 2);
    return i32 + i16;
}

/* ==================== Complex MEM patterns ==================== */

/* Struct with array for complex addressing */
struct ArrayStruct {
    int arr[100];
    int pad;
    int other[50];
};

int complex_mem_access(struct ArrayStruct *as, int i, int j) {
    /* Complex addressing with multiple calculations */
    return as->arr[i * 4 + j * 3] + as->other[i * 2];
}

/* Multi-dimensional array with pointer arithmetic */
int multi_dim_access(int matrix[10][10], int i, int j) {
    /* Address calculation involving multiplication */
    return matrix[i][j] + matrix[j][i];
}

/* ==================== Combined function ==================== */

/* Function that combines multiple patterns in control flow */
unsigned int combined_operations(volatile int flag) {
    unsigned int result = 0;
    static union TypePun u;
    static struct BitFieldStruct bfs = {0xAA, 0xBB, 0xCCDD};
    static struct ArrayStruct as;
    static int matrix[10][10];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        as.arr[i] = i * 2;
    }
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i + j;
        }
    }
    
    /* Use volatile flag to create unpredictable control flow */
    if (flag & 0x1) {
        /* ZERO_EXTRACT pattern */
        result += extract_bits_shift(&g_bitfield_source);
        result += extract_bitfield(&bfs);
    }
    
    if (flag & 0x2) {
        /* STRICT_LOW_PART patterns */
        set_low_byte((volatile unsigned int*)&result, 0x55);
        write_low_half((int32_t*)&result);
    }
    
    if (flag & 0x4) {
        /* SUBREG patterns */
        result += subreg_via_union(&u);
        result += mixed_size_ops(result);
    }
    
    if (flag & 0x8) {
        /* Complex MEM patterns */
        result += complex_mem_access(&as, result % 10, (result >> 8) % 10);
        result += multi_dim_access(matrix, result % 10, (result >> 4) % 10);
    }
    
    return result;
}

/* ==================== Main function ==================== */

int main(void) {
    unsigned int final_result = 0;
    
    /* Loop to increase pass activity */
    for (int i = 0; i < 100; i++) {
        g_counter++;
        g_condition = (g_condition * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call combined function with varying conditions */
        unsigned int iter_result = combined_operations(g_condition);
        
        /* Mix results in non-trivial way */
        final_result ^= (iter_result << (i % 32));
        final_result += iter_result;
        
        /* Modify global to create side effects */
        g_bitfield_source = (g_bitfield_source * 1664525 + 1013904223);
    }
    
    /* Additional individual pattern calls */
    {
        union TypePun main_u;
        struct BitFieldStruct main_bfs = {0x11, 0x22, 0x3344};
        volatile unsigned int test_var = 0x87654321;
        
        /* Ensure all patterns are exercised */
        final_result += extract_bits_shift(&test_var);
        final_result += extract_bitfield(&main_bfs);
        
        set_low_byte(&test_var, final_result & 0xFF);
        write_low_half((int32_t*)&test_var);
        
        final_result += subreg_via_union(&main_u);
        final_result += mixed_size_ops(final_result);
    }
    
    /* Print result to prevent optimization */
    printf("Final result: 0x%08X\n", final_result);
    
    return (final_result != 0) ? 0 : 1;
}
