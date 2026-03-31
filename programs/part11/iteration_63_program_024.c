/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Pattern 1: ZERO_EXTRACT destination */
NOINLINE void test_zero_extract(volatile int seed) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int middle: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    /* Volatile to prevent constant propagation */
    volatile unsigned int mask = seed & 0xFF;
    
    /* Multiple assignments to bitfields */
    u.bits.low = mask;
    u.bits.middle = (mask >> 2) & 0x3F;
    u.bits.high = (mask << 8) & 0xFFFF;
    
    /* Force usage */
    use(u.full);
    
    /* Another approach: explicit bitfield extraction */
    unsigned int dest = 0;
    unsigned int src = seed;
    
    /* This pattern may generate ZERO_EXTRACT when storing into masked portion */
    dest = (dest & ~0xFF00) | ((src & 0xFF) << 8);
    use(dest);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(volatile int seed) {
    /* Assigning smaller types to larger ones */
    short s = (short)(seed & 0xFFFF);
    int i = 0;
    
    /* This should preserve high bits while setting low bits */
    i = (i & ~0xFFFF) | (s & 0xFFFF);
    use(i);
    
    /* Using pointer casting */
    long long big = 0x123456789ABCDEF0LL;
    int *p = (int*)&big;
    volatile int val = seed;
    
    /* Store into low part of larger object */
    *p = val;
    use(big);
    
    /* Another approach with union */
    union {
        long long ll;
        int i[2];
    } pun;
    pun.ll = 0;
    pun.i[0] = val;  /* Store into low part */
    use(pun.ll);
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(volatile int seed) {
    /* Type punning with different sizes */
    long long big_array[4];
    volatile int idx = seed & 3;
    
    /* Access sub-word of larger object */
    int *p_int = (int*)&big_array[idx];
    *p_int = seed;
    use(big_array[0]);
    
    /* Array with sub-word access */
    int array[8];
    short *ps = (short*)array;
    volatile int offset = (seed & 7) * 2;
    
    ps[offset] = (short)seed;
    use(array[0]);
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    /* Access different sized members */
    m.i = seed;
    m.s = (short)(seed >> 8);
    use(m.ll);
}

/* Pattern 4: Complex MEM destinations */
NOINLINE void test_complex_mem(volatile int seed) {
    /* Global-like storage */
    static int storage[256];
    volatile int index = seed & 255;
    
    /* Complex addressing modes */
    int *ptr1 = &storage[index] + (seed & 3);
    *ptr1 = seed;
    use(storage[0]);
    
    /* Structure with pointer arithmetic */
    struct point {
        int x, y, z;
    } points[10];
    
    volatile int idx = seed % 10;
    int *ptr2 = &points[idx].y + (seed & 1);
    *ptr2 = seed;
    use(points[0].x);
    
    /* Multi-dimensional array */
    int matrix[10][10];
    volatile int row = seed % 10;
    volatile int col = seed % 10;
    
    int *ptr3 = &matrix[row][col];
    *ptr3 = seed;
    use(matrix[0][0]);
    
    /* Pointer chain */
    int a = 0, b = 0, c = 0;
    int *p1 = &a;
    int **p2 = &p1;
    int ***p3 = &p2;
    
    ***p3 = seed;
    use(a);
}

/* Pattern 5: Combined patterns in loops */
NOINLINE void test_combined(volatile int seed) {
    int result = 0;
    volatile int limit = (seed & 7) + 1;
    
    for (int i = 0; i < limit; i++) {
        /* Mix different patterns */
        union {
            unsigned int val;
            struct {
                unsigned int a: 4;
                unsigned int b: 4;
                unsigned int c: 8;
                unsigned int d: 16;
            } fields;
        } u;
        
        u.val = 0;
        u.fields.a = (seed + i) & 0xF;
        u.fields.c = ((seed + i) >> 4) & 0xFF;
        
        /* SUBREG access */
        short *ps = (short*)&u.val;
        ps[1] = (short)(seed - i);  /* High part */
        
        /* Complex MEM */
        static int buffer[16];
        volatile int idx = (seed + i) & 15;
        buffer[idx] = u.val;
        
        result ^= u.val ^ buffer[idx];
    }
    
    use(result);
}

/* Main driver */
int main(int argc, char **argv) {
    /* Use volatile seed from command line or timer */
    volatile int seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    printf("Testing with seed: %d\n", seed);
    
    /* Call all test patterns */
    test_zero_extract(seed);
    test_strict_low_part(seed);
    test_subreg(seed);
    test_complex_mem(seed);
    test_combined(seed);
    
    /* Dummy external function definitions to satisfy linker */
    void use(int x) {
        static volatile int sink;
        sink = x;
    }
    
    void sink(void *p) {
        static volatile void *vsink;
        vsink = p;
    }
    
    printf("All patterns executed\n");
    return 0;
}
