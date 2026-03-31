/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ========== BIT-FIELD TESTS (ZERO_EXTRACT) ========== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int padding:21;
};

struct NestedBitField {
    struct {
        volatile unsigned int low:4;
        unsigned int high:4;
    } byte;
    unsigned int word:16;
};

void test_bitfield_zero_extract(void) {
    struct BitFieldStruct s1 = {0, 0, 0};
    struct BitFieldStruct s2 = {1, 512, 0};
    
    /* Direct bit-field assignments */
    s1.flag = s2.flag;
    s1.value = s2.value;
    
    /* Complex bit-field expressions */
    unsigned int temp = s1.value;
    s2.value = (temp + 1) & 0x3FF;
    
    /* Cross-structure bit-field operations */
    struct NestedBitField nbf;
    nbf.byte.low = s1.value & 0xF;
    nbf.byte.high = (s1.value >> 4) & 0xF;
    nbf.word = (s1.value << 6) | (s2.value & 0x3F);
    
    /* Bit-field in conditional */
    if (s1.flag) {
        s2.value = nbf.word & 0x3FF;
    }
    
    /* Loop with bit-field updates */
    for (int i = 0; i < 3; i++) {
        s1.value = (s1.value + i) & 0x3FF;
    }
}

/* ========== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ========== */

void test_partial_register_strict_low_part(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 100;
    
    /* Direct assignments to partial types */
    vs1 = (short)vi + 5;
    vc1 = (char)(vi * 2);
    
    /* Arithmetic on partial types */
    vs2 = vs1 + 10;
    vc2 = vc1 - 5;
    
    /* Mixed-size operations */
    int result = vs1 * vc1;
    vs2 = (short)(result & 0xFFFF);
    
    /* Partial register in loop */
    for (short i = 0; i < 10; i++) {
        vs1 = vs1 + i;
        vc1 = vc1 - (char)i;
    }
    
    /* Complex expression with partial types */
    vs1 = (short)((vi * vs2) / (vc1 + 1));
    
    /* Use volatile to force memory operations */
    *(volatile short*)&vs1 = vs2;
    *(volatile char*)&vc1 = vc2;
}

/* ========== SUBREG TESTS ========== */

typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

union MixedAccess {
    uint32_t full;
    struct {
        volatile uint16_t low;
        uint16_t high;
    } parts;
    struct {
        volatile uint8_t b0;
        uint8_t b1;
        uint8_t b2;
        uint8_t b3;
    } bytes;
};

void test_subreg_patterns(void) {
    /* Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3;
    
    /* Vector element access (creates SUBREG) */
    int element = v1[2];
    v1[1] = element + 10;
    
    /* Vector arithmetic */
    v3 = v1 + v2;
    element = v3[0] + v3[3];
    
    /* Mixed vector types */
    v8hi hv1 = {1, 2, 3, 4, 5, 6, 7, 8};
    short selement = hv1[4];
    hv1[5] = selement * 2;
    
    /* Union with different sized accesses */
    union MixedAccess ma;
    ma.full = 0x12345678;
    
    /* Access different parts (creates SUBREG) */
    uint16_t low_part = ma.parts.low;
    uint16_t high_part = ma.parts.high;
    ma.parts.high = low_part;
    ma.parts.low = high_part;
    
    /* Byte access */
    ma.bytes.b0 = ma.bytes.b3;
    ma.bytes.b2 = ma.bytes.b1 + 1;
    
    /* Float/int conversions (often use SUBREG) */
    volatile float fv = 3.14f;
    volatile double dv = 2.71828;
    
    /* Type punning through casts */
    int ifv = *(int*)&fv;
    int64_t idv = *(int64_t*)&dv;
    
    /* Back conversions */
    fv = *(float*)&ifv;
    dv = *(double*)&idv;
}

/* ========== COMBINED PATTERN TESTS ========== */

struct Combined {
    volatile unsigned int bf1:3;
    unsigned int bf2:5;
    volatile short partial;
    char small;
};

void test_combined_patterns(void) {
    struct Combined c1, c2;
    
    /* Bit-field to partial register */
    c1.bf1 = 3;
    c1.partial = (short)c1.bf1 * 100;
    
    /* Partial register to bit-field */
    c2.bf2 = c1.partial & 0x1F;
    
    /* Complex expression mixing patterns */
    c1.small = (char)((c1.bf1 << 3) | c2.bf2);
    c2.partial = (short)c1.small + (c1.partial >> 2);
    
    /* Union with bit-fields and full types */
    union {
        struct {
            volatile unsigned int low_bits:8;
            unsigned int high_bits:8;
        } bf;
        volatile uint16_t full;
    } u;
    
    u.bf.low_bits = c1.small;
    u.bf.high_bits = c2.bf2;
    c1.partial = u.full;
    
    /* Loop with combined operations */
    for (int i = 0; i < 4; i++) {
        c1.bf1 = (c1.bf1 + i) & 0x7;
        c2.partial = (short)(c2.partial + c1.bf1);
        c1.small = (char)(c2.partial & 0xFF);
    }
}

/* ========== ARCHITECTURE-SPECIFIC TESTS ========== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile uint32_t eax_val, ebx_val;
    volatile uint16_t ax_val, bx_val;
    volatile uint8_t al_val, bl_val;
    
    /* Simulate partial register access patterns common on x86 */
    eax_val = 0x12345678;
    ax_val = eax_val & 0xFFFF;  /* Access AX register part */
    al_val = eax_val & 0xFF;     /* Access AL register part */
    
    /* Mix sizes */
    bx_val = ax_val + 0x100;
    bl_val = al_val - 0x10;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $1, %%ax\n\t"
        "movw %%ax, %0\n\t"
        : "=r" (ax_val)
        : "r" (bx_val)
        : "%ax"
    );
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile uint32_t reg;
    volatile uint16_t half;
    volatile uint8_t byte;
    
    /* ARM often uses SUBREG for byte/halfword accesses */
    reg = 0xA5A5A5A5;
    half = (reg >> 16) & 0xFFFF;
    byte = reg & 0xFF;
    
    /* Recombine */
    reg = (half << 8) | byte;
}
#endif

/* ========== BUILTIN FUNCTION TESTS ========== */

void test_builtin_functions(void) {
    unsigned int x = 0x12345678;
    volatile int result;
    
    /* Bit manipulation builtins that may use ZERO_EXTRACT */
    result = __builtin_popcount(x);
    result = __builtin_ctz(x);
    result = __builtin_clz(x);
    result = __builtin_parity(x);
    
    /* Bit field builtins */
    result = __builtin_ffs(x);
    
    /* Extract bits */
    result = (x >> 8) & 0xFF;  /* Manual extraction */
}

/* ========== MAIN TEST DRIVER ========== */

typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_bitfield_zero_extract,
    test_partial_register_strict_low_part,
    test_subreg_patterns,
    test_combined_patterns,
    test_builtin_functions,
#ifdef __i386__
    test_x86_specific,
#endif
#ifdef __arm__
    test_arm_specific,
#endif
};

int main(int argc, char *argv[]) {
    /* Use argc to control which tests run, preventing optimization */
    int test_to_run = control;
    
    if (argc > 1) {
        test_to_run = argv[1][0] - '0';
    }
    
    /* Run all tests in sequence, using volatile to prevent dead code elimination */
    for (int i = 0; i < sizeof(test_functions)/sizeof(test_functions[0]); i++) {
        if (test_to_run == 0 || test_to_run == i + 1) {
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something useful */
    volatile int final_result = 0;
    for (int i = 0; i < 100; i++) {
        final_result += i;
    }
    
    return final_result > 0 ? 0 : 1;
}
