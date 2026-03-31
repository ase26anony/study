/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates code that
   compiles to RTL containing ZERO_EXTRACT, STRICT_LOW_PART, SUBREG,
   and complex MEM expressions. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global/volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_bit_source = 0xDEADBEEF;
volatile unsigned char g_byte_val = 0x42;

/* ===== ZERO_EXTRACT patterns ===== */
/* Bit-field extraction using shift/mask - may generate ZERO_EXTRACT */
unsigned int extract_bits_shiftmask(volatile unsigned int *p) {
    /* Complex enough to avoid simplification */
    return (*p >> 8) & 0xFF;
}

/* Bit-field struct - taking address may create ZERO_EXTRACT */
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
};

unsigned int extract_bitfield(struct BitFieldStruct *bfs) {
    /* Taking address of bit-field member */
    unsigned int val = bfs->mid8;
    return val * 2;
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Writing only low part of a variable */
void set_low_byte_direct(volatile unsigned int *p, unsigned char v) {
    *p = (*p & ~0xFF) | v;  /* Only affects low 8 bits */
}

/* Cast to smaller type assignment - may create STRICT_LOW_PART */
void set_low_part_cast(int32_t *x, int16_t v) {
    *(int16_t*)x = v;  /* Only writes low 16 bits */
}

/* ===== SUBREG patterns ===== */
/* Union for type-punning - creates SUBREG when accessing parts */
union TypePun {
    int32_t i32;
    int16_t i16[2];
    int8_t  i8[4];
};

int32_t subreg_via_union(union TypePun *up) {
    up->i16[1] = 0x1234;  /* Access part of larger register */
    return up->i32;
}

/* Pointer cast between different sizes */
int32_t subreg_via_cast(int64_t *ll) {
    int32_t val = *(int32_t*)ll;  /* Access part of 64-bit value */
    return val;
}

/* ===== Complex MEM patterns ===== */
/* Struct with array - complex addressing */
struct ArrayStruct {
    int arr[100];
    int pad;
};

int mem_complex_index(struct ArrayStruct *as, int i, int j) {
    /* Non-trivial addressing: base + (i + j*4) * sizeof(int) */
    return as->arr[i + j * 4];
}

/* Multi-dimensional array access */
int mem_multi_dim(int matrix[10][10], int i, int j) {
    /* Address calculation: base + (i*10 + j) * sizeof(int) */
    return matrix[i][j];
}

/* Pointer arithmetic with multiple terms */
int mem_pointer_arithmetic(int *base, int offset1, int offset2) {
    /* Complex address: base + offset1 + offset2*3 */
    return *(base + offset1 + offset2 * 3);
}

/* ===== Combined function with control flow ===== */
/* This function combines multiple patterns in a single basic block
   with complex control flow to increase RTL traversal depth */
int combined_operations(volatile int flag) {
    int result = 0;
    union TypePun u = {0};
    struct BitFieldStruct bfs = {0};
    struct ArrayStruct as = {{0}};
    int local_array[50] = {0};
    
    /* Initialize with some values */
    for (int i = 0; i < 50; i++) {
        local_array[i] = i * 3;
    }
    for (int i = 0; i < 100; i++) {
        as.arr[i] = i * 2;
    }
    
    /* Complex control flow based on volatile flag */
    if (flag & 0x1) {
        /* ZERO_EXTRACT pattern */
        result += extract_bits_shiftmask(&g_bit_source);
        result += extract_bitfield(&bfs);
    }
    
    if (flag & 0x2) {
        /* STRICT_LOW_PART patterns */
        set_low_byte_direct((volatile unsigned int*)&result, g_byte_val);
        set_low_part_cast(&result, 0xABCD);
    }
    
    if (flag & 0x4) {
        /* SUBREG patterns */
        result += subreg_via_union(&u);
        result += subreg_via_cast((int64_t*)&g_bit_source);
    }
    
    if (flag & 0x8) {
        /* Complex MEM patterns */
        result += mem_complex_index(&as, 5, 3);
        result += mem_pointer_arithmetic(local_array, 10, 5);
    }
    
    /* Loop with mixed operations */
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0) {
            /* Alternate between patterns in loop */
            result += extract_bits_shiftmask(&g_bit_source);
        } else if (i % 3 == 1) {
            u.i16[i % 2] = i;
            result += u.i32;
        } else {
            result += as.arr[i * 7 % 100];
        }
    }
    
    return result;
}

/* ===== Helper functions emphasizing specific patterns ===== */
int helper_zero_extract(void) {
    struct BitFieldStruct bfs = {0xAA, 0xBB, 0xCCDD};
    unsigned int sum = 0;
    
    /* Multiple bit-field extractions */
    sum += extract_bitfield(&bfs);
    sum += extract_bits_shiftmask(&g_bit_source);
    
    /* Nested extractions */
    for (int i = 0; i < 4; i++) {
        sum += (g_bit_source >> (i * 8)) & 0xFF;
    }
    
    return sum;
}

int helper_strict_low_part(void) {
    volatile unsigned int target = 0x12345678;
    int32_t data = 0x87654321;
    
    /* Multiple low-part writes */
    set_low_byte_direct(&target, 0x99);
    set_low_part_cast(&data, 0x2468);
    
    /* Chain of assignments */
    unsigned char *p = (unsigned char*)&data;
    for (int i = 0; i < 4; i++) {
        p[i] = i * 0x11;  /* Individual byte writes */
    }
    
    return target + data;
}

int helper_subreg(void) {
    union TypePun u;
    u.i32 = 0xDEADBEEF;
    int64_t big_val = 0x0123456789ABCDEFLL;
    
    /* Multiple subreg accesses */
    int32_t part1 = subreg_via_cast(&big_val);
    int32_t part2 = subreg_via_union(&u);
    
    /* Mixed-size operations */
    u.i16[0] = part1 & 0xFFFF;
    u.i8[3] = part2 >> 24;
    
    return u.i32 + part1 + part2;
}

int helper_mem_complex(void) {
    struct ArrayStruct as;
    int matrix[10][10];
    int *dynamic_array = malloc(100 * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        as.arr[i] = i * 3;
    }
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    for (int i = 0; i < 100; i++) {
        dynamic_array[i] = i * 2;
    }
    
    /* Various complex memory accesses */
    int sum = 0;
    sum += mem_complex_index(&as, 25, 6);
    sum += mem_multi_dim(matrix, 3, 7);
    sum += mem_pointer_arithmetic(dynamic_array, 15, 8);
    
    /* Nested addressing */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            sum += as.arr[i * 15 + j * 3];
        }
    }
    
    free(dynamic_array);
    return sum;
}

/* ===== Main function ===== */
int main(void) {
    int final_result = 0;
    
    printf("Starting pattern generation...\n");
    
    /* Call all helper functions in sequence */
    final_result += helper_zero_extract();
    printf("After zero_extract: %d\n", final_result);
    
    final_result += helper_strict_low_part();
    printf("After strict_low_part: %d\n", final_result);
    
    final_result += helper_subreg();
    printf("After subreg: %d\n", final_result);
    
    final_result += helper_mem_complex();
    printf("After mem_complex: %d\n", final_result);
    
    /* Main combined function with volatile control */
    for (int i = 0; i < 5; i++) {
        g_volatile_flag = i;  /* Change volatile each iteration */
        final_result += combined_operations(g_volatile_flag);
        printf("Iteration %d: %d\n", i, final_result);
    }
    
    /* Final computation using all patterns */
    struct BitFieldStruct final_bfs = {0};
    union TypePun final_u = {0};
    
    final_bfs.low8 = final_result & 0xFF;
    final_bfs.mid8 = (final_result >> 8) & 0xFF;
    final_bfs.high16 = (final_result >> 16) & 0xFFFF;
    
    final_u.i32 = final_result;
    set_low_part_cast(&final_u.i32, 0x1234);
    
    /* One last complex memory access */
    struct ArrayStruct final_as;
    for (int i = 0; i < 100; i++) {
        final_as.arr[i] = i + final_result;
    }
    final_result += mem_complex_index(&final_as, 33, 11);
    
    printf("Final result: %d (0x%08X)\n", final_result, final_result);
    
    return final_result != 0 ? 0 : 1;
}
