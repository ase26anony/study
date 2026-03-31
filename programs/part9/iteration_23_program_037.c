/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ===== Test 1: Bit-field operations for ZERO_EXTRACT ===== */
struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    unsigned int padding:21;
};

struct NestedBitField {
    struct {
        volatile unsigned int low:4;
        volatile unsigned int high:4;
    } bytes;
    volatile unsigned int full:16;
};

void test_bitfields(void) {
    struct BitFieldStruct s1 = {0};
    struct NestedBitField s2 = {0};
    
    /* Simple bit-field assignments */
    s1.flag = 1;
    s1.value = 511; /* Max for 10 bits */
    
    /* Cross assignments */
    unsigned int temp = s1.value;
    s1.flag = temp & 1;
    
    /* Nested bit-field access */
    s2.bytes.low = 0xF;
    s2.bytes.high = 0xA;
    s2.full = (s2.bytes.high << 4) | s2.bytes.low;
    
    /* Complex expression with bit-fields */
    g_volatile_int = (s1.flag << 10) | s1.value;
}

/* ===== Test 2: Partial register operations for STRICT_LOW_PART ===== */
void test_partial_registers(void) {
    volatile short vs;
    volatile char vc;
    int i = g_volatile_int;
    
    /* Explicit casts to smaller types */
    vs = (short)(i + 100);
    vc = (char)(vs * 2);
    
    /* Arithmetic on sub-word types */
    short s1 = 100;
    short s2 = 200;
    vs = s1 + s2 - 50;
    
    /* Store partial results */
    g_volatile_short = vs;
    g_volatile_char = vc;
    
    /* Mixed-size operations */
    long long ll = 0x123456789ABCDEF0LL;
    vs = (short)ll;
    vc = (char)(ll >> 32);
}

/* ===== Test 3: Sub-register accesses for SUBREG ===== */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

union PackedUnion {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    uint8_t bytes[4];
};

void test_subregisters(void) {
    /* Vector operations */
    v4si v = {1, 2, 3, 4};
    v8hi vh = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access */
    int element = v[2];
    short selement = vh[3];
    
    /* Vector operations that may create SUBREG */
    v[0] = element + selement;
    vh[1] = (short)element;
    
    /* Union type-punning */
    union PackedUnion u;
    u.full = 0xDEADBEEF;
    g_volatile_short = u.parts.low;
    g_volatile_char = u.bytes[3];
    
    /* Float/int conversions */
    float f = 3.14159f;
    int fi = *(int*)&f;  /* Type punning */
    short fs = (short)fi;
    g_volatile_short = fs;
}

/* ===== Test 4: Combined patterns ===== */
struct Combined {
    volatile unsigned int bf1:3;
    volatile unsigned int bf2:5;
    volatile unsigned int bf3:8;
    volatile short partial;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.bf1 = 5;
    c.partial = (short)c.bf1;
    
    /* Complex expression with multiple patterns */
    unsigned int val = (c.bf2 << 3) | c.bf1;
    c.partial = (short)(val + c.bf3);
    
    /* Nested accesses */
    union {
        struct Combined c;
        uint32_t raw;
    } u;
    u.c = c;
    g_volatile_int = u.raw;
    
    /* Loop with combined operations */
    for (int i = 0; i < 4; i++) {
        c.bf1 = (c.bf1 + 1) & 0x7;
        c.partial = (short)(c.partial + c.bf2);
    }
}

/* ===== Test 5: Architecture-specific patterns ===== */
#ifdef __i386__
void test_x86_specific(void) {
    /* Inline assembly that might generate partial register ops */
    int a = g_volatile_int;
    short b = g_volatile_short;
    
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (b)
        : "r" (b)
        : "ax"
    );
    
    g_volatile_short = b;
    
    /* Byte operations */
    char c = g_volatile_char;
    __asm__ volatile (
        "movb %1, %%al\n\t"
        "incb %%al\n\t"
        "movb %%al, %0"
        : "=r" (c)
        : "r" (c)
        : "al"
    );
    
    g_volatile_char = c;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM may generate different patterns */
    int a = g_volatile_int;
    short b = g_volatile_short;
    
    /* Use ARM-specific builtins */
    b = __builtin_arm_rev16(b);
    g_volatile_short = b;
}
#endif

/* ===== Test 6: Builtin bit operations ===== */
void test_builtin_bitops(void) {
    unsigned int x = g_volatile_int;
    
    /* Builtins that may involve bit extraction */
    int leading_zeros = __builtin_clz(x);
    int parity = __builtin_parity(x);
    int popcount = __builtin_popcount(x);
    
    /* Combine results */
    g_volatile_short = (short)(leading_zeros + parity + popcount);
    
    /* Bit field extract (GCC extension) */
    struct {
        unsigned int a:5;
        unsigned int b:10;
        unsigned int c:15;
    } bits = {0};
    
    bits.a = (x >> 0) & 0x1F;
    bits.b = (x >> 5) & 0x3FF;
    bits.c = (x >> 15) & 0x7FFF;
    
    g_volatile_int = bits.a + bits.b + bits.c;
}

/* ===== Main test driver ===== */
typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
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
    
    if (argc > 1) {
        test_selector = argv[1][0] - '0';
    }
    
    /* Run all tests or specific ones based on input */
    if (test_selector == 0) {
        /* Run all tests */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    } else {
        /* Run specific test */
        int idx = test_selector - 1;
        if (idx >= 0 && test_functions[idx] != NULL) {
            test_functions[idx]();
        }
    }
    
    /* Ensure all volatile variables are used to prevent optimization */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    
    /* Simple computation to ensure program does something */
    for (int i = 0; i < 10; i++) {
        result += i * 2;
    }
    
    return result > 0 ? 0 : 1;
}
