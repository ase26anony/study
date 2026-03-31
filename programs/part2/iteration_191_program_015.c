/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates operations that
   generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM_P
   expressions in the RTL intermediate representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable branches */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global variables for memory access patterns */
volatile unsigned int global_bitfield = 0xDEADBEEF;
int global_array[256];
struct ComplexStruct {
    int32_t data[64];
    int16_t shorts[128];
    uint8_t bytes[256];
} global_struct;

/* Union for SUBREG pattern generation */
union MixedSizeUnion {
    int64_t large;
    int32_t medium[2];
    int16_t small[4];
    uint8_t tiny[8];
};

/* ========== ZERO_EXTRACT patterns ========== */
/* Bit-field extraction using shift/mask - may generate ZERO_EXTRACT */
int extract_bits_shiftmask(volatile unsigned int *p) {
    /* Extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Bit-field struct for ZERO_EXTRACT */
struct BitFieldStruct {
    unsigned int low8:8;
    unsigned int mid8:8;
    unsigned int high16:16;
};

int extract_bitfield(struct BitFieldStruct *bfs) {
    /* Taking address and accessing bitfield may generate ZERO_EXTRACT */
    unsigned int val = bfs->mid8;
    return val * 2;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Writing only low part of a variable */
void write_low_byte(volatile unsigned int *p, unsigned char v) {
    /* This pattern may generate STRICT_LOW_PART */
    *p = (*p & ~0xFF) | v;
}

void write_low_halfword(volatile uint32_t *p, uint16_t v) {
    /* Writing only 16 bits to a 32-bit location */
    *p = (*p & 0xFFFF0000) | v;
}

/* ========== SUBREG patterns ========== */
/* Access parts of larger types through smaller types */
int32_t subreg_via_union(union MixedSizeUnion *u) {
    /* Access 32-bit part of 64-bit value */
    u->medium[1] = 0x1234;
    return u->medium[0] + u->small[2];
}

int16_t subreg_via_pointer_cast(int64_t *large_ptr) {
    /* Cast to access part of larger type */
    int16_t *small_ptr = (int16_t *)large_ptr;
    small_ptr[2] = 0x55AA;
    return small_ptr[3];
}

/* ========== Complex MEM_P patterns ========== */
/* Memory access with complex addressing */
int complex_mem_access(int *base, int idx1, int idx2, int idx3) {
    /* Multiple index calculations with scaling */
    return base[(idx1 * 3 + idx2) * 2 + idx3];
}

int struct_mem_access(struct ComplexStruct *cs, int i, int j, int k) {
    /* Mixed array accesses within struct */
    return cs->data[i] + cs->shorts[j] * cs->bytes[k];
}

int pointer_arithmetic_mem(int *arr, int offset1, int offset2) {
    /* Complex pointer arithmetic */
    return *(arr + offset1 * 4 + offset2) + *(arr - offset1 + offset2 * 2);
}

/* ========== Combined function with control flow ========== */
/* This function combines multiple patterns in complex control flow */
int combined_operations(int seed) {
    union MixedSizeUnion u;
    struct BitFieldStruct bfs = {0};
    int result = seed;
    volatile int local_flag = v_flag1;
    
    u.large = (int64_t)seed * 0x10001;
    
    /* Loop with volatile condition to prevent optimization */
    for (int i = 0; i < (local_flag ? 4 : 8); i++) {
        if (v_flag2 || (i % 2 == 0)) {
            /* ZERO_EXTRACT pattern */
            bfs.mid8 = (seed + i) & 0xFF;
            result += extract_bitfield(&bfs);
            
            /* Complex MEM_P pattern */
            result += complex_mem_access(global_array, i, result & 0xF, (i * 3) & 0xF);
        } else {
            /* STRICT_LOW_PART pattern */
            write_low_byte((volatile unsigned int *)&result, i & 0xFF);
            
            /* SUBREG pattern */
            result += subreg_via_union(&u);
        }
        
        /* Another MEM_P pattern */
        result += struct_mem_access(&global_struct, 
                                   i & 0x3F, 
                                   (i * 2) & 0x7F, 
                                   (i * 3) & 0xFF);
        
        v_counter++;
    }
    
    return result;
}

/* Helper functions that emphasize specific patterns */
int emphasize_zero_extract(int x) {
    struct BitFieldStruct bfs;
    bfs.low8 = x & 0xFF;
    bfs.mid8 = (x >> 8) & 0xFF;
    bfs.high16 = (x >> 16) & 0xFFFF;
    
    volatile unsigned int v = global_bitfield;
    int a = extract_bits_shiftmask(&v);
    int b = extract_bitfield(&bfs);
    
    return a + b + bfs.high16;
}

int emphasize_strict_low_part(int x) {
    volatile uint32_t v32 = x;
    volatile uint16_t v16 = x & 0xFFFF;
    
    write_low_byte((volatile unsigned int *)&v32, x & 0xFF);
    write_low_halfword((volatile uint32_t *)&v32, v16);
    
    return v32 + v16;
}

int emphasize_subreg(int x) {
    union MixedSizeUnion u;
    u.large = (int64_t)x * 0x100000001LL;
    
    int64_t big_val = u.large + 0x12345678;
    int a = subreg_via_union(&u);
    int b = subreg_via_pointer_cast(&big_val);
    
    return a + b + u.small[0];
}

int emphasize_mem_p(int x) {
    int temp[128];
    for (int i = 0; i < 128; i++) {
        temp[i] = x + i * 3;
    }
    
    int result = 0;
    for (int i = 0; i < 32; i++) {
        result += pointer_arithmetic_mem(temp, i, (i * 2) & 0x1F);
        result += complex_mem_access(temp, i & 0xF, (i >> 4) & 0xF, i & 0x3);
    }
    
    return result;
}

/* Main function that orchestrates all patterns */
int main() {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
        global_struct.bytes[i] = i & 0xFF;
        if (i < 128) global_struct.shorts[i] = i * 2;
        if (i < 64) global_struct.data[i] = i * 5;
    }
    
    int result = 0;
    volatile int input = 42;
    
    /* Call pattern-emphasizing functions in a loop */
    for (int i = 0; i < 8; i++) {
        v_flag1 = i & 1;
        v_flag2 = (i >> 1) & 1;
        
        result ^= emphasize_zero_extract(input + i);
        result += emphasize_strict_low_part(result);
        result -= emphasize_subreg(result);
        result ^= emphasize_mem_p(result);
        
        /* Combined operations with complex control flow */
        result = combined_operations(result);
        
        input += v_counter;
    }
    
    /* Use result to prevent optimization */
    printf("Final result: %d (v_counter: %d)\n", result, v_counter);
    
    return result & 0xFF;
}
