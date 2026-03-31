/* test_resource_marking.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Global variables for memory patterns */
int glob_array[100];
volatile int volatile_seed;

/* 1. ZERO_EXTRACT pattern - bitfield assignment */
NOINLINE void test_zero_extract(int seed) {
    volatile int src = seed;
    
    /* Union with bitfields - explicit ZERO_EXTRACT candidate */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = 0xFFFFFFFF;
    /* This should generate SET with ZERO_EXTRACT destination */
    u.bits.mid = src & 0xFF;
    
    /* Complex bitfield manipulation in loop */
    for (int i = 0; i < (seed & 3); i++) {
        u.bits.low = (src >> (i * 4)) & 0xF;
        u.bits.high = (src >> (16 + i * 2)) & 0x3;
    }
    
    use(u.full);
}

/* 2. STRICT_LOW_PART pattern - low part assignment */
NOINLINE void test_strict_low_part(int seed) {
    volatile short s = seed & 0xFFFF;
    int i = 0x12345678;
    
    /* Multiple patterns that could generate STRICT_LOW_PART */
    
    /* Pattern A: Direct low part assignment via masking */
    i = (i & ~0xFFFF) | (s & 0xFFFF);
    
    /* Pattern B: Pointer cast to short */
    int val = 0x87654321;
    *(short*)&val = s;
    
    /* Pattern C: In conditional with volatile */
    if (seed & 1) {
        i = (i & ~0xFFFF) | (s & 0xFFFF);
    } else {
        *(short*)&val = s + 1;
    }
    
    /* Pattern D: Loop with low part assignments */
    for (int j = 0; j < (seed & 7); j++) {
        val = (val & ~0xFFFF) | ((s + j) & 0xFFFF);
    }
    
    use(i + val);
}

/* 3. SUBREG pattern - sub-register access */
NOINLINE void test_subreg(int seed) {
    volatile int idx = seed;
    
    /* Pattern A: Array with sub-word access */
    int array[4] = {0, 0, 0, 0};
    short *ps = (short*)&array[idx & 3];
    *ps = seed & 0xFFFF;
    
    /* Pattern B: Type punning with different sizes */
    long long big = 0x1122334455667788LL;
    int *p32 = (int*)&big;
    p32[(idx >> 1) & 1] = seed;
    
    /* Pattern C: Structure with mixed types */
    struct Mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = seed;
    m.s = seed & 0xFFFF;
    m.c = seed & 0xFF;
    
    /* Pattern D: Pointer arithmetic with sub-word types */
    char *byte_ptr = (char*)array;
    for (int i = 0; i < (seed & 15); i++) {
        byte_ptr[i] = (seed + i) & 0xFF;
    }
    
    use(array[0] + array[1] + array[2] + array[3]);
    use(big & 0xFFFFFFFF);
    use(m.i + m.s);
}

/* 4. Complex MEM pattern - memory with addressing modes */
NOINLINE void test_complex_mem(int seed) {
    volatile int off = seed;
    
    /* Pattern A: Structure with offset */
    struct S {
        int a;
        int b[10];
        int c;
    } s;
    
    int *ptr = &s.a + (off & 7);
    *ptr = seed;
    
    /* Pattern B: Global array with index calculation */
    int index = (off * 37) % 100;
    glob_array[index] = seed;
    glob_array[(index + 1) % 100] = seed + 1;
    
    /* Pattern C: Pointer chain */
    int **pptr = &ptr;
    **pptr = seed * 2;
    
    /* Pattern D: Memory with scaled index */
    for (int i = 0; i < (seed & 3); i++) {
        s.b[i * 2] = seed + i;
    }
    
    /* Pattern E: Stack and global mix */
    int local = 0;
    int *addr = (seed & 1) ? &local : &glob_array[0];
    *addr = seed;
    
    use(s.a + s.b[0] + s.c);
    use(glob_array[0] + glob_array[99]);
}

/* 5. Combined patterns in complex control flow */
NOINLINE void test_combined(int seed) {
    volatile int v = seed;
    int result = 0;
    
    /* Mixed patterns in switch */
    switch (v & 3) {
        case 0: {
            /* ZERO_EXTRACT-like */
            union {
                unsigned int val;
                struct {
                    unsigned int a:5;
                    unsigned int b:11;
                    unsigned int c:16;
                } f;
            } u;
            u.val = 0;
            u.f.b = v & 0x7FF;
            result = u.val;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART-like */
            int x = 0xABCD1234;
            x = (x & ~0xFFFF) | (v & 0xFFFF);
            result = x;
            break;
        }
        case 2: {
            /* SUBREG-like */
            long long ll = 0;
            int *p = (int*)&ll;
            p[v & 1] = v;
            result = ll & 0xFFFFFFFF;
            break;
        }
        case 3: {
            /* MEM-like */
            int arr[2] = {0};
            int idx = v & 1;
            arr[idx] = v;
            result = arr[0] + arr[1];
            break;
        }
    }
    
    /* Loop with varying patterns */
    for (int i = 0; i < (v & 7); i++) {
        if (i & 1) {
            /* SUBREG access */
            char *cptr = (char*)&result;
            cptr[i & 3] = v + i;
        } else {
            /* Bit manipulation */
            result ^= (v << (i * 4));
        }
    }
    
    use(result);
}

/* Main driver */
int main(int argc, char **argv) {
    /* Use argument or time as volatile seed */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    volatile_seed = seed;
    
    /* Call all test functions with volatile variations */
    test_zero_extract(volatile_seed);
    test_strict_low_part(volatile_seed + 1);
    test_subreg(volatile_seed + 2);
    test_complex_mem(volatile_seed + 3);
    test_combined(volatile_seed + 4);
    
    /* Create checksum from globals to prevent elimination */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= glob_array[i];
    }
    
    printf("Result: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy linker */
void use(int x) {
    volatile_seed += x;
}

void sink(void *p) {
    (void)p;
}
