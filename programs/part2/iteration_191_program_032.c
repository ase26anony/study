/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates operations that
   generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM_P
   expressions in the RTL intermediate representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global/volatile variables to prevent optimization */
volatile unsigned int g_volatile_int = 0xDEADBEEF;
volatile unsigned char g_volatile_char = 0xAB;
volatile int g_control = 1;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field extraction using shift and mask - may generate ZERO_EXTRACT */
unsigned int extract_bits_shift(volatile unsigned int *p) {
    /* Extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Bit-field struct to potentially generate ZERO_EXTRACT */
struct bitfield_s {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
};

unsigned int extract_bitfield(struct bitfield_s *bf) {
    /* Taking address and accessing bitfield */
    unsigned int val = bf->mid8;
    return val;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Writing only low byte of a larger integer */
void set_low_byte(volatile unsigned int *p, unsigned char v) {
    *p = (*p & ~0xFF) | v;
}

/* Cast and assignment to create partial write */
void write_low_half(int32_t *x, int16_t v) {
    *(int16_t*)x = v;
}

/* ==================== SUBREG patterns ==================== */

/* Union for type punning - may generate SUBREG */
union mixed_types {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

int32_t access_via_subreg(union mixed_types *u) {
    /* Access part of larger register */
    u->halves[0] = 0x1234;
    u->halves[1] = 0x5678;
    return u->full;
}

/* Pointer cast between different sizes */
int64_t subreg_via_cast(int64_t ll) {
    int32_t *p = (int32_t*)&ll;
    return *p + *(p + 1);
}

/* ==================== Complex MEM patterns ==================== */

/* Struct with array for complex addressing */
struct data_block {
    int arr[100];
    int pad;
    int metadata[10];
};

/* Complex memory addressing with multiple indices */
int complex_mem_access(struct data_block *block, int i, int j, int k) {
    /* Non-trivial addressing mode */
    return block->arr[i + j * 10 + k * 5];
}

/* Array with pointer arithmetic */
int array_with_arithmetic(int *base, int idx1, int idx2, int idx3) {
    /* Complex address calculation */
    return base[(idx1 * idx2) + (idx3 << 2)];
}

/* ==================== Combined function ==================== */

/* Function that combines multiple patterns with control flow */
unsigned int combined_operations(int iterations) {
    unsigned int result = 0;
    volatile int flag = g_control;
    
    /* Local variables for patterns */
    union mixed_types u;
    struct bitfield_s bf = {0xAA, 0xBB, 0xCCDD};
    struct data_block db;
    int local_array[256];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        db.arr[i] = i * 3;
    }
    for (int i = 0; i < 256; i++) {
        local_array[i] = i;
    }
    
    for (int i = 0; i < iterations; i++) {
        /* Use volatile to prevent dead code elimination */
        if (flag & 0x1) {
            /* ZERO_EXTRACT pattern */
            result ^= extract_bits_shift(&g_volatile_int);
            result += extract_bitfield(&bf);
        }
        
        if (flag & 0x2) {
            /* STRICT_LOW_PART patterns */
            set_low_byte((volatile unsigned int*)&result, g_volatile_char + i);
            write_low_half((int32_t*)&result, (int16_t)(result + i));
        }
        
        if (flag & 0x4) {
            /* SUBREG patterns */
            result += access_via_subreg(&u);
            u.full = result;
            result += subreg_via_cast((int64_t)result * 3);
        }
        
        if (flag & 0x8) {
            /* Complex MEM patterns */
            int idx = result % 50;
            result += complex_mem_access(&db, idx, (idx + 1) % 10, (idx + 2) % 5);
            result += array_with_arithmetic(local_array, 
                                          idx % 16, 
                                          (idx + 1) % 16, 
                                          (idx + 2) % 16);
        }
        
        /* Change control flow pattern */
        flag = (flag << 1) | (flag >> 31);
    }
    
    return result;
}

/* ==================== Helper functions ==================== */

/* Additional functions to increase pass activity */
unsigned int helper_zero_extract() {
    struct bitfield_s local_bf = {0x11, 0x22, 0x3344};
    volatile unsigned int v = 0x87654321;
    
    unsigned int r1 = extract_bits_shift(&v);
    unsigned int r2 = extract_bitfield(&local_bf);
    
    return r1 + r2;
}

unsigned int helper_strict_low() {
    volatile unsigned int target = 0;
    set_low_byte(&target, 0xCD);
    
    int32_t var = 0xFFFFFFFF;
    write_low_half(&var, 0x1234);
    
    return target + var;
}

unsigned int helper_subreg() {
    union mixed_types u2;
    u2.full = 0;
    
    access_via_subreg(&u2);
    int64_t big = 0x1122334455667788LL;
    unsigned int r = subreg_via_cast(big);
    
    return u2.full + r;
}

unsigned int helper_mem() {
    struct data_block db2;
    for (int i = 0; i < 100; i++) {
        db2.arr[i] = i * 7;
    }
    
    int arr2[128];
    for (int i = 0; i < 128; i++) {
        arr2[i] = i * 11;
    }
    
    unsigned int r1 = complex_mem_access(&db2, 10, 2, 3);
    unsigned int r2 = array_with_arithmetic(arr2, 5, 6, 7);
    
    return r1 + r2;
}

/* ==================== Main function ==================== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize with command line or default */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 100;
    
    printf("Starting operations with %d iterations...\n", iterations);
    
    /* Call combined function */
    final_result = combined_operations(iterations);
    
    /* Call individual helpers to increase compiler pass activity */
    final_result ^= helper_zero_extract();
    final_result += helper_strict_low();
    final_result ^= helper_subreg();
    final_result += helper_mem();
    
    /* Use volatile to ensure all operations complete */
    g_volatile_int = final_result;
    
    printf("Result: 0x%08X\n", final_result);
    
    /* Return result to prevent optimization */
    return (int)(final_result & 0x7FFFFFFF);
}
