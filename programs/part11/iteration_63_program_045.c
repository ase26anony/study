/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Volatile seed to prevent compile-time optimization */
static volatile int seed = 0;

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Pattern 1: ZERO_EXTRACT destination */
NOINLINE void test_zero_extract(volatile int input) {
    /* Use union with bitfields to encourage ZERO_EXTRACT */
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
    
    /* Assignment to bitfield - may generate ZERO_EXTRACT destination */
    u.bits.mid = input & 0xFF;
    
    /* Use result to keep computation live */
    sink(u.full);
    
    /* Another pattern: manual bitfield manipulation */
    unsigned int val = seed;
    unsigned int mask = 0xFF00;
    unsigned int insert = (input & 0xFF) << 8;
    
    /* This may generate ZERO_EXTRACT in RTL */
    val = (val & ~mask) | insert;
    
    sink(val);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(volatile short input) {
    int large_val = seed;
    
    /* Assignment from short to int - may generate STRICT_LOW_PART */
    large_val = (large_val & ~0xFFFF) | (input & 0xFFFF);
    
    sink(large_val);
    
    /* Using pointer cast to access low part */
    long long big_val = seed * 1000LL;
    short *p = (short*)&big_val;
    
    /* This may generate STRICT_LOW_PART */
    *p = input;
    
    sink((int)big_val);
    
    /* Another pattern with explicit masking */
    unsigned int x = seed;
    unsigned short y = input;
    x = (x & 0xFFFF0000) | y;
    
    sink(x);
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(volatile int input) {
    /* Type punning with different sizes */
    long long big_array[2] = {seed, seed + 1};
    volatile int idx = input & 1;
    
    /* Access sub-word part - may generate SUBREG */
    int *p_int = (int*)&big_array[idx];
    *p_int = input;
    
    sink((int)big_array[0]);
    
    /* Array with sub-word access */
    int array[4] = {seed, seed + 1, seed + 2, seed + 3};
    short *p_short = (short*)array;
    
    /* Volatile index prevents constant addressing */
    p_short[input & 3] = (short)input;
    
    sink(array[0]);
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.ll = seed;
    /* Access subpart - may generate SUBREG */
    *(short*)((char*)&m + 1) = (short)input;
    
    sink((int)m.ll);
}

/* Pattern 4: Complex MEM destination */
NOINLINE void test_complex_mem(volatile int input) {
    /* Global/static variable with computed address */
    static int globals[10];
    volatile int offset = input % 8;
    
    /* Complex addressing mode */
    int *addr = &globals[offset + 1];
    *addr = input;
    
    sink(globals[0]);
    
    /* Structure with pointer arithmetic */
    struct point {
        int x;
        int y;
        int z;
    } pt;
    
    volatile int field = input % 3;
    int *field_ptr = (int*)((char*)&pt + field * sizeof(int));
    *field_ptr = input;
    
    sink(pt.x);
    
    /* Multi-dimensional array with volatile index */
    int matrix[3][3];
    volatile int row = input % 3;
    volatile int col = input % 3;
    
    int *elem = &matrix[row][col];
    *elem = input;
    
    sink(matrix[0][0]);
    
    /* Pointer chain */
    int a = seed;
    int *p1 = &a;
    int **p2 = &p1;
    int ***p3 = &p2;
    
    ***p3 = input;
    
    sink(a);
}

/* Pattern 5: Combined patterns in loops */
NOINLINE void test_combined(volatile int bound) {
    int sum = 0;
    
    /* Loop with multiple patterns */
    for (int i = 0; i < (bound & 7) + 1; i++) {
        volatile int iter_seed = seed + i;
        
        /* ZERO_EXTRACT pattern */
        union {
            unsigned int val;
            struct {
                unsigned int a: 4;
                unsigned int b: 4;
                unsigned int c: 24;
            } fields;
        } u;
        
        u.val = iter_seed;
        u.fields.b = i & 0xF;
        sum += u.val;
        
        /* STRICT_LOW_PART pattern */
        if (i & 1) {
            long x = sum;
            short *sp = (short*)&x;
            sp[0] = (short)iter_seed;
            sum += (int)x;
        }
        
        /* SUBREG pattern */
        long long buffer[2];
        int *ip = (int*)buffer;
        ip[i & 1] = iter_seed;
        sum += (int)buffer[0];
        
        /* Complex MEM pattern */
        static int counters[8];
        volatile int idx = iter_seed % 8;
        int *ptr = &counters[idx];
        *ptr += i;
        sum += *ptr;
    }
    
    sink(sum);
}

/* Main driver */
int main(int argc, char **argv) {
    /* Initialize volatile seed from command line or time */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    printf("Starting with seed: %d\n", seed);
    
    /* Call each test function with volatile inputs */
    volatile int input1 = seed ^ 0x55AA;
    volatile short input2 = (short)(seed ^ 0x33CC);
    volatile int input3 = seed * 7919;
    volatile int input4 = seed % 100;
    volatile int input5 = (seed >> 4) & 0xF;
    
    test_zero_extract(input1);
    test_strict_low_part(input2);
    test_subreg(input3);
    test_complex_mem(input4);
    test_combined(input5);
    
    /* Create checksum to use all results */
    int checksum = input1 + input2 + input3 + input4 + input5;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_long(long x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void sink(int x) { (void)x; }
