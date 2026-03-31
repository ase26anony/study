/* test_resource_cc.c - Generate RTL patterns for uncovered lines in resource.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void *);
extern void sink(int);

/* Volatile seed to prevent compile-time optimization */
static volatile int seed;

/* Global variables for memory addressing patterns */
int global_array[100];
struct compound {
    int a;
    long b;
    short c[4];
} global_struct;

/* Pattern 1: ZERO_EXTRACT destination via bitfield operations */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int input) {
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    /* Force initialization */
    u.full = seed;
    
    /* Store into bitfield - may generate ZERO_EXTRACT destination */
    u.bits.mid = input & 0xFF;
    
    /* Use result to keep computation live */
    sink(u.full);
    
    /* Another pattern: explicit bitfield in struct */
    struct {
        unsigned int field1 : 10;
        unsigned int field2 : 6;
        unsigned int field3 : 16;
    } s;
    
    s.field1 = seed;
    s.field2 = input & 0x3F;  /* Could generate ZERO_EXTRACT */
    s.field3 = (input >> 6) & 0xFFFF;
    
    sink(*(int*)&s);
}

/* Pattern 2: STRICT_LOW_PART destination via partial word assignment */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile short input) {
    int val = seed * 3;
    
    /* Assign short to low part of int - may generate STRICT_LOW_PART */
    *(short*)&val = input;
    
    sink(val);
    
    /* Alternative: using type punning */
    long long big_val = seed * 5LL;
    int *p = (int*)&big_val;
    *p = input;  /* Assign to low part of long long */
    
    sink(big_val);
    
    /* In loop with condition */
    for (int i = 0; i < (input & 3); i++) {
        int temp = seed + i;
        *(char*)&temp = (input >> i) & 0xFF;  /* Partial assignment */
        sink(temp);
    }
}

/* Pattern 3: SUBREG destination via type punning and sub-word access */
__attribute__((noinline, noipa))
void test_subreg(volatile int index) {
    /* Array with sub-word access */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = seed + i;
    }
    
    /* Access via short pointer - may generate SUBREG */
    short *ps = (short*)&arr[index % 8];
    *ps = (short)(seed * 2);
    
    sink(arr[0]);
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long l;
    } m;
    
    m.l = seed;
    m.i = index;
    m.s = (short)(seed >> 4);  /* Could generate SUBREG */
    m.c = (char)index;
    
    sink(*(int*)&m);
    
    /* Pointer arithmetic with different types */
    char *buffer = (char*)arr;
    int *alias = (int*)(buffer + 2);  /* Misaligned access */
    *alias = seed;  /* May generate SUBREG due to alignment */
    
    sink(*alias);
}

/* Pattern 4: Complex MEM destination with non-trivial addressing */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int offset) {
    /* Global variable with offset */
    int *ptr = &global_array[offset % 50];
    *ptr = seed;
    
    sink(*ptr);
    
    /* Structure member with computed offset */
    struct compound local_struct;
    local_struct.a = seed;
    local_struct.b = seed * 2L;
    
    int *member_ptr = &local_struct.a + (offset & 1);
    *member_ptr = offset;
    
    sink(local_struct.a);
    
    /* Multi-dimensional array with volatile index */
    int matrix[5][5];
    volatile int idx = offset % 5;
    
    for (int i = 0; i < 5; i++) {
        matrix[idx][i] = seed + i;  /* Complex addressing */
        matrix[i][idx] = offset + i;
    }
    
    sink(matrix[0][0]);
    
    /* Pointer chain */
    int **pptr = &ptr;
    **pptr = seed * 3;
    
    sink(**pptr);
}

/* Pattern 5: Combined patterns in complex control flow */
__attribute__((noinline, noipa))
void test_combined(volatile int control) {
    int result = 0;
    
    /* Switch with different patterns */
    switch (control & 3) {
        case 0: {
            /* ZERO_EXTRACT pattern */
            union {
                unsigned int val;
                struct {
                    unsigned int low: 12;
                    unsigned int high: 20;
                } parts;
            } u;
            u.val = seed;
            u.parts.low = control & 0xFFF;
            result = u.val;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART pattern */
            long long big = seed * 7LL;
            *(int*)&big = control;
            result = (int)big;
            break;
        }
        case 2: {
            /* SUBREG pattern */
            int arr[4] = {seed, seed+1, seed+2, seed+3};
            short *sp = (short*)arr;
            sp[control % 4] = (short)control;
            result = arr[0];
            break;
        }
        case 3: {
            /* Complex MEM pattern */
            struct {
                int a;
                int b;
                int c;
            } s = {seed, seed+1, seed+2};
            int *p = (control & 1) ? &s.a : &s.b;
            *p = control;
            result = s.a + s.b;
            break;
        }
    }
    
    /* Loop with mixed operations */
    for (int i = 0; i < (control & 7); i++) {
        int temp = result + i;
        
        /* Alternate between patterns */
        if (i & 1) {
            *(short*)&temp = (short)(control + i);  /* STRICT_LOW_PART */
        } else {
            /* Bitfield operation - ZERO_EXTRACT */
            union {
                int full;
                struct {
                    int low: 4;
                    int mid: 8;
                    int high: 20;
                } bits;
            } u;
            u.full = temp;
            u.bits.mid = (control >> i) & 0xFF;
            temp = u.full;
        }
        
        result ^= temp;  /* Combine results */
    }
    
    sink(result);
}

/* Main driver with volatile control flow */
int main(int argc, char *argv[]) {
    /* Initialize volatile seed */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    volatile int iter = (seed & 15) + 5;
    volatile short short_input = (short)(seed * 3);
    volatile int index = seed % 100;
    volatile int offset = (seed >> 4) % 50;
    volatile int control = seed;
    
    int checksum = 0;
    
    /* Call test functions multiple times with volatile inputs */
    for (int i = 0; i < iter; i++) {
        test_zero_extract(seed + i);
        test_strict_low_part(short_input + i);
        test_subreg(index + i);
        test_complex_mem(offset + i);
        test_combined(control + i);
        
        checksum += seed * i;
    }
    
    /* Print result to prevent complete optimization */
    printf("Result: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions to satisfy external references */
void use_int(int x) { (void)x; }
void use_long(long x) { (void)x; }
void use_ptr(void *x) { (void)x; }
void sink(int x) { (void)x; }
