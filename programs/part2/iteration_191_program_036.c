/* Program to generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM RTL patterns */
#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_counter = 0;

/* ===== ZERO_EXTRACT patterns ===== */
/* Bit-field extraction using shift/mask */
unsigned int extract_bits_shift(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT: extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Bit-field struct for ZERO_EXTRACT */
struct bitfield_s {
    unsigned int f1 : 4;
    unsigned int f2 : 8;
    unsigned int f3 : 4;
};

unsigned int extract_bitfield(struct bitfield_s *s) {
    /* Taking address and accessing bitfield may generate ZERO_EXTRACT */
    unsigned int val = s->f2;
    return val;
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Writing only low part of a variable */
void write_low_byte(volatile unsigned int *p, unsigned char v) {
    /* Write only low byte, preserving high bytes */
    *p = (*p & ~0xFF) | v;
}

/* Cast to smaller type assignment */
void strict_low_part_cast(int32_t *x) {
    /* Write to low 16 bits */
    *(int16_t*)x = 0x1234;
}

/* ===== SUBREG patterns ===== */
/* Union for SUBREG access */
union subreg_u {
    int32_t i32;
    int16_t s16[2];
    int8_t  s8[4];
};

int32_t subreg_union_access(union subreg_u *u) {
    /* Access parts of larger type through smaller types */
    u->s16[0] = 100;
    u->s8[2] = 50;
    return u->i32;
}

/* Pointer cast for SUBREG */
int32_t subreg_pointer_cast(int64_t *ll) {
    /* Access 32-bit part of 64-bit value */
    int32_t val = *(int32_t*)ll;
    return val;
}

/* ===== Complex MEM patterns ===== */
/* Struct with array for complex addressing */
struct mem_struct {
    int arr[100];
    int pad;
    int arr2[50];
};

int complex_mem_access(struct mem_struct *s, int i, int j) {
    /* Complex addressing with multiple indices */
    return s->arr[i * 3 + j * 7] + s->arr2[j * 2];
}

/* Array with pointer arithmetic */
int array_index_complex(int *base, int idx1, int idx2, int idx3) {
    /* Complex addressing expression */
    return base[idx1 + idx2 * 4 + idx3 * 16];
}

/* ===== Combined function with control flow ===== */
int combined_operations(void) {
    int result = 0;
    volatile int flag = g_volatile_flag;
    
    /* Local variables for various operations */
    volatile unsigned int extract_var = 0xABCD1234;
    unsigned int bitfield_var = 0;
    int32_t low_part_var = 0xFFFFFFFF;
    union subreg_u subreg_var;
    struct mem_struct mem_var;
    int array[256];
    
    /* Initialize structures */
    for (int i = 0; i < 100; i++) {
        mem_var.arr[i] = i;
    }
    for (int i = 0; i < 50; i++) {
        mem_var.arr2[i] = i * 2;
    }
    for (int i = 0; i < 256; i++) {
        array[i] = i;
    }
    
    /* Loop with conditional operations */
    for (int i = 0; i < 10; i++) {
        if (flag & 0x1) {
            /* ZERO_EXTRACT pattern */
            bitfield_var = extract_bits_shift(&extract_var);
            result += bitfield_var;
            
            /* Another ZERO_EXTRACT via bitfield struct */
            struct bitfield_s bf = {1, 2, 3};
            result += extract_bitfield(&bf);
        }
        
        if (flag & 0x2) {
            /* STRICT_LOW_PART patterns */
            write_low_byte((volatile unsigned int*)&low_part_var, i & 0xFF);
            strict_low_part_cast(&low_part_var);
            result += low_part_var;
        }
        
        if (flag & 0x4) {
            /* SUBREG patterns */
            result += subreg_union_access(&subreg_var);
            int64_t ll_val = 0x123456789ABCDEF0LL;
            result += subreg_pointer_cast(&ll_val);
        }
        
        if (flag & 0x8) {
            /* Complex MEM patterns */
            result += complex_mem_access(&mem_var, i % 10, i % 5);
            result += array_index_complex(array, i, i*2, i*3);
        }
        
        /* Modify flag to change control flow */
        flag = (flag << 1) | (flag >> 31);
    }
    
    return result;
}

/* Helper functions that emphasize specific patterns */
int zero_extract_heavy(void) {
    volatile unsigned int data = 0xDEADBEEF;
    int sum = 0;
    
    /* Multiple ZERO_EXTRACT patterns */
    sum += (data >> 0) & 0xFF;   /* Extract byte 0 */
    sum += (data >> 8) & 0xFF;   /* Extract byte 1 */
    sum += (data >> 16) & 0xFF;  /* Extract byte 2 */
    sum += (data >> 24) & 0xFF;  /* Extract byte 3 */
    
    /* Bitfield variations */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 10;
        unsigned int d : 14;
    } bf;
    
    bf.a = 5;
    bf.b = 10;
    bf.c = 500;
    bf.d = 10000;
    
    sum += bf.a + bf.b + bf.c + bf.d;
    
    return sum;
}

int strict_low_part_heavy(void) {
    int32_t values[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    int sum = 0;
    
    /* Multiple STRICT_LOW_PART writes */
    for (int i = 0; i < 4; i++) {
        /* Write low byte */
        *(int8_t*)&values[i] = i * 10;
        
        /* Write low 16 bits */
        *(int16_t*)&values[i] = i * 100;
        
        sum += values[i];
    }
    
    return sum;
}

int subreg_heavy(void) {
    union {
        int64_t ll[2];
        int32_t i[4];
        int16_t s[8];
        int8_t  c[16];
    } data;
    
    int sum = 0;
    
    /* Multiple SUBREG accesses */
    for (int i = 0; i < 16; i++) {
        data.c[i] = i;
    }
    
    /* Access through different sized views */
    sum += data.s[0] + data.s[3] + data.s[7];
    sum += data.i[0] + data.i[2];
    sum += (int)data.ll[0];
    
    return sum;
}

int complex_mem_heavy(void) {
    struct nested {
        int a[10][10];
        struct {
            int b[5];
            int c;
        } inner[3];
    } complex;
    
    int sum = 0;
    
    /* Complex nested array accesses */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum += complex.a[i][j * 2 % 10];
        }
    }
    
    /* Complex struct/array combination */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 5; j++) {
            sum += complex.inner[i].b[(i + j) % 5];
        }
        sum += complex.inner[i].c;
    }
    
    return sum;
}

/* Main function that exercises all patterns */
int main(void) {
    int final_result = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Call combined function with control flow */
    final_result += combined_operations();
    
    /* Call pattern-specific functions */
    final_result += zero_extract_heavy();
    final_result += strict_low_part_heavy();
    final_result += subreg_heavy();
    final_result += complex_mem_heavy();
    
    /* Loop to increase pass activity */
    for (int iter = 0; iter < 3; iter++) {
        g_counter++;
        if (g_volatile_flag) {
            final_result += combined_operations();
        }
    }
    
    printf("Final result: %d\n", final_result);
    
    /* Return result to prevent optimization */
    return final_result & 0xFF;
}
