/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(int);
extern void barrier(void);

/* Volatile seed to prevent compile-time optimization */
static volatile int seed = 0;

/* Pattern 1: ZERO_EXTRACT destination */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int input) {
    /* Using union with bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    /* Initialize with volatile to prevent constant propagation */
    u.full = input & 0xFFFFFFFF;
    
    /* Store into bitfield - may generate ZERO_EXTRACT destination */
    u.bits.mid = (input >> 8) & 0xFF;
    
    /* Use result to keep computation live */
    use(u.full);
    
    /* Alternative: Bitwise operations that mask specific bits */
    unsigned int val = input;
    /* Store into bits 8-15 of val */
    val = (val & ~(0xFF << 8)) | (((input + 1) & 0xFF) << 8);
    sink(val);
}

/* Pattern 2: STRICT_LOW_PART destination */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile int input) {
    /* Assigning smaller type to larger type */
    short s = (short)(input & 0xFFFF);
    int i = input;
    
    /* This may generate STRICT_LOW_PART when storing short into int */
    i = (i & ~0xFFFF) | (s & 0xFFFF);
    use(i);
    
    /* Alternative: Pointer casting approach */
    int val = input;
    volatile short vs = (short)(input + 1);
    *(short*)&val = vs;  /* May generate STRICT_LOW_PART */
    sink(val);
    
    /* Using explicit masking of low bits */
    long long big = input;
    big = (big & ~0xFFFFFFFFLL) | (input & 0xFFFFFFFF);
    use((int)big);
}

/* Pattern 3: SUBREG destination */
__attribute__((noinline, noipa))
void test_subreg(volatile int input) {
    /* Type punning with different sized accesses */
    long long big_array[2];
    big_array[0] = input;
    big_array[1] = input + 1;
    
    /* Access sub-parts through different type pointers */
    int* p_int = (int*)&big_array[0];
    p_int[1] = input + 2;  /* May generate SUBREG */
    use(p_int[0]);
    
    /* Array with sub-word access */
    int array[4];
    volatile int idx = input & 3;
    short* ps = (short*)&array[idx];
    *ps = (short)(input & 0xFFFF);  /* SUBREG likely here */
    sink(array[0]);
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    m.ll = input;
    *(int*)((char*)&m.ll + 2) = input + 3;  /* Complex SUBREG pattern */
    use(m.i);
}

/* Pattern 4: Complex MEM destination with addressing modes */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int input) {
    /* Global/static variables with computed addresses */
    static int globals[10];
    volatile int idx = (input & 7) + 1;
    
    /* Complex addressing mode */
    int* ptr = &globals[0] + idx * 2;
    *ptr = input;  /* MEM with index*2 addressing */
    use(globals[0]);
    
    /* Structure with pointer arithmetic */
    struct S {
        int a;
        int b[3];
        int c;
    } s;
    
    volatile int off = input & 2;
    int* s_ptr = &s.a + off;
    *s_ptr = input + 1;  /* MEM with structure offset */
    sink(s.a);
    
    /* Multi-dimensional array */
    int matrix[4][4];
    volatile int row = input & 3;
    volatile int col = (input >> 2) & 3;
    matrix[row][col] = input + 2;  /* MEM with 2D indexing */
    use(matrix[0][0]);
}

/* Pattern 5: Combined patterns in loops */
__attribute__((noinline, noipa))
void test_combined(volatile int input) {
    int result = 0;
    
    /* Loop with varying patterns */
    for (int i = 0; i < (input & 3) + 1; i++) {
        /* Mix different patterns in loop body */
        if (i & 1) {
            /* ZERO_EXTRACT pattern */
            union {
                unsigned int val;
                struct {
                    unsigned int a: 4;
                    unsigned int b: 12;
                    unsigned int c: 16;
                } fields;
            } u;
            u.val = input + i;
            u.fields.b = (input + i * 2) & 0xFFF;
            result += u.val;
        } else {
            /* SUBREG pattern */
            long long ll = input;
            int* p = (int*)&ll + (i & 1);
            *p = input - i;
            result += (int)ll;
        }
        
        /* MEM pattern with computed address */
        static int buffer[8];
        volatile int offset = (input + i) & 7;
        buffer[offset] = result;
    }
    
    sink(result);
}

/* Helper to force different control flow paths */
__attribute__((noinline, noipa))
int control_flow(volatile int cond) {
    int x = 0;
    
    /* Conditional that can't be optimized away */
    if (cond & 1) {
        x = test_zero_extract_pattern(cond);
    } else if (cond & 2) {
        x = test_strict_low_part_pattern(cond);
    } else {
        x = test_subreg_pattern(cond);
    }
    
    return x;
}

/* Additional specialized pattern generators */
__attribute__((noinline, noipa))
int test_zero_extract_pattern(volatile int v) {
    /* Explicit bitfield store that should generate ZERO_EXTRACT */
    struct {
        unsigned int a: 5;
        unsigned int b: 11;
        unsigned int c: 16;
    } bits;
    
    bits.a = v & 0x1F;
    bits.b = (v >> 5) & 0x7FF;
    bits.c = (v >> 16) & 0xFFFF;
    
    return bits.a + bits.b + bits.c;
}

__attribute__((noinline, noipa))
int test_strict_low_part_pattern(volatile int v) {
    int x = v;
    short s = (short)(v + 1);
    
    /* Force STRICT_LOW_PART through explicit low-part assignment */
    asm volatile("" : "+r"(x) : : "memory");
    x = (x & ~0xFFFF) | (s & 0xFFFF);
    
    return x;
}

__attribute__((noinline, noipa))
int test_subreg_pattern(volatile int v) {
    long long ll[2];
    ll[0] = v;
    ll[1] = v * 2;
    
    /* Access different parts of the long long */
    int* p1 = (int*)&ll[0];
    int* p2 = (int*)&ll[1];
    
    p1[1] = v + 100;  /* SUBREG access to high part of first long long */
    p2[0] = v - 100;  /* SUBREG access to low part of second long long */
    
    return (int)(ll[0] + ll[1]);
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize volatile seed from command line or timer */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    volatile int v = seed;
    int checksum = 0;
    
    /* Call pattern generators with volatile input */
    test_zero_extract(v);
    checksum += v;
    
    test_strict_low_part(v + 1);
    checksum += v + 1;
    
    test_subreg(v + 2);
    checksum += v + 2;
    
    test_complex_mem(v + 3);
    checksum += v + 3;
    
    test_combined(v + 4);
    checksum += v + 4;
    
    /* Additional calls with control flow */
    checksum += control_flow(v);
    checksum += test_zero_extract_pattern(v + 5);
    checksum += test_strict_low_part_pattern(v + 6);
    checksum += test_subreg_pattern(v + 7);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions to satisfy external references */
void use(int x) {
    /* Empty but prevents optimization */
    asm volatile("" : : "r"(x) : "memory");
}

void sink(int x) {
    /* Use inline asm to prevent optimization */
    asm volatile("" : : "r"(x) : "memory");
}

void barrier(void) {
    asm volatile("" : : : "memory");
}
