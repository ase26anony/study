/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long long);
extern void use_ptr(void *);
extern void sink(int);

/* Volatile seed to prevent compile-time optimization */
static volatile int volatile_seed;

/* Pattern 1: ZERO_EXTRACT destination */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int input) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = volatile_seed;
    /* This assignment to a bitfield may generate ZERO_EXTRACT */
    u.bits.mid = input & 0xFF;
    
    /* Complex control flow to keep it alive */
    for (int i = 0; i < (input & 3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
    }
    
    use_int(u.full);
}

/* Pattern 2: STRICT_LOW_PART destination */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile short input) {
    int val = volatile_seed;
    
    /* Assignment to low part of larger integer */
    /* May generate STRICT_LOW_PART */
    short *ps = (short *)&val;
    *ps = input;
    
    /* Alternative: using masking */
    int val2 = volatile_seed;
    val2 = (val2 & ~0xFFFF) | (input & 0xFFFF);
    
    /* Mix with control flow */
    if (input & 1) {
        use_int(val);
    } else {
        use_int(val2);
    }
}

/* Pattern 3: SUBREG destination */
__attribute__((noinline, noipa))
void test_subreg(volatile int input) {
    /* Type punning with different sizes */
    long long big = (long long)volatile_seed * 1000LL;
    
    /* Accessing sub-parts - may generate SUBREG */
    int *p1 = (int *)&big;
    *p1 = input;
    
    int *p2 = (int *)((char *)&big + 4);
    *p2 = input + 1;
    
    /* Array with sub-word access */
    int arr[4] = {0};
    volatile int idx = input & 3;
    short *ps = (short *)&arr[idx];
    *ps = (short)input;
    
    /* Complex addressing */
    for (int i = 0; i < (input & 3); i++) {
        int *ptr = (int *)((char *)arr + i * sizeof(short));
        *(short *)ptr = (short)(input + i);
    }
    
    use_long(big);
    use_int(arr[0] + arr[1]);
}

/* Pattern 4: Complex MEM destination */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int input) {
    /* Structure with pointer arithmetic */
    struct S {
        int a;
        int b;
        int c;
        int d;
    } s = {0};
    
    volatile int offset = input & 3;
    
    /* Complex addressing mode for MEM */
    int *ptr = &s.a + offset;
    *ptr = input;
    
    /* More complex addressing with scaling */
    int *ptr2 = (int *)((char *)&s + offset * sizeof(int));
    *ptr2 = input + 1;
    
    /* Global-like access */
    static int globals[10];
    volatile int idx = input % 10;
    int *addr = &globals[0] + idx;
    *addr = input * 2;
    
    /* Even more complex: pointer to pointer */
    int **pp = &addr;
    **pp = input * 3;
    
    use_int(s.a + s.b);
    use_int(globals[idx]);
}

/* Pattern 5: Combined patterns in loop */
__attribute__((noinline, noipa))
void test_combined(volatile int iterations) {
    union {
        unsigned int full;
        struct {
            unsigned int low:12;
            unsigned int high:20;
        } bits;
    } data;
    
    long long accumulator = 0;
    
    for (int i = 0; i < (iterations & 7); i++) {
        /* Mix different patterns */
        volatile int val = volatile_seed + i;
        
        /* ZERO_EXTRACT pattern */
        data.full = val;
        data.bits.low = (val + i) & 0xFFF;
        
        /* SUBREG pattern */
        long long big = accumulator;
        int *p = (int *)&big;
        *p = val;
        
        /* MEM pattern with complex addressing */
        int array[8] = {0};
        int *mem_ptr = array + (val & 7);
        *mem_ptr = val;
        
        /* STRICT_LOW_PART-like */
        int temp = val;
        short *sp = (short *)&temp;
        *sp = (short)(val >> 4);
        
        accumulator = big + data.full + array[0] + temp;
    }
    
    use_long(accumulator);
}

/* Main driver */
int main(int argc, char *argv[]) {
    /* Initialize volatile seed */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    
    /* Call pattern generators with volatile inputs */
    test_zero_extract(volatile_seed);
    test_strict_low_part((short)(volatile_seed & 0xFFFF));
    test_subreg(volatile_seed + 1);
    test_complex_mem(volatile_seed + 2);
    test_combined(volatile_seed + 3);
    
    /* Create a checksum to prevent dead code elimination */
    int checksum = volatile_seed;
    checksum ^= (volatile_seed >> 16);
    
    printf("Result: %d\n", checksum);
    return checksum & 1;
}

/* Dummy definitions to satisfy linker (in real use would be in separate file) */
void use_int(int x) { sink(x); }
void use_long(long long x) { sink((int)x); }
void use_ptr(void *x) { sink((int)(long)x); }
void sink(int x) { volatile int dummy = x; (void)dummy; }
