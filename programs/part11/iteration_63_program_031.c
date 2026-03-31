/* test_resource_patterns.c
 * Designed to generate RTL SET destinations with:
 * - ZERO_EXTRACT
 * - STRICT_LOW_PART  
 * - SUBREG
 * - Complex MEM patterns
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

/* Pattern 1: Generate ZERO_EXTRACT destination */
NOINLINE void test_zero_extract(int seed) {
    volatile int src = seed;
    
    /* Method 1: Using bitfields (explicit ZERO_EXTRACT) */
    union {
        unsigned int full;
        struct {
            unsigned int low:16;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = 0xFFFFFFFF;
    u.bits.high = src & 0xFFFF;  /* Should generate ZERO_EXTRACT */
    use(u.full);
    
    /* Method 2: Manual bitfield with masking */
    unsigned int dest = 0x12345678;
    unsigned int mask = 0xFF00;
    dest = (dest & ~mask) | ((src << 8) & mask);  /* Another ZERO_EXTRACT pattern */
    use(dest);
    
    /* Method 3: Nested in loop with volatile control */
    for (volatile int i = 0; i < (seed & 3); i++) {
        unsigned int val = i * 0x11111111;
        unsigned int field = (src + i) & 0xFF;
        val = (val & ~0xFF00) | (field << 8);  /* ZERO_EXTRACT in loop */
        use(val);
    }
}

/* Pattern 2: Generate STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(int seed) {
    volatile short s = seed;
    
    /* Method 1: Assign short to int (preserve high bits) */
    int i = 0x12345678;
    i = (i & ~0xFFFF) | (s & 0xFFFF);  /* STRICT_LOW_PART candidate */
    use(i);
    
    /* Method 2: Pointer casting to short */
    int val = 0x87654321;
    *(short*)&val = s;  /* Direct low-part assignment */
    use(val);
    
    /* Method 3: In conditional with volatile */
    volatile int control = seed & 1;
    long long big = 0x123456789ABCDEF0LL;
    if (control) {
        *(int*)&big = s;  /* Low part of 64-bit */
    } else {
        *((int*)&big + 1) = s;  /* High part of 64-bit */
    }
    use(big & 0xFFFFFFFF);
    
    /* Method 4: With arithmetic */
    for (volatile int j = 0; j < 2; j++) {
        int x = j * 0x10001000;
        x = (x & ~0xFF) | ((s + j) & 0xFF);  /* STRICT_LOW_PART in loop */
        use(x);
    }
}

/* Pattern 3: Generate SUBREG destination */
NOINLINE void test_subreg(int seed) {
    volatile int idx = seed & 3;
    
    /* Method 1: Array with type punning */
    int array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    short *ps = (short*)&array[idx];
    *ps = seed & 0xFFFF;  /* SUBREG destination */
    use(array[idx]);
    
    /* Method 2: 64-bit to 32-bit access */
    long long big = 0x123456789ABCDEF0LL;
    int *p32 = (int*)&big;
    p32[idx & 1] = seed;  /* SUBREG of 64-bit value */
    use(big & 0xFFFFFFFF);
    
    /* Method 3: Structure with mixed types */
    struct Mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.ll = 0x1122334455667788LL;
    *(int*)((char*)&m.ll + idx) = seed;  /* Complex SUBREG */
    use(m.ll & 0xFFFFFFFF);
    
    /* Method 4: Nested in control flow */
    volatile int control = seed & 1;
    unsigned int buffer[2] = {0xAAAAAAAA, 0xBBBBBBBB};
    
    if (control) {
        *(short*)buffer = seed & 0xFFFF;
    } else {
        *((short*)buffer + 1) = seed & 0xFFFF;
    }
    use(buffer[0] + buffer[1]);
}

/* Pattern 4: Generate complex MEM destinations */
NOINLINE void test_complex_mem(int seed) {
    volatile int offset = seed & 7;
    
    /* Method 1: Structure with pointer arithmetic */
    struct Point {
        int x;
        int y;
        int z;
    } p = {100, 200, 300};
    
    int *ptr = &p.x + offset;  /* Computed address */
    *ptr = seed;  /* MEM with complex address */
    use(p.x + p.y + p.z);
    
    /* Method 2: Global-like access */
    static int globals[8] = {0};
    int *addr = &globals[0] + offset;
    *addr = seed * 2;  /* MEM with global base + offset */
    use(globals[offset]);
    
    /* Method 3: Array with volatile index */
    int matrix[4][4];
    volatile int row = seed & 3;
    volatile int col = (seed >> 2) & 3;
    
    int *elem = &matrix[row][col];
    *elem = seed * 3;  /* MEM with 2D array indexing */
    use(matrix[row][col]);
    
    /* Method 4: Pointer chain */
    int a = 10, b = 20, c = 30;
    int *chain[3] = {&a, &b, &c};
    int **pp = &chain[offset % 3];
    **pp = seed * 4;  /* MEM with pointer dereference chain */
    use(a + b + c);
    
    /* Method 5: With loop and condition */
    int buffer[10];
    for (volatile int i = 0; i < (seed & 3) + 1; i++) {
        int *p = &buffer[i] + offset;
        *p = seed + i;  /* MEM in loop with computed address */
    }
    use(buffer[0] + buffer[1]);
}

/* Pattern 5: Combined patterns in complex control flow */
NOINLINE void test_combined(int seed) {
    volatile int mode = seed & 3;
    
    /* Mixed operations that could generate multiple patterns */
    union {
        unsigned int full;
        struct {
            unsigned int a:10;
            unsigned int b:10;
            unsigned int c:12;
        } fields;
    } data;
    
    data.full = 0;
    
    switch (mode) {
        case 0:
            /* ZERO_EXTRACT pattern */
            data.fields.b = seed & 0x3FF;
            break;
        case 1:
            /* STRICT_LOW_PART pattern */
            *(short*)&data.full = seed & 0xFFFF;
            break;
        case 2:
            /* SUBREG pattern */
            *(char*)&data.full = seed & 0xFF;
            break;
        case 3:
            /* MEM pattern */
            {
                int *p = &data.full;
                *p = seed;
            }
            break;
    }
    
    /* Additional computation to keep values live */
    for (volatile int i = 0; i < 2; i++) {
        data.full = (data.full << 4) | (seed & 0xF);
    }
    
    use(data.full);
}

/* Main driver with volatile control flow */
int main(int argc, char *argv[]) {
    /* Use volatile seed from command line or timer */
    volatile int seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) & 0xFFFF;
    }
    
    printf("Starting with seed: %d\n", seed);
    
    /* Call all pattern generators with volatile control */
    volatile int iter = (seed & 3) + 1;
    
    for (volatile int i = 0; i < iter; i++) {
        int current_seed = seed + i * 0x1234;
        
        test_zero_extract(current_seed);
        test_strict_low_part(current_seed);
        test_subreg(current_seed);
        test_complex_mem(current_seed);
        test_combined(current_seed);
    }
    
    /* Final computation to prevent optimization */
    volatile int result = seed;
    result ^= (result >> 16);
    result ^= (result >> 8);
    
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}

/* Dummy definitions to satisfy linker */
void use(int x) {
    /* Volatile asm to prevent optimization */
    asm volatile("" : : "r"(x) : "memory");
}

void sink(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}
