/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ==================== ZERO_EXTRACT patterns (bit-fields) ==================== */

/* Test 1: Basic bit-field operations */
struct BitFieldStruct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int padding:21;
};

struct NestedBitField {
    struct {
        volatile unsigned int a:3;
        unsigned int b:5;
    } inner;
    unsigned int c:8;
};

void test_zero_extract_basic(void) {
    struct BitFieldStruct s = {0, 0, 0};
    struct NestedBitField n = {{{0}, 0}, 0};
    
    /* Various bit-field assignments */
    s.flag = 1;
    s.value = 511; /* Max for 10 bits */
    
    /* Cross assignments */
    unsigned int temp = s.value;
    s.value = temp + 1;
    
    /* Nested bit-field access */
    n.inner.a = 5;
    n.inner.b = n.inner.a + 2;
    n.c = n.inner.b;
    
    /* Complex expression with bit-fields */
    s.value = (s.flag ? 100 : 200) | (n.c & 0x7F);
}

/* Test 2: Bit-fields in unions */
union BitFieldUnion {
    struct {
        volatile unsigned int low:8;
        unsigned int high:8;
    } parts;
    volatile unsigned int full;
};

void test_zero_extract_union(void) {
    union BitFieldUnion u;
    u.full = 0x1234;
    
    /* Access through bit-field members */
    u.parts.low = 0xAB;
    unsigned int high_val = u.parts.high;
    u.parts.high = high_val ^ 0xFF;
    
    /* Mixed access patterns */
    u.full = (u.parts.low << 8) | u.parts.high;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Test 3: Partial register updates with small types */
void test_strict_low_part(void) {
    volatile short vs;
    volatile char vc;
    volatile unsigned char vuc;
    
    int i = control + 100;
    long l = control + 1000L;
    
    /* Assignments to partial registers */
    vs = (short)i + 5;
    vc = (char)(l >> 8);
    vuc = (unsigned char)(i * 2);
    
    /* Arithmetic on partial types */
    vs += (short)vc;
    vc = vc - 32;
    vuc = vuc * 2 + 1;
    
    /* Complex expressions with partial types */
    short result = (short)((vs & 0xFF) | ((int)vc << 8));
    vs = result;
}

/* Test 4: Mixed-size operations */
void test_mixed_sizes(void) {
    volatile int vi = control;
    volatile short vs = vi & 0xFFFF;
    volatile char vc = vi & 0xFF;
    
    /* Operations that may generate partial updates */
    vi = (vi & 0xFFFF0000) | (int)vs;
    vs = (short)((vc << 1) | (vs & 0xFF00));
    
    /* Pointer casting to smaller types */
    int array[4] = {1, 2, 3, 4};
    short *sp = (short *)array;
    sp[1] = (short)vi;
    
    /* Byte operations */
    char *cp = (char *)array;
    cp[3] = (char)vs;
}

/* ==================== SUBREG patterns ==================== */

/* Test 5: Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));

void test_vector_subreg(void) {
    v4si v = {1, 2, 3, 4};
    v8hi h = {10, 20, 30, 40, 50, 60, 70, 80};
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector element access */
    volatile int element = v[2];
    volatile short helement = h[5];
    volatile float felement = f[1];
    
    /* Vector operations */
    v[0] = element + 10;
    h[3] = (short)(helement * 2);
    f[2] = felement * 2.0f;
    
    /* Vector-scalar mixing */
    v = v + (v4si){element, element, element, element};
    h = h - (v8hi){helement, helement, helement, helement,
                   helement, helement, helement, helement};
}
#endif

/* Test 6: Type punning and conversions */
void test_type_conversions(void) {
    volatile float f = 3.14159f + control;
    volatile double d = 2.71828 + control;
    
    /* Float/double to integer conversions */
    int fi = (int)f;
    short fs = (short)f;
    char fc = (char)f;
    
    /* Integer to float conversions */
    f = (float)fi;
    d = (double)(fs * 2);
    
    /* Bit-level type punning */
    union {
        float f;
        int i;
    } u;
    
    u.f = f;
    u.i = u.i & 0x7FFFFFFF; /* Clear sign bit */
    f = u.f;
    
    /* Double precision manipulation */
    union {
        double d;
        long long ll;
    } du;
    
    du.d = d;
    du.ll = du.ll ^ 0x8000000000000000LL; /* Flip sign bit */
    d = du.d;
}

/* ==================== COMBINED patterns ==================== */

/* Test 7: Combined bit-field and partial register */
void test_combined_patterns(void) {
    struct {
        volatile unsigned int low_bits:4;
        volatile unsigned int high_bits:12;
    } bf;
    
    bf.low_bits = 7;
    bf.high_bits = 255;
    
    /* Combine bit-field with partial register */
    volatile short combined = (bf.high_bits << 4) | bf.low_bits;
    bf.low_bits = combined & 0xF;
    bf.high_bits = (combined >> 4) & 0xFFF;
    
    /* Complex expression mixing types */
    int result = (int)bf.high_bits * (int)bf.low_bits;
    bf.low_bits = result & 0xF;
    bf.high_bits = (result >> 4) & 0xFFF;
}

/* Test 8: Nested patterns in loops */
void test_nested_loops(void) {
    struct BitFieldStruct array[10];
    volatile short partial[10];
    
    for (int i = 0; i < 10; i++) {
        array[i].flag = i & 1;
        array[i].value = i * 10;
        
        /* Combine bit-field with partial register in loop */
        partial[i] = (short)((array[i].value << 1) | array[i].flag);
        
        /* Modify through partial register */
        array[i].value = partial[i] >> 1;
        array[i].flag = partial[i] & 1;
    }
    
    /* Conditional with mixed patterns */
    for (int i = 0; i < 10; i++) {
        if (array[i].flag) {
            partial[i] = (short)(array[i].value & 0x3FF);
        } else {
            partial[i] = (short)(~array[i].value & 0x3FF);
        }
    }
}

/* ==================== Architecture-specific patterns ==================== */

#ifdef __i386__
void test_x86_specific(void) {
    /* Inline assembly that might generate partial register ops */
    unsigned int eax_val, edx_val;
    unsigned short ax_val;
    unsigned char al_val;
    
    __asm__ volatile (
        "movl $0x12345678, %%eax\n"
        "movw %%ax, %0\n"
        "movb %%al, %1\n"
        : "=m"(ax_val), "=m"(al_val)
        : 
        : "%eax"
    );
    
    /* Use the values to prevent optimization */
    control = ax_val + al_val;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM may generate different patterns */
    volatile unsigned short hs;
    volatile unsigned char hc;
    
    /* Operations that might use partial registers */
    unsigned int reg = control;
    hs = (reg >> 16) & 0xFFFF;
    hc = reg & 0xFF;
    
    /* Combine back */
    reg = (hs << 16) | hc;
    control = reg;
}
#endif

/* ==================== Builtin functions ==================== */

void test_builtins(void) {
    unsigned int x = control | 0x12345678;
    
    /* Builtins that may involve bit manipulation */
    int count = __builtin_popcount(x);
    int parity = __builtin_parity(x);
    int clz = __builtin_clz(x);
    int ctz = __builtin_ctz(x);
    
    /* Use results in bit-field context */
    struct {
        volatile unsigned int pop:8;
        unsigned int par:1;
        unsigned int leading:6;
        unsigned int trailing:6;
    } bits;
    
    bits.pop = count & 0xFF;
    bits.par = parity & 1;
    bits.leading = clz & 0x3F;
    bits.trailing = ctz & 0x3F;
    
    control = bits.pop + bits.leading + bits.trailing;
}

/* ==================== Main test driver ==================== */

typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_zero_extract_basic,
    test_zero_extract_union,
    test_strict_low_part,
    test_mixed_sizes,
#ifdef __GNUC__
    test_vector_subreg,
#endif
    test_type_conversions,
    test_combined_patterns,
    test_nested_loops,
#ifdef __i386__
    test_x86_specific,
#endif
#ifdef __arm__
    test_arm_specific,
#endif
    test_builtins,
    NULL
};

int main(int argc, char *argv[]) {
    /* Use command line to control which tests run */
    int start_test = 0;
    int end_test = sizeof(test_functions)/sizeof(test_functions[0]) - 2;
    
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
    for (int i = start_test; i <= end_test && test_functions[i] != NULL; i++) {
        test_functions[i]();
    }
    
    /* Ensure all operations have side effects */
    printf("Final control value: %d\n", control);
    
    /* Simple computation to ensure program is valid */
    int result = 0;
    for (int i = 0; i < 100; i++) {
        result += i * control;
    }
    
    return result > 0 ? 0 : 1;
}
