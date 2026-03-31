/* test_resource_coverage.c
 * Designed to generate RTL patterns that exercise uncovered lines in GCC's resource.cc
 * Specifically targeting SET destinations with: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* External functions to prevent dead code elimination */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Volatile variables to prevent compile-time optimization */
static volatile int volatile_seed;
static volatile int volatile_index;
static volatile short volatile_short;
static volatile int volatile_int;
static volatile long long volatile_ll;

/* Global variables for memory patterns */
int global_array[100];
struct ComplexStruct {
    int a;
    int b;
    long long c;
    short d;
} global_struct;

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Pattern 1: Generate SET destination with ZERO_EXTRACT */
NOINLINE void test_zero_extract(int seed) {
    /* Using union with bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    /* Volatile operations to prevent optimization */
    u.full = seed;
    volatile_int = u.full;
    
    /* Assignment to bitfield - may generate ZERO_EXTRACT */
    u.bits.mid = (unsigned char)(seed * 3);
    u.bits.high = (unsigned short)(seed * 5);
    
    /* Complex bitfield manipulation in loop */
    for (int i = 0; i < (seed & 3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
        u.bits.mid ^= (i << 4);
    }
    
    use_int(u.full);
    
    /* Another approach: explicit bitfield extraction */
    unsigned int val = seed;
    unsigned int mask = 0x0000FF00;
    unsigned int field = (seed * 7) & 0xFF;
    
    /* This pattern: store into extracted field */
    val = (val & ~mask) | ((field << 8) & mask);
    volatile_int = val;
    
    sink(u.full + val);
}

/* Pattern 2: Generate SET destination with STRICT_LOW_PART */
NOINLINE void test_strict_low_part(int seed) {
    /* Assigning smaller type to larger type */
    int big_val = seed * 11;
    short small_val = (short)(seed * 13);
    
    /* This may generate STRICT_LOW_PART */
    big_val = (big_val & ~0xFFFF) | (small_val & 0xFFFF);
    
    volatile_int = big_val;
    
    /* Using pointer casting */
    long long big_ll = seed * 17LL;
    int *p_int = (int*)&big_ll;
    
    /* Store into low part of long long */
    *p_int = seed * 19;
    
    volatile_ll = big_ll;
    
    /* Array with type punning */
    int arr[4] = {seed, seed*2, seed*3, seed*4};
    short *ps = (short*)arr;
    
    for (int i = 0; i < (seed & 7); i++) {
        /* Store into low part of int through short pointer */
        ps[i] = (short)(seed + i);
    }
    
    use_int(arr[0] + arr[1]);
    sink(big_val + *p_int);
}

/* Pattern 3: Generate SET destination with SUBREG */
NOINLINE void test_subreg(int seed) {
    /* Type punning with different sizes */
    long long big_var = seed * 23LL;
    
    /* Access parts of long long */
    int *p_low = (int*)&big_var;
    int *p_high = p_low + 1;
    
    *p_low = seed * 29;
    *p_high = seed * 31;
    
    volatile_ll = big_var;
    
    /* Array with sub-word access */
    int data[10];
    volatile_index = seed % 8;
    
    /* Access via different type pointers */
    short *sptr = (short*)&data[volatile_index];
    char *cptr = (char*)&data[volatile_index + 1];
    
    for (int i = 0; i < (seed & 3); i++) {
        sptr[i] = (short)(seed + i * 7);
        cptr[i] = (char)(seed + i * 11);
    }
    
    /* Structure with mixed types */
    struct Mixed {
        char a;
        short b;
        int c;
        char d;
    } m;
    
    m.a = (char)seed;
    m.b = (short)(seed * 37);
    m.c = seed * 41;
    m.d = (char)(seed * 43);
    
    /* Pointer arithmetic to force SUBREG */
    int *ptr = &m.c;
    short *sptr2 = (short*)ptr;
    *sptr2 = (short)(seed * 47);
    
    use_int(data[0] + m.c);
    sink(*p_low + *p_high);
}

/* Pattern 4: Generate SET destination with complex MEM */
NOINLINE void test_complex_mem(int seed) {
    /* Complex addressing modes */
    int *ptr = &global_array[seed % 50];
    
    /* Store with index computation */
    for (int i = 0; i < (seed & 15); i++) {
        ptr[i] = seed * (i + 1);
        ptr[i + 10] = seed * (i + 2);
    }
    
    /* Structure member access with offset */
    struct ComplexStruct local_struct;
    int *member_ptr = &local_struct.a + (seed & 3);
    
    for (int i = 0; i < (seed & 7); i++) {
        member_ptr[i] = seed * (50 + i);
    }
    
    /* Pointer arithmetic with volatile */
    volatile int offset = seed % 20;
    int *dynamic_ptr = &global_struct.a + offset;
    
    *dynamic_ptr = seed * 53;
    *(dynamic_ptr + 1) = seed * 59;
    
    /* Multi-dimensional array access */
    int matrix[10][10];
    volatile int row = seed % 10;
    volatile int col = seed % 10;
    
    for (int i = 0; i < (seed & 3); i++) {
        matrix[row + i][col] = seed * (61 + i);
        matrix[row][col + i] = seed * (67 + i);
    }
    
    /* Global variable with computed address */
    extern int extern_global;
    int *ext_ptr = &extern_global + (seed & 1);
    *ext_ptr = seed * 71;
    
    use_int(ptr[0] + local_struct.a);
    sink(matrix[0][0] + *dynamic_ptr);
}

/* Pattern 5: Combined patterns in complex control flow */
NOINLINE void test_combined_patterns(int seed) {
    int result = 0;
    
    /* Switch with different patterns */
    switch (seed & 3) {
        case 0: {
            /* ZERO_EXTRACT pattern */
            union {
                unsigned int val;
                struct {
                    unsigned int a: 4;
                    unsigned int b: 12;
                    unsigned int c: 16;
                } fields;
            } u;
            u.val = seed;
            u.fields.b = (seed * 73) & 0xFFF;
            result += u.val;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART pattern */
            long x = seed * 79;
            *(int*)&x = seed * 83;
            result += (int)x;
            break;
        }
        case 2: {
            /* SUBREG pattern */
            int arr[2] = {seed, seed*2};
            short *sp = (short*)arr;
            sp[1] = (short)(seed * 89);
            result += arr[0];
            break;
        }
        case 3: {
            /* Complex MEM pattern */
            int *p = &global_array[seed % 20];
            p[(seed >> 4) & 3] = seed * 97;
            result += *p;
            break;
        }
    }
    
    /* Loop with mixed operations */
    for (int i = 0; i < (seed & 7); i++) {
        volatile int idx = i;
        
        /* Alternate between patterns */
        if (i & 1) {
            /* Bitfield store */
            unsigned int v = result;
            unsigned int field = (seed + i) & 0xF;
            v = (v & ~0xF0) | (field << 4);
            result = v;
        } else {
            /* Memory store with complex address */
            global_array[idx] = result + i;
        }
    }
    
    sink(result);
}

/* Main driver */
int main(int argc, char **argv) {
    /* Initialize volatile seed from command line or timer */
    if (argc > 1) {
        volatile_seed = atoi(argv[1]);
    } else {
        volatile_seed = time(NULL);
    }
    
    /* Additional volatile inputs */
    volatile_index = (volatile_seed >> 8) & 0xFF;
    volatile_short = (short)(volatile_seed * 101);
    volatile_int = volatile_seed * 103;
    volatile_ll = volatile_seed * 107LL;
    
    int seed = volatile_seed;
    
    /* Call all test functions */
    test_zero_extract(seed);
    test_strict_low_part(seed + 1);
    test_subreg(seed + 2);
    test_complex_mem(seed + 3);
    test_combined_patterns(seed + 4);
    
    /* Compute and print a checksum to ensure all code runs */
    int checksum = global_array[0] + global_struct.a + volatile_int;
    printf("Result checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy definitions to satisfy external references */
void use_int(int x) { volatile_int = x; }
void use_long(long x) { volatile_ll = x; }
void use_ptr(void *p) { volatile_int = (int)(long)p; }
void sink(int x) { volatile_int = x; }
int extern_global = 0;
