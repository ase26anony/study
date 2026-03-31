/* test_rtl_patterns.c - Program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ==================== BIT-FIELD PATTERNS (ZERO_EXTRACT) ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int padding:21;
    volatile unsigned int mode:4;
    unsigned int data:8;
};

struct NestedBitField {
    struct {
        volatile unsigned int low:3;
        unsigned int high:5;
    } part;
    volatile unsigned int full:16;
};

void test_bitfield_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {1, 511, 0, 7, 255};
    
    /* Basic bit-field assignments - should generate ZERO_EXTRACT */
    s1.flag = s2.flag;
    s1.value = s2.value + control;
    s1.mode = s2.mode | 0x3;
    s1.data = s2.data & 0xF;
    
    /* Cross-structure bit-field operations */
    unsigned int temp = s1.value;
    s2.value = temp >> 1;
    
    /* Complex expression with bit-fields */
    s1.flag = (s1.value > 100) && (s2.mode < 5);
    
    /* Nested bit-field structure */
    struct NestedBitField nbf;
    nbf.part.low = 5;
    nbf.part.high = 20;
    nbf.full = (nbf.part.high << 3) | nbf.part.low;
    
    /* Bit-field in loop - ensures RTL generation */
    for (int i = 0; i < 3; i++) {
        s1.mode = (s1.mode + 1) & 0xF;
        nbf.part.low = (nbf.part.low + i) & 0x7;
    }
}

/* ==================== PARTIAL REGISTER PATTERNS (STRICT_LOW_PART) ==================== */

void test_partial_register(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = control;
    
    /* Assignments to sub-word types - may generate STRICT_LOW_PART */
    vs1 = (short)vi + 100;
    vs2 = (short)(vi * 2);
    vc1 = (char)(vi & 0xFF);
    vc2 = (char)((vi >> 8) & 0xFF);
    
    /* Arithmetic on partial registers */
    vs1 = vs1 + vs2;
    vc1 = vc1 - vc2;
    
    /* Mixed-size operations */
    int result = vs1 * vc1;
    vs2 = (short)(result & 0xFFFF);
    
    /* Conditional partial updates */
    if (vi > 0) {
        vs1 = 1000;
    } else {
        vs1 = -1000;
    }
    
    /* Loop with partial register updates */
    for (char c = 0; c < 10; c++) {
        vc1 = c + vc2;
        vs1 = vs1 + (short)c;
    }
}

/* ==================== SUB-REGISTER PATTERNS (SUBREG) ==================== */

/* Vector type using GCC extension */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

/* Packed structure for SUBREG generation */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
    char d;
};

union TypePunning {
    float f;
    int i;
    unsigned short us[2];
};

void test_subreg(void) {
    /* Vector operations - often generate SUBREG */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    
    /* Vector element access - should generate SUBREG */
    int element = vec3[2] + control;
    vec1[0] = element;
    
    /* Vector type conversion */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    short_vec[3] = (short)element;
    
    /* Packed structure access - misaligned access may use SUBREG */
    struct PackedStruct ps;
    ps.a = 1;
    ps.b = 0x12345678;
    ps.c = 0x9ABC;
    ps.d = 0xDE;
    
    int b_val = ps.b;  /* This may involve SUBREG due to packing */
    
    /* Type punning through union */
    union TypePunning tp;
    tp.f = 3.14159f;
    unsigned short low = tp.us[0];
    unsigned short high = tp.us[1];
    
    tp.i = (high << 16) | low;
    
    /* Float/integer conversions - may involve SUBREG */
    float f = (float)tp.i;
    int i = (int)f;
    
    /* Complex expression with mixed types */
    vec1[1] = (int)(f * 100.0f) + i;
}

/* ==================== COMBINED PATTERNS ==================== */

void test_combined_patterns(void) {
    struct BitFieldStruct bfs;
    volatile short partial;
    v4si vector;
    
    /* Combine bit-field extract with partial register */
    bfs.value = 500;
    partial = (short)bfs.value;  /* ZERO_EXTRACT -> SUBREG/STRICT_LOW_PART */
    
    /* Combine with vector operations */
    vector[0] = bfs.flag ? 100 : 200;
    
    /* Complex nested expression */
    union TypePunning tp;
    tp.f = 1.5f;
    
    bfs.data = (tp.i >> 8) & 0xFF;  /* Multiple extracts possible */
    
    /* Loop combining multiple patterns */
    for (int i = 0; i < 4; i++) {
        bfs.mode = (bfs.mode + i) & 0xF;
        partial = (short)(bfs.mode * 10);
        vector[i] = partial + control;
    }
}

/* ==================== ARCHITECTURE-SPECIFIC PATTERNS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate partial register updates */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "movw %%ax, %0\n\t"
        : "=m" (result)
        : 
        : "%ax"
    );
    
    /* Bit test operations */
    unsigned int value = 0x87654321;
    unsigned char bit3 = (value >> 3) & 1;
    
    /* Byte-wise operations */
    volatile char bytes[4];
    bytes[0] = (value >> 0) & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 24) & 0xFF;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM may have specific patterns for byte/halfword access */
    volatile short halfword;
    volatile char byte;
    int word = 0x12345678;
    
    halfword = (word >> 16) & 0xFFFF;
    byte = (word >> 8) & 0xFF;
    
    /* Use builtins that might generate interesting RTL */
    int leading_zeros = __builtin_clz(word);
    int parity = __builtin_parity(word);
}
#endif

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_extract,
    test_partial_register,
    test_subreg,
    test_combined_patterns,
#ifdef __i386__
    test_x86_specific,
#endif
#ifdef __arm__
    test_arm_specific,
#endif
    NULL
};

int main(int argc, char *argv[]) {
    int num_tests = 0;
    
    /* Use command line argument to control which tests run */
    int test_mask = 0;
    if (argc > 1) {
        test_mask = atoi(argv[1]);
    }
    
    /* Run tests based on mask or all if mask is 0 */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_mask == 0 || (test_mask & (1 << i))) {
            test_functions[i]();
            num_tests++;
        }
    }
    
    /* Ensure some computation happens to prevent dead code elimination */
    int final_result = control + num_tests;
    
    /* Print something to ensure execution */
    printf("Ran %d tests, control=%d, result=%d\n", 
           num_tests, control, final_result);
    
    return final_result > 0 ? 0 : 1;
}
