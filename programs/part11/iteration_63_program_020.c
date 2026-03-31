/* test_resource_patterns.c
 * Designed to trigger specific SET destination patterns in GCC RTL:
 * - ZERO_EXTRACT destinations
 * - STRICT_LOW_PART destinations  
 * - SUBREG destinations
 * - Complex MEM destinations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Pattern 1: ZERO_EXTRACT destinations */
NOINLINE void test_zero_extract(int seed) {
    volatile int src = seed;
    
    /* Using union with bitfields - likely to generate ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = 0;
    u.bits.mid = src & 0xFF;  /* Should generate SET with ZERO_EXTRACT dest */
    use(u.full);
    
    /* Another pattern: bitfield assignment with masking */
    unsigned int val = 0;
    unsigned int mask = 0xFF00;
    val = (val & ~mask) | ((src << 8) & mask);  /* ZERO_EXTRACT pattern */
    use(val);
    
    /* Loop with bitfield operations */
    for (volatile int i = 0; i < (seed & 3); i++) {
        u.bits.high = (src + i) & 0xFFFF;
        use(u.full);
    }
}

/* Pattern 2: STRICT_LOW_PART destinations */
NOINLINE void test_strict_low_part(int seed) {
    volatile short s = seed;
    volatile int i = seed * 2;
    
    /* Assigning short to int - should generate STRICT_LOW_PART */
    i = (i & ~0xFFFF) | (s & 0xFFFF);
    use(i);
    
    /* Pointer casting pattern */
    int val = seed;
    *(short*)&val = s;  /* Likely STRICT_LOW_PART destination */
    use(val);
    
    /* Array with type punning */
    int arr[4] = {0};
    volatile int idx = seed & 3;
    short* ps = (short*)&arr[idx];
    *ps = s;  /* STRICT_LOW_PART through pointer */
    use(arr[idx]);
    
    /* In loop with condition */
    for (volatile int j = 0; j < (seed & 7); j++) {
        long long big = j;
        int* p = (int*)&big;
        *p = s + j;  /* STRICT_LOW_PART on 64-bit */
        use(big);
    }
}

/* Pattern 3: SUBREG destinations */
NOINLINE void test_subreg(int seed) {
    volatile int v = seed;
    
    /* Type punning between different sizes */
    long long big = v;
    int* p_int = (int*)&big;
    *p_int = v * 2;  /* SUBREG destination */
    use(big);
    
    /* Array access with sub-word type */
    int array[8];
    volatile int idx = v & 7;
    short* ps = (short*)&array[idx];
    *ps = v & 0xFFFF;  /* SUBREG destination */
    use(array[idx]);
    
    /* Structure with mixed types */
    struct Mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = v;
    short* ps2 = (short*)&m.i;
    *ps2 = v & 0xFFFF;  /* SUBREG destination */
    use(m.i);
    
    /* Complex addressing with SUBREG */
    for (volatile int i = 0; i < (v & 3); i++) {
        long long buffer[4];
        int* ptr = (int*)&buffer[i];
        *ptr = v + i;  /* SUBREG destination */
        use(buffer[i]);
    }
}

/* Pattern 4: Complex MEM destinations */
NOINLINE void test_complex_mem(int seed) {
    volatile int v = seed;
    volatile int off = v & 0xF;
    
    /* Structure with pointer arithmetic */
    struct S {
        int a;
        int b[4];
        int c;
    } s;
    
    int* ptr = &s.a + off;  /* Complex address calculation */
    *ptr = v;  /* MEM destination with complex address */
    use(s.a);
    
    /* Global/extern variable simulation */
    static int glob[16];
    volatile int idx = v & 15;
    int* addr = &glob[0] + idx;
    *addr = v * 3;  /* MEM with indexed address */
    use(glob[idx]);
    
    /* Multi-dimensional array */
    int matrix[4][4];
    volatile int row = v & 3;
    volatile int col = (v >> 2) & 3;
    int* mptr = &matrix[row][col];
    *mptr = v;  /* MEM with 2D addressing */
    use(matrix[row][col]);
    
    /* Pointer chain */
    int val = v;
    int* p1 = &val;
    int** p2 = &p1;
    ***&p2 = v + 1;  /* Complex MEM destination */
    use(val);
    
    /* Loop with varying addresses */
    int buffer[32];
    for (volatile int i = 0; i < (v & 31); i++) {
        int* bptr = buffer + ((i + off) & 31);
        *bptr = v + i;  /* MEM with computed address */
        use(buffer[i]);
    }
}

/* Combined test with all patterns */
NOINLINE int test_all_patterns(int seed) {
    int result = seed;
    
    test_zero_extract(result);
    result ^= 0x5555;
    
    test_strict_low_part(result);
    result ^= 0xAAAA;
    
    test_subreg(result);
    result ^= 0x3333;
    
    test_complex_mem(result);
    result ^= 0xCCCC;
    
    return result;
}

int main(int argc, char** argv) {
    /* Use volatile seed from command line or timer */
    volatile int seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    /* Call test functions multiple times with different seeds */
    int checksum = 0;
    for (volatile int i = 0; i < 3; i++) {
        int r = test_all_patterns(seed + i);
        checksum ^= r;
        use(checksum);
    }
    
    /* Prevent dead code elimination */
    sink((void*)&checksum);
    
    printf("Result: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy external references */
void use(int x) {
    volatile static int sink;
    sink = x;
}

void sink(void* p) {
    volatile static void* psink;
    psink = p;
}
