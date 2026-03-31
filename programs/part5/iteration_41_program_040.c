/* test_resource_patterns.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates
 * - SUBREG for register sub-parts
 * - Complex memory references
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function 1: Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, int shift) {
    /* Multiple bit-field extractions that should generate ZERO_EXTRACT */
    unsigned int result = 0;
    
    /* Pattern 1: Explicit mask and shift */
    result += (x >> shift) & 0x1F;  /* Extract 5 bits */
    
    /* Pattern 2: Multiple extractions with different widths */
    result += (x >> 8) & 0xFF;      /* Extract byte */
    result += (x >> 16) & 0x7;      /* Extract 3 bits */
    result += (x >> 4) & 0x3FF;     /* Extract 10 bits */
    
    /* Pattern 3: Nested extractions */
    unsigned int temp = (x & 0xFF00) >> 8;
    result += (temp & 0xF) << 2;
    
    return result;
}

/* Function 2: Bit-field structure operations */
unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Pattern 1: Bit-field assignment */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x7F;
    
    /* Pattern 2: Bit-field comparison */
    if (s->field3 == 3) {
        result += 100;
    }
    
    /* Pattern 3: Bit-field arithmetic */
    result += s->field1 * 2;
    result += s->field2 << 1;
    
    /* Pattern 4: Complex bit-field expression */
    s->field4 = ((val >> 12) & 0x1FFFF) | (s->field3 << 17);
    
    return result + s->field4;
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Function 3: Partial register updates with small types */
unsigned int test_strict_low_part(short *arr, int size, char *chars) {
    unsigned int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Pattern 1: char to int promotion and partial write-back */
        int temp = chars[i];          /* Load byte, promote to int */
        temp = temp * 2 + 1;          /* Arithmetic in full register */
        chars[i] = temp;              /* Store only low byte - STRICT_LOW_PART */
        
        /* Pattern 2: short operations with volatile */
        volatile short *vs = &arr[i];
        *vs = (*vs + i) & 0x7FFF;     /* Partial 16-bit store */
        
        /* Pattern 3: Multiple partial writes */
        if (i & 1) {
            char c = (char)(temp & 0xFF);
            chars[i] = c;             /* Another byte store */
        }
        
        result += chars[i] + arr[i];
    }
    
    return result;
}

/* Function 4: Inline assembly for partial registers */
unsigned int test_strict_low_part_asm(int x) {
    unsigned int result = x;
    
    /* Pattern: Inline assembly that operates on byte registers */
    #if defined(__i386__) || defined(__x86_64__)
    char byte_val;
    asm volatile (
        "movb %1, %0\n\t"
        "incb %0\n\t"
        : "=q"(byte_val)        /* 'q' = a, b, c, or d byte register */
        : "r"((char)x)
        : "cc"
    );
    result += byte_val;
    #endif
    
    return result;
}

/* ========== SUBREG PATTERNS ========== */

/* Function 5: Union for type-punning (SUBREG generation) */
unsigned int test_subreg_union(unsigned int value) {
    union {
        uint32_t i;
        uint16_t s[2];
        uint8_t b[4];
    } u;
    
    u.i = value;
    
    /* Pattern 1: Access different views of the same register */
    unsigned int result = u.s[0] + u.s[1];  /* SUBREG for 16-bit parts */
    
    /* Pattern 2: Byte access */
    result += u.b[0] << 8;
    result += u.b[3];
    
    /* Pattern 3: Mixed-size operations */
    u.s[1] = (u.s[0] & 0xFF) | 0x100;
    
    return result + u.i;
}

/* Function 6: Casting between integer sizes */
unsigned int test_subreg_casts(int *arr, int size) {
    unsigned int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Pattern 1: 32-bit to 16-bit cast */
        short s = (short)(arr[i] & 0xFFFF);
        
        /* Pattern 2: 16-bit to 32-bit with sign extension */
        int extended = (int)s;  /* May involve SUBREG */
        
        /* Pattern 3: Byte extraction through shifting */
        char c1 = (char)(arr[i] >> 8);
        char c2 = (char)(arr[i] >> 16);
        
        result += extended + c1 + c2;
        
        /* Pattern 4: Partial write through pointer */
        *((volatile short*)&arr[i]) = s + i;  /* STRICT_LOW_PART + SUBREG */
    }
    
    return result;
}

/* ========== COMPLEX MEMORY REFERENCES ========== */

/* Function 7: Complex addressing modes */
unsigned int test_complex_memory(int *base, int index1, int index2) {
    unsigned int result = 0;
    
    /* Pattern 1: Array indexing with multiple dimensions */
    result += base[index1 * 16 + index2];
    result += base[index2 * 8 + index1];
    
    /* Pattern 2: Pointer arithmetic with scaling */
    int *ptr = base + index1;
    for (int i = 0; i < 4; i++) {
        result += ptr[i * 4];  /* Scaled indexing */
    }
    
    /* Pattern 3: Structure pointer chasing */
    struct {
        int a;
        int b;
        short s;
        char c;
    } *sptr = (void*)base;
    
    result += sptr->b;
    sptr->s = (short)result;  /* Partial store */
    
    return result;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data with some randomness */
    int data_size = 100;
    unsigned int *int_data = malloc(data_size * sizeof(unsigned int));
    short *short_data = malloc(data_size * sizeof(short));
    char *char_data = malloc(data_size);
    
    /* Use volatile to prevent compile-time optimization */
    int seed = g_volatile_seed;
    for (int i = 0; i < data_size; i++) {
        int_data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        short_data[i] = (short)(int_data[i] & 0xFFFF);
        char_data[i] = (char)(int_data[i] & 0xFF);
    }
    
    /* Test 1: ZERO_EXTRACT patterns */
    struct bitfield_struct bf;
    bf.field1 = 10;
    bf.field2 = 50;
    bf.field3 = 3;
    bf.field4 = 1000;
    
    final_result += test_zero_extract_int(int_data[0], 3);
    final_result += test_zero_extract_struct(&bf, int_data[1]);
    
    /* Test 2: STRICT_LOW_PART patterns */
    final_result += test_strict_low_part(short_data, data_size / 2, char_data);
    final_result += test_strict_low_part_asm(seed);
    
    /* Test 3: SUBREG patterns */
    final_result += test_subreg_union(int_data[2]);
    final_result += test_subreg_casts(int_data, data_size / 4);
    
    /* Test 4: Complex memory references */
    final_result += test_complex_memory(int_data, seed % 16, (seed * 3) % 16);
    
    /* Mix in array processing to ensure no dead code elimination */
    for (int i = 0; i < data_size; i++) {
        /* Use all test data in final computation */
        final_result ^= int_data[i];
        final_result += short_data[i];
        final_result ^= char_data[i] << ((i % 4) * 8);
    }
    
    /* Clean up */
    free(int_data);
    free(short_data);
    free(char_data);
    
    /* Print result to prevent optimization */
    printf("Final result: %u\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
