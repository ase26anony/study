/* Test program to trigger uncovered RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
static volatile int control = 0;

/* ====== Test 1: Bit-field operations for ZERO_EXTRACT ====== */
void test_bitfields(void) {
    /* Structure with various bit-field widths */
    struct S1 {
        volatile unsigned int flag:1;
        unsigned int value:10;
        unsigned int pad:21;
    } s1;
    
    struct S2 {
        unsigned int a:3;
        unsigned int b:5;
        unsigned int c:8;
        unsigned int d:16;
    } s2;
    
    /* Initialize */
    s1.flag = 1;
    s1.value = 512;
    
    s2.a = 3;
    s2.b = 15;
    s2.c = 127;
    s2.d = 32767;
    
    /* Bit-field assignments that should generate ZERO_EXTRACT */
    s1.flag = s2.a & 1;               /* Extract single bit */
    s1.value = s2.c;                  /* Extract 8-bit field */
    
    /* Cross-structure bit-field assignment */
    s2.b = s1.value & 0x1F;           /* Mask to fit 5-bit field */
    
    /* Complex expression with bit-fields */
    unsigned int temp = (s1.value << 3) | s2.a;
    s2.d = temp & 0xFFFF;             /* Store to 16-bit field */
    
    /* Prevent dead code elimination */
    control = s1.flag + s2.b;
}

/* ====== Test 2: Partial register operations for STRICT_LOW_PART ====== */
void test_partial_registers(void) {
    volatile char vc;
    volatile short vs;
    volatile int vi = 1000;
    
    /* Assignments to sub-word types */
    vc = (char)vi + 25;               /* Should generate partial register update */
    vs = (short)(vi * 2);             /* Another partial register update */
    
    /* Arithmetic on partial registers */
    vc = vc + 1;
    vs = vs - 100;
    
    /* Mixed-size operations */
    int i = vc;                       /* Load from char, extend */
    short s = (short)i + vs;          /* Operate on shorts */
    vs = s;                           /* Store back */
    
    /* Array with sub-word types */
    volatile char arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = (char)(i * 10);      /* Array stores generate partial updates */
    }
    
    control = vc + vs + arr[5];
}

/* ====== Test 3: Sub-register accesses for SUBREG ====== */
void test_subregisters(void) {
    /* Use GCC vector extension */
    typedef int v4si __attribute__ ((vector_size (16)));
    typedef short v8hi __attribute__ ((vector_size (16)));
    
    v4si v1 = {1, 2, 3, 4};
    v8hi v2 = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Access vector elements - should generate SUBREG */
    int elem1 = v1[2];
    short elem2 = v2[5];
    
    /* Modify vector elements */
    v1[1] = elem1 * 2;
    v2[3] = (short)elem2 + 100;
    
    /* Union for type-punning */
    union U {
        unsigned int full;
        struct {
            unsigned short low;
            unsigned short high;
        } parts;
    } u;
    
    u.full = 0x12345678;
    unsigned short low_part = u.parts.low;   /* SUBREG access */
    u.parts.high = low_part + 1;             /* SUBREG store */
    
    /* Float/integer conversions */
    volatile float f = 3.14159f;
    volatile int fi = *(int*)&f;             /* Type punning */
    float f2 = *(float*)&fi;                 /* And back */
    
    control = elem1 + elem2 + low_part + fi;
}

/* ====== Test 4: Combined patterns ====== */
void test_combined_patterns(void) {
    /* Bit-field in union with full-width integer */
    union BFUnion {
        struct {
            unsigned int low:12;
            unsigned int high:12;
            unsigned int pad:8;
        } bits;
        unsigned int full;
    } u;
    
    u.full = 0xABCD1234;
    
    /* Read bit-field, store to partial register */
    volatile short vs;
    vs = (short)u.bits.low;           /* ZERO_EXTRACT -> SUBREG/STRICT_LOW_PART */
    
    /* Modify through bit-field, read through full integer */
    u.bits.high = vs + 100;
    unsigned int temp = u.full;
    
    /* Nested structure with bit-fields */
    struct Outer {
        struct Inner {
            unsigned int a:4;
            unsigned int b:4;
            unsigned int c:8;
        } inner;
        volatile short outer_short;
    } outer;
    
    outer.inner.a = 5;
    outer.inner.b = 10;
    outer.outer_short = (short)(outer.inner.a * outer.inner.b);
    
    /* Complex expression combining all patterns */
    control = vs + temp + outer.outer_short;
}

/* ====== Test 5: Architecture-specific patterns ====== */
#ifdef __i386__
void test_x86_specific(void) {
    /* Inline assembly that might generate partial register ops */
    unsigned int eax_val;
    unsigned short ax_val;
    
    __asm__ volatile (
        "movl $0x12345678, %%eax\n\t"
        "movw %%ax, %0\n\t"
        : "=m" (ax_val)
        : 
        : "%eax"
    );
    
    /* Use builtins that may involve bit manipulation */
    unsigned int x = 0xF0F0F0F0;
    int count = __builtin_popcount(x);       /* May use bit-field extracts */
    int parity = __builtin_parity(x);
    
    control = ax_val + count + parity;
}
#elif defined(__arm__)
void test_arm_specific(void) {
    /* ARM may have different patterns for partial registers */
    volatile unsigned short hs;
    volatile unsigned int hi = 0x87654321;
    
    /* Access halfword */
    hs = (unsigned short)hi;
    
    /* Use ARM-specific builtin if available */
    #ifdef __ARM_FEATURE_QBIT
    /* Some ARM architectures have bit-field instructions */
    unsigned int x = 0xA5A5A5A5;
    unsigned int y = __builtin_arm_rbit(x);  /* Reverse bits */
    control = hs + (y & 0xFFFF);
    #else
    control = hs + 1;
    #endif
}
#else
void test_x86_specific(void) { control = 1; }
void test_arm_specific(void) { control = 2; }
#endif

/* ====== Test 6: Complex control flow with patterns ====== */
void test_with_control_flow(void) {
    struct {
        unsigned int a:3;
        unsigned int b:5;
        unsigned int c:8;
    } s;
    
    volatile short vs;
    int array[4] = {0};
    
    /* Loop with bit-field operations */
    for (int i = 0; i < 100; i++) {
        s.a = i & 0x07;               /* 3-bit field */
        s.b = (i >> 3) & 0x1F;        /* 5-bit field */
        s.c = i & 0xFF;               /* 8-bit field */
        
        /* Store to partial register in loop */
        vs = (short)(s.a + s.b + s.c);
        
        /* Conditional based on bit-field */
        if (s.a == 3) {
            array[i % 4] = s.b;
        }
        
        /* Switch on partial register value */
        switch (vs & 0x7) {
            case 0: array[0]++; break;
            case 1: array[1]++; break;
            case 2: array[2]++; break;
            default: array[3]++; break;
        }
    }
    
    control = vs + array[0] + array[1];
}

/* ====== Main test driver ====== */
int main(int argc, char *argv[]) {
    /* Use argc to control which tests run, preventing optimization */
    int test_to_run = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    /* Array of test functions */
    void (*tests[])(void) = {
        test_bitfields,
        test_partial_registers,
        test_subregisters,
        test_combined_patterns,
        test_x86_specific,
        test_arm_specific,
        test_with_control_flow
    };
    
    /* Run selected test or all tests */
    if (test_to_run >= 1 && test_to_run <= 7) {
        tests[test_to_run - 1]();
        result = test_to_run;
    } else {
        /* Run all tests */
        for (int i = 0; i < 7; i++) {
            tests[i]();
            result += (i + 1);
        }
    }
    
    /* Final computation to ensure program does something */
    printf("Result: %d (control: %d)\n", result, control);
    
    return result == 0 ? 1 : 0;
}
