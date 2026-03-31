/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ========== Test 1: ZERO_EXTRACT patterns (bit-field operations) ========== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int padding:21;
    volatile unsigned int mode:4;
    unsigned int data:8;
};

struct NestedBitFields {
    struct {
        volatile unsigned int low:3;
        unsigned int high:5;
    } part1;
    struct {
        unsigned int a:2;
        volatile unsigned int b:6;
        unsigned int c:8;
    } part2;
};

void test_zero_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    struct NestedBitFields n = {0};
    
    /* Basic bit-field assignments */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 7;
    s1.data = 42;
    
    /* Cross assignments between bit-fields */
    s2.value = s1.value;
    s2.flag = s1.flag ^ 1;
    
    /* Complex expression with bit-fields */
    unsigned int temp = (s1.value << 3) | s1.mode;
    s2.data = temp & 0xFF;
    
    /* Nested structure bit-field operations */
    n.part1.low = 5;
    n.part2.b = n.part1.low + 2;
    
    /* Bit-field in loop (ensures RTL generation) */
    for (int i = 0; i < 3; i++) {
        s1.mode = (s1.mode + 1) & 0xF;
        n.part2.b = s1.mode;
    }
    
    /* Prevent dead code elimination */
    control = s1.flag | s2.flag | n.part1.low;
}

/* ========== Test 2: STRICT_LOW_PART patterns (partial register updates) ========== */

void test_strict_low_part(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 1000;
    
    /* Explicit casts to smaller types */
    vs1 = (short)vi + 50;
    vc1 = (char)(vi >> 2);
    
    /* Arithmetic on sub-word types */
    vs2 = vs1 * 2;
    vc2 = vc1 + 10;
    
    /* Mixed-size operations */
    short temp = (short)(vi / 3);
    vs1 = temp - 100;
    
    /* Partial updates in expressions */
    vi = (vi & 0xFFFF0000) | (vs1 & 0xFFFF);
    
    /* Loop with partial register updates */
    for (char c = 0; c < 10; c++) {
        vc1 = c * 5;
        vs1 = vc1 + 100;
    }
    
    /* Prevent dead code elimination */
    control = vs1 + vs2 + vc1 + vc2;
}

/* ========== Test 3: SUBREG patterns (sub-register accesses) ========== */

/* Vector extension for SUBREG patterns */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));

/* Packed structure for SUBREG accesses */
struct PackedData {
    int a;
    short b;
    char c;
    int d;
} __attribute__((packed));

union TypePun {
    float f;
    int i;
    short s[2];
};

void test_subreg(void) {
    /* Vector operations */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector element access (triggers SUBREG) */
    int elem1 = vec_int[2];
    short elem2 = vec_short[5];
    float elem3 = vec_float[1];
    
    /* Vector operations with mixing */
    vec_int[0] = elem1;
    vec_short[3] = (short)elem3;
    
    /* Packed structure access */
    struct PackedData pd = {0x12345678, 0x9ABC, 0xDE, 0xFEDCBA98};
    volatile int packed_val = pd.a;
    volatile short packed_short = pd.b;
    
    /* Type punning through union */
    union TypePun pun;
    pun.f = 3.14159f;
    volatile int int_from_float = pun.i;
    volatile short short_from_float = pun.s[0];
    
    /* Mixed-type conversions */
    float f = (float)vec_int[1];
    int i = (int)vec_float[2];
    
    /* Prevent dead code elimination */
    control = elem1 + elem2 + (int)elem3 + packed_val + packed_short;
}

/* ========== Test 4: Combined patterns ========== */

struct Combined {
    volatile unsigned int bits:4;
    volatile short half;
    char byte;
    int full;
};

void test_combined(void) {
    struct Combined c1 = {0}, c2 = {0};
    
    /* Bit-field to partial register */
    c1.bits = 7;
    c2.half = (short)c1.bits * 100;
    
    /* Partial register to bit-field */
    c1.half = 0x1234;
    c2.bits = (c1.half >> 4) & 0xF;
    
    /* Complex expression with all patterns */
    volatile int temp = 0;
    for (int i = 0; i < 4; i++) {
        c1.bits = i;
        c1.half = (short)(c1.bits << 8) | 0x55;
        c1.byte = (char)c1.half;
        temp += c1.byte;
    }
    
    /* Union with bit-field and full register */
    union {
        struct {
            unsigned int low:16;
            unsigned int high:16;
        } bits;
        volatile int full;
    } u;
    
    u.full = 0xDEADBEEF;
    c1.bits = u.bits.low & 0xF;
    c2.half = (short)u.bits.high;
    
    /* Prevent dead code elimination */
    control = c1.bits + c2.bits + c1.half + c2.half + temp;
}

/* ========== Test 5: Architecture-specific patterns ========== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    asm volatile (
        "movw $0x1234, %%ax\n\t"
        "movb $0x56, %%ah\n\t"
        "movw %%ax, %0"
        : "=r" (result)
        : 
        : "ax"
    );
    
    /* Bit test and set */
    unsigned int flags = 0;
    asm volatile (
        "btsl $5, %0"
        : "+r" (flags)
        :
        : "cc"
    );
    
    control = result + flags;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int result;
    
    /* ARM inline assembly with partial register access */
    asm volatile (
        "mov r0, #0x1234\n\t"
        "movt r0, #0x5678\n\t"
        "uxth %0, r0"
        : "=r" (result)
        :
        : "r0"
    );
    
    control = result;
}
#endif

/* ========== Test 6: Builtin functions ========== */

void test_builtins(void) {
    unsigned int x = 0x12345678;
    volatile int result;
    
    /* Builtins that may involve bit extraction */
    result = __builtin_popcount(x);
    result += __builtin_ctz(x);
    result += __builtin_parity(x);
    
    /* Bit field builtins */
    unsigned int y = __builtin_bf_insert(0, 0xFF, 8, 4);
    unsigned int z = __builtin_bf_extract(x, 8, 8);
    
    control = result + y + z;
}

/* ========== Main test driver ========== */

typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_zero_extract,
    test_strict_low_part,
    test_subreg,
    test_combined,
    test_builtins,
#ifdef __i386__
    test_x86_specific,
#endif
#ifdef __arm__
    test_arm_specific,
#endif
    NULL
};

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Use command line or random selection to prevent optimization */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 6;
    } else {
        test_to_run = control % 6;
    }
    
    /* Run all tests or specific one based on input */
    if (test_to_run == 0) {
        /* Run all tests */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    } else {
        /* Run specific test */
        int idx = (test_to_run - 1) % 6;
        if (test_functions[idx] != NULL) {
            test_functions[idx]();
        }
    }
    
    /* Final computation to ensure program does something useful */
    int final_result = control;
    for (int i = 0; i < 100; i++) {
        final_result = (final_result * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Result: %d\n", final_result);
    return final_result == 0 ? 0 : 1;
}
