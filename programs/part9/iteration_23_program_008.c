/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ====== 1. Bit-field patterns for ZERO_EXTRACT ====== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    unsigned int pad:21;
};

struct NestedBitFields {
    struct {
        volatile unsigned int a:3;
        volatile unsigned int b:5;
    } inner;
    volatile unsigned int c:8;
    volatile unsigned int d:16;
};

void test_bitfields(void) {
    struct BitFieldStruct bfs1, bfs2;
    struct NestedBitFields nbf;
    
    /* Basic bit-field assignments */
    bfs1.flag = 1;
    bfs1.value = 511;  /* Max for 10 bits */
    
    /* Cross-structure bit-field copy */
    bfs2.flag = bfs1.flag;
    bfs2.value = bfs1.value;
    
    /* Complex bit-field expression */
    nbf.inner.a = (bfs1.value & 0x7);  /* Extract lower 3 bits */
    nbf.inner.b = (bfs1.value >> 3) & 0x1F;  /* Extract next 5 bits */
    nbf.c = nbf.inner.a + nbf.inner.b;
    nbf.d = nbf.c * 2;
    
    /* Store to volatile global to prevent dead code elimination */
    g_volatile_int = bfs2.value;
}

/* ====== 2. Partial register patterns for STRICT_LOW_PART ====== */

void test_partial_registers(void) {
    volatile short local_short;
    volatile char local_char;
    int temp;
    
    /* Explicit casts to smaller types */
    temp = g_volatile_int;
    local_short = (short)(temp + 100);
    local_char = (char)(temp + 50);
    
    /* Arithmetic on sub-word types */
    local_short = local_short + 5;
    local_char = local_char - 3;
    
    /* Mixed-size operations */
    g_volatile_short = (short)(local_short * 2);
    g_volatile_char = (char)(local_char / 2);
    
    /* Store to global volatile */
    g_volatile_short = local_short;
    g_volatile_char = local_char;
}

/* ====== 3. Sub-register patterns for SUBREG ====== */

/* Vector type using GCC extension */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

/* Packed structure */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

union TypePun {
    int i;
    float f;
    short s[2];
};

void test_subregs(void) {
    /* Vector operations */
    v4si vec = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element access - likely generates SUBREG */
    int elem = vec[2];
    short selem = vec_short[5];
    
    /* Vector operations */
    vec = vec + 1;
    vec_short = vec_short * 2;
    
    /* Packed structure access */
    struct PackedStruct ps;
    ps.a = 0xAA;
    ps.b = 0x12345678;
    ps.c = 0xABCD;
    
    /* Type punning through union */
    union TypePun tp;
    tp.i = 0x40000000;  /* ~0.5 in float */
    float fval = tp.f;
    short sval = tp.s[1];
    
    /* Store results to prevent optimization */
    g_volatile_int = elem + ps.b;
    g_volatile_short = selem + ps.c;
}

/* ====== 4. Combined patterns ====== */

struct Combined {
    volatile unsigned int low:8;
    volatile unsigned int high:8;
    volatile unsigned int full;
};

void test_combined_patterns(void) {
    struct Combined comb;
    volatile short result_short;
    
    /* Initialize */
    comb.low = 0x12;
    comb.high = 0x34;
    comb.full = 0x5678;
    
    /* Bit-field read to partial register */
    result_short = (short)((comb.high << 8) | comb.low);
    
    /* Modify through bit-field, read as whole */
    comb.low = result_short & 0xFF;
    comb.high = (result_short >> 8) & 0xFF;
    
    /* Complex expression mixing types */
    g_volatile_int = comb.full + (comb.high << 16) + comb.low;
    g_volatile_short = result_short;
}

/* ====== 5. Architecture-specific patterns ====== */

#ifdef __i386__
void test_x86_specific(void) {
    int result;
    short sresult;
    
    /* Inline assembly that might generate partial register ops */
    asm volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0\n\t"
        : "=r" (sresult)
        : "r" (g_volatile_short)
        : "%ax"
    );
    
    /* Use builtins that may involve bit manipulation */
    result = __builtin_popcount(g_volatile_int);
    result += __builtin_ctz(g_volatile_int | 1);  /* Avoid undefined behavior */
    
    g_volatile_int = result;
    g_volatile_short = sresult;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    /* ARM-specific patterns */
    int result = g_volatile_int;
    short sresult = g_volatile_short;
    
    /* Use ARM-specific builtins if available */
    result = __builtin_clz(result);
    
    /* Force partial register operations */
    sresult = (short)(result & 0xFFFF);
    
    g_volatile_int = result;
    g_volatile_short = sresult;
}
#endif

/* ====== Main test driver ====== */

typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_bitfields,
    test_partial_registers,
    test_subregs,
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
    int i, num_tests = 0;
    volatile int test_selector = 0;
    
    /* Use command line or environment to control which tests run */
    if (argc > 1) {
        test_selector = atoi(argv[1]);
    } else {
        /* Use a volatile source to prevent compile-time optimization */
        test_selector = g_volatile_int;
    }
    
    /* Run all tests or specific ones based on selector */
    for (i = 0; test_functions[i] != NULL; i++) {
        num_tests++;
        if (test_selector == 0 || test_selector == i + 1) {
            test_functions[i]();
        }
    }
    
    /* Final computation to ensure program does something useful */
    int final_result = g_volatile_int + g_volatile_short + g_volatile_char;
    
    /* Print something to prevent entire program elimination */
    printf("Ran %d tests, final marker: %d\n", 
           (test_selector == 0 ? num_tests : 1), 
           final_result & 0xFF);
    
    return (final_result > 0) ? 0 : 1;
}
