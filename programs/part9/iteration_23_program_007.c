/* test_resources.c - Program to trigger specific RTL patterns in GCC's resource tracking */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int global_control = 0;
volatile int global_result = 0;

/* ==================== BIT-FIELD PATTERNS (ZERO_EXTRACT) ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    volatile unsigned int mode:3;
    unsigned int padding:18;
};

struct NestedBitFields {
    struct {
        volatile unsigned int a:4;
        volatile unsigned int b:4;
    } nibbles;
    volatile unsigned int full:8;
};

void test_bitfield_zero_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    
    /* Basic bit-field assignments - should generate ZERO_EXTRACT */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 3;
    
    /* Bit-field to bit-field assignment */
    s2.flag = s1.flag;
    s2.value = s1.value + 1;
    s2.mode = s1.mode | 1;
    
    /* Complex expression with bit-fields */
    global_result += s1.flag * 1000;
    global_result += s1.value;
    global_result += s1.mode << 12;
    
    /* Nested bit-field structure */
    struct NestedBitFields nbf;
    nbf.nibbles.a = 5;
    nbf.nibbles.b = 10;
    nbf.full = (nbf.nibbles.a << 4) | nbf.nibbles.b;
    
    global_result += nbf.full;
}

/* Union for bit-field and integer access */
union BitFieldUnion {
    struct {
        volatile unsigned int low:16;
        volatile unsigned int high:16;
    } parts;
    volatile unsigned int whole;
};

void test_bitfield_union(void) {
    union BitFieldUnion u;
    u.whole = 0x12345678;
    
    /* Access through bit-field members - should generate ZERO_EXTRACT */
    unsigned int low_part = u.parts.low;
    unsigned int high_part = u.parts.high;
    
    /* Modify through bit-fields */
    u.parts.low = (low_part + 1) & 0xFFFF;
    u.parts.high = (high_part - 1) & 0xFFFF;
    
    global_result += u.whole;
}

/* ==================== PARTIAL REGISTER PATTERNS (STRICT_LOW_PART) ==================== */

void test_partial_register_strict_low_part(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    
    /* Operations on sub-word types - may generate STRICT_LOW_PART */
    int i = 100;
    vs1 = (short)i + 50;
    vs2 = (short)(i * 2);
    
    /* Arithmetic on partial registers */
    vc1 = (char)(vs1 + vs2);
    vc2 = (char)(vs1 - vs2);
    
    /* Complex expression with partial updates */
    for (int j = 0; j < 4; j++) {
        vs1 = (short)(vs1 + vc1);
        vc2 = (char)(vc2 + j);
    }
    
    global_result += vs1 + vs2 + vc1 + vc2;
    
    /* Array of small types */
    volatile char char_array[8];
    volatile short short_array[4];
    
    for (int i = 0; i < 8; i++) {
        char_array[i] = (char)(i * 7);
    }
    
    for (int i = 0; i < 4; i++) {
        short_array[i] = (short)(char_array[i*2] + char_array[i*2+1]);
        global_result += short_array[i];
    }
}

/* ==================== SUB-REGISTER PATTERNS (SUBREG) ==================== */

/* Vector types for SUBREG patterns */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));
typedef char v16qi __attribute__ ((vector_size (16)));

void test_subreg_patterns(void) {
    /* Vector operations - should generate SUBREG for element access */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    v16qi vec_char = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    
    /* Access vector elements - likely generates SUBREG */
    int elem_int = vec_int[2];
    short elem_short = vec_short[3];
    char elem_char = vec_char[5];
    
    /* Modify vector elements */
    vec_int[1] = elem_int * 2;
    vec_short[4] = (short)(elem_short + elem_char);
    
    /* Vector arithmetic */
    v4si vec_int2 = {5, 6, 7, 8};
    v4si vec_sum = vec_int + vec_int2;
    v4si vec_prod = vec_int * vec_int2;
    
    /* Extract results */
    for (int i = 0; i < 4; i++) {
        global_result += vec_sum[i];
        global_result += vec_prod[i];
    }
    
    /* Type punning through unions - can generate SUBREG */
    union TypePun {
        float f;
        int i;
        short s[2];
        char c[4];
    } pun;
    
    pun.f = 3.14159f;
    global_result += pun.i;
    global_result += pun.s[0] + pun.s[1];
    
    /* Mixed-type operations */
    double d = 2.71828;
    pun.i = (int)d * 100;
    short half = pun.s[1];
    global_result += half;
}

/* ==================== COMBINED PATTERNS ==================== */

struct Combined {
    volatile unsigned int data:24;
    volatile unsigned int tag:8;
};

void test_combined_patterns(void) {
    struct Combined c1 = {0}, c2 = {0};
    
    /* Bit-field to partial register */
    c1.data = 0xABCDEF;
    c1.tag = 0x12;
    
    /* Extract to smaller type - may combine ZERO_EXTRACT and SUBREG/STRICT_LOW_PART */
    volatile short partial = (short)(c1.data >> 8);
    volatile char small = (char)c1.tag;
    
    /* Modify and store back */
    c2.data = (c1.data + partial) & 0xFFFFFF;
    c2.tag = (c1.tag + small) & 0xFF;
    
    /* Complex expression */
    for (int i = 0; i < 3; i++) {
        partial = (short)((c1.data >> (i * 8)) & 0xFF);
        small = (char)((c2.tag + i) & 0xFF);
        global_result += partial * small;
    }
    
    /* Vector with bit-field-like extraction */
    v4si vec = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    for (int i = 0; i < 4; i++) {
        /* Extract lower 16 bits - may involve multiple RTL patterns */
        short low_half = (short)(vec[i] & 0xFFFF);
        short high_half = (short)((vec[i] >> 16) & 0xFFFF);
        global_result += low_half + high_half;
    }
}

/* ==================== ARCHITECTURE-SPECIFIC PATTERNS ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    /* x86-specific patterns that might generate interesting RTL */
    volatile int result;
    
    /* Inline assembly with partial register output */
    asm volatile (
        "movw $0x1234, %%ax\n\t"
        "movw $0x5678, %%bx\n\t"
        "addw %%bx, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (result)
        : 
        : "ax", "bx"
    );
    
    global_result += result;
    
    /* Use builtins that might generate bit operations */
    unsigned int x = 0xF0F0F0F0;
    int count = __builtin_popcount(x);
    int parity = __builtin_parity(x);
    int clz = __builtin_clz(x);
    
    global_result += count + parity + clz;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM-specific patterns */
    volatile int arm_result;
    
    /* ARM inline assembly */
    asm volatile (
        "mov r0, #100\n\t"
        "add r0, r0, #50\n\t"
        "mov %0, r0"
        : "=r" (arm_result)
        :
        : "r0"
    );
    
    global_result += arm_result;
}
#endif

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_zero_extract,
    test_bitfield_union,
    test_partial_register_strict_low_part,
    test_subreg_patterns,
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
    
    /* Use argc to control which tests run, preventing dead code elimination */
    int test_mask = (argc > 1) ? atoi(argv[1]) : 0xFF;
    
    printf("Running resource pattern tests (mask: 0x%02x)\n", test_mask);
    
    /* Run selected tests */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_mask & (1 << i)) {
            test_functions[i]();
            num_tests++;
        }
    }
    
    /* Ensure the result is used */
    printf("Completed %d tests, global_result = %d\n", num_tests, global_result);
    
    /* Simple computation to ensure program is valid */
    int final_check = global_result % 1000;
    if (final_check < 0) final_check = -final_check;
    
    return final_check;
}
