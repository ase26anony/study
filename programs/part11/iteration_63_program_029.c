/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa, noclone))

/* Pattern 1: ZERO_EXTRACT destination */
NOINLINE void test_zero_extract(int seed) {
    volatile int src = seed;
    
    /* Using union with bitfields - likely to generate ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = 0xFFFFFFFF;
    /* This assignment to bitfield may generate ZERO_EXTRACT destination */
    u.bits.mid = src & 0xFF;
    
    /* Complex control flow to keep it alive */
    for (int i = 0; i < (src & 3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
    }
    
    use(u.full);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(int seed) {
    volatile short s = (short)seed;
    volatile int i = seed * 2;
    
    /* Assigning short to int with masking - may generate STRICT_LOW_PART */
    i = (i & ~0xFFFF) | (s & 0xFFFF);
    
    /* Pointer casting approach */
    int val = seed;
    short *ps = (short*)&val;
    *ps = s + 1;
    
    /* Another variation with explicit low-part preservation */
    long long big = (long long)seed * 1000;
    int *pi = (int*)&big;
    *pi = s;
    
    use(val + i + (int)big);
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(int seed) {
    volatile int idx = (seed & 3);
    
    /* Array with sub-word access */
    int array[4] = {0, 0, 0, 0};
    short *ps = (short*)&array[idx];
    *ps = (short)seed;
    
    /* Type punning with different sizes */
    long long big = 0x123456789ABCDEF0LL;
    int *p32 = (int*)&big;
    p32[1] = seed;  /* Access high 32 bits on little-endian */
    
    /* Structure with mixed types */
    struct Mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = seed;
    short *pms = (short*)&m.i;
    pms[0] = (short)seed;
    
    use(array[0] + array[1] + array[2] + array[3] + (int)big + m.i);
}

/* Pattern 4: Complex MEM destination with addressing modes */
NOINLINE void test_complex_mem(int seed) {
    volatile int off = (seed & 7);
    volatile int idx = (seed & 3);
    
    /* Structure with pointer arithmetic */
    struct S {
        int a;
        int b[4];
        int c;
    } s;
    
    int *ptr = &s.a + off;
    *ptr = seed;
    
    /* Array with volatile index */
    int arr[8];
    int *addr = &arr[idx + 1];
    *addr = seed * 2;
    
    /* Global-like variable (static) */
    static int glob[10];
    int *gptr = &glob[off] + idx;
    *gptr = seed * 3;
    
    /* Two-dimensional access */
    int matrix[4][4];
    int (*mptr)[4] = &matrix[off & 3];
    mptr[idx][0] = seed * 4;
    
    /* Compute checksum to use results */
    int sum = s.a + arr[0] + glob[0] + matrix[0][0];
    use(sum);
}

/* Pattern 5: Combined patterns in loop */
NOINLINE void test_combined(int seed) {
    volatile int bound = (seed & 7) + 1;
    int result = 0;
    
    for (int i = 0; i < bound; i++) {
        volatile int val = seed + i;
        
        /* Mix different patterns in loop */
        union {
            unsigned int full;
            struct {
                unsigned int low:12;
                unsigned int high:20;
            } bits;
        } u;
        
        u.full = result;
        u.bits.low = (val & 0xFFF);  /* ZERO_EXTRACT candidate */
        
        /* SUBREG access */
        short *ps = (short*)&u.full;
        ps[1] = (val >> 12) & 0xFFFF;  /* May generate SUBREG */
        
        /* MEM with addressing */
        int arr[4] = {0};
        arr[i & 3] = u.full;
        
        result += arr[0] + u.full;
    }
    
    use(result);
}

/* Pattern 6: More ZERO_EXTRACT variations */
NOINLINE void test_bitfield_ops(int seed) {
    volatile int mask = 0x00FF00FF;
    volatile int src = seed;
    
    /* Multiple bitfield operations */
    struct BitFields {
        unsigned int a:4;
        unsigned int b:8;
        unsigned int c:4;
        unsigned int d:16;
    } bf;
    
    bf.a = src & 0xF;
    bf.b = (src >> 4) & 0xFF;
    bf.c = (src >> 12) & 0xF;
    bf.d = src & 0xFFFF;
    
    /* Extract and reinsert */
    unsigned int packed = 0;
    packed |= (bf.a << 0);
    packed |= (bf.b << 4);
    packed |= (bf.c << 12);
    packed |= (bf.d << 16);
    
    /* Another ZERO_EXTRACT pattern */
    int value = seed * 3;
    value = (value & ~mask) | (src & mask);
    
    use(packed + value);
}

/* Pattern 7: STRICT_LOW_PART with arithmetic */
NOINLINE void test_low_part_arith(int seed) {
    volatile char c = (char)seed;
    volatile short s = (short)(seed >> 8);
    
    /* Operations that preserve high bits */
    int x = seed * 100;
    x = x + (c & 0xFF);  /* Only affects low 8 bits */
    
    long long y = (long long)seed << 32;
    short *py = (short*)&y;
    py[0] = s;  /* STRICT_LOW_PART on 64-bit */
    
    /* Chain of operations */
    int z = 0;
    for (int i = 0; i < 4; i++) {
        z = (z << 8) | (c + i);
    }
    
    use(x + (int)y + z);
}

int main(int argc, char *argv[]) {
    /* Use volatile seed to prevent compile-time optimization */
    volatile int seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) & 0x7FFFFFFF;
    }
    
    printf("Seed: %d\n", seed);
    
    /* Call all test functions */
    test_zero_extract(seed);
    test_strict_low_part(seed);
    test_subreg(seed);
    test_complex_mem(seed);
    test_combined(seed);
    test_bitfield_ops(seed);
    test_low_part_arith(seed + 1);
    
    /* Additional calls with different seeds */
    for (int i = 0; i < 3; i++) {
        test_subreg(seed + i * 100);
        test_complex_mem(seed + i * 200);
    }
    
    printf("Tests completed.\n");
    return 0;
}

/* Dummy definitions to satisfy linker */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void sink(void* p) {
    /* Empty */
    (void)p;
}
