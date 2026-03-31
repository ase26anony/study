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

/* Pattern 1: ZERO_EXTRACT destination via bitfields */
NOINLINE void test_zero_extract(volatile int seed) {
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = seed;
    volatile int src = volatile_input();
    
    /* Multiple assignments to different bitfields */
    u.bits.low = src & 0xFF;
    u.bits.mid = (src >> 8) & 0xFF;
    u.bits.high = (src >> 16) & 0xFFFF;
    
    /* Force computation with control flow */
    for (int i = 0; i < (seed & 3); i++) {
        u.bits.low ^= (src >> i) & 1;
    }
    
    use(u.full);
}

/* Pattern 2: STRICT_LOW_PART destination via type punning */
NOINLINE void test_strict_low_part(volatile int seed) {
    volatile short s = (short)(seed ^ 0x1234);
    int i = seed * 3;
    
    /* Assign short to low part of int */
    i = (i & ~0xFFFF) | (s & 0xFFFF);
    
    /* Alternative via pointer casting */
    int val = seed;
    *(short*)&val = s;
    
    /* In loop to create multiple SETs */
    for (int j = 0; j < (seed & 7); j++) {
        val = (val & ~0xFFFF) | ((s + j) & 0xFFFF);
    }
    
    use(i + val);
}

/* Pattern 3: SUBREG destination via mixed-size accesses */
NOINLINE void test_subreg(volatile int seed) {
    long long big = (long long)seed * 1000LL;
    volatile int idx = seed & 3;
    
    /* Access different parts of long long */
    int* p32 = (int*)&big;
    p32[idx] = volatile_input();
    
    /* Array with sub-word accesses */
    int arr[4] = {seed, seed+1, seed+2, seed+3};
    short* ps = (short*)arr;
    ps[idx * 2] = (short)volatile_input();
    
    /* Structure with mixed types */
    struct Mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = seed;
    *(short*)((char*)&m + 1) = (short)volatile_input(); /* Unaligned access */
    
    use(p32[0] + ps[0] + m.i);
}

/* Pattern 4: Complex MEM destination with addressing modes */
NOINLINE void test_complex_mem(volatile int seed) {
    static int globals[16];
    volatile int off = seed & 15;
    
    /* Pointer arithmetic with volatile offset */
    int* ptr1 = &globals[0] + off;
    *ptr1 = volatile_input();
    
    /* Two-dimensional indexing */
    int matrix[4][4];
    volatile int row = seed & 3;
    volatile int col = seed & 3;
    matrix[row][col] = volatile_input();
    
    /* Structure with pointer chain */
    struct Node {
        int value;
        struct Node* next;
    } nodes[4];
    
    for (int i = 0; i < 4; i++) {
        nodes[i].value = seed + i;
        nodes[i].next = &nodes[(i + 1) & 3];
    }
    
    volatile int idx = seed & 3;
    nodes[idx].next->value = volatile_input();
    
    /* Global with computed address */
    extern int extern_global;
    int* addr = &extern_global + (seed & 1);
    *addr = 1;
    
    use(globals[0] + matrix[0][0] + nodes[0].value);
}

/* Pattern 5: Combined patterns in complex control flow */
NOINLINE void test_combined(volatile int seed) {
    volatile int mode = seed & 3;
    int result = 0;
    
    for (int i = 0; i < (seed & 7) + 1; i++) {
        switch (mode) {
            case 0: {
                /* ZERO_EXTRACT pattern */
                union {
                    unsigned int val;
                    struct { unsigned int a:4, b:4, c:24; } f;
                } u;
                u.val = seed + i;
                u.f.b = (volatile_input() + i) & 0xF;
                result += u.val;
                break;
            }
            case 1: {
                /* STRICT_LOW_PART pattern */
                int x = result;
                short y = (short)(volatile_input() + i);
                x = (x & ~0xFFFF) | (y & 0xFFFF);
                result = x ^ i;
                break;
            }
            case 2: {
                /* SUBREG pattern */
                long long ll = (long long)result * i;
                int* p = (int*)&ll + (i & 1);
                *p = volatile_input();
                result += (int)ll;
                break;
            }
            case 3: {
                /* MEM pattern */
                int arr[8];
                volatile int idx = (seed + i) & 7;
                int* p = &arr[0] + idx;
                *p = volatile_input() + i;
                result += arr[idx];
                break;
            }
        }
        
        /* Conditional to create more complex CFG */
        if (result & 1) {
            mode = (mode + 1) & 3;
        }
    }
    
    use(result);
}

/* Main driver */
int main(int argc, char** argv) {
    volatile int seed;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    printf("Seed: %d\n", seed);
    
    /* Call all test patterns */
    test_zero_extract(seed);
    test_strict_low_part(seed);
    test_subreg(seed);
    test_complex_mem(seed);
    test_combined(seed);
    
    /* Create checksum to prevent optimization */
    volatile int checksum = seed;
    checksum ^= (seed * 3);
    checksum ^= (seed >> 4);
    
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy extern declarations */
void use(int x) {
    static volatile int sink;
    sink = x;
}

void sink(void* p) {
    static volatile void* vsink;
    vsink = p;
}

int volatile_input(void) {
    static volatile int counter = 0;
    return ++counter;
}

int extern_global = 0;
