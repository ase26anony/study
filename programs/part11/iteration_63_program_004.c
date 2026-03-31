/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void sink(int);
extern void use(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Pattern 1: ZERO_EXTRACT destination via bitfields */
NOINLINE void test_zero_extract(volatile int seed) {
    /* Union with bitfield to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = seed;
    volatile int v = volatile_input();
    
    /* These assignments may generate ZERO_EXTRACT destinations */
    u.bits.low = v & 0xFF;
    u.bits.mid = (v >> 8) & 0xFF;
    u.bits.high = (v >> 16) & 0xFFFF;
    
    /* Complex bitfield manipulation in loop */
    for (int i = 0; i < (seed & 3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
        u.bits.mid ^= (v >> (i * 2)) & 0xFF;
    }
    
    sink(u.full);
}

/* Pattern 2: STRICT_LOW_PART destination via partial word stores */
NOINLINE void test_strict_low_part(volatile int seed) {
    volatile short vs = seed & 0xFFFF;
    int val = seed * 3;
    
    /* Cast to short pointer for partial store */
    *(short*)&val = vs;
    
    /* Multiple partial stores with volatile control */
    for (int i = 0; i < (seed & 7); i++) {
        volatile short temp = vs + i;
        *(short*)&val = temp;
        
        /* Alternate between low and "high" short */
        if (i & 1) {
            *((short*)&val + 1) = temp ^ 0x1234;
        }
    }
    
    /* Mixed-size operations */
    long long big = seed;
    int* p = (int*)&big;
    *p = val;  /* Store int into long long */
    
    sink(val + (int)big);
}

/* Pattern 3: SUBREG destination via type punning and arrays */
NOINLINE void test_subreg(volatile int seed) {
    /* Array with sub-word access */
    int array[8];
    volatile int idx = seed & 7;
    
    /* Initialize array */
    for (int i = 0; i < 8; i++) {
        array[i] = seed * i;
    }
    
    /* Various sub-word accesses */
    short* ps = (short*)&array[idx];
    *ps = (short)volatile_input();
    
    /* Cast between different integer sizes */
    long long big_val = seed * 1000LL;
    int* pi = (int*)&big_val;
    *pi = volatile_input();
    
    /* Structure with mixed types */
    struct Mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = seed;
    *(short*)&m.i = (short)volatile_input();  /* SUBREG store */
    
    /* Pointer arithmetic with different types */
    char* byte_ptr = (char*)array;
    byte_ptr[idx * 4 + 1] = (char)volatile_input();
    
    sink(array[idx] + m.i);
}

/* Pattern 4: Complex MEM destinations with addressing modes */
NOINLINE void test_complex_mem(volatile int seed) {
    /* Global-like variable */
    static int globals[16];
    volatile int off = seed & 15;
    
    /* Various addressing modes */
    int* ptr1 = &globals[off];
    *ptr1 = volatile_input();
    
    /* Pointer arithmetic */
    int* ptr2 = globals + (off * 2) % 16;
    *ptr2 = seed;
    
    /* Structure with pointer arithmetic */
    struct Point {
        int x, y, z;
    } points[4];
    
    volatile int idx = seed & 3;
    Point* pp = &points[idx];
    pp->y = volatile_input();
    
    /* Computed address with multiple operations */
    int* complex_ptr = &globals[(off + idx * 3) & 15];
    *complex_ptr = *ptr1 + *ptr2;
    
    /* Memory access through double pointer */
    int** pptr = &ptr1;
    **pptr = volatile_input();
    
    /* Loop with memory stores */
    for (int i = 0; i < (seed & 3); i++) {
        globals[(off + i) & 15] = volatile_input() + i;
    }
    
    sink(globals[off]);
}

/* Pattern 5: Combined patterns in complex control flow */
NOINLINE void test_combined(volatile int seed) {
    volatile int v = volatile_input();
    
    /* Mixed operations that may generate multiple patterns */
    union {
        unsigned long full;
        struct {
            unsigned int low;
            unsigned int high;
        } parts;
    } data;
    
    data.full = seed;
    
    /* Conditional ZERO_EXTRACT/STRICT_LOW_PART */
    if (v & 1) {
        data.parts.low = (data.parts.low & ~0xFF00) | ((v & 0xFF) << 8);
    } else {
        *(short*)&data.parts.high = (short)v;
    }
    
    /* Array with SUBREG access */
    int buffer[4];
    for (int i = 0; i < 4; i++) {
        buffer[i] = seed * (i + 1);
    }
    
    volatile int idx = v & 3;
    short* sp = (short*)&buffer[idx];
    *sp = (short)data.parts.low;
    
    /* Complex MEM addressing */
    int* mem_ptr = buffer + ((idx + 1) & 3);
    *mem_ptr = data.parts.high;
    
    /* Nested bitfield operations */
    struct Nested {
        struct {
            unsigned a:4;
            unsigned b:4;
            unsigned c:8;
        } inner;
        unsigned int rest;
    } nested;
    
    nested.inner.a = v & 0xF;
    nested.inner.b = (v >> 4) & 0xF;
    nested.inner.c = (v >> 8) & 0xFF;
    nested.rest = data.parts.low;
    
    sink(buffer[0] + nested.rest);
}

/* Main driver */
int main(int argc, char** argv) {
    /* Volatile seed from command line or timer */
    volatile int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    /* Call all test patterns */
    test_zero_extract(seed);
    test_strict_low_part(seed + 1);
    test_subreg(seed + 2);
    test_complex_mem(seed + 3);
    test_combined(seed + 4);
    
    /* Generate checksum to prevent optimization */
    volatile int result = seed;
    for (int i = 0; i < 10; i++) {
        result ^= volatile_input();
    }
    
    printf("Result: %d\n", result);
    return 0;
}

/* Dummy definitions to satisfy linker (in real use would be external) */
void sink(int x) {
    volatile static int sink_var;
    sink_var = x;
}

int volatile_input(void) {
    static volatile int counter = 0;
    return ++counter;
}
