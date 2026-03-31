/* test_resource_patterns.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates
 * - SUBREG for register sub-parts
 * - Complex memory addressing modes
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Explicit bit-field extraction */
unsigned int test_zero_extract_explicit(unsigned int x, unsigned int shift) {
    /* This should generate ZERO_EXTRACT for the mask and shift */
    unsigned int mask = (1 << 8) - 1;  /* 8-bit mask */
    return (x >> shift) & mask;
}

/* Bit-field structure operations */
unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Multiple bit-field assignments and comparisons */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x7F;
    
    /* Bit-field comparison - should generate ZERO_EXTRACT */
    if (s->field1 == 0x10) {
        result |= 1;
    }
    
    /* Complex bit-field expression */
    s->field3 = (s->field2 >> 2) & 0x7;
    
    /* Combine multiple bit-fields */
    result |= (s->field1 << 16);
    result |= (s->field2 << 8);
    result |= s->field3;
    
    return result;
}

/* Mixed bit-field and arithmetic */
unsigned int test_zero_extract_mixed(unsigned int x) {
    /* Multiple ZERO_EXTRACT patterns */
    unsigned int a = (x & 0xFF00) >> 8;      /* Extract byte 1 */
    unsigned int b = (x & 0x00FF0000) >> 16; /* Extract byte 2 */
    unsigned int c = (x & 0x1F);             /* Extract lower 5 bits */
    
    return a + b * c;
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Partial register updates */
unsigned int test_strict_low_part_chars(char *data, int count) {
    unsigned int result = 0;
    
    /* char operations that may use partial registers */
    for (int i = 0; i < count; i++) {
        char temp = data[i];
        /* Multiple char operations forcing register allocation */
        temp = temp + g_volatile_seed;
        temp = temp * 3;
        result += (unsigned int)temp;
    }
    
    return result;
}

/* short operations with volatile */
unsigned int test_strict_low_part_shorts(short *array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Volatile pointer write - may generate STRICT_LOW_PART */
        volatile short *ptr = &array[i];
        *ptr = (short)(array[i] + g_volatile_seed);
        
        /* Read back and accumulate */
        sum += (unsigned int)*ptr;
    }
    
    return sum;
}

/* Inline assembly for byte operations */
unsigned int test_strict_low_part_asm(unsigned int x) {
    unsigned char result;
    
    /* Inline assembly modifying byte register */
    __asm__ volatile (
        "movb %%al, %0\n\t"
        : "=r" (result)
        : "a" (x)
        : "cc"
    );
    
    return result;
}

/* Function with small integer parameters */
unsigned int test_strict_low_part_args(unsigned short a, unsigned char b) {
    /* Operations on small types in registers */
    unsigned int x = a + b;
    x = x * 3;
    
    /* Partial write back */
    unsigned short y = (unsigned short)(x & 0xFFFF);
    return x + y;
}

/* ========== SUBREG PATTERNS ========== */

/* Union for type-punning */
union type_pun {
    uint32_t i;
    uint16_t s[2];
    uint8_t  b[4];
};

unsigned int test_subreg_union(union type_pun *u, int index) {
    unsigned int result = 0;
    
    /* Access different views of the same register */
    result += u->s[0];      /* SUBREG for half-word access */
    result += u->s[1];      /* Another SUBREG */
    result += u->b[index];  /* SUBREG for byte access */
    
    /* Modify through one view, read through another */
    u->s[1] = (uint16_t)g_volatile_seed;
    result += u->i;         /* Read full word */
    
    return result;
}

/* Casting between different sizes */
unsigned int test_subreg_casts(unsigned int x) {
    unsigned int result = 0;
    
    /* Multiple casts generating SUBREG */
    unsigned short s1 = (unsigned short)(x & 0xFFFF);
    unsigned short s2 = (unsigned short)((x >> 16) & 0xFFFF);
    
    unsigned char c1 = (unsigned char)(x & 0xFF);
    unsigned char c2 = (unsigned char)((x >> 8) & 0xFF);
    unsigned char c3 = (unsigned char)((x >> 16) & 0xFF);
    unsigned char c4 = (unsigned char)((x >> 24) & 0xFF);
    
    result = s1 + s2 + c1 + c2 + c3 + c4;
    
    /* More complex expression with mixed sizes */
    result += (unsigned int)((short)(result & 0xFFFF) * (char)(result & 0xFF));
    
    return result;
}

/* Packed structure */
struct __attribute__((packed)) packed_data {
    uint16_t a;
    uint8_t  b;
    uint16_t c;
    uint8_t  d;
};

unsigned int test_subreg_packed(struct packed_data *p) {
    /* Accesses to packed structure generate SUBREG */
    unsigned int result = p->a + p->b + p->c + p->d;
    
    /* Modify and read back */
    p->a = (uint16_t)(result & 0xFFFF);
    p->b = (uint8_t)((result >> 8) & 0xFF);
    
    return result + p->a + p->b;
}

/* ========== COMPLEX MEMORY ADDRESSING ========== */

/* Array with complex indexing */
unsigned int test_complex_addressing(int *array, int size, int *indices) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Complex addressing mode */
        int idx = indices[i] & (size - 1);
        sum += array[idx * 2 + g_volatile_seed % 4];
        
        /* Pointer arithmetic */
        int *ptr = array + idx;
        sum += *(ptr + 1);
    }
    
    return sum;
}

/* Structure with arrays */
struct complex_struct {
    int data[32];
    struct bitfield_struct bf;
    union type_pun pun;
};

unsigned int test_mixed_patterns(struct complex_struct *cs, int iterations) {
    unsigned int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix all patterns */
        
        /* ZERO_EXTRACT from bit-field */
        cs->bf.field1 = (i * 3) & 0x1F;
        total += cs->bf.field1;
        
        /* STRICT_LOW_PART through char pointer */
        volatile char *cptr = (volatile char *)&cs->data[i % 32];
        *cptr = (char)(total & 0xFF);
        
        /* SUBREG through union */
        cs->pun.i = total;
        total += cs->pun.s[0] + cs->pun.b[1];
        
        /* Complex memory addressing */
        int idx = (i * 7) % 32;
        total += cs->data[idx] + cs->data[(idx + 1) % 32];
    }
    
    return total;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data with some randomness */
    int array_size = 100;
    int *int_array = (int*)malloc(array_size * sizeof(int));
    short *short_array = (short*)malloc(array_size * sizeof(short));
    char *char_array = (char*)malloc(array_size * sizeof(char));
    int *indices = (int*)malloc(array_size * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < array_size; i++) {
        int_array[i] = (i * 37 + g_volatile_seed) & 0xFFFF;
        short_array[i] = (short)((i * 53 + g_volatile_seed) & 0x7FFF);
        char_array[i] = (char)((i * 71 + g_volatile_seed) & 0x7F);
        indices[i] = (i * 29) % array_size;
    }
    
    /* Test structures */
    struct bitfield_struct bf_struct = {0};
    union type_pun pun_union;
    struct packed_data packed = {0};
    struct complex_struct complex_struct = {0};
    
    /* Initialize union */
    pun_union.i = 0x12345678;
    
    /* Initialize complex struct */
    for (int i = 0; i < 32; i++) {
        complex_struct.data[i] = i * i;
    }
    
    /* Run all tests and accumulate results */
    
    /* ZERO_EXTRACT tests */
    final_result += test_zero_extract_explicit(0xABCD1234, 4);
    final_result += test_zero_extract_struct(&bf_struct, 0x3A5F);
    final_result += test_zero_extract_mixed(0x89ABCDEF);
    
    /* STRICT_LOW_PART tests */
    final_result += test_strict_low_part_chars(char_array, array_size / 2);
    final_result += test_strict_low_part_shorts(short_array, array_size / 2);
    final_result += test_strict_low_part_asm(0x87654321);
    final_result += test_strict_low_part_args(0x1234, 0x56);
    
    /* SUBREG tests */
    final_result += test_subreg_union(&pun_union, 2);
    final_result += test_subreg_casts(0x9ABCDEF0);
    final_result += test_subreg_packed(&packed);
    
    /* Complex addressing tests */
    final_result += test_complex_addressing(int_array, array_size, indices);
    final_result += test_mixed_patterns(&complex_struct, 50);
    
    /* Clean up */
    free(int_array);
    free(short_array);
    free(char_array);
    free(indices);
    
    /* Use result to prevent optimization */
    printf("Final checksum: %u\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
