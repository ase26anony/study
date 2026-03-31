/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT Patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x) {
    unsigned int sum = 0;
    /* Multiple bit-field extractions with different widths */
    sum += (x >> 3) & 0x1F;      /* Extract bits 3-7 (5 bits) */
    sum += (x >> 8) & 0xFF;      /* Extract bits 8-15 (8 bits) */
    sum += (x >> 16) & 0x7FF;    /* Extract bits 16-26 (11 bits) */
    sum += (x >> 0) & 0x3;       /* Extract bits 0-1 (2 bits) */
    return sum;
}

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 8;
    unsigned int field3 : 11;
    unsigned int field4 : 2;
    unsigned int padding : 6;
};

unsigned int test_zero_extract_struct(struct BitFieldStruct *s) {
    unsigned int sum = 0;
    /* Bit-field comparisons and arithmetic */
    if (s->field1 == 3) {
        sum += 1;
    }
    if (s->field2 > 100) {
        sum += s->field2;
    }
    sum += s->field3 * 2;
    sum += s->field4 << 1;
    
    /* Bit-field assignment */
    s->field1 = (g_volatile_seed & 0x1F);
    s->field2 = (g_volatile_seed >> 5) & 0xFF;
    
    return sum;
}

/* Complex bit-field extraction with memory reference */
unsigned int test_zero_extract_mem(unsigned int *arr, int idx) {
    unsigned int x = arr[idx];
    /* Combined mask and shift with memory operand */
    return ((x & 0xFF00) >> 8) + ((x & 0xF) << 4);
}

/* ========== STRICT_LOW_PART Patterns ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part_chars(char *chars, short *shorts, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        /* These assignments should generate STRICT_LOW_PART */
        char c = chars[i];
        short s = shorts[i];
        
        /* Arithmetic that promotes then writes back partial result */
        c = (c + g_volatile_seed) & 0xFF;
        s = (s * 2) & 0xFFFF;
        
        chars[i] = c;
        shorts[i] = s;
        
        sum += c + s;
    }
    return sum;
}

/* Volatile pointer write */
unsigned int test_strict_low_part_volatile(volatile short *ptr, int count) {
    unsigned int sum = 0;
    for (int i = 0; i < count; i++) {
        /* Volatile write of short - should preserve high bits if any */
        ptr[i] = (short)(g_volatile_seed + i);
        sum += ptr[i];
    }
    return sum;
}

/* Inline assembly for byte register access */
unsigned int test_strict_low_part_asm(void) {
    unsigned int result = 0;
    unsigned char byte1 = 0xAA;
    unsigned char byte2 = 0x55;
    
    /* Assembly that operates on byte registers */
    __asm__ volatile (
        "movb %1, %%al\n\t"
        "xorb %2, %%al\n\t"
        "movb %%al, %0"
        : "=r" (byte1)
        : "r" (byte1), "r" (byte2)
        : "%al"
    );
    
    result = byte1;
    
    /* Another assembly sequence with different constraints */
    unsigned short word = 0x1234;
    __asm__ volatile (
        "rolw $4, %0"
        : "+r" (word)
        :
        : "cc"
    );
    
    result += word;
    return result;
}

/* ========== SUBREG Patterns ========== */

/* Union for type-punning */
unsigned int test_subreg_union(unsigned int value) {
    union {
        uint32_t i;
        uint16_t s[2];
        uint8_t b[4];
    } u;
    
    u.i = value;
    
    /* Access different sub-parts */
    unsigned int sum = u.s[0] + u.s[1];
    sum += u.b[0] + u.b[1] + u.b[2] + u.b[3];
    
    /* Modify through sub-register */
    u.s[1] = (u.s[1] & 0xFF) | 0x5500;
    sum += u.i;
    
    return sum;
}

/* Casting between integer sizes */
unsigned int test_subreg_casts(unsigned int x) {
    unsigned int sum = 0;
    
    /* Multiple casts to different sizes */
    uint16_t s1 = (uint16_t)(x & 0xFFFF);
    uint16_t s2 = (uint16_t)((x >> 16) & 0xFFFF);
    uint8_t b1 = (uint8_t)(x & 0xFF);
    uint8_t b2 = (uint8_t)((x >> 8) & 0xFF);
    
    /* Arithmetic that may keep values in registers */
    sum = s1 * s2 + b1 * b2;
    
    /* Cast back and forth */
    uint32_t temp = (uint32_t)s1 | ((uint32_t)s2 << 16);
    sum += temp & 0xFFFFFF;
    
    return sum;
}

/* SIMD-like operations using sub-registers */
unsigned int test_subreg_simd_like(unsigned int a, unsigned int b) {
    /* Treat 32-bit as packed 16-bit values */
    unsigned int low_a = a & 0xFFFF;
    unsigned int high_a = (a >> 16) & 0xFFFF;
    unsigned int low_b = b & 0xFFFF;
    unsigned int high_b = (b >> 16) & 0xFFFF;
    
    /* SIMD-like operations */
    unsigned int sum_low = low_a + low_b;
    unsigned int sum_high = high_a + high_b;
    
    /* Re-pack results */
    return (sum_high << 16) | (sum_low & 0xFFFF);
}

/* ========== Complex Memory References ========== */

/* Memory references with addressing modes */
unsigned int test_complex_memref(int *base, int *offsets, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Complex addressing: base + offset */
        int *ptr = base + offsets[i];
        
        /* Access with potential SUBREG extraction */
        short low_half = (short)(*ptr & 0xFFFF);
        short high_half = (short)((*ptr >> 16) & 0xFFFF);
        
        sum += low_half + high_half;
        
        /* Modify through pointer with partial update */
        *ptr = (*ptr & 0xFFFF0000) | ((*ptr + i) & 0xFFFF);
    }
    return sum;
}

/* Structure with mixed types accessed via pointer */
struct MixedStruct {
    int a;
    short b;
    char c;
    int d;
};

unsigned int test_mixed_struct_memref(struct MixedStruct *arr, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Access different sized members */
        sum += arr[i].a;
        sum += arr[i].b;  /* short access */
        sum += arr[i].c;  /* char access */
        
        /* Modify members */
        arr[i].b = (arr[i].b + g_volatile_seed) & 0xFFFF;
        arr[i].c = (arr[i].c ^ 0x55) & 0xFF;
    }
    return sum;
}

/* ========== Main Test Driver ========== */

int main(int argc, char **argv) {
    unsigned int final_sum = 0;
    
    /* Initialize test data */
    unsigned int test_int = g_volatile_seed;
    struct BitFieldStruct bf = {3, 150, 1024, 1, 0};
    unsigned int array[4] = {0x12345678, 0x9ABCDEF0, 0x11223344, 0x55667788};
    
    char chars[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    short shorts[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    volatile short volatile_shorts[4];
    
    int base_array[16];
    int offsets[4] = {0, 2, 4, 6};
    for (int i = 0; i < 16; i++) {
        base_array[i] = i * 0x1111;
    }
    
    struct MixedStruct mixed_arr[4] = {
        {100, 200, 50, 1000},
        {200, 400, 100, 2000},
        {300, 600, 150, 3000},
        {400, 800, 200, 4000}
    };
    
    /* Run all tests */
    final_sum += test_zero_extract_int(test_int);
    final_sum += test_zero_extract_struct(&bf);
    final_sum += test_zero_extract_mem(array, 1);
    
    final_sum += test_strict_low_part_chars(chars, shorts, 8);
    final_sum += test_strict_low_part_volatile(volatile_shorts, 4);
    final_sum += test_strict_low_part_asm();
    
    final_sum += test_subreg_union(0xDEADBEEF);
    final_sum += test_subreg_casts(0x12345678);
    final_sum += test_subreg_simd_like(0xAABBCCDD, 0x11223344);
    
    final_sum += test_complex_memref(base_array, offsets, 4);
    final_sum += test_mixed_struct_memref(mixed_arr, 4);
    
    /* Use argc to prevent over-optimization */
    if (argc > 1) {
        final_sum += atoi(argv[1]);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %u\n", final_sum);
    
    return (int)(final_sum & 0x7FFFFFFF);
}
