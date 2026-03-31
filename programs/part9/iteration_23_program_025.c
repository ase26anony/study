/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent optimization */
volatile int control = 0;

/* ==================== Bit-field patterns for ZERO_EXTRACT ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    volatile unsigned int mode:3;
    unsigned int padding:18;
};

struct NestedBitField {
    struct {
        volatile unsigned int low:4;
        volatile unsigned int high:4;
    } nibbles;
    volatile unsigned int full:8;
};

void test_zero_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {1, 512, 7, 0};
    
    /* Basic bit-field assignments */
    s1.flag = 1;
    s1.value = 255;
    s1.mode = 3;
    
    /* Cross assignments between bit-fields */
    s2.flag = s1.flag;
    s2.value = s1.value + 100;
    
    /* Complex expression with bit-fields */
    volatile unsigned int temp = (s1.flag << 1) | (s1.mode << 2);
    s2.mode = temp & 0x7;
    
    /* Nested bit-field structure */
    struct NestedBitField nbf = {{1, 2}, 0xFF};
    nbf.nibbles.low = nbf.full & 0x0F;
    nbf.nibbles.high = (nbf.full >> 4) & 0x0F;
    
    /* Prevent dead code elimination */
    control += s1.flag + s2.value + nbf.nibbles.low;
}

/* ==================== Partial register patterns for STRICT_LOW_PART ==================== */

void test_strict_low_part(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 1000;
    
    /* Casts to smaller types */
    vs1 = (short)vi + 50;
    vc1 = (char)(vi & 0xFF);
    
    /* Arithmetic on sub-word types */
    vs2 = vs1 * 2;
    vc2 = vc1 + 1;
    
    /* Mixed-size operations */
    vi = (int)vs1 + (int)vc1;
    
    /* Loop with partial register updates */
    for (volatile char i = 0; i < 10; i++) {
        vc1 = i * 2;
        vs1 = i * 100;
    }
    
    /* Prevent dead code elimination */
    control += vs1 + vs2 + vc1 + vc2;
}

/* ==================== Sub-register patterns for SUBREG ==================== */

/* Vector extension for SUBREG patterns */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Packed structure for SUBREG */
struct PackedStruct {
    char a;
    short b;
    int c;
} __attribute__((packed));

union TypePunning {
    float f;
    int i;
    char bytes[4];
};

void test_subreg(void) {
    /* Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    
    /* Individual vector element access */
    volatile int elem = vec3[2];
    vec1[0] = elem * 2;
    
    /* Mixed vector types */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    volatile short s_elem = short_vec[3];
    
    /* Packed structure access */
    struct PackedStruct ps;
    ps.a = 0x12;
    ps.b = 0x3456;
    ps.c = 0x789ABCDE;
    
    volatile int packed_val = ps.c;
    ps.b = (short)(packed_val >> 8);
    
    /* Type punning through union */
    union TypePunning tp;
    tp.f = 3.14159f;
    volatile int int_from_float = tp.i;
    tp.bytes[2] = 0xFF;
    
    /* Prevent dead code elimination */
    control += elem + s_elem + packed_val + int_from_float;
}

/* ==================== Combined patterns ==================== */

struct Combined {
    volatile unsigned int bits:5;
    volatile short half;
    volatile char byte;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.bits = 0x1F;
    c.half = (short)c.bits * 2;
    
    /* Partial register to bit-field */
    c.byte = 0x7F;
    c.bits = c.byte & 0x1F;
    
    /* Complex nested expression */
    volatile int temp = ((int)c.half << 8) | c.byte;
    c.bits = (temp >> 3) & 0x1F;
    
    /* Loop with combined operations */
    for (volatile int i = 0; i < 5; i++) {
        c.bits = i & 0x1F;
        c.half = (short)(c.bits * 100);
        c.byte = (char)(c.half >> 2);
    }
    
    /* Prevent dead code elimination */
    control += c.bits + c.half + c.byte;
}

/* ==================== Architecture-specific patterns ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate partial register ops */
    asm volatile (
        "movw $0x1234, %%ax\n\t"
        "movb $0x56, %%ah\n\t"
        "movw %%ax, %0"
        : "=r" (result)
        :
        : "%ax"
    );
    
    /* Bit manipulation builtins */
    volatile unsigned int x = 0x12345678;
    volatile int count = __builtin_popcount(x);
    volatile int parity = __builtin_parity(x);
    
    control += result + count + parity;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int result;
    
    /* ARM-specific operations */
    asm volatile (
        "uxth %0, %1\n\t"
        : "=r" (result)
        : "r" (0x12345678)
    );
    
    control += result;
}
#endif

/* ==================== Main test driver ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_zero_extract,
    test_strict_low_part,
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
    int i;
    
    /* Use command line arguments to control which tests run */
    int start_test = 0;
    int end_test = sizeof(test_functions)/sizeof(test_functions[0]) - 2; /* Exclude NULL */
    
    if (argc > 1) {
        start_test = atoi(argv[1]) % (end_test + 1);
    }
    if (argc > 2) {
        end_test = atoi(argv[2]) % (end_test + 1);
        if (end_test < start_test) {
            int temp = start_test;
            start_test = end_test;
            end_test = temp;
        }
    }
    
    printf("Running tests %d to %d\n", start_test, end_test);
    
    /* Run selected tests */
    for (i = start_test; i <= end_test && test_functions[i] != NULL; i++) {
        test_functions[i]();
    }
    
    /* Final computation to ensure program does something */
    volatile int final_result = control;
    
    /* Use the result to prevent optimization */
    if (final_result > 1000000) {
        printf("Unexpected large result: %d\n", final_result);
    } else {
        printf("Test completed. Control sum: %d\n", final_result);
    }
    
    return 0;
}
