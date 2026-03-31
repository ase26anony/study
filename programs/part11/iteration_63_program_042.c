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
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    /* Volatile to prevent constant propagation */
    volatile unsigned int mask = (seed & 0xFF);
    
    /* Multiple operations that may generate ZERO_EXTRACT */
    u.full = seed;
    u.bits.mid = mask;  /* This may become SET with ZERO_EXTRACT dest */
    
    /* More complex bitfield manipulation */
    u.bits.low = (mask >> 2) & 0x3F;
    u.bits.high = (mask << 3) & 0x7FFF;
    
    /* Use result to keep it live */
    use(u.full);
    
    /* Another approach: explicit bitfield store */
    struct {
        unsigned int a: 4;
        unsigned int b: 12;
        unsigned int c: 16;
    } s;
    
    s.a = (mask >> 4) & 0xF;
    s.b = mask & 0xFFF;
    s.c = (mask * 3) & 0xFFFF;
    
    use(s.a + s.b + s.c);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(volatile int seed) {
    /* Assigning smaller types to larger ones */
    volatile short s_val = (short)(seed & 0xFFFF);
    int i_val = seed * 2;
    
    /* This may generate STRICT_LOW_PART */
    i_val = (i_val & ~0xFFFF) | (s_val & 0xFFFF);
    
    use(i_val);
    
    /* Using pointer casting */
    long long big_val = (long long)seed * 1000LL;
    int *p_int = (int*)&big_val;
    
    volatile int temp = seed + 100;
    *p_int = temp;  /* May generate STRICT_LOW_PART for 32-bit store into 64-bit */
    
    use(big_val);
    
    /* Another pattern with char/short to int */
    char c = (char)(seed & 0xFF);
    int x = seed;
    x = (x & ~0xFF) | (c & 0xFF);  /* Preserve high bits, set low byte */
    
    use(x);
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(volatile int seed) {
    /* Type punning with different sizes */
    long long big_array[4];
    volatile int idx = seed & 0x3;
    
    /* Access subparts of larger objects */
    int *p_int = (int*)&big_array[idx];
    *p_int = seed + idx;  /* May generate SUBREG */
    
    use(big_array[0]);
    
    /* Array with sub-word access */
    int array[8];
    short *p_short = (short*)array;
    
    for (int i = 0; i < 4; i++) {
        p_short[i] = (short)(seed + i);  /* SUBREG for 16-bit store into 32-bit array */
    }
    
    /* Use volatile to prevent loop unrolling */
    volatile int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += array[i];
    }
    use(sum);
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.c = (char)seed;
    m.s = (short)(seed >> 8);
    m.i = seed;
    m.ll = (long long)seed * seed;
    
    /* Access through different type pointers */
    short *s_ptr = (short*)&m.i;
    *s_ptr = (short)(seed & 0xFFFF);  /* SUBREG for 16-bit store into int */
    
    use(m.i);
}

/* Pattern 4: Complex MEM destination */
NOINLINE void test_complex_mem(volatile int seed) {
    /* Complex addressing modes */
    int buffer[256];
    volatile int offset = (seed & 0xFF);
    
    /* Pointer arithmetic with volatile index */
    int *ptr = buffer + offset;
    *ptr = seed * 2;  /* Complex MEM address */
    
    /* Nested addressing */
    ptr = &buffer[offset * 2 + (seed & 0xF)];
    *ptr = seed - offset;
    
    /* Structure with pointer arithmetic */
    struct nested {
        int data[4][4];
        int extra;
    } ns;
    
    volatile int row = (seed >> 4) & 0x3;
    volatile int col = (seed >> 6) & 0x3;
    
    ns.data[row][col] = seed;  /* Multi-dimensional array access */
    
    /* Global/extern variable simulation */
    static int globals[100];
    volatile int gidx = seed % 100;
    
    int *gptr = &globals[gidx];
    *gptr = seed + gidx;
    
    /* Compute address with multiple operations */
    int base = seed & 0x7F;
    int index = (seed >> 7) & 0x7F;
    int scale = 2;
    
    int *computed = &buffer[base + index * scale];
    *computed = base * index;
    
    /* Use results */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += buffer[i];
    }
    total += ns.data[0][0] + *gptr + *computed;
    
    use(total);
}

/* Combined test with control flow */
NOINLINE void test_combined(volatile int seed) {
    volatile int mode = seed & 0x3;
    
    /* Loop with volatile bound to prevent unrolling */
    volatile int iterations = (seed & 0x7) + 1;
    
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch based on mode to generate different patterns */
        switch ((mode + i) & 0x3) {
            case 0: {
                /* ZERO_EXTRACT-like */
                union {
                    int val;
                    struct {
                        unsigned low: 10;
                        unsigned high: 22;
                    } bits;
                } u;
                u.val = result;
                u.bits.low = (seed + i) & 0x3FF;
                result = u.val;
                break;
            }
            case 1: {
                /* STRICT_LOW_PART-like */
                short s = (short)((seed * i) & 0xFFFF);
                result = (result & ~0xFFFF) | (s & 0xFFFF);
                break;
            }
            case 2: {
                /* SUBREG-like */
                long long temp = (long long)result * i;
                int *p = (int*)&temp;
                *p = seed + i;
                result += *p;
                break;
            }
            case 3: {
                /* Complex MEM-like */
                int local[8];
                volatile int idx = i & 0x7;
                int *p = local + idx;
                *p = seed * i;
                result += *p;
                break;
            }
        }
    }
    
    use(result);
}

/* Main driver */
int main(int argc, char **argv) {
    /* Use volatile seed from args or time */
    volatile int seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    printf("Testing with seed: %d\n", seed);
    
    /* Call all test functions */
    test_zero_extract(seed);
    test_strict_low_part(seed + 1);
    test_subreg(seed + 2);
    test_complex_mem(seed + 3);
    test_combined(seed + 4);
    
    /* Create checksum of global state */
    extern int use_result;
    printf("Test completed.\n");
    
    return 0;
}

/* Dummy definitions to satisfy linker */
void use(int x) {
    static volatile int sink;
    sink = x;
}

void sink(void *p) {
    static volatile void *sink_ptr;
    sink_ptr = p;
}
