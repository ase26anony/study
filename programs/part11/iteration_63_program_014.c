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

/* Pattern 1: ZERO_EXTRACT destination via bitfield operations */
NOINLINE void test_zero_extract(volatile int seed) {
    /* Use union with bitfields for explicit ZERO_EXTRACT pattern */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    /* Initialize with volatile to prevent constant propagation */
    u.full = seed;
    volatile int mask = (seed >> 8) & 0xFF;
    
    /* Assignment to bitfield - may generate ZERO_EXTRACT destination */
    u.bits.mid = mask;
    
    /* Use result to keep computation live */
    use(u.full);
    
    /* Another pattern: explicit bitwise operation */
    unsigned int val = seed;
    unsigned int extract = (seed * 3) & 0xFF;
    
    /* Store into specific bits - may generate ZERO_EXTRACT */
    val = (val & ~(0xFF << 8)) | (extract << 8);
    use(val);
}

/* Pattern 2: STRICT_LOW_PART destination via partial word assignment */
NOINLINE void test_strict_low_part(volatile int seed) {
    /* Assign to low part of larger integer */
    long long big = (long long)seed * 1000;
    short low_part = (short)(seed & 0xFFFF);
    
    /* This may generate STRICT_LOW_PART destination */
    *(short*)&big = low_part;
    use((int)big);
    
    /* Another pattern using type punning */
    int value = seed;
    char *byte_ptr = (char*)&value;
    volatile char byte = (char)(seed >> 16);
    
    /* Assign to low byte - may use STRICT_LOW_PART */
    *byte_ptr = byte;
    use(value);
    
    /* Explicit masking for low part preservation */
    unsigned int reg = seed;
    unsigned short low = (seed * 7) & 0xFFFF;
    reg = (reg & ~0xFFFF) | low;
    use(reg);
}

/* Pattern 3: SUBREG destination via sub-word access */
NOINLINE void test_subreg(volatile int seed) {
    /* Array access with type punning */
    int array[4] = {seed, seed + 1, seed + 2, seed + 3};
    volatile int idx = seed % 3;
    
    /* Access sub-word via pointer cast - may generate SUBREG */
    short *ps = (short*)&array[idx];
    *ps = (short)(seed & 0xFFFF);
    use(array[idx]);
    
    /* Structure with mixed types */
    struct mixed {
        long long a;
        int b;
        short c;
        char d;
    } m;
    
    m.a = seed;
    m.b = seed * 2;
    
    /* Access part of structure - may use SUBREG */
    int *partial = (int*)&m.a;
    *partial = seed * 3;
    use(m.a);
    
    /* Union type punning */
    union {
        double dbl;
        unsigned int parts[2];
    } u;
    
    u.dbl = seed * 1.5;
    u.parts[0] = seed;  /* May generate SUBREG access */
    use(u.parts[1]);
}

/* Pattern 4: Complex MEM destination with addressing modes */
NOINLINE void test_complex_mem(volatile int seed) {
    /* Global/static variable */
    static int globals[10];
    volatile int offset = seed % 8;
    
    /* Complex addressing mode */
    int *addr = &globals[offset + 1];
    *addr = seed * 2;
    use(globals[offset + 1]);
    
    /* Structure with pointer arithmetic */
    struct point {
        int x, y, z;
    } pt;
    
    volatile int field = seed % 3;
    int *field_ptr = (int*)((char*)&pt + field * sizeof(int));
    *field_ptr = seed;
    use(pt.x + pt.y + pt.z);
    
    /* Two-dimensional array with computed index */
    int matrix[5][5];
    volatile int i = seed % 5;
    volatile int j = (seed >> 4) % 5;
    
    /* Complex memory destination */
    matrix[i][j] = seed * i + j;
    use(matrix[i][j]);
    
    /* Pointer chasing */
    int *ptr1 = &matrix[0][0];
    int *ptr2 = ptr1 + (seed % 24);
    *ptr2 = seed;
    use(*ptr2);
}

/* Pattern 5: Combined patterns in loops */
NOINLINE void test_combined(volatile int seed) {
    int result = 0;
    volatile int limit = (seed % 10) + 1;
    
    for (int i = 0; i < limit; i++) {
        /* Mix different patterns in loop */
        union {
            unsigned int val;
            struct {
                unsigned short low;
                unsigned short high;
            } parts;
        } u;
        
        u.val = seed + i;
        
        /* ZERO_EXTRACT-like */
        u.parts.low = (seed * i) & 0xFFFF;
        
        /* SUBREG-like access */
        unsigned short *ptr = (unsigned short*)&u.val + (i & 1);
        *ptr = (seed + i * 3) & 0xFFFF;
        
        /* Complex MEM with addressing */
        static int buffer[100];
        volatile int idx = (seed + i) % 50;
        buffer[idx * 2] = u.val;
        
        result += u.val + buffer[idx * 2];
    }
    
    use(result);
}

/* Main driver with volatile control flow */
int main(int argc, char **argv) {
    /* Volatile seed from command line or timer */
    volatile int seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    /* Call pattern generators with volatile conditions */
    if (seed & 1) {
        test_zero_extract(seed);
    }
    
    if (seed & 2) {
        test_strict_low_part(seed * 3);
    }
    
    volatile int counter = seed % 5;
    for (int i = 0; i < counter; i++) {
        test_subreg(seed + i * 7);
    }
    
    if (seed & 4) {
        test_complex_mem(seed * 11);
    }
    
    test_combined(seed);
    
    /* Prevent dead code elimination */
    sink(&seed);
    
    printf("Seed: %d\n", seed);
    return 0;
}

/* Dummy definitions to satisfy linker (in real use would be external) */
void use(int x) {
    static volatile int sink;
    sink = x;
}

void sink(void *p) {
    static volatile void *vsink;
    vsink = p;
}
