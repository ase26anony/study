/* Test program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int global_control = 0;
volatile int global_result = 0;

/* ========== Test 1: Bit-field operations for ZERO_EXTRACT ========== */
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
    } bytes;
    volatile unsigned int full:16;
};

void test_bitfield_operations(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {1, 512, 7, 0};
    
    /* Basic bit-field assignments */
    s1.flag = 1;
    s1.value = 255;
    s1.mode = 3;
    
    /* Cross assignments */
    s2.flag = s1.flag;
    s2.value = s1.value + 1;
    
    /* Complex expression with bit-fields */
    unsigned int temp = (s1.value << 3) | s1.mode;
    s2.value = temp & 0x3FF;
    
    /* Nested bit-field structure */
    struct NestedBitField nbf;
    nbf.bytes.low = 0xF;
    nbf.bytes.high = 0xA;
    nbf.full = (nbf.bytes.high << 8) | nbf.bytes.low;
    
    /* Bit-field in loop */
    for (int i = 0; i < 4; i++) {
        s1.value = (s1.value + i) & 0x3FF;
        nbf.bytes.low = (nbf.bytes.low + 1) & 0xF;
    }
    
    global_result += s1.value + s2.value + nbf.full;
}

/* ========== Test 2: Partial register operations for STRICT_LOW_PART ========== */
void test_partial_register_ops(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 1000;
    
    /* Assignments to partial types */
    vs1 = (short)vi + 50;
    vs2 = (short)(vi * 2);
    
    /* Arithmetic on partial types */
    vc1 = (char)65;
    vc2 = (char)(vc1 + 32);
    
    /* Complex partial register expressions */
    for (int i = 0; i < 10; i++) {
        vs1 = (short)(vs1 + vs2 - i);
        vc1 = (char)(vc1 ^ vc2);
    }
    
    /* Mixed-size operations */
    int result = (int)vs1 + (int)vc1;
    vs2 = (short)(result & 0xFFFF);
    
    /* Pointer to partial type */
    volatile short *ps = &vs1;
    *ps = (short)(*ps + 100);
    
    global_result += vs1 + vs2 + vc1 + vc2;
}

/* ========== Test 3: Sub-register accesses for SUBREG ========== */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

union MixedUnion {
    struct {
        volatile short a;
        volatile char b;
        volatile int c;
    } parts;
    volatile long long whole;
};

void test_subreg_operations(void) {
    /* Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3;
    
    /* Vector element access (triggers SUBREG) */
    for (int i = 0; i < 4; i++) {
        int element = vec1[i];
        vec2[i] = element * 2;
    }
    
    /* Vector arithmetic */
    vec3 = vec1 + vec2;
    
    /* Mixed vector types */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    short_vec[3] = (short)vec1[0];
    
    /* Union with mixed types */
    union MixedUnion mu;
    mu.parts.a = 0x1234;
    mu.parts.b = 0x56;
    mu.parts.c = 0x789ABCD0;
    
    /* Type punning through union */
    mu.whole = mu.whole + 1;
    mu.parts.a = (short)(mu.parts.c >> 16);
    
    /* Float/integer conversions */
    volatile float fv = 3.14159f;
    volatile int *fp = (volatile int*)&fv;
    int float_bits = *fp;
    *fp = float_bits + 1;
    
    global_result += vec1[0] + vec2[1] + vec3[2] + mu.parts.a + float_bits;
}

/* ========== Test 4: Combined patterns ========== */
struct Combined {
    volatile unsigned int field1:5;
    volatile unsigned int field2:11;
    volatile short partial;
    int regular;
};

void test_combined_patterns(void) {
    struct Combined c1, c2;
    
    /* Initialize */
    c1.field1 = 0x1F;
    c1.field2 = 0x7FF;
    c1.partial = 0;
    c1.regular = 1000;
    
    /* Combined bit-field and partial register */
    c1.partial = (short)c1.field2;
    c2.field1 = (unsigned int)c1.partial & 0x1F;
    
    /* Complex nested expression */
    for (int i = 0; i < 5; i++) {
        c1.field1 = (c1.field1 + i) & 0x1F;
        c1.partial = (short)(c1.partial + c1.field1);
        c2.field2 = (unsigned int)c1.partial & 0x7FF;
    }
    
    /* Pointer to bit-field member */
    volatile short *ptr = &c1.partial;
    *ptr = (short)(*ptr + c2.field2);
    
    /* Mixed operations */
    c1.regular = (int)c1.partial * (int)c2.field1;
    
    global_result += c1.field1 + c1.field2 + c1.partial + c1.regular;
}

/* ========== Test 5: Architecture-specific patterns ========== */
void test_arch_specific(void) {
    /* Use builtins that may generate bit manipulation patterns */
    unsigned int x = 0x12345678;
    
    /* Bit counting builtins */
    int popcnt = __builtin_popcount(x);
    int clz = __builtin_clz(x);
    int ctz = __builtin_ctz(x);
    
    /* Parity */
    int parity = __builtin_parity(x);
    
    /* Byte swap */
    unsigned int bswap = __builtin_bswap32(x);
    
    /* Bit field extract/insert (if available) */
    #ifdef __BMI__
    unsigned int extracted = __builtin_ia32_bextr_u32(x, 0x0810);
    #endif
    
    /* Partial register via inline assembly on x86 */
    #ifdef __i386__
    volatile short asm_result;
    __asm__ volatile (
        "movw $0x1234, %0\n\t"
        "addw $0x100, %0"
        : "=r" (asm_result)
        :
        : "cc"
    );
    global_result += asm_result;
    #endif
    
    #ifdef __x86_64__
    volatile char asm_char;
    __asm__ volatile (
        "movb $0x42, %0\n\t"
        "incb %0"
        : "=r" (asm_char)
        :
        : "cc"
    );
    global_result += asm_char;
    #endif
    
    global_result += popcnt + clz + ctz + parity + bswap;
}

/* ========== Main test driver ========== */
typedef void (*test_func_t)(void);

const test_func_t test_functions[] = {
    test_bitfield_operations,
    test_partial_register_ops,
    test_subreg_operations,
    test_combined_patterns,
    test_arch_specific,
    NULL
};

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Use command line or volatile to control execution */
    if (argc > 1) {
        test_to_run = atoi(argv[1]) % 5;
    } else {
        test_to_run = global_control % 5;
    }
    
    /* Run all tests or specific one based on input */
    if (test_to_run == 0) {
        /* Run all tests */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    } else {
        /* Run specific test */
        test_functions[test_to_run - 1]();
    }
    
    /* Ensure the result is used */
    printf("Result: %d\n", global_result);
    
    /* Additional complex expression to ensure RTL generation */
    struct BitFieldStruct final_s;
    final_s.flag = global_result & 1;
    final_s.value = (global_result >> 1) & 0x3FF;
    final_s.mode = (global_result >> 11) & 0x7;
    
    volatile short final_partial = (short)final_s.value;
    v4si final_vec = {final_s.value, final_partial, global_result, 0};
    
    return (final_vec[0] + final_vec[1] + final_vec[2]) % 256;
}
