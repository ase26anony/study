/* test_resource_patterns.c
 * 
 * This program generates RTL patterns to test GCC's resource tracking:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates
 * - SUBREG for register sub-parts
 * - Complex memory references
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int shift) {
    /* Multiple bit-field extracts that should generate ZERO_EXTRACT */
    unsigned int result = 0;
    
    /* Pattern 1: Explicit mask and shift */
    result += (x >> shift) & 0x1F;  /* Extract 5 bits */
    
    /* Pattern 2: Multiple extracts with different widths */
    result += (x >> 8) & 0xFF;      /* Extract byte */
    result += (x >> 16) & 0x7F;     /* Extract 7 bits */
    result += (x >> 24) & 0x3;      /* Extract 2 bits */
    
    /* Pattern 3: Variable width extract */
    unsigned int mask = (1 << (shift & 0x7)) - 1;
    result += (x >> (shift & 0x1F)) & mask;
    
    return result;
}

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignments and comparisons */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x7F;
    s->field3 = (val >> 12) & 0x7;
    
    /* Bit-field comparisons */
    if (s->field1 == 0x10) result += 1;
    if (s->field2 > 0x20) result += 2;
    if (s->field3 != 0) result += 4;
    
    /* Combine bit-fields */
    result += (s->field1 << 16) | (s->field2 << 8) | s->field3;
    
    return result;
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part(void) {
    unsigned int result = 0;
    volatile int i;
    
    /* Pattern 1: char operations that promote to int */
    for (i = 0; i < 100; i++) {
        char c = (char)(g_volatile_seed + i);
        short s = (short)(g_volatile_seed * i);
        
        /* These should generate partial register writes */
        unsigned int temp = c;      /* Zero-extend char to int */
        temp = s;                   /* Zero-extend short to int */
        
        /* Modify only lower parts */
        c = (temp & 0xFF) + i;
        s = (temp & 0xFFFF) * 2;
        
        result += c + s;
    }
    
    /* Pattern 2: Pointer to volatile short */
    unsigned int data = g_volatile_seed;
    volatile short *ptr = (volatile short *)&data;
    *ptr = (short)(data & 0xFFFF);  /* Partial write to memory */
    
    /* Pattern 3: Inline assembly for byte register (x86 specific) */
    #if defined(__i386__) || defined(__x86_64__)
    unsigned char byte_var = 0;
    unsigned int int_var = 0x12345678;
    
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(byte_var)            /* "q" selects byte register */
        : "r"((unsigned char)(int_var & 0xFF))
        : "cc"
    );
    result += byte_var;
    #endif
    
    return result;
}

/* Function with small integer parameters */
unsigned int test_partial_args(unsigned char a, unsigned short b) {
    /* Parameters arrive in partial registers */
    unsigned int result = a;
    
    /* Modify partial register */
    a = a + 1;
    b = b * 2;
    
    result += a + b;
    
    /* Cast to smaller type forces partial representation */
    unsigned short s = (unsigned short)(result & 0xFFFF);
    unsigned char c = (unsigned char)(result & 0xFF);
    
    return s + c;
}

/* ========== SUBREG PATTERNS ========== */

/* Union for type-punning */
union type_pun {
    uint32_t i;
    uint16_t s[2];
    uint8_t  b[4];
};

unsigned int test_subreg_union(union type_pun *u) {
    unsigned int result = 0;
    
    /* Access different views of the same register */
    u->i = g_volatile_seed;
    
    /* SUBREG accesses to parts of the register */
    result += u->s[0];      /* Low 16 bits */
    result += u->s[1];      /* High 16 bits */
    result += u->b[0];      /* Byte 0 */
    result += u->b[3];      /* Byte 3 */
    
    /* Modify through subreg view */
    u->s[1] = (result & 0xFFFF);
    u->b[2] = (result >> 8) & 0xFF;
    
    return result + u->i;
}

/* Casting between integer sizes */
unsigned int test_subreg_casts(unsigned int x) {
    unsigned int result = 0;
    
    /* Multiple casts create SUBREG operations */
    uint16_t s1 = (uint16_t)(x & 0xFFFF);
    uint16_t s2 = (uint16_t)((x >> 16) & 0xFFFF);
    uint8_t  b1 = (uint8_t)(x & 0xFF);
    
    /* Operations on subregs */
    result = s1 * s2;
    result += (unsigned int)b1 << 8;
    
    /* Packed structure simulation */
    struct packed {
        uint16_t a;
        uint16_t b;
    } __attribute__((packed));
    
    /* Force subreg access through pointer casting */
    uint32_t combined = (s2 << 16) | s1;
    uint16_t *ptr = (uint16_t *)&combined;
    result += ptr[0] + ptr[1];
    
    return result;
}

/* ========== COMPLEX MEMORY REFERENCES ========== */

/* Array operations with complex addressing */
unsigned int test_complex_memory(unsigned int *array, int size) {
    unsigned int result = 0;
    volatile int i, j;
    
    /* Complex addressing modes */
    for (i = 0; i < size; i++) {
        for (j = 0; j < 4; j++) {
            /* Array access with index calculation */
            unsigned int val = array[(i * 4 + j) % size];
            
            /* Combine with bit-field extract */
            result += (val >> (j * 8)) & 0xFF;
            
            /* Partial write back */
            array[(i * 2 + j) % size] = (result & 0xFFFF);
        }
    }
    
    /* Pointer arithmetic with type punning */
    uint16_t *short_ptr = (uint16_t *)array;
    for (i = 0; i < size * 2; i++) {
        result += short_ptr[i];  /* SUBREG access through memory */
    }
    
    return result;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data */
    unsigned int test_array[64];
    union type_pun u;
    struct bitfield_struct bf = {0};
    
    /* Fill array with pseudo-random data */
    for (int i = 0; i < 64; i++) {
        test_array[i] = g_volatile_seed * i + 1;
    }
    
    /* Test 1: ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    for (int i = 0; i < 10; i++) {
        final_result ^= test_zero_extract_int(test_array[i], i & 0x1F);
        final_result += test_zero_extract_struct(&bf, test_array[i]);
    }
    
    /* Test 2: STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    final_result += test_strict_low_part();
    final_result += test_partial_args(0xAB, 0x1234);
    
    /* Test 3: SUBREG patterns */
    printf("Testing SUBREG patterns...\n");
    final_result += test_subreg_union(&u);
    for (int i = 0; i < 10; i++) {
        final_result += test_subreg_casts(test_array[i]);
    }
    
    /* Test 4: Complex memory references */
    printf("Testing complex memory references...\n");
    final_result += test_complex_memory(test_array, 64);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %u\n", final_result);
    
    /* Return non-zero if all tests produced some result */
    return (final_result != 0) ? 0 : 1;
}
