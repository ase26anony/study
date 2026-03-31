/* test_resource_marking.c - Generate RTL patterns for resource.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Global variables for memory patterns */
int global_array[256];
struct compound {
    int a;
    long b;
    short c[4];
} global_struct;

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* 1. Generate SET with ZERO_EXTRACT destination */
NOINLINE void test_zero_extract(volatile int seed) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low : 8;
            unsigned int mid : 8;
            unsigned int high : 16;
        } bits;
    } u;
    
    volatile unsigned int input = seed;
    
    /* Multiple assignments to different bitfields */
    u.bits.low = input & 0xFF;
    u.bits.mid = (input >> 8) & 0xFF;
    u.bits.high = (input >> 16) & 0xFFFF;
    
    /* Complex bitfield manipulation in loop */
    for (int i = 0; i < (seed & 3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
        u.bits.mid ^= (input >> (i * 4)) & 0xFF;
    }
    
    sink(u.full);
}

/* 2. Generate SET with STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(volatile int seed) {
    volatile short s_val = seed;
    volatile char c_val = seed ^ 0x55;
    
    /* Assigning to parts of larger integers */
    int i1 = 0;
    long l1 = 0;
    
    /* These should generate STRICT_LOW_PART */
    *(short*)&i1 = s_val;
    *(char*)&l1 = c_val;
    
    /* More complex pattern with arithmetic */
    for (int i = 0; i < (seed & 7); i++) {
        int temp = i1;
        *(short*)&temp = s_val + i;
        i1 = temp;
    }
    
    /* Using union for type punning */
    union {
        long full;
        struct {
            short low;
            short mid_low;
            short mid_high;
            short high;
        } parts;
    } lu;
    
    lu.parts.low = s_val;
    lu.parts.mid_low = s_val + 1;
    
    sink(i1 + (int)lu.full);
}

/* 3. Generate SET with SUBREG destination */
NOINLINE void test_subreg(volatile int seed) {
    volatile int idx = seed & 255;
    
    /* Array with sub-word access */
    int array[8];
    short *ps = (short*)array;
    
    /* SUBREG through pointer casting */
    ps[idx % 16] = seed;
    
    /* Different sized accesses to same memory */
    char *pc = (char*)array;
    pc[(idx * 3) % 32] = seed ^ 0xAA;
    
    /* Structure with mixed types */
    struct mixed {
        long a;
        int b;
        short c;
        char d;
    } m;
    
    /* Accessing sub-parts */
    *(int*)((char*)&m + 2) = seed;  /* Misaligned access */
    *(short*)((char*)&m + sizeof(long) + sizeof(int)) = seed >> 8;
    
    /* Global structure access */
    short *gs = (short*)&global_struct;
    gs[idx % 8] = seed;
    
    sink(array[0] + m.b + global_struct.c[0]);
}

/* 4. Generate SET with complex MEM destination */
NOINLINE void test_complex_mem(volatile int seed) {
    volatile int offset = seed & 255;
    volatile int scale = (seed >> 8) & 3;
    
    /* Complex addressing modes */
    int *ptr1 = &global_array[offset];
    *ptr1 = seed;
    
    /* Pointer arithmetic */
    int *ptr2 = ptr1 + (scale * 2);
    *ptr2 = seed * 2;
    
    /* Structure pointer with offset */
    struct compound *sp = &global_struct;
    int *member_ptr = &sp->a + offset % 4;
    *member_ptr = seed ^ 0x1234;
    
    /* Two-dimensional indexing */
    int matrix[16][16];
    int (*row_ptr)[16] = &matrix[offset % 16];
    (*row_ptr)[(offset * 7) % 16] = seed;
    
    /* Loop with memory stores */
    for (int i = 0; i < (seed & 3); i++) {
        int *loop_ptr = &global_array[(offset + i * 17) & 255];
        *loop_ptr = seed + i;
    }
    
    sink(*ptr1 + *ptr2 + *member_ptr + matrix[0][0]);
}

/* 5. Combined test with all patterns */
NOINLINE int test_combined(volatile int seed) {
    int result = 0;
    
    /* Mix different patterns */
    union bitfield_union {
        unsigned int full;
        struct {
            unsigned int a : 5;
            unsigned int b : 11;
            unsigned int c : 16;
        } fields;
    } bu;
    
    bu.fields.a = seed & 0x1F;
    bu.fields.b = (seed >> 5) & 0x7FF;
    result += bu.full;
    
    /* SUBREG access */
    long long big = 0;
    int *big_part = (int*)&big;
    big_part[seed & 1] = seed;
    result += (int)big;
    
    /* Complex MEM with global */
    int *dyn_ptr = &global_array[(seed * 13) & 255];
    *dyn_ptr = seed;
    result += *dyn_ptr;
    
    /* STRICT_LOW_PART style */
    int word = 0xFFFF0000;
    *(short*)&word = seed;
    result += word;
    
    return result;
}

/* Main driver with volatile control flow */
int main(int argc, char **argv) {
    volatile int seed;
    
    /* Get volatile seed from various sources */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    /* Initialize globals */
    memset(global_array, 0, sizeof(global_array));
    memset(&global_struct, 0, sizeof(global_struct));
    
    int total = 0;
    
    /* Call test functions with volatile conditions */
    if (seed & 1) {
        test_zero_extract(seed);
        total += 1;
    }
    
    for (int i = 0; i < (seed & 3); i++) {
        test_strict_low_part(seed + i);
        total += 2;
    }
    
    if ((seed >> 4) & 1) {
        test_subreg(seed ^ 0x5A);
        total += 3;
    }
    
    volatile int iter = (seed >> 2) & 7;
    for (int i = 0; i < iter; i++) {
        test_complex_mem(seed + i * 11);
        total += 4;
    }
    
    /* Final combined test */
    int combined = test_combined(seed);
    total += combined;
    
    printf("Result: %d (seed: %d)\n", total, seed);
    
    return total & 255;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_long(long x) { (void)x; }
void use_ptr(void *x) { (void)x; }
void sink(int x) { (void)x; }
