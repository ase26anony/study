/* Comprehensive test for GCC RTL resource tracking patterns */
#include <stdint.h>
#include <string.h>

/* Test 1: Bit-field operations for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int padding:21;
    volatile unsigned int mode:4;
};

struct nested_bitfield {
    struct {
        volatile unsigned int low:8;
        volatile unsigned int high:8;
    } bytes;
    volatile unsigned int full:16;
};

void test_bitfields(void) {
    struct bitfield_struct s1 = {0};
    struct bitfield_struct s2 = {1, 512, 0, 7};
    
    /* Various bit-field assignments */
    s1.flag = s2.flag;
    s1.value = s2.value + 1;
    s1.mode = s2.mode | 0x8;
    
    /* Cross-structure bit-field operations */
    unsigned int temp = s1.value;
    s2.value = temp >> 1;
    
    /* Nested bit-field structure */
    struct nested_bitfield nb;
    nb.bytes.low = 0xAB;
    nb.bytes.high = 0xCD;
    nb.full = (nb.bytes.high << 8) | nb.bytes.low;
    
    /* Complex bit-field expression */
    s1.mode = (s1.flag << 3) | (s2.value & 0x7);
}

/* Test 2: Partial register operations for STRICT_LOW_PART */
void test_partial_registers(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 1000;
    
    /* Direct assignments to partial types */
    vs1 = (short)vi;
    vc1 = (char)(vi + 50);
    
    /* Arithmetic on partial types */
    vs2 = vs1 + (short)100;
    vc2 = vc1 - 25;
    
    /* Mixed-size operations */
    vs1 = (short)(vc1 * 2);
    vi = vs1 + vc2;
    
    /* Pointer dereference with partial types */
    short *ps = &vs1;
    *ps = (short)(*ps + 1);
    
    /* Array with partial types */
    volatile char char_array[4] = {1, 2, 3, 4};
    char_array[2] = char_array[0] + char_array[1];
}

/* Test 3: Sub-register operations for SUBREG */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

union type_pun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    float f;
};

void test_subregisters(void) {
    /* Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    int element = v3[2];  /* SUBREG access */
    
    /* Vector type conversion */
    v8hi vh = {1, 2, 3, 4, 5, 6, 7, 8};
    v4si vs = __builtin_convertvector(vh, v4si);
    
    /* Type punning through union */
    union type_pun pun;
    pun.full = 0x12345678;
    uint16_t low_part = pun.parts.low;  /* SUBREG access */
    pun.parts.high = low_part + 1;
    pun.f = (float)pun.full / 1000.0f;
    
    /* Mixed floating/integer operations */
    volatile float fv = 3.14159f;
    volatile double dv = 2.71828;
    int int_from_float = (int)fv;
    short short_from_double = (short)dv;
    
    /* Packed structure */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        short c;
    } ps;
    
    ps.a = 1;
    ps.b = 0xDEADBEEF;
    ps.c = ps.a + (short)ps.b;  /* SUBREG access */
}

/* Test 4: Combined patterns */
struct combined {
    volatile unsigned int bits:12;
    volatile short half;
    volatile char quarter;
};

void test_combined_patterns(void) {
    struct combined c1, c2;
    
    /* Bit-field to partial register */
    c1.bits = 0xABC;
    c2.half = (short)c1.bits;
    
    /* Partial register to bit-field */
    c1.half = 0x1234;
    c2.bits = c1.half & 0xFFF;
    
    /* Complex nested expression */
    c1.quarter = (char)((c1.bits >> 4) & 0xFF);
    c2.half = (short)(c1.quarter * 2);
    
    /* Loop with combined operations */
    for (int i = 0; i < 4; i++) {
        c1.bits = (c1.bits << 3) | (c2.half & 0x7);
        c2.half = (short)(c1.bits >> (i * 2));
    }
}

/* Test 5: Architecture-specific patterns */
#ifdef __i386__
void test_x86_specific(void) {
    volatile uint32_t dword;
    volatile uint16_t word;
    volatile uint8_t byte;
    
    /* Operations likely to generate partial register updates on x86 */
    dword = 0x12345678;
    word = (uint16_t)dword;
    byte = (uint8_t)(dword >> 16);
    
    /* Inline assembly for explicit partial register access */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movb %%al, %0"
        : "=m" (byte)
        : "m" (word)
        : "ax"
    );
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile uint32_t word;
    volatile uint16_t halfword;
    
    /* ARM often uses STRICT_LOW_PART for 16-bit stores */
    word = 0xA5A5A5A5;
    halfword = (uint16_t)word;
    
    /* Load/store with type conversion */
    word = (uint32_t)halfword | 0xFFFF0000;
}
#endif

/* Test 6: Builtin bit operations */
void test_builtin_bitops(void) {
    volatile unsigned int x = 0x12345678;
    volatile int count;
    
    /* Builtins that may involve bit extraction */
    count = __builtin_popcount(x);
    count += __builtin_ctz(x);
    count += __builtin_clz(x);
    
    /* Parity check */
    volatile int parity = __builtin_parity(x);
    
    /* Bit reversal */
    volatile unsigned int reversed = __builtin_bswap32(x);
    
    /* Extract bits using builtins */
    volatile unsigned int extracted = (x >> 8) & 0xFFF;
}

/* Main test driver */
typedef void (*test_func_t)(void);

static const test_func_t test_functions[] = {
    test_bitfields,
    test_partial_registers,
    test_subregisters,
    test_combined_patterns,
    test_builtin_bitops,
#ifdef __i386__
    test_x86_specific,
#endif
#ifdef __arm__
    test_arm_specific,
#endif
    NULL
};

int main(int argc, char *argv[]) {
    volatile int test_selector = 0;
    
    /* Use command line or volatile to prevent optimization */
    if (argc > 1) {
        test_selector = argv[1][0] - '0';
    }
    
    /* Execute all tests or specific ones based on selector */
    for (int i = 0; test_functions[i] != NULL; i++) {
        if (test_selector == 0 || test_selector == i + 1) {
            test_functions[i]();
        }
    }
    
    /* Ensure program has observable output */
    volatile int result = 0;
    for (int i = 0; test_functions[i] != NULL; i++) {
        result += i;
    }
    
    return result > 0 ? 0 : 1;
}
