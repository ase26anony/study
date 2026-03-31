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

/* Volatile helpers */
volatile int volatile_seed;
volatile int volatile_bound;

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
    
    /* Force initialization */
    u.full = input;
    
    /* Store into bitfield - may generate ZERO_EXTRACT destination */
    u.bits.mid = (input >> 4) & 0xFF;
    
    /* Complex control flow to keep it alive */
    for (int i = 0; i < (volatile_bound & 3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
    }
    
    use_int(u.full);
}

/* Pattern 2: STRICT_LOW_PART destination */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile short input) {
    int val = 0x12345678;
    
    /* Assign short to int - may generate STRICT_LOW_PART */
    /* Using pointer cast approach */
    *(short*)&val = input;
    
    /* Alternative: using bitwise operations */
    int val2 = 0x87654321;
    short s = input + 1;
    val2 = (val2 & ~0xFFFF) | (s & 0xFFFF);
    
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
    long long big = 0x1122334455667788LL;
    
    /* Access part of long long as int - may generate SUBREG */
    int* p_int = (int*)&big;
    *p_int = input;
    
    /* Array with sub-word access */
    int arr[4] = {0, 0, 0, 0};
    volatile int idx = input & 3;
    short* p_short = (short*)&arr[idx];
    *p_short = (short)input;
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = input;
    m.s = (short)(input >> 16);
    
    /* Loop to increase complexity */
    for (int i = 0; i < (volatile_bound & 7); i++) {
        p_int[i & 1] += i;
    }
    
    use_int(arr[0] + arr[1]);
    use_long(big);
}

/* Pattern 4: Complex MEM destination with addressing modes */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int input) {
    /* Global/static variables */
    static int globals[16];
    volatile int idx = input & 15;
    
    /* Complex addressing mode */
    int* ptr = &globals[idx + 1];
    *ptr = input;
    
    /* More complex addressing with pointer arithmetic */
    int* ptr2 = &globals[0] + (idx * 2);
    *ptr2 = input * 2;
    
    /* Structure with pointer arithmetic */
    struct S {
        int a;
        int b[4];
        int c;
    } s;
    
    int* member_ptr = &s.a + idx;
    *member_ptr = input;
    
    /* Multi-dimensional array */
    int matrix[4][4];
    volatile int row = input & 3;
    volatile int col = (input >> 2) & 3;
    matrix[row][col] = input;
    
    /* Pointer to pointer */
    int** pp = &ptr;
    **pp = input + 1;
    
    /* Use results to keep them alive */
    for (int i = 0; i < 4; i++) {
        use_int(globals[i]);
        use_int(s.b[i & 3]);
    }
}

/* Pattern 5: Combined patterns in complex control flow */
__attribute__((noinline, noipa))
void test_combined(volatile int input) {
    int result = 0;
    
    /* Switch with different patterns in each case */
    switch (input & 3) {
        case 0: {
            /* ZERO_EXTRACT-like */
            union {
                unsigned int val;
                struct {
                    unsigned int a:10;
                    unsigned int b:10;
                    unsigned int c:12;
                } fields;
            } u;
            u.val = input;
            u.fields.b = (input >> 5) & 0x3FF;
            result = u.val;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART-like */
            long l = 0xFFFFFFFF;
            int* p = (int*)&l;
            *p = input;
            result = (int)l;
            break;
        }
        case 2: {
            /* SUBREG with array */
            int arr[8];
            short* sp = (short*)arr;
            sp[input & 7] = (short)input;
            result = arr[0];
            break;
        }
        case 3: {
            /* Complex MEM */
            static int buffer[32];
            int index = (input * 13) & 31;
            buffer[index] = input;
            result = buffer[0];
            break;
        }
    }
    
    /* Loop with varying patterns */
    for (int i = 0; i < (volatile_bound & 15); i++) {
        if (i & 1) {
            /* Alternate between patterns */
            short* sp = (short*)&result;
            sp[0] = (short)(result + i);  /* STRICT_LOW_PART */
        } else {
            result ^= (1 << (i & 15));    /* Bit manipulation */
        }
    }
    
    use_int(result);
}

/* Main driver */
int main(int argc, char** argv) {
    /* Initialize volatile seed from command line or time */
    if (argc > 1) {
        volatile_seed = atoi(argv[1]);
    } else {
        volatile_seed = time(NULL);
    }
    
    volatile_bound = (volatile_seed & 0xFF) + 10;
    
    /* Call all test patterns with volatile inputs */
    test_zero_extract(volatile_seed);
    test_strict_low_part((short)volatile_seed);
    test_subreg(volatile_seed ^ 0x12345678);
    test_complex_mem(volatile_seed * 3);
    test_combined(volatile_seed + 1);
    
    /* Create a checksum to prevent optimization */
    int checksum = volatile_seed;
    checksum ^= volatile_bound;
    
    printf("Result: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions to satisfy linker (in real use, these would be external) */
void use_int(int x) { sink(x); }
void use_long(long x) { sink((int)x); }
void use_ptr(void* x) { sink((int)(long)x); }
void sink(int x) { volatile int dummy = x; (void)dummy; }
