/* test_resource_patterns.c
 * Designed to generate RTL SET destinations with:
 * - ZERO_EXTRACT
 * - STRICT_LOW_PART  
 * - SUBREG
 * - Complex MEM patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Global variables for complex addressing */
int glob_array[100];
volatile int volatile_idx = 0;

/* Pattern 1: ZERO_EXTRACT destination */
NOINLINE void test_zero_extract(volatile int seed) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } u;
    
    /* Volatile operations to prevent optimization */
    u.full = seed;
    u.bits.mid = volatile_input() & 0xFF;  /* Should generate ZERO_EXTRACT */
    u.bits.high = (volatile_input() >> 16) & 0xFFFF;
    
    /* Complex control flow */
    for (int i = 0; i < (seed & 3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
    }
    
    use(u.full);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(volatile int seed) {
    int val = seed;
    short s_val;
    
    /* Access via volatile to prevent constant propagation */
    s_val = volatile_input() & 0xFFFF;
    
    /* Multiple patterns that could generate STRICT_LOW_PART */
    
    /* Pattern 2a: Direct assignment to low part */
    *(short*)&val = s_val;  /* Type punning */
    
    /* Pattern 2b: Arithmetic preserving high bits */
    if (seed & 1) {
        val = (val & ~0xFFFF) | (s_val & 0xFFFF);
    }
    
    /* Pattern 2c: Loop with low-part modifications */
    for (int i = 0; i < (seed & 7); i++) {
        short temp = volatile_input() & 0xFF;
        *(char*)&val = temp;  /* Even smaller part */
    }
    
    use(val);
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(volatile int seed) {
    /* Various type-punning scenarios */
    
    /* Pattern 3a: Array with sub-word access */
    int array[4];
    volatile int idx = seed & 3;
    
    /* Initialize array */
    for (int i = 0; i < 4; i++) {
        array[i] = volatile_input() + i;
    }
    
    /* Sub-word access through pointer */
    short *ps = (short*)&array[idx];
    *ps = volatile_input() & 0xFFFF;
    
    /* Pattern 3b: Large type with sub-part access */
    long long big_val = (long long)seed * 1000;
    int *p_int = (int*)&big_val;
    
    if (seed & 2) {
        *p_int = volatile_input();
    } else {
        p_int[1] = volatile_input();
    }
    
    /* Pattern 3c: Structure with mixed types */
    struct Mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.s = volatile_input() & 0xFFFF;
    m.i = volatile_input();
    
    /* Access through different type pointers */
    char *pc = (char*)&m.i;
    pc[1] = volatile_input() & 0xFF;
    
    sink(&array);
    sink(&big_val);
    sink(&m);
}

/* Pattern 4: Complex MEM destinations */
NOINLINE void test_complex_mem(volatile int seed) {
    /* Pattern 4a: Structure with offset computation */
    struct S {
        int a;
        int b;
        int c[10];
    } s;
    
    volatile int off = seed % 20;
    int *ptr;
    
    /* Complex addressing modes */
    if (seed & 1) {
        ptr = &s.a + off;
    } else {
        ptr = &s.c[0] + (off % 10);
    }
    
    *ptr = volatile_input();
    
    /* Pattern 4b: Global array with index computation */
    volatile int idx1 = volatile_idx++;
    volatile int idx2 = volatile_input() % 50;
    
    glob_array[idx1 % 100] = volatile_input();
    glob_array[(idx1 + idx2) % 100] = volatile_input() * 2;
    
    /* Pattern 4c: Pointer arithmetic with scaling */
    int *base = glob_array;
    for (int i = 0; i < (seed & 3); i++) {
        int offset = (volatile_input() + i * 4) % 50;
        base[offset] = volatile_input() + i;
    }
    
    /* Pattern 4d: Nested addressing */
    int **pptr = &base;
    (*pptr)[seed % 20] = volatile_input();
    
    sink(&s);
    sink(ptr);
}

/* Pattern 5: Combined patterns in loops */
NOINLINE void test_combined(volatile int seed) {
    int result = 0;
    volatile int limit = (seed & 0xF) + 1;
    
    for (int i = 0; i < limit; i++) {
        /* Mix different patterns */
        if (i & 1) {
            /* SUBREG-like */
            long long temp = result;
            *(int*)&temp = volatile_input() + i;
            result += (int)temp;
        } else {
            /* ZERO_EXTRACT-like */
            union {
                int val;
                struct {
                    unsigned short low;
                    unsigned short high;
                } parts;
            } u;
            u.val = result;
            u.parts.low = (volatile_input() + i) & 0xFFFF;
            result = u.val;
        }
        
        /* MEM pattern in loop */
        glob_array[i % 10] = result;
    }
    
    use(result);
}

/* Main driver */
int main(int argc, char *argv[]) {
    volatile int seed;
    
    /* Get volatile seed */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        glob_array[i] = volatile_input() + i;
    }
    
    /* Execute all pattern tests */
    test_zero_extract(seed);
    test_strict_low_part(seed + 1);
    test_subreg(seed + 2);
    test_complex_mem(seed + 3);
    test_combined(seed + 4);
    
    /* Create checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 50; i++) {
        checksum ^= glob_array[i];
    }
    
    printf("Result: %d\n", checksum);
    return 0;
}

/* Dummy implementation of external functions to satisfy linker */
void use(int x) {
    /* Empty - just to prevent optimization */
}

void sink(void *p) {
    /* Empty - just to prevent optimization */
}

int volatile_input(void) {
    static int counter = 0;
    return counter++ + 1;
}
