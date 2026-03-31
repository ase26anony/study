/* test_resource_patterns.c
 * Designed to generate RTL SET destinations with:
 * 1. ZERO_EXTRACT patterns (bitfield assignments)
 * 2. STRICT_LOW_PART patterns (low-part assignments)
 * 3. SUBREG patterns (sub-register accesses)
 * 4. Complex MEM patterns (non-trivial memory addressing)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int x);
extern void use_long(long x);
extern void use_ptr(void *p);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Global variables for memory patterns */
int global_array[256];
struct compound {
    int a;
    int b;
    long long c;
    short d;
} global_struct;

/* 1. ZERO_EXTRACT patterns - bitfield assignments */
NOINLINE void test_zero_extract(int seed) {
    volatile int v = seed;
    
    /* Union with bitfields */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = v;
    /* This should generate ZERO_EXTRACT for bitfield store */
    u.bits.mid = (v >> 3) & 0xFF;
    u.bits.high = (v >> 8) & 0xFFFF;
    
    /* Another pattern: explicit masking */
    unsigned int val = v;
    /* Store into specific bits */
    val = (val & ~0xFF00) | ((v & 0xFF) << 8);
    
    use_int(u.full + val);
}

NOINLINE void test_zero_extract2(int seed) {
    volatile int v = seed;
    
    /* Large bitfield structure */
    struct {
        unsigned int field1 : 3;
        unsigned int field2 : 5;
        unsigned int field3 : 10;
        unsigned int field4 : 14;
    } bf;
    
    bf.field1 = v & 0x7;
    bf.field2 = (v >> 3) & 0x1F;
    bf.field3 = (v >> 8) & 0x3FF;
    /* This assignment may generate ZERO_EXTRACT */
    bf.field4 = (v >> 18) & 0x3FFF;
    
    /* Force use of bitfield */
    use_int(bf.field1 + bf.field2 + bf.field3 + bf.field4);
}

/* 2. STRICT_LOW_PART patterns - low-part assignments */
NOINLINE void test_strict_low_part(int seed) {
    volatile short vs = (short)seed;
    volatile int vi = seed * 3;
    
    /* Assign short to low part of int */
    int result = vi;
    /* This may generate STRICT_LOW_PART */
    *(short*)&result = vs;
    
    /* Another pattern with masking */
    long long big = seed * 5LL;
    int *p = (int*)&big;
    /* Store to low 32 bits */
    *p = vi;
    
    /* Type punning through union */
    union {
        long long ll;
        int i[2];
    } pun;
    pun.ll = big;
    pun.i[0] = vi;  /* Low part assignment */
    
    use_long(pun.ll + result);
}

NOINLINE void test_strict_low_part2(int seed) {
    volatile char vc = (char)seed;
    volatile int vi = seed;
    
    /* Multiple low-part assignments in loop */
    for (int i = 0; i < (seed & 3); i++) {
        int val = vi + i;
        /* Assign char to low byte */
        *(char*)&val = vc + i;
        use_int(val);
    }
    
    /* Mixed-size assignments */
    struct {
        int a;
        short b;
        char c;
    } s;
    s.a = vi;
    s.b = (short)vc;  /* May generate STRICT_LOW_PART for struct store */
    s.c = vc;
    
    use_int(s.a + s.b + s.c);
}

/* 3. SUBREG patterns - sub-register accesses */
NOINLINE void test_subreg(int seed) {
    volatile int v = seed;
    
    /* Array with sub-word access */
    int arr[4] = {v, v+1, v+2, v+3};
    volatile int idx = v & 3;
    
    /* Access via different type pointer - may generate SUBREG */
    short *ps = (short*)&arr[idx];
    *ps = (short)(v + 100);
    
    /* Another SUBREG pattern */
    long long big_val = (long long)v * 1000LL;
    int *pi = (int*)&big_val;
    pi[1] = v * 2;  /* Access high part on 32-bit */
    
    /* Structure with mixed types */
    struct mixed {
        long long a;
        int b;
        short c;
        char d;
    } m;
    m.a = big_val;
    m.b = *pi;
    m.c = *(short*)&v;
    m.d = (char)v;
    
    use_long(m.a + m.b);
}

NOINLINE void test_subreg2(int seed) {
    volatile int v = seed;
    
    /* Complex pointer arithmetic with type changes */
    char *buffer = (char*)global_array;
    int offset = v % 200;
    
    /* Access int at non-aligned boundary (with -fno-strict-aliasing) */
    int *unaligned_int = (int*)(buffer + offset + 1);
    *unaligned_int = v;
    
    /* Multiple indirections */
    int **pp = &unaligned_int;
    short *sp = (short*)*pp;
    sp[0] = (short)v;
    sp[1] = (short)(v >> 16);
    
    use_int(*unaligned_int + sp[0] + sp[1]);
}

/* 4. Complex MEM patterns - non-trivial memory addressing */
NOINLINE void test_complex_mem(int seed) {
    volatile int v = seed;
    volatile int idx = v & 255;
    
    /* Complex addressing mode */
    int *ptr = &global_array[idx + (v >> 8)];
    *ptr = v * 2;
    
    /* Structure with offset */
    struct compound *sptr = &global_struct;
    int *field_ptr = &sptr->a + (v & 1);  /* Choose a or b */
    *field_ptr = v;
    
    /* Pointer arithmetic in loop */
    for (int i = 0; i < (v & 7); i++) {
        int *p = &global_array[(idx + i * 17) & 255];
        *p = v + i;
    }
    
    /* Multi-dimensional access */
    int matrix[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = v + i * 8 + j;
        }
    }
    
    /* Complex computed address */
    int (*matrix_ptr)[8] = &matrix[v & 7];
    (*matrix_ptr)[v & 7] = v * 3;
    
    use_int(*ptr + *field_ptr + matrix[0][0]);
}

NOINLINE void test_complex_mem2(int seed) {
    volatile int v = seed;
    
    /* Stack-based complex addressing */
    int local_arr[128];
    volatile int offset = v % 100;
    
    /* Address computation with multiple operations */
    int *addr = &local_arr[offset] + (v >> 4) - (v >> 2);
    *addr = v;
    
    /* Chain of pointer dereferences */
    int **pptr = &addr;
    int ***ppptr = &pptr;
    ***ppptr = v + 1;
    
    /* Memory access through pointer array */
    int *ptr_array[4];
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &local_arr[i * 16 + offset];
        *ptr_array[i] = v + i;
    }
    
    /* Indirect indexed access */
    int index = v & 3;
    *ptr_array[index] = v * 2;
    
    use_int(*addr + **pptr + *ptr_array[0]);
}

/* Combined test that mixes patterns */
NOINLINE int test_combined(int seed) {
    volatile int v = seed;
    int result = 0;
    
    /* Mix different patterns in control flow */
    if (v & 1) {
        test_zero_extract(v);
        result += 1;
    }
    
    if (v & 2) {
        test_strict_low_part(v);
        result += 2;
    }
    
    for (int i = 0; i < (v & 3); i++) {
        test_subreg(v + i);
        result += 4;
    }
    
    switch (v % 4) {
        case 0:
            test_complex_mem(v);
            result += 8;
            break;
        case 1:
            test_complex_mem2(v);
            result += 16;
            break;
        case 2:
            test_zero_extract2(v);
            result += 32;
            break;
        case 3:
            test_strict_low_part2(v);
            result += 64;
            break;
    }
    
    return result;
}

int main(int argc, char **argv) {
    /* Use command line argument or timer for volatile seed */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    volatile int vseed = seed;
    
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    global_struct.a = 1;
    global_struct.b = 2;
    global_struct.c = 3LL;
    global_struct.d = 4;
    
    /* Run tests */
    int result = 0;
    
    result += test_zero_extract(vseed);
    result += test_strict_low_part(vseed + 1);
    result += test_subreg(vseed + 2);
    result += test_complex_mem(vseed + 3);
    result += test_zero_extract2(vseed + 4);
    result += test_strict_low_part2(vseed + 5);
    result += test_subreg2(vseed + 6);
    result += test_complex_mem2(vseed + 7);
    
    /* Combined test with control flow */
    result += test_combined(vseed + 8);
    
    /* Print result to prevent optimization */
    printf("Result: %d (seed: %d)\n", result, vseed);
    
    return result != 0 ? 0 : 1;
}

/* Dummy definitions to satisfy external references */
void use_int(int x) {
    volatile static int sink;
    sink = x;
}

void use_long(long x) {
    volatile static long sink;
    sink = x;
}

void use_ptr(void *p) {
    volatile static void *sink;
    sink = p;
}
