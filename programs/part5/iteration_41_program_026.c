/* test_resource_patterns.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates
 * - SUBREG for register sub-parts
 * - Complex memory addressing modes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_bitfield(unsigned int x, unsigned int shift) {
    /* This should generate ZERO_EXTRACT for the bit-field */
    unsigned int mask = (1u << 5) - 1;  /* 5-bit mask */
    return (x >> shift) & mask;
}

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

unsigned int test_zero_extract_struct(struct BitFieldStruct *s, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Bit-field reads should generate ZERO_EXTRACT */
        sum += s->field1;
        sum += s->field2 * 2;
        sum += s->field3 << 1;
        
        /* Bit-field writes */
        s->field1 = (i & 0x1F);          /* 5-bit assignment */
        s->field2 = ((i * 3) & 0x7F);    /* 7-bit assignment */
        
        /* Complex bit-field expression */
        unsigned int temp = s->field4;
        s->field3 = (temp >> 8) & 0x7;   /* Extract 3 bits */
    }
    
    return sum;
}

/* Explicit mask and shift operations */
unsigned int test_zero_extract_explicit(unsigned int x) {
    /* Multiple ZERO_EXTRACT patterns */
    unsigned int part1 = (x & 0x000000FF) >> 0;    /* Low byte */
    unsigned int part2 = (x & 0x0000FF00) >> 8;    /* Second byte */
    unsigned int part3 = (x & 0x00FF0000) >> 16;   /* Third byte */
    unsigned int part4 = (x & 0xFF000000) >> 24;   /* High byte */
    
    /* Combine with arithmetic */
    return part1 + (part2 * 2) + (part3 * 3) + (part4 * 4);
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part_chars(char *buffer, int size) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < size; i++) {
        /* char assignment in loop - may use partial register */
        char c = buffer[i];
        c = c ^ 0x55;          /* Modify byte */
        buffer[i] = c;         /* Store back - STRICT_LOW_PART */
        checksum += c;
    }
    
    return checksum;
}

/* Mixed-size operations */
unsigned int test_strict_low_part_mixed(short *sarray, char *carray, int n) {
    unsigned int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* short operations */
        short s = sarray[i];
        s = (s * 3) & 0x7FFF;  /* Keep 15 bits */
        sarray[i] = s;         /* Partial register store */
        sum += s;
        
        /* char operations */
        char c = carray[i];
        c = ~c;                /* Invert bits */
        carray[i] = c;         /* Another partial store */
        sum += c;
    }
    
    return sum;
}

/* Volatile pointer for STRICT_LOW_PART */
unsigned int test_strict_low_part_volatile(volatile short *vptr, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Volatile write to short - likely STRICT_LOW_PART */
        *vptr = (short)(i * 7);
        result += *vptr;       /* Volatile read */
        vptr++;                /* Pointer arithmetic */
    }
    
    return result;
}

/* ========== SUBREG PATTERNS ========== */

/* Union for type-punning */
union TypePun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } halves;
    uint8_t bytes[4];
};

unsigned int test_subreg_union(union TypePun *u, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Access different views of the same data */
        u->full = i * 0x01010101;
        
        /* SUBREG accesses */
        sum += u->halves.low;      /* Access low 16 bits */
        sum += u->halves.high;     /* Access high 16 bits */
        sum += u->bytes[1];        /* Access single byte */
        
        /* Modify through sub-register */
        u->halves.low = (u->halves.low * 3) & 0xFFFF;
    }
    
    return sum;
}

/* Casting between different integer sizes */
unsigned int test_subreg_casts(unsigned int *data, int n) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple casts generating SUBREG */
        uint32_t val32 = data[i];
        uint16_t val16 = (uint16_t)(val32 & 0xFFFF);
        uint8_t val8 = (uint8_t)(val32 >> 24);
        
        /* Operations on sub-parts */
        checksum += val16;
        checksum += val8 << 8;
        
        /* Reconstruct with SUBREG */
        data[i] = (val32 & 0xFFFF0000) | (val16 ^ 0xAAAA);
    }
    
    return checksum;
}

/* ========== COMPLEX MEMORY ADDRESSING ========== */

/* Array with complex indexing */
unsigned int test_complex_addressing(int *array, int *indices, int n) {
    unsigned int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex address calculation */
        int idx1 = indices[i] & 0xFF;
        int idx2 = (indices[i] >> 8) & 0xFF;
        
        /* Memory operations with addressing modes */
        int val1 = array[idx1 * 2];
        int val2 = array[idx2 * 2 + 1];
        
        /* Modify and store back */
        array[idx1 * 2] = val1 ^ val2;
        array[idx2 * 2 + 1] = val1 & val2;
        
        sum += val1 + val2;
    }
    
    return sum;
}

/* Structure with mixed types */
struct MixedData {
    int a;
    short b;
    char c;
    int d;
};

unsigned int test_struct_addressing(struct MixedData *data, int count) {
    unsigned int total = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access structure members with different sizes */
        data[i].a = data[i].a * 2 + i;
        data[i].b = (short)(data[i].b ^ 0x55AA);
        data[i].c = ~data[i].c;
        
        /* Complex addressing for next element */
        total += data[i].a + data[i].b + data[i].c;
    }
    
    return total;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    /* Use argc to prevent constant folding */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Initialize test data */
    unsigned int bitfield_data = g_volatile_seed;
    struct BitFieldStruct bf_struct = {0};
    bf_struct.field4 = 0x12345678;
    
    char char_buffer[256];
    short short_array[128];
    volatile short volatile_short;
    
    union TypePun pun_union;
    int int_array[256];
    int indices[128];
    
    struct MixedData mixed_array[50];
    
    /* Initialize arrays with non-constant data */
    for (int i = 0; i < 256; i++) {
        char_buffer[i] = (char)(i ^ 0x37);
        if (i < 128) {
            short_array[i] = (short)(i * 3);
            indices[i] = (i * 7) & 0xFFFF;
        }
        int_array[i] = i * 0x01010101;
        if (i < 50) {
            mixed_array[i].a = i * 2;
            mixed_array[i].b = (short)(i * 3);
            mixed_array[i].c = (char)i;
            mixed_array[i].d = i * 4;
        }
    }
    
    unsigned int final_result = 0;
    
    /* Test ZERO_EXTRACT patterns */
    final_result += test_zero_extract_bitfield(bitfield_data, 3);
    final_result += test_zero_extract_struct(&bf_struct, iterations % 50);
    final_result += test_zero_extract_explicit(0x89ABCDEF);
    
    /* Test STRICT_LOW_PART patterns */
    final_result += test_strict_low_part_chars(char_buffer, 64);
    final_result += test_strict_low_part_mixed(short_array, char_buffer, 32);
    final_result += test_strict_low_part_volatile(&volatile_short, iterations % 20);
    
    /* Test SUBREG patterns */
    final_result += test_subreg_union(&pun_union, iterations % 30);
    final_result += test_subreg_casts(int_array, 64);
    
    /* Test complex addressing */
    final_result += test_complex_addressing(int_array, indices, 32);
    final_result += test_struct_addressing(mixed_array, 25);
    
    /* Mix everything together */
    final_result ^= (final_result >> 16);
    final_result ^= (final_result >> 8);
    
    /* Ensure result is used */
    printf("Final checksum: %u\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
