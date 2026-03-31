/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int g_volatile_sink;

/* ==================== BIT-FIELD TESTS (ZERO_EXTRACT) ==================== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int padding:21;
};

struct NestedBitFields {
    struct {
        unsigned int a:3;
        unsigned int b:5;
    } inner;
    volatile unsigned int c:7;
    unsigned int d:17;
};

void test_bit_fields(void) {
    struct BitFieldStruct bfs = {0};
    struct NestedBitFields nbf = {0};
    
    /* Direct bit-field assignments */
    bfs.flag = 1;
    bfs.value = 511; /* Max for 10 bits */
    
    /* Cross bit-field assignment */
    nbf.inner.a = bfs.flag;
    nbf.c = bfs.value & 0x7F;
    
    /* Complex bit-field expression */
    unsigned int temp = nbf.inner.b;
    nbf.d = (temp << 3) | nbf.inner.a;
    
    /* Prevent optimization */
    g_volatile_sink = bfs.flag + nbf.c;
}

/* Union with bit-fields for type-punning */
union BitFieldUnion {
    struct {
        volatile unsigned int low:8;
        unsigned int high:8;
        unsigned int ext:16;
    } bits;
    uint32_t full;
};

void test_bitfield_union(void) {
    union BitFieldUnion u = {0};
    
    /* Access through bit-field members */
    u.bits.low = 0xAB;
    u.bits.high = 0xCD;
    
    /* Access through full integer (type punning) */
    uint32_t val = u.full;
    u.bits.ext = (val >> 16) & 0xFFFF;
    
    g_volatile_sink = u.bits.low;
}

/* ==================== PARTIAL REGISTER TESTS (STRICT_LOW_PART) ==================== */

void test_partial_registers(void) {
    volatile short vs;
    volatile char vc;
    volatile int vi = 100;
    
    /* Cast to smaller types with arithmetic */
    vs = (short)(vi + 50);
    vc = (char)(vs * 2);
    
    /* Partial updates in expressions */
    int result = (int)vc + (int)vs;
    vs = (short)(result & 0xFFFF);
    
    /* Nested partial operations */
    struct {
        volatile short a;
        volatile char b;
    } s = {0};
    
    s.a = (short)(vi - 25);
    s.b = (char)(s.a >> 4);
    
    g_volatile_sink = s.a + s.b;
}

/* Architecture-specific partial register operations */
#ifdef __i386__
void test_x86_partial_registers(void) {
    volatile uint16_t w;
    volatile uint8_t b;
    uint32_t d;
    
    /* These often generate STRICT_LOW_PART on x86 */
    w = 0x1234;
    b = 0xAB;
    
    /* Mixed-size operations */
    d = (uint32_t)w + (uint32_t)b;
    w = (uint16_t)(d >> 8);
    
    /* Inline assembly forcing partial register updates */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "movb %2, %%al\n\t"
        "movw %%ax, %0"
        : "=m" (w)
        : "r" ((uint16_t)0x5678), "r" ((uint8_t)0xCD)
        : "ax"
    );
    
    g_volatile_sink = w;
}
#endif

#ifdef __arm__
void test_arm_partial_registers(void) {
    volatile uint16_t hw;
    volatile uint8_t b;
    
    hw = 0x1234;
    b = 0xAB;
    
    /* ARM may use STRICT_LOW_PART for byte/halfword stores */
    uint32_t temp = (uint32_t)hw | ((uint32_t)b << 16);
    hw = (uint16_t)(temp & 0xFFFF);
    
    g_volatile_sink = hw;
}
#endif

/* ==================== SUBREG TESTS ==================== */

/* Vector types for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

void test_vector_subreg(void) {
    v4si vi = {1, 2, 3, 4};
    v8hi vh = {10, 20, 30, 40, 50, 60, 70, 80};
    v4sf vf = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Element access generates SUBREG */
    volatile int elem_i = vi[2];
    volatile short elem_h = vh[5];
    volatile float elem_f = vf[1];
    
    /* Vector-scalar operations */
    vi[0] = elem_i + 10;
    vh[3] = (short)(elem_h * 2);
    vf[2] = elem_f * 3.0f;
    
    /* Type conversion between vector elements */
    vi[1] = (int)vf[0];
    vh[0] = (short)vi[3];
    
    g_volatile_sink = elem_i + elem_h;
}

/* Packed structure for SUBREG accesses */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
    char d;
};

void test_packed_struct(void) {
    struct PackedStruct ps;
    memset(&ps, 0, sizeof(ps));
    
    /* Accesses to misaligned members generate SUBREG */
    ps.a = 0x11;
    ps.b = 0x22334455;  /* May be misaligned */
    ps.c = 0x6677;
    ps.d = 0x88;
    
    /* Type-punning through pointer casts */
    short *pshort = (short *)&ps.b;
    volatile short sval = pshort[0];  /* Unaligned access */
    
    g_volatile_sink = ps.a + sval;
}

/* Float/int conversions for SUBREG */
void test_float_conversions(void) {
    volatile float f = 3.14159f;
    volatile double d = 2.71828;
    
    /* Type punning through unions */
    union {
        float f;
        uint32_t i;
    } u1;
    
    union {
        double d;
        uint64_t i;
    } u2;
    
    u1.f = f;
    u2.d = d;
    
    /* Conversions generate SUBREG on some arches */
    uint32_t ifloat = u1.i;
    uint64_t idouble = u2.i;
    
    /* Mixed precision */
    float f_from_d = (float)u2.d;
    double d_from_f = (double)u1.f;
    
    g_volatile_sink = ifloat + (int)(idouble & 0xFFFFFFFF);
}

/* ==================== COMBINED PATTERNS ==================== */

/* Complex expression combining multiple patterns */
void test_combined_patterns(void) {
    /* Bit-field in struct */
    struct {
        volatile unsigned int low_bits:4;
        unsigned int mid_bits:8;
        volatile unsigned int high_bits:4;
    } bf;
    
    /* Vector for SUBREG */
    v4si vec = {100, 200, 300, 400};
    
    /* Partial register variable */
    volatile short partial;
    
    /* Combined operations */
    bf.low_bits = vec[0] & 0xF;          /* ZERO_EXTRACT from vector element */
    bf.mid_bits = (vec[1] >> 4) & 0xFF;  /* Another extract */
    
    /* Assign to partial register */
    partial = (short)bf.mid_bits;        /* Potential STRICT_LOW_PART */
    
    /* Modify through partial register */
    partial = (short)(partial + bf.low_bits);
    
    /* Back to bit-field */
    bf.high_bits = (partial >> 8) & 0xF;
    
    /* Vector update with bit-field value */
    vec[2] = (bf.high_bits << 12) | (bf.mid_bits << 4) | bf.low_bits;
    
    g_volatile_sink = partial + vec[2];
}

/* Loop with mixed patterns */
void test_loop_patterns(void) {
    struct BitFieldStruct arr[4];
    v4si vectors[2];
    
    /* Initialize */
    for (int i = 0; i < 4; i++) {
        arr[i].flag = i & 1;
        arr[i].value = i * 64;
    }
    
    vectors[0] = (v4si){1, 2, 3, 4};
    vectors[1] = (v4si){5, 6, 7, 8};
    
    /* Complex loop with multiple patterns */
    for (int i = 0; i < 4; i++) {
        /* Bit-field read (ZERO_EXTRACT) */
        unsigned int val = arr[i].value;
        
        /* Vector element access (SUBREG) */
        int vec_elem = vectors[i/2][i%2];
        
        /* Partial register operation */
        volatile short partial = (short)(val + vec_elem);
        
        /* Update bit-field */
        arr[i].value = partial & 0x3FF;
        
        /* Update vector */
        vectors[i/2][i%2] = arr[i].value;
    }
    
    g_volatile_sink = arr[0].value;
}

/* ==================== MAIN TEST DRIVER ==================== */

typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_bit_fields,
    test_bitfield_union,
    test_partial_registers,
    test_vector_subreg,
    test_packed_struct,
    test_float_conversions,
    test_combined_patterns,
    test_loop_patterns,
#ifdef __i386__
    test_x86_partial_registers,
#endif
#ifdef __arm__
    test_arm_partial_registers,
#endif
};

int main(int argc, char *argv[]) {
    volatile int test_selector = 0;
    
    /* Use command line or external input to control which tests run */
    if (argc > 1) {
        test_selector = argv[1][0] - '0';
    }
    
    /* Run all tests or specific ones based on selector */
    int num_tests = sizeof(test_functions) / sizeof(test_functions[0]);
    
    if (test_selector >= 0 && test_selector < num_tests) {
        /* Run single test */
        test_functions[test_selector]();
    } else {
        /* Run all tests */
        for (int i = 0; i < num_tests; i++) {
            test_functions[i]();
        }
    }
    
    /* Ensure program has observable output */
    return g_volatile_sink != 0 ? 0 : 1;
}
