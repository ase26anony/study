/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ========== 1. Bit-field patterns for ZERO_EXTRACT ========== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int pad:21;
};

struct NestedBitFields {
    struct {
        volatile unsigned int a:3;
        unsigned int b:5;
    } inner;
    volatile unsigned int c:7;
    unsigned int d:17;
};

void test_bitfields(void) {
    struct BitFieldStruct bfs = {0};
    struct NestedBitFields nbf = {0};
    
    /* Simple bit-field assignments */
    bfs.flag = 1;
    bfs.value = 511; /* Max for 10 bits */
    
    /* Cross assignments */
    unsigned int temp = bfs.value;
    bfs.flag = temp & 1;
    
    /* Nested bit-field operations */
    nbf.inner.a = 5;
    nbf.inner.b = nbf.inner.a + 2;
    nbf.c = nbf.inner.b;
    nbf.d = (nbf.c << 10) | bfs.value;
    
    /* Complex expression with bit-fields */
    bfs.value = (nbf.inner.a << 2) | (nbf.inner.b >> 1);
}

/* ========== 2. Partial register patterns for STRICT_LOW_PART ========== */

void test_partial_registers(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    int i = 1000;
    long l = 50000;
    
    /* Casts to smaller types */
    vs1 = (short)i;
    vs2 = (short)(i * 2);
    vc1 = (char)l;
    vc2 = (char)(l + 100);
    
    /* Arithmetic on sub-word types */
    vs1 = vs1 + 50;
    vs2 = vs2 - 25;
    vc1 = vc1 * 2;
    vc2 = vc2 / 2;
    
    /* Mixed operations */
    i = (int)vs1 + (int)vc1;
    l = (long)vs2 * (long)vc2;
    
    /* Pointer casting for partial access */
    volatile int *pvi = &i;
    vs1 = *(volatile short *)pvi;  /* Access lower 16 bits */
    vc1 = *(volatile char *)pvi;   /* Access lowest 8 bits */
}

/* ========== 3. Sub-register patterns for SUBREG ========== */

/* Vector types using GCC extension */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));
typedef char v16qi __attribute__ ((vector_size (16)));

/* Packed structure */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

union TypePunning {
    float f;
    int i;
    char bytes[4];
};

void test_subregs(void) {
    /* Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v8hi v2 = {10, 20, 30, 40, 50, 60, 70, 80};
    v16qi v3 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    
    /* Vector element access (triggers SUBREG) */
    int elem1 = v1[2];
    short elem2 = v2[5];
    char elem3 = v3[10];
    
    /* Vector operations */
    v1[0] = elem1 + elem2;
    v2[3] = elem2 - elem3;
    v3[7] = elem3 * 2;
    
    /* Packed structure access */
    struct PackedStruct ps;
    ps.a = 10;
    ps.b = 0x12345678;
    ps.c = 0x9ABC;
    
    /* Access misaligned members */
    int b_val = ps.b;  /* May require SUBREG due to packing */
    short c_val = ps.c;
    
    /* Type punning through union */
    union TypePunning tp;
    tp.f = 3.14159f;
    int int_view = tp.i;
    char byte_view = tp.bytes[2];
    
    /* Float/integer conversions */
    float f = (float)int_view;
    int i2 = (int)f;
    
    /* Mixed size operations */
    long long ll = (long long)int_view * 1000LL;
    int truncated = (int)ll;
}

/* ========== 4. Combined patterns ========== */

struct Combined {
    volatile unsigned int field1:4;
    volatile unsigned int field2:12;
    volatile short partial;
    char subreg;
};

void test_combined(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.partial = (short)c.field1;
    
    /* Partial register to bit-field */
    c.field2 = (unsigned int)c.partial;
    
    /* Through memory with different views */
    volatile int *as_int = (volatile int *)&c;
    volatile short *as_short = (volatile short *)&c;
    volatile char *as_char = (volatile char *)&c;
    
    /* Mixed accesses */
    *as_short = (short)(*as_int >> 4);
    c.subreg = (char)(*as_short & 0xFF);
    *as_int = (c.field1 << 28) | (c.field2 << 16) | (c.partial << 8) | c.subreg;
    
    /* Complex expression combining all */
    c.field1 = ((c.partial >> 4) & 0xF) | (c.subreg & 0x1);
    c.partial = (c.field2 << 4) | c.field1;
    c.subreg = (char)((c.partial >> 8) & 0xFF);
}

/* ========== 5. Architecture-specific patterns ========== */

#ifdef __i386__
void test_x86_specific(void) {
    volatile int result;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "movw $0x1234, %%ax\n\t"
        "movb $0x56, %%ah\n\t"
        "movw %%ax, %0"
        : "=m" (result)
        : 
        : "%ax"
    );
    
    /* Bit manipulation builtins */
    unsigned int x = 0x12345678;
    int count = __builtin_popcount(x);
    int parity = __builtin_parity(x);
    int clz = __builtin_clz(x);
    
    /* Use results to prevent optimization */
    result = count + parity + clz;
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile int result;
    
    /* ARM-specific operations that might generate SUBREG */
    __asm__ volatile (
        "uxth %0, %1\n\t"
        : "=r" (result)
        : "r" (0x12345678)
    );
}
#endif

/* ========== Main test driver ========== */

typedef void (*test_func_t)(void);

static test_func_t test_functions[] = {
    test_bitfields,
    test_partial_registers,
    test_subregs,
    test_combined,
#ifdef __i386__
    test_x86_specific,
#endif
#ifdef __arm__
    test_arm_specific,
#endif
    NULL
};

int main(int argc, char *argv[]) {
    int i;
    
    /* Use argc to control which tests run (prevents dead code elimination) */
    int test_mask = argc > 1 ? atoi(argv[1]) : 0xFF;
    
    for (i = 0; test_functions[i] != NULL; i++) {
        if (test_mask & (1 << i)) {
            test_functions[i]();
        }
    }
    
    /* Ensure all volatile accesses are actually performed */
    control = 1;
    
    /* Simple computation to ensure program does something useful */
    int sum = 0;
    for (i = 0; i < 100; i++) {
        sum += i * control;
    }
    
    printf("Result: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
