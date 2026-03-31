/* test_resource_patterns.c
 * 
 * This program generates RTL patterns to test GCC's resource.cc:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates
 * - SUBREG for register sub-parts
 * - Complex memory references
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ==================== ZERO_EXTRACT PATTERNS ==================== */

/* Bit-field structure for ZERO_EXTRACT operations */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 8;
    unsigned int field3 : 3;
    unsigned int field4 : 16;
};

/* Function 1: Explicit bit-field extraction */
unsigned int test_zero_extract_explicit(unsigned int x, unsigned int shift) {
    /* Force ZERO_EXTRACT: mask width < word size */
    unsigned int mask = (1u << 9) - 1;  /* 9-bit mask */
    return (x >> shift) & mask;  /* Should generate ZERO_EXTRACT */
}

/* Function 2: Bit-field structure operations */
unsigned int test_bitfield_struct(struct BitFieldStruct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignment - may generate ZERO_EXTRACT */
    s->field2 = val & 0xFF;
    
    /* Bit-field comparison - may generate ZERO_EXTRACT */
    if (s->field1 == 3) {
        result += 1;
    }
    
    /* Bit-field arithmetic */
    result += s->field3 * 2;
    
    /* Complex bit-field extraction */
    result += (s->field4 >> 4) & 0x0F;  /* Extract 4 bits from middle */
    
    return result;
}

/* Function 3: Multiple bit-field extractions in loop */
unsigned int test_bitfield_loop(unsigned int *array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Different extraction patterns to prevent optimization */
        unsigned int val = array[i] ^ g_volatile_seed;
        
        /* Pattern 1: Extract lower 6 bits */
        sum += val & 0x3F;
        
        /* Pattern 2: Extract middle 10 bits */
        sum += (val >> 8) & 0x3FF;
        
        /* Pattern 3: Extract scattered bits */
        sum += ((val >> 1) & 0x1) + ((val >> 3) & 0x1) + ((val >> 5) & 0x1);
    }
    
    return sum;
}

/* ==================== STRICT_LOW_PART PATTERNS ==================== */

/* Function 4: Partial register updates with small types */
unsigned int test_strict_low_part_chars(char *chars, short *shorts, int count) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < count; i++) {
        /* char operations that may promote to int then write back low part */
        char c = chars[i];
        c = c + g_volatile_seed;  /* Operation in int, then store low 8 bits */
        checksum += c;
        
        /* short operations */
        short s = shorts[i];
        s = s * 3;  /* Operation in int, then store low 16 bits */
        checksum += s;
        
        /* Mixed-size operations */
        int temp = c + s;
        chars[i] = temp & 0xFF;  /* Explicit low-part store */
    }
    
    return checksum;
}

/* Function 5: Volatile pointer writes for STRICT_LOW_PART */
unsigned int test_volatile_partial_writes(volatile short *vshorts, 
                                          volatile char *vchars,
                                          int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Volatile write of partial register */
        *vshorts = (short)(g_volatile_seed + i);
        sum += *vshorts;
        
        /* Multiple volatile partial writes */
        *vchars = (char)(sum & 0xFF);
        sum += *vchars;
        
        /* Prevent loop unrolling */
        vshorts++;
        vchars++;
    }
    
    return sum;
}

/* Function 6: Inline assembly for byte register operations */
#ifdef __x86_64__
unsigned int test_asm_strict_low_part(unsigned int x) {
    unsigned char result;
    
    /* Inline assembly that operates on byte register */
    __asm__ volatile (
        "movb %%al, %0\n\t"
        : "=q" (result)  /* "q" constraint selects byte register */
        : "a" (x)        /* Input in eax/rax */
        : "cc"
    );
    
    return result;
}
#else
/* Fallback for non-x86 */
unsigned int test_asm_strict_low_part(unsigned int x) {
    return (unsigned char)x;  /* Still generates partial register access */
}
#endif

/* ==================== SUBREG PATTERNS ==================== */

/* Function 7: Union for type-punning (SUBREG generation) */
unsigned int test_union_subreg(unsigned int value) {
    union {
        uint32_t i;
        uint16_t s[2];
        uint8_t  b[4];
    } u;
    
    u.i = value ^ g_volatile_seed;
    
    /* Access sub-parts through union - generates SUBREG */
    unsigned int sum = u.s[0] + u.s[1];
    sum += u.b[0] + u.b[1] + u.b[2] + u.b[3];
    
    /* Cast between sizes */
    uint16_t low = (uint16_t)(u.i & 0xFFFF);
    uint16_t high = (uint16_t)(u.i >> 16);
    
    return sum + low + high;
}

/* Function 8: Packed structure with mixed types */
struct __attribute__((packed)) PackedStruct {
    char a;
    short b;
    char c;
    int d;
    short e;
};

unsigned int test_packed_struct(struct PackedStruct *ps, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access misaligned fields - may generate SUBREG */
        sum += ps[i].b;  /* short access might use SUBREG */
        sum += ps[i].d;  /* int access */
        
        /* Modify and write back */
        ps[i].b = (sum & 0xFFFF) ^ g_volatile_seed;
    }
    
    return sum;
}

/* Function 9: SIMD-like operations using unions */
unsigned int test_simd_subreg(unsigned int *data, int size) {
    union SIMD {
        unsigned int words[4];
        unsigned short halves[8];
        unsigned char bytes[16];
    } vec;
    
    unsigned int sum = 0;
    
    for (int i = 0; i < size && i < 4; i++) {
        vec.words[i] = data[i];
        
        /* Extract elements - generates SUBREG */
        sum += vec.halves[i * 2];
        sum += vec.halves[i * 2 + 1];
        
        /* Byte extraction */
        for (int j = 0; j < 4; j++) {
            sum += vec.bytes[i * 4 + j];
        }
    }
    
    return sum;
}

/* ==================== COMPLEX MEMORY REFERENCES ==================== */

/* Function 10: Complex addressing modes with bit operations */
unsigned int test_complex_memory(unsigned int *base, int *indices, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Complex address calculation */
        unsigned int *ptr = base + indices[i];
        
        /* Bit-field extraction from memory */
        unsigned int val = *ptr;
        unsigned int extracted = (val >> (i % 16)) & ((1u << 8) - 1);
        
        /* Store partial result back */
        *(volatile unsigned short *)((char *)ptr + 2) = (unsigned short)extracted;
        
        result += extracted;
    }
    
    return result;
}

/* Function 11: Mixed patterns in single function */
unsigned int test_mixed_patterns(struct BitFieldStruct *bfs,
                                 unsigned int *array,
                                 short *shorts,
                                 int count) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < count; i++) {
        /* ZERO_EXTRACT pattern */
        unsigned int bits = (array[i] >> bfs[i].field1) & 0x1F;
        
        /* STRICT_LOW_PART pattern */
        short s = shorts[i];
        s = s + bits;  /* Operation in int, store low 16 bits */
        shorts[i] = s;
        
        /* SUBREG pattern through union */
        union { unsigned int i; unsigned short s[2]; } u;
        u.i = array[i];
        checksum += u.s[0] + u.s[1];
        
        /* Complex memory reference */
        checksum += *(volatile unsigned short *)((char *)&array[i] + 2);
    }
    
    return checksum;
}

/* ==================== MAIN TEST DRIVER ==================== */

int main(int argc, char **argv) {
    /* Use argc to prevent constant folding */
    int data_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (data_size > 1000) data_size = 1000;
    if (data_size < 10) data_size = 10;
    
    /* Initialize test data */
    unsigned int *array = malloc(data_size * sizeof(unsigned int));
    short *shorts = malloc(data_size * sizeof(short));
    char *chars = malloc(data_size * sizeof(char));
    struct BitFieldStruct *bfs = malloc(data_size * sizeof(struct BitFieldStruct));
    struct PackedStruct *ps = malloc(data_size * sizeof(struct PackedStruct));
    int *indices = malloc(data_size * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < data_size; i++) {
        array[i] = (i * 1103515245u + 12345) ^ g_volatile_seed;
        shorts[i] = (short)(array[i] & 0xFFFF);
        chars[i] = (char)(array[i] & 0xFF);
        
        bfs[i].field1 = (array[i] >> 0) & 0x1F;
        bfs[i].field2 = (array[i] >> 5) & 0xFF;
        bfs[i].field3 = (array[i] >> 13) & 0x07;
        bfs[i].field4 = (array[i] >> 16) & 0xFFFF;
        
        ps[i].a = chars[i];
        ps[i].b = shorts[i];
        ps[i].c = chars[i] ^ 0x55;
        ps[i].d = array[i];
        ps[i].e = shorts[i] ^ 0xAAAA;
        
        indices[i] = i % (data_size / 2);
    }
    
    unsigned int final_result = 0;
    
    /* Test ZERO_EXTRACT patterns */
    final_result += test_zero_extract_explicit(array[0], 3);
    final_result += test_bitfield_struct(&bfs[0], g_volatile_seed);
    final_result += test_bitfield_loop(array, data_size / 4);
    
    /* Test STRICT_LOW_PART patterns */
    final_result += test_strict_low_part_chars(chars, shorts, data_size / 4);
    
    volatile short vshort = 0;
    volatile char vchar = 0;
    final_result += test_volatile_partial_writes(&vshort, &vchar, 10);
    
    final_result += test_asm_strict_low_part(array[0]);
    
    /* Test SUBREG patterns */
    final_result += test_union_subreg(array[0]);
    final_result += test_packed_struct(ps, data_size / 4);
    final_result += test_simd_subreg(array, 4);
    
    /* Test complex memory references */
    final_result += test_complex_memory(array, indices, data_size / 4);
    
    /* Test mixed patterns */
    final_result += test_mixed_patterns(bfs, array, shorts, data_size / 4);
    
    /* Clean up */
    free(array);
    free(shorts);
    free(chars);
    free(bfs);
    free(ps);
    free(indices);
    
    /* Return result to prevent dead code elimination */
    printf("Final checksum: %u\n", final_result);
    return (final_result > 0) ? 0 : 1;
}
