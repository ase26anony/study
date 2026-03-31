/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Global variables for complex addressing */
int glob_array[100];
volatile int volatile_idx = 0;

/* Pattern 1: ZERO_EXTRACT destination via bitfields */
NOINLINE void test_zero_extract(volatile int seed) {
    /* Method 1: Union with bitfields */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = seed;
    /* This assignment to bitfield may generate ZERO_EXTRACT */
    u.bits.mid = volatile_input() & 0xFF;
    use(u.full);
    
    /* Method 2: Manual bitfield extraction */
    unsigned int val = seed;
    unsigned int mask = 0xFF00;
    unsigned int insert = (volatile_input() & 0xFF) << 8;
    /* May generate: (set (zero_extract:SI (reg:SI X) (const_int 8) (const_int 8))
     *                 (and:SI (reg:SI Y) (const_int 255))) */
    val = (val & ~mask) | insert;
    use(val);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(volatile int seed) {
    /* Method 1: Short to int assignment */
    int i = seed;
    short s = volatile_input() & 0xFFFF;
    
    /* This may generate STRICT_LOW_PART */
    i = (i & ~0xFFFF) | (s & 0xFFFF);
    use(i);
    
    /* Method 2: Pointer casting */
    long long big = seed;
    short *ps = (short*)&big;
    *ps = volatile_input() & 0xFFFF;
    use(big);
    
    /* Method 3: In loop with volatile */
    for (int j = 0; j < (seed & 3); j++) {
        int val = j * 1000;
        short low = volatile_input() & 0x7FFF;
        /* Force STRICT_LOW_PART pattern */
        val = (val & ~0xFFFF) | low;
        use(val);
    }
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(volatile int seed) {
    /* Method 1: Type punning with different sizes */
    long long big_val = seed;
    int *p_int = (int*)&big_val;
    
    /* This assignment may use SUBREG to access part of big_val */
    *p_int = volatile_input();
    use(big_val);
    
    /* Method 2: Array with sub-word access */
    int arr[4] = {seed, seed+1, seed+2, seed+3};
    short *ps = (short*)arr;
    
    volatile int idx = volatile_idx & 3;
    /* SUBREG for accessing half-word */
    ps[idx] = volatile_input() & 0xFFFF;
    use(arr[0]);
    
    /* Method 3: Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.ll = seed;
    int *p = (int*)((char*)&m + 2);
    *p = volatile_input();  /* Misaligned access may use SUBREG */
    use(m.ll);
}

/* Pattern 4: Complex MEM destinations */
NOINLINE void test_complex_mem(volatile int seed) {
    /* Method 1: Pointer arithmetic with volatile offset */
    int *ptr = &glob_array[0];
    volatile int offset = volatile_idx % 50;
    
    /* Complex addressing: MEM with PLUS and MULT */
    ptr[offset * 2] = volatile_input();
    use(glob_array[0]);
    
    /* Method 2: Structure with computed member access */
    struct point {
        int x;
        int y;
        int z;
    } pt = {seed, seed+1, seed+2};
    
    int *member_ptr = &pt.x + (volatile_idx & 2);
    *member_ptr = volatile_input();
    use(pt.x);
    
    /* Method 3: Global variable with index calculation */
    extern int extern_glob;
    int *addr = &extern_glob + (seed & 0xF);
    *addr = volatile_input() + 1;
    
    /* Method 4: Two-dimensional indexing */
    int matrix[10][10];
    volatile int row = volatile_idx % 10;
    volatile int col = volatile_idx % 10;
    
    matrix[row][col] = volatile_input();
    use(matrix[0][0]);
}

/* Pattern 5: Combined patterns in control flow */
NOINLINE void test_combined(volatile int seed) {
    int result = 0;
    
    /* Loop with multiple patterns */
    for (int i = 0; i < (seed & 7); i++) {
        volatile int cond = volatile_input() & 1;
        
        if (cond) {
            /* ZERO_EXTRACT pattern */
            unsigned int val = i * 100;
            unsigned int field = volatile_input() & 0xF;
            val = (val & ~0xF0) | (field << 4);
            result += val;
        } else {
            /* STRICT_LOW_PART pattern */
            int val = i * 1000;
            short low = volatile_input() & 0xFF;
            val = (val & ~0xFF) | low;
            result -= val;
        }
        
        /* SUBREG access every iteration */
        long long big = result;
        int *half = (int*)&big + (i & 1);
        *half = volatile_input();
        result = big & 0xFFFFFFFF;
    }
    
    /* Complex MEM at the end */
    glob_array[(result ^ seed) % 100] = result;
    use(result);
}

/* Main driver with volatile control flow */
int main(int argc, char *argv[]) {
    /* Use argv for volatile seed */
    volatile int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize global */
    for (int i = 0; i < 100; i++) {
        glob_array[i] = i;
    }
    
    /* Call pattern generators with volatile control */
    if (seed & 1) {
        test_zero_extract(seed);
    }
    
    if (seed & 2) {
        test_strict_low_part(seed + 1);
    }
    
    volatile int iter = (seed >> 2) & 3;
    for (int i = 0; i < iter; i++) {
        test_subreg(seed + i);
    }
    
    test_complex_mem(seed);
    test_combined(seed ^ 0x55AA);
    
    /* Create checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= glob_array[i];
    }
    
    printf("Result: %d\n", checksum);
    return 0;
}

/* Dummy definitions to satisfy extern declarations */
void use(int x) {
    /* Empty but prevents dead code elimination */
    volatile static int sink;
    sink = x;
}

void sink(void *p) {
    volatile static void *vsink;
    vsink = p;
}

int volatile_input(void) {
    static volatile int counter = 0;
    return ++counter;
}

/* External global */
int extern_glob = 42;
