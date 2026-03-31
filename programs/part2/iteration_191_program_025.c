/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates code patterns
   that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex
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

/* Pattern 2: Struct with bit-fields */
struct bitfield_struct {
    unsigned int f1 : 4;
    unsigned int f2 : 8;
    unsigned int f3 : 12;
    unsigned int f4 : 8;
};

unsigned int extract_from_bitfield(struct bitfield_struct *s) {
    /* Accessing bit-fields often generates ZERO_EXTRACT */
    unsigned int val = s->f2;
    val |= (s->f3 << 8);
    return val;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Pattern 1: Writing only low part of a variable */
void set_low_part_32(volatile unsigned int *p, unsigned char v) {
    /* This pattern may generate STRICT_LOW_PART */
    *p = (*p & ~0xFF) | v;
}

/* Pattern 2: Cast and assignment to smaller type */
void set_low_part_cast(volatile uint32_t *p, uint16_t v) {
    /* Writing 16-bit value to 32-bit location */
    *(uint16_t*)p = v;
}

/* Pattern 3: Using char pointer to modify part of int */
void set_low_part_charptr(volatile unsigned int *p, unsigned char v) {
    unsigned char *cp = (unsigned char*)p;
    cp[0] = v;  /* Modify low byte */
}

/* ==================== SUBREG patterns ==================== */

/* Pattern 1: Union for type aliasing */
union subreg_union {
    int32_t i32;
    int16_t i16[2];
    int8_t  i8[4];
};

int32_t subreg_via_union(union subreg_union *u) {
    /* Access different views of the same storage */
    u->i16[0] = 0x1234;
    u->i16[1] = 0x5678;
    return u->i32;  /* Should involve SUBREG accesses */
}

/* Pattern 2: Mixed-size operations */
int32_t subreg_mixed_ops(int32_t a, int16_t b) {
    /* Operations mixing different sizes */
    int16_t temp = (int16_t)a + b;
    return (int32_t)temp * 2;  /* May generate SUBREG */
}

/* Pattern 3: Pointer casting between types */
int32_t subreg_pointer_cast(int64_t *llp) {
    /* Access 64-bit as 32-bit */
    int32_t *ip = (int32_t*)llp;
    return ip[0] + ip[1];
}

/* ==================== Complex MEM patterns ==================== */

/* Pattern 1: Array with complex indexing */
int mem_complex_index(int *base, int idx1, int idx2, int idx3) {
    /* Complex address calculation */
    return base[idx1 + idx2 * 4 + idx3 * 16];
}

/* Pattern 2: Struct with array and pointer arithmetic */
struct mem_struct {
    int arr[100];
    int pad;
    int arr2[50];
};

int mem_struct_access(struct mem_struct *s, int i, int j) {
    /* Multiple memory accesses with different base addresses */
    int val1 = s->arr[i * 3 + j];
    int val2 = s->arr2[j * 2 - i];
    return val1 + val2;
}

/* Pattern 3: Multi-dimensional array */
int mem_multi_dim(int matrix[10][10], int i, int j) {
    /* 2D array access */
    return matrix[i][j] + matrix[j][i];
}

/* ==================== Combined patterns ==================== */

/* Function that combines multiple patterns */
unsigned int combined_patterns(volatile unsigned int *mem_base) {
    unsigned int result = 0;
    
    /* ZERO_EXTRACT pattern */
    result ^= extract_bits_shift_mask(&g_bitfield_source);
    
    /* STRICT_LOW_PART pattern */
    set_low_part_32((volatile unsigned int*)&result, 0xAB);
    
    /* SUBREG pattern via union */
    union subreg_union u;
    u.i32 = result;
    result = subreg_via_union(&u);
    
    /* Complex MEM pattern */
    static int local_array[256];
    for (int i = 0; i < 16; i++) {
        local_array[i] = i * 2;
    }
    result += mem_complex_index(local_array, 1, 2, 3);
    
    return result;
}

/* ==================== Main function ==================== */

int main(void) {
    unsigned int final_result = 0;
    int i;
    
    /* Initialize some data */
    struct bitfield_struct bfs = {0x1, 0x23, 0x456, 0x78};
    union subreg_union su;
    struct mem_struct ms;
    
    /* Initialize memory struct */
    for (i = 0; i < 100; i++) {
        ms.arr[i] = i * 3;
    }
    for (i = 0; i < 50; i++) {
        ms.arr2[i] = i * 5;
    }
    
    /* Multi-dimensional array */
    int matrix[10][10];
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            matrix[row][col] = row * 10 + col;
        }
    }
    
    /* Loop with volatile condition to create control flow */
    for (g_volatile_counter = 0; 
         g_volatile_counter < 10; 
         g_volatile_counter++) {
        
        /* Use volatile flag to create unpredictable control flow */
        if (g_volatile_flag & (1 << g_volatile_counter)) {
            /* ZERO_EXTRACT patterns */
            final_result ^= extract_from_bitfield(&bfs);
            
            /* Update bitfield source */
            g_bitfield_source = final_result * 0x1234567;
            
            /* STRICT_LOW_PART patterns */
            set_low_part_cast((volatile uint32_t*)&final_result, 
                             (uint16_t)g_volatile_counter);
            set_low_part_charptr((volatile unsigned int*)&final_result,
                                (unsigned char)g_volatile_counter);
        } else {
            /* SUBREG patterns */
            su.i32 = final_result;
            final_result += subreg_via_union(&su);
            final_result += subreg_mixed_ops(final_result, (int16_t)g_volatile_counter);
            
            /* Complex MEM patterns */
            final_result += mem_struct_access(&ms, 
                                            g_volatile_counter % 20,
                                            (g_volatile_counter * 3) % 20);
            final_result += mem_multi_dim(matrix,
                                         g_volatile_counter % 10,
                                         (g_volatile_counter * 7) % 10);
        }
        
        /* Combined patterns */
        final_result ^= combined_patterns((volatile unsigned int*)ms.arr);
        
        /* SUBREG via pointer cast */
        int64_t ll_value = (int64_t)final_result * 0x100000000LL;
        final_result += subreg_pointer_cast(&ll_value);
    }
    
    /* Use the result to prevent optimization */
    printf("Final result: 0x%08X\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
