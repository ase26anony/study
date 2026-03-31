/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ==================== ZERO_EXTRACT patterns (bit-fields) ==================== */

/* Test 1: Basic bit-field operations */
struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    volatile unsigned int pad:5;
    volatile unsigned int mode:3;
};

/* Test 2: Mixed volatile/non-volatile bit-fields */
struct MixedBitField {
    unsigned int a:4;
    volatile unsigned int b:4;
    unsigned int c:8;
    volatile unsigned int d:8;
};

/* Test 3: Bit-field in union for type-punning */
union BitFieldUnion {
    struct {
        volatile unsigned int low:16;
        volatile unsigned int high:16;
    } parts;
    volatile unsigned int whole;
};

void test_zero_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct MixedBitField s2 = {0};
    union BitFieldUnion u1 = {0};
    
    /* Direct assignments to bit-fields */
    s1.flag = 1;
    s1.value = 511;  /* Max for 10 bits */
    s1.mode = 3;
    
    /* Cross assignments between bit-fields */
    unsigned int temp = s1.value;
    s1.pad = temp & 0x1F;
    
    /* Complex expression with bit-field */
    s1.value = (s1.flag ? 100 : 200) + g_volatile_int;
    
    /* Mixed bit-field operations */
    s2.a = 5;
    s2.b = s2.a + 1;
    s2.c = s2.b * 2;
    s2.d = s2.c | 0xF0;
    
    /* Union-based bit-field access */
    u1.whole = 0x12345678;
    u1.parts.low = u1.parts.high & 0xFF;
    u1.parts.high = (u1.parts.low << 8) | 0xAA;
    
    /* Prevent dead code elimination */
    g_volatile_int = s1.value + s2.d + u1.parts.low;
}

/* ==================== STRICT_LOW_PART patterns (partial registers) ==================== */

void test_strict_low_part(void) {
    volatile short vs;
    volatile char vc;
    int i;
    
    /* Explicit casts to smaller types */
    i = g_volatile_int;
    vs = (short)(i + 1000);
    vc = (char)(vs * 2);
    
    /* Arithmetic on sub-word types */
    volatile short vs1 = 100, vs2 = 200;
    vs = vs1 + vs2 - 50;
    
    volatile char vc1 = 10, vc2 = 20;
    vc = vc1 * vc2 / 5;
    
    /* Compound assignment on partial types */
    vs += 100;
    vc -= 5;
    
    /* Mixed-size operations */
    vs = (short)((int)vc * 256 + 128);
    
    /* Prevent dead code elimination */
    g_volatile_short = vs;
    g_volatile_char = vc;
}

/* ==================== SUBREG patterns (sub-register accesses) ==================== */

/* Vector extension for SUBREG patterns */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));
typedef char v16qi __attribute__ ((vector_size (16)));

/* Packed structure for SUBREG */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
    char d;
};

/* Union for type-punning */
union TypePun {
    float f;
    int i;
    short s[2];
    char c[4];
};

void test_subreg(void) {
    /* Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v8hi v2 = {10, 20, 30, 40, 50, 60, 70, 80};
    v16qi v3 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    
    /* Vector element access (triggers SUBREG) */
    int elem1 = v1[2];
    short elem2 = v2[5];
    char elem3 = v3[10];
    
    /* Vector operations with mixing */
    v1[0] = elem1 + elem2;
    v2[3] = (short)(elem3 * 2);
    
    /* Packed structure access */
    struct PackedStruct ps;
    ps.a = 1;
    ps.b = 0x12345678;
    ps.c = 0x9ABC;
    ps.d = 0xDE;
    
    /* Access misaligned members (triggers SUBREG) */
    int b_val = ps.b;  /* May be unaligned access */
    short c_val = ps.c;
    
    /* Union type-punning */
    union TypePun tp;
    tp.f = 3.14159f;
    tp.i = tp.i ^ 0x80000000;  /* Flip sign bit */
    tp.s[0] = (short)(tp.i >> 16);
    tp.c[3] = 0xFF;
    
    /* Float/integer conversions */
    float f = (float)tp.i;
    int i = (int)f;
    short s = (short)i;
    
    /* Prevent dead code elimination */
    g_volatile_int = elem1 + b_val + tp.i;
    g_volatile_short = elem2 + c_val + s;
}

/* ==================== Combined patterns ==================== */

void test_combined_patterns(void) {
    /* Structure with bit-fields */
    struct {
        volatile unsigned int data:12;
        volatile unsigned int control:4;
    } device_reg;
    
    /* Initialize */
    device_reg.data = 0xABC;
    device_reg.control = 0x5;
    
    /* Read bit-field and store to partial register */
    volatile short partial;
    partial = (short)device_reg.data;  /* ZERO_EXTRACT -> SUBREG/STRICT_LOW_PART */
    
    /* Modify and write back */
    device_reg.data = partial + 0x100;
    
    /* Use vector as intermediate */
    v4si vec = {0};
    vec[0] = device_reg.control;
    vec[1] = partial;
    
    /* Complex expression combining patterns */
    device_reg.data = (vec[0] << 8) | (vec[1] & 0xFF);
    
    /* Union with bit-field */
    union {
        struct {
            volatile unsigned int low:8;
            volatile unsigned int high:8;
        } bytes;
        volatile unsigned short word;
    } converter;
    
    converter.word = 0x1234;
    device_reg.data = converter.bytes.high;
    converter.bytes.low = device_reg.data & 0xFF;
    
    /* Prevent dead code elimination */
    g_volatile_int = device_reg.data + converter.word + vec[2];
}

/* ==================== Architecture-specific patterns ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    /* Inline assembly that might generate partial register ops */
    int result;
    short sresult;
    
    /* Assembly with byte/word register outputs */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "movb $0x56, %%ah\n\t"
        "movw %%ax, %0\n\t"
        : "=r" (sresult)
        :
        : "%ax"
    );
    
    /* Bit test and set */
    unsigned int value = g_volatile_int;
    __asm__ volatile (
        "btsl $8, %0\n\t"
        : "+r" (value)
        :
        : "cc"
    );
    
    g_volatile_short = sresult;
    g_volatile_int = value;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM-specific bit-field operations */
    struct __attribute__((packed)) {
        unsigned int a:5;
        unsigned int b:11;
        unsigned int c:16;
    } bf;
    
    bf.a = 0x1F;
    bf.b = 0x7FF;
    bf.c = bf.a | (bf.b << 5);
    
    /* Use ARM bit-field insert/extract intrinsics if available */
    #ifdef __ARM_FEATURE_BITFIELD
    unsigned int val = 0x12345678;
    unsigned int extracted = __builtin_arm_bfx(val, 4, 8);
    unsigned int inserted = __builtin_arm_bfi(val, 0xAA, 8, 8);
    g_volatile_int = extracted + inserted;
    #endif
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
    int test_num = 0;
    
    /* Use command-line argument or volatile to control test selection */
    if (argc > 1) {
        test_num = atoi(argv[1]) % 5;  /* Limit to available tests */
    } else {
        test_num = g_volatile_int % 5;
    }
    
    printf("Running resource tracking tests...\n");
    
    /* Run all tests or specific one based on input */
    if (test_num == 0) {
        /* Run all tests */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    } else {
        /* Run specific test */
        int idx = (test_num - 1) % 4;
        if (test_functions[idx] != NULL) {
            test_functions[idx]();
        }
    }
    
    /* Final computation to ensure program does something useful */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    printf("Final result: %d\n", result);
    
    /* Use result to affect control flow */
    if (result > 1000) {
        printf("High result detected\n");
    }
    
    return result & 0xFF;  /* Return non-zero exit code */
}
