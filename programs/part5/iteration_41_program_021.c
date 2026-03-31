/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization */
volatile int g_volatile = 0;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int y) {
    unsigned int sum = 0;
    
    /* Multiple bit-field extractions with different widths */
    sum += (x >> 3) & 0x1F;        /* Extract 5 bits */
    sum += (y >> 8) & 0xFF;        /* Extract 8 bits */
    sum += (x >> 16) & 0x7;        /* Extract 3 bits */
    sum += (y >> 0) & 0x3;         /* Extract 2 bits */
    
    /* Combined mask and shift (common ZERO_EXTRACT pattern) */
    sum += ((x & 0xFF00) >> 8);    /* Extract middle byte */
    sum += ((y & 0xF0F0) >> 4);    /* Extract scattered bits */
    
    return sum;
}

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int a : 5;
    unsigned int b : 8;
    unsigned int c : 3;
    unsigned int d : 16;
};

unsigned int test_zero_extract_struct(struct BitFieldStruct *s, unsigned int val) {
    unsigned int sum = 0;
    
    /* Bit-field assignments (can generate ZERO_EXTRACT in SET_DEST) */
    s->a = val & 0x1F;
    s->b = (val >> 5) & 0xFF;
    s->c = (val >> 13) & 0x7;
    
    /* Bit-field comparisons */
    if (s->a == 0x10) sum += 1;
    if (s->b == 0x55) sum += 2;
    if (s->c == 0x3) sum += 3;
    
    /* Complex bit-field expression */
    sum += (s->d & 0xFF) | ((s->b & 0x7) << 8);
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part_chars(unsigned int iterations) {
    volatile char c = 0;
    volatile short s = 0;
    unsigned int sum = 0;
    
    for (unsigned int i = 0; i < iterations; i++) {
        /* These assignments often generate STRICT_LOW_PART */
        c = (i & 0xFF);           /* Only modifies low 8 bits */
        s = (i & 0xFFFF);         /* Only modifies low 16 bits */
        
        /* Use results to prevent elimination */
        sum += c + s;
        
        /* Pointer to volatile short (may generate partial store) */
        volatile short *ps = &s;
        *ps = (i * 2) & 0xFFFF;
        sum += *ps;
    }
    
    return sum;
}

/* Inline assembly for byte register operations */
unsigned int test_strict_low_part_asm(unsigned int x) {
    unsigned char result;
    
    /* Inline assembly that modifies only part of a register */
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=q" (result)          /* "q" constraint selects byte register */
        : "r" ((unsigned char)(x & 0xFF))
        : "cc"
    );
    
    return result;
}

/* Function with small integer parameters */
unsigned int test_strict_low_part_args(unsigned short a, unsigned char b) {
    unsigned int sum = 0;
    
    /* Modifying parameters (may be in registers) */
    a = a + 1;          /* Partial update of register */
    b = b * 2;          /* Partial update of register */
    
    sum = a + b;
    
    /* Array of shorts with volatile store */
    volatile short arr[4];
    for (int i = 0; i < 4; i++) {
        arr[i] = (a + i) & 0xFFFF;  /* STRICT_LOW_PART store */
        sum += arr[i];
    }
    
    return sum;
}

/* ========== SUBREG patterns ========== */

/* Union for type-punning */
unsigned int test_subreg_union(unsigned int x) {
    union {
        uint32_t i;
        uint16_t s[2];
        uint8_t b[4];
    } u;
    
    u.i = x;
    
    /* Access sub-parts (generates SUBREG) */
    unsigned int sum = u.s[0] + u.s[1];
    sum += u.b[0] + u.b[1] + u.b[2] + u.b[3];
    
    /* Modify through sub-parts */
    u.s[1] = (u.s[1] & 0xFF) | 0x100;
    sum += u.i;
    
    return sum;
}

/* Casting between different integer sizes */
unsigned int test_subreg_casts(unsigned int x) {
    unsigned int sum = 0;
    
    /* Casts that may generate SUBREG */
    uint16_t s1 = (uint16_t)(x & 0xFFFF);
    uint16_t s2 = (uint16_t)((x >> 16) & 0xFFFF);
    uint8_t b1 = (uint8_t)(x & 0xFF);
    
    sum = s1 + s2 + b1;
    
    /* More complex casting in expressions */
    sum += (unsigned int)((short)(x & 0x7FFF)) * 2;
    sum += (unsigned int)((signed char)(x & 0x7F)) * 3;
    
    return sum;
}

/* SIMD-like operations using unions */
unsigned int test_subreg_simd(unsigned int a, unsigned int b) {
    /* Pack two 16-bit values into one 32-bit register */
    unsigned int packed = ((a & 0xFFFF) << 16) | (b & 0xFFFF);
    
    /* Extract and process (generates SUBREG) */
    unsigned short hi = (packed >> 16) & 0xFFFF;
    unsigned short lo = packed & 0xFFFF;
    
    return hi * lo + (hi - lo);
}

/* ========== Memory references with complex addresses ========== */

/* Array processing with bit-field operations */
unsigned int test_memory_complex(unsigned int *array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Complex memory address calculation */
        unsigned int val = array[i] + array[(i + 1) % size];
        
        /* Bit-field extraction from memory value */
        sum += (val >> 4) & 0xF;          /* ZERO_EXTRACT */
        sum += (val >> 16) & 0xFF;        /* ZERO_EXTRACT */
        
        /* Store partial result back (may use STRICT_LOW_PART) */
        unsigned short *ptr = (unsigned short *)&array[i];
        *ptr = (val & 0xFFFF);           /* Partial store */
        
        /* Union access through pointer */
        union {
            unsigned int full;
            unsigned short parts[2];
        } *uptr = (union { unsigned int full; unsigned short parts[2]; } *)&array[i];
        
        sum += uptr->parts[0] + uptr->parts[1];  /* SUBREG accesses */
    }
    
    return sum;
}

/* Structure with mixed types and bit-fields */
struct ComplexStruct {
    unsigned int data;
    unsigned short shorts[4];
    unsigned char bytes[8];
    struct {
        unsigned int field1 : 6;
        unsigned int field2 : 10;
        unsigned int field3 : 8;
    } bits;
};

unsigned int test_complex_struct(struct ComplexStruct *cs, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access bit-fields */
        sum += cs->bits.field1;
        sum += cs->bits.field2 << 2;
        sum += cs->bits.field3 * 3;
        
        /* Modify bit-fields */
        cs->bits.field1 = (cs->data >> i) & 0x3F;
        cs->bits.field2 = (cs->data >> (i + 6)) & 0x3FF;
        
        /* Access array elements with type punning */
        unsigned short *sptr = &cs->shorts[i % 4];
        *sptr = (*sptr + i) & 0xFFFF;  /* STRICT_LOW_PART store */
        
        /* Byte access through pointer */
        unsigned char *bptr = &cs->bytes[i % 8];
        *bptr = (*bptr * 2) & 0xFF;    /* STRICT_LOW_PART store */
        
        /* Complex address calculation */
        sum += cs->shorts[(i + 1) % 4] + cs->bytes[(i + 2) % 8];
    }
    
    return sum;
}

/* ========== Main test driver ========== */

int main(int argc, char **argv) {
    unsigned int final_sum = 0;
    
    /* Initialize test data with some randomness */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Test data arrays */
    unsigned int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = rand();
    }
    
    /* Bit-field structure */
    struct BitFieldStruct bfs = {0};
    bfs.d = rand();
    
    /* Complex structure */
    struct ComplexStruct cs = {0};
    cs.data = rand();
    for (int i = 0; i < 4; i++) cs.shorts[i] = rand() & 0xFFFF;
    for (int i = 0; i < 8; i++) cs.bytes[i] = rand() & 0xFF;
    
    /* Run all tests */
    final_sum += test_zero_extract_int(array[0], array[1]);
    final_sum += test_zero_extract_struct(&bfs, array[2]);
    
    final_sum += test_strict_low_part_chars(10);
    final_sum += test_strict_low_part_asm(array[3]);
    final_sum += test_strict_low_part_args(array[4] & 0xFFFF, array[4] >> 16);
    
    final_sum += test_subreg_union(array[5]);
    final_sum += test_subreg_casts(array[6]);
    final_sum += test_subreg_simd(array[7] & 0xFFFF, array[7] >> 16);
    
    final_sum += test_memory_complex(array, 8);
    final_sum += test_complex_struct(&cs, 4);
    
    /* Add volatile to prevent dead code elimination */
    final_sum += g_volatile;
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %u\n", final_sum);
    
    return (int)(final_sum & 0x7FFFFFFF);
}
