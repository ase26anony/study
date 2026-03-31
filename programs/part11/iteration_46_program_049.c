/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations alive */
static volatile int sink;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    /* Source variables in registers */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex expressions */
    bf.field4 = (a & 0xF) + (b & 0xF);          /* Should generate ZERO_EXTRACT */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF); /* Complex bitfield store */
    bf.field12 = (c & 0xFFF) | ((a >> 8) & 0xF);      /* Mixed operations */
    
    /* Use bitfield in computation to prevent elimination */
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG operations through type narrowing */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32;
    
    /* Register sources of different sizes */
    register int32_t r32 = 0x12345678;
    register int64_t r64 = 0x9ABCDEF012345678ULL;
    
    /* Narrowing assignments that should create SUBREG */
    v8 = (int8_t)(r32 + 0x100);      /* 32-bit to 8-bit with overflow */
    v16 = (int16_t)(r32 * 2);        /* 32-bit to 16-bit with computation */
    v32 = (int32_t)(r64 >> 16);      /* 64-bit to 32-bit with shift */
    
    /* Arithmetic with implicit narrowing */
    register int16_t r16a = 30000;
    register int16_t r16b = 10000;
    v16 = r16a + r16b;               /* Potential overflow, kept as 16-bit */
    
    /* Prevent dead code elimination */
    sink = v8 + v16 + v32;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int array[64][8] = {0};
    int * restrict ptr = &array[0][0];  /* restrict helps keep address computation */
    
    /* Complex index calculations */
    for (int i = 0; i < 16; i++) {
        register int val = i * 0x1111;  /* Register source */
        
        /* Various complex addressing modes */
        array[i * 3 + 1][i & 7] = val;                     /* 2D with arithmetic */
        *(ptr + i * 8 + (i % 4)) = val >> 8;               /* Pointer arithmetic */
        array[(i << 2) % 64][(i * 7) % 8] = val & 0xFF;    /* Shift and modulo */
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } s = {0};
    
    int * restrict sptr = s.data;
    for (int i = 0; i < 16; i++) {
        register int tmp = i * 0x2222;
        s.data[i * 2] = tmp;                     /* Struct array access */
        *(sptr + i + 5) = tmp >> 4;              /* Pointer to struct member */
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            sum += array[i][j];
        }
    }
    sink = sum + s.header + s.footer;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        unsigned int count : 12;
        int16_t values[16];
        int32_t large_value;
    } combined = {0};
    
    /* Register sources */
    register uint32_t r1 = 0x89ABCDEF;
    register uint32_t r2 = 0x13579BDF;
    register int32_t r3 = 0x2468ACE0;
    
    /* Combined assignment: bitfield + array with complex index */
    combined.flags = (r1 & 0xFF) | ((r2 >> 4) & 0xF0);
    combined.count = (r1 >> 8) & 0xFFF;
    
    /* Array store with narrowing and complex index */
    for (int i = 0; i < 8; i++) {
        register int32_t src = r3 + i * 0x1000;
        combined.values[i * 2 + 1] = (int16_t)(src >> 2);  /* SUBREG + complex address */
    }
    
    /* Complex memory store to large_value */
    int *ptr = &combined.large_value;
    *(ptr + ((r1 & 1) ? 0 : 0)) = r2;  /* Conditional but predictable address */
    
    sink = combined.flags + combined.count + combined.values[0] + combined.large_value;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_asm(void) {
    int array[32] = {0};
    int index = 0;
    
    /* Complex addressing in asm output */
    for (int i = 0; i < 8; i++) {
        register int val = i * 0x3333;
        index = (i * 5 + 3) & 31;
        
        /* Inline asm with memory output and complex addressing */
        asm volatile (
            "# Force complex memory store\n"
            : "=m" (array[index])   /* Complex address calculation */
            : "r" (val)             /* Value in register */
            : "memory"
        );
    }
    
    /* Bitfield-like operation via asm */
    volatile uint32_t bit_target = 0;
    uint32_t bit_value = 0xAA55;
    
    asm volatile (
        "# Suggest bitfield operation\n"
        : "=m" (bit_target)
        : "r" (bit_value)
        : "memory"
    );
    
    /* Compute sum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += array[i];
    }
    sink = sum + bit_target;
}

int main(void) {
    int total = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    total += sink;
    
    test_subreg_operations();
    total += sink;
    
    test_complex_addressing();
    total += sink;
    
    test_combined_patterns();
    total += sink;
    
    test_inline_asm();
    total += sink;
    
    /* Final output to prevent optimization */
    printf("Result checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
