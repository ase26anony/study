/* test_resource_patterns.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates
 * - SUBREG for register sub-parts
 * - Complex memory addressing modes
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT Patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int shift) {
    /* Various bit-field extraction patterns */
    unsigned int result = 0;
    
    /* Pattern 1: Explicit mask and shift (common ZERO_EXTRACT) */
    result += (x >> shift) & 0x1F;  /* Extract 5-bit field */
    
    /* Pattern 2: Multiple extractions with different widths */
    result += (x >> 8) & 0xFF;      /* Extract byte */
    result += (x >> 16) & 0x7;      /* Extract 3-bit field */
    result += (x >> 24) & 0x1F;     /* Extract 5-bit field */
    
    /* Pattern 3: Variable width extraction */
    unsigned int mask = (1 << (shift & 7)) - 1;
    result += (x >> (shift & 31)) & mask;
    
    return result;
}

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 8;
    unsigned int field3 : 3;
    unsigned int field4 : 16;
};

unsigned int test_zero_extract_struct(struct bitfield_struct *s, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Bit-field comparisons and arithmetic */
        if (s->field1 == (i & 0x1F)) {
            sum += s->field1;
        }
        
        if (s->field2 > 100) {
            sum += s->field2;
        }
        
        /* Bit-field assignment */
        s->field3 = (i & 0x7);
        sum += s->field3;
        
        /* Complex bit-field expression */
        sum += (s->field4 >> 4) & 0xFFF;
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART Patterns ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part(int base) {
    unsigned int result = 0;
    
    /* Pattern 1: char operations in loop (promotion/demotion) */
    for (int i = 0; i < 100; i++) {
        char c = (char)((base + i) & 0xFF);
        short s = (short)(c * 2);
        
        /* These assignments often generate STRICT_LOW_PART */
        volatile char *vc = &c;
        volatile short *vs = &s;
        *vc = (char)(s & 0x7F);
        *vs = (short)(c * 3);
        
        result += c + s;
    }
    
    /* Pattern 2: Pointer to volatile short */
    volatile short vshort = 0;
    for (int i = 0; i < 50; i++) {
        vshort = (short)((base + i * 2) & 0xFFFF);
        result += vshort;
    }
    
    /* Pattern 3: Function with small integer parameters */
    auto short process_short(short a, short b) {
        /* Local modification of parameter */
        a = (short)(a + b);
        return a;
    }
    
    result += process_short(100, 200);
    
    return result;
}

/* Inline assembly for partial register access */
unsigned int test_strict_low_part_asm(int value) {
    unsigned int result = 0;
    
    /* Byte register operation - may generate STRICT_LOW_PART */
    unsigned char byte_val = 0;
    
    #if defined(__i386__) || defined(__x86_64__)
    asm volatile (
        "movb %1, %0\n\t"
        "addb $1, %0"
        : "=q"(byte_val)
        : "r"((unsigned char)(value & 0xFF))
        : "cc"
    );
    #endif
    
    result += byte_val;
    
    /* Short register operation */
    unsigned short short_val = 0;
    
    #if defined(__i386__) || defined(__x86_64__)
    asm volatile (
        "movw %1, %0\n\t"
        "addw $1, %0"
        : "=r"(short_val)
        : "r"((unsigned short)(value & 0xFFFF))
        : "cc"
    );
    #endif
    
    result += short_val;
    
    return result;
}

/* ========== SUBREG Patterns ========== */

/* Union for type-punning (generates SUBREG) */
unsigned int test_subreg_union(int value) {
    union pun {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } u;
    
    u.full = (uint32_t)value;
    
    /* Access sub-parts through union */
    unsigned int sum = 0;
    sum += u.halves[0];      /* SUBREG for half-word access */
    sum += u.halves[1];
    sum += u.bytes[0];       /* SUBREG for byte access */
    sum += u.bytes[3];
    
    /* Modify through one view, read through another */
    u.halves[1] = (uint16_t)(g_volatile_seed & 0xFFFF);
    sum += u.full;
    
    return sum;
}

/* Casting between integer sizes */
unsigned int test_subreg_casts(int *array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Various casts that may generate SUBREG */
        short s = (short)(array[i] & 0xFFFF);
        char c = (char)(array[i] & 0xFF);
        
        /* Promote back with sign extension */
        int from_short = (int)s;
        int from_char = (int)c;
        
        sum += from_short + from_char;
        
        /* Access through different pointer types */
        int val = array[i];
        short *sp = (short *)&val;
        sum += sp[0] + sp[1];
    }
    
    return sum;
}

/* ========== Complex Memory Addressing ========== */

/* Memory operations with complex addressing */
unsigned int test_complex_memory(int *base_array, int index, int offset) {
    unsigned int sum = 0;
    
    /* Array access with index computation */
    sum += base_array[index];
    sum += base_array[index + 1];
    sum += base_array[index * 2];
    sum += base_array[index + offset];
    
    /* Pointer arithmetic */
    int *ptr = base_array + index;
    for (int i = 0; i < 10; i++) {
        sum += ptr[i * 2];
    }
    
    /* Structure with array member */
    struct with_array {
        int data[8];
        int count;
    } sa;
    
    memcpy(sa.data, base_array, sizeof(sa.data));
    for (int i = 0; i < 8; i++) {
        sum += sa.data[i] * (i + 1);
    }
    
    return sum;
}

/* ========== Combined Test Function ========== */

unsigned int run_all_tests(int seed) {
    unsigned int total = 0;
    
    /* Initialize test data */
    int test_array[64];
    for (int i = 0; i < 64; i++) {
        test_array[i] = seed + i * 3;
    }
    
    struct bitfield_struct bf = {
        .field1 = (seed & 0x1F),
        .field2 = (seed & 0xFF),
        .field3 = ((seed >> 8) & 0x7),
        .field4 = seed * 2
    };
    
    /* Run all tests */
    total += test_zero_extract_int(seed, (seed & 0x7));
    total += test_zero_extract_struct(&bf, 10);
    total += test_strict_low_part(seed);
    total += test_strict_low_part_asm(seed);
    total += test_subreg_union(seed);
    total += test_subreg_casts(test_array, 16);
    total += test_complex_memory(test_array, (seed & 0xF), 4);
    
    return total;
}

/* ========== Main Function ========== */

int main(int argc, char **argv) {
    /* Use command line argument or volatile to prevent constant folding */
    int base_seed = g_volatile_seed;
    if (argc > 1) {
        base_seed += atoi(argv[1]);
    }
    
    /* Run tests multiple times to ensure execution */
    unsigned int final_result = 0;
    for (int iteration = 0; iteration < 3; iteration++) {
        final_result += run_all_tests(base_seed + iteration);
    }
    
    /* Use result to affect return value */
    printf("Final checksum: %u\n", final_result);
    return (final_result & 0xFF);
}
