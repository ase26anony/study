/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function 1: Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int shift) {
    /* This should generate ZERO_EXTRACT for bit-field operations */
    unsigned int mask = (1 << 9) - 1;  /* 9-bit mask */
    unsigned int result = 0;
    
    /* Multiple extractions to ensure they're not optimized away */
    result += (x >> shift) & mask;                    /* Simple extract */
    result += (x & 0xFF00) >> 8;                      /* Explicit mask+shift */
    result += (x >> 3) & 0x1F;                        /* 5-bit field */
    
    /* Chain extractions to create dependency */
    unsigned int temp = x;
    for (int i = 0; i < 4; i++) {
        result += (temp >> (i * 3)) & 0x7;            /* 3-bit sliding window */
        temp ^= result;                               /* Create dependency */
    }
    
    return result;
}

/* Function 2: Bit-field structure operations */
unsigned int test_zero_extract_struct(struct BitFieldStruct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignments */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x7F;
    s->field3 = (val >> 12) & 0x7;
    
    /* Bit-field comparisons and arithmetic */
    if (s->field1 == 0x10) {
        result += 1;
    }
    if (s->field2 > 0x20) {
        result += s->field2;
    }
    
    /* Combine bit-fields */
    result += (s->field1 << 16) | (s->field2 << 8) | s->field3;
    
    /* Complex bit-field expression */
    s->field4 = ((s->field1 + s->field2) << 3) | s->field3;
    result += s->field4;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Function 3: Partial register updates with small types */
unsigned int test_strict_low_part(void) {
    unsigned int result = 0;
    volatile unsigned int base = g_volatile_seed;
    
    /* char operations that may use partial registers */
    unsigned char c1, c2, c3;
    c1 = (base >> 0) & 0xFF;
    c2 = (base >> 8) & 0xFF;
    c3 = (base >> 16) & 0xFF;
    
    /* Operations that might generate STRICT_LOW_PART */
    c1 = c1 + c2;                    /* char addition */
    c2 = c1 * 3;                     /* char multiplication */
    c3 = c2 - c1;                    /* char subtraction */
    
    result = c1 + (c2 << 8) + (c3 << 16);
    
    /* short operations */
    unsigned short s1, s2;
    s1 = base & 0xFFFF;
    s2 = (base >> 16) & 0xFFFF;
    
    /* Force partial register updates */
    s1 = s1 + s2;                    /* short addition */
    s2 = s1 * 2;                     /* short multiplication */
    
    result += s1 + (s2 << 16);
    
    return result;
}

/* Function 4: Pointer-based partial writes */
unsigned int test_strict_low_part_ptr(volatile unsigned short *ptr, unsigned int count) {
    unsigned int result = 0;
    
    for (unsigned int i = 0; i < count; i++) {
        /* Partial write to short pointer - may generate STRICT_LOW_PART */
        ptr[i] = (i * 0x1001) & 0xFFFF;
        result += ptr[i];
        
        /* Alternate between high and low bytes */
        unsigned char *byte_ptr = (unsigned char *)&ptr[i];
        byte_ptr[0] = i & 0xFF;
        byte_ptr[1] = (i >> 8) & 0xFF;
        result += byte_ptr[0] + byte_ptr[1];
    }
    
    return result;
}

/* ========== SUBREG patterns ========== */
/* Function 5: Union-based type punning */
unsigned int test_subreg_union(void) {
    union {
        uint32_t i;
        uint16_t s[2];
        uint8_t b[4];
    } u;
    
    u.i = g_volatile_seed * 0x1234567;
    
    /* Access sub-parts through union - should generate SUBREG */
    unsigned int result = 0;
    result += u.s[0];                /* Access low 16 bits */
    result += u.s[1];                /* Access high 16 bits */
    result += u.b[0] + u.b[1] + u.b[2] + u.b[3];  /* Access individual bytes */
    
    /* Modify through sub-parts */
    u.s[0] = (u.s[0] + 1) & 0xFFFF;
    u.b[2] = u.b[1] ^ 0xAA;
    
    result += u.i;
    
    return result;
}

/* Function 6: Casting between integer sizes */
unsigned int test_subreg_casts(unsigned int x) {
    unsigned int result = 0;
    
    /* Casts that may generate SUBREG */
    uint16_t s1 = (uint16_t)(x & 0xFFFF);
    uint16_t s2 = (uint16_t)((x >> 16) & 0xFFFF);
    
    result = s1 + s2;
    
    /* Promote and truncate */
    uint32_t temp = s1 * s2;
    uint16_t s3 = (uint16_t)(temp & 0xFFFF);
    result += s3;
    
    /* Byte extraction through casting */
    uint8_t b1 = (uint8_t)(x & 0xFF);
    uint8_t b2 = (uint8_t)((x >> 8) & 0xFF);
    uint8_t b3 = (uint8_t)((x >> 16) & 0xFF);
    uint8_t b4 = (uint8_t)((x >> 24) & 0xFF);
    
    result += b1 + b2 + b3 + b4;
    
    return result;
}

/* Function 7: SIMD-like operations (manual vectorization) */
unsigned int test_subreg_simd(unsigned int a, unsigned int b) {
    /* Manual SIMD operations on packed data */
    union {
        uint32_t words[4];
        uint16_t halves[8];
    } vec;
    
    vec.words[0] = a;
    vec.words[1] = b;
    vec.words[2] = a ^ b;
    vec.words[3] = a + b;
    
    /* Process 16-bit elements */
    unsigned int result = 0;
    for (int i = 0; i < 8; i++) {
        result += vec.halves[i];
        vec.halves[i] = (vec.halves[i] * 3) & 0xFFFF;  /* Keep in 16-bit range */
    }
    
    /* Process 32-bit elements */
    for (int i = 0; i < 4; i++) {
        result += vec.words[i];
    }
    
    return result;
}

/* ========== Memory references with complex addresses ========== */
/* Function 8: Complex memory addressing with bit-fields */
unsigned int test_complex_memory(struct BitFieldStruct *array, int size) {
    unsigned int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Complex addressing with index */
        struct BitFieldStruct *elem = &array[i];
        
        /* Access through pointer with offset */
        result += elem->field1;
        result += elem->field2 << 8;
        
        /* Modify through pointer */
        elem->field3 = (i & 0x7);
        elem->field4 = (result & 0x1FFFF);
        
        /* Pointer arithmetic */
        unsigned char *byte_ptr = (unsigned char *)elem;
        for (int j = 0; j < sizeof(struct BitFieldStruct); j++) {
            result += byte_ptr[j];
            byte_ptr[j] ^= (i + j) & 0xFF;  /* Modify each byte */
        }
    }
    
    return result;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data */
    struct BitFieldStruct bf_struct = {0};
    struct BitFieldStruct bf_array[10];
    volatile unsigned short mem_buffer[100];
    
    /* Initialize arrays with non-constant data */
    for (int i = 0; i < 10; i++) {
        bf_array[i].field1 = i & 0x1F;
        bf_array[i].field2 = (i * 3) & 0x7F;
        bf_array[i].field3 = (i * 5) & 0x7;
        bf_array[i].field4 = i * 0x1111;
    }
    
    for (int i = 0; i < 100; i++) {
        mem_buffer[i] = (i * 0xABCD) & 0xFFFF;
    }
    
    /* Test 1: ZERO_EXTRACT patterns */
    unsigned int test_val = g_volatile_seed;
    final_result ^= test_zero_extract_int(test_val, 3);
    final_result ^= test_zero_extract_struct(&bf_struct, test_val);
    
    /* Test 2: STRICT_LOW_PART patterns */
    final_result ^= test_strict_low_part();
    final_result ^= test_strict_low_part_ptr(mem_buffer, 50);
    
    /* Test 3: SUBREG patterns */
    final_result ^= test_subreg_union();
    final_result ^= test_subreg_casts(test_val);
    final_result ^= test_subreg_simd(test_val, test_val ^ 0x87654321);
    
    /* Test 4: Complex memory references */
    final_result ^= test_complex_memory(bf_array, 10);
    
    /* Mix all results to prevent optimization */
    final_result = (final_result >> 16) | (final_result << 16);
    final_result ^= g_volatile_seed;
    
    /* Use result to affect control flow */
    if (final_result > 1000000) {
        printf("Result: %u\n", final_result);
    } else {
        printf("Alternative: %u\n", final_result ^ 0xFFFFFFFF);
    }
    
    return (int)(final_result & 0x7FFFFFFF);
}
