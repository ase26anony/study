/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Volatile variables to prevent constant propagation */
static volatile int volatile_seed;
static volatile int volatile_index;
static volatile short volatile_short;
static volatile int volatile_int;
static volatile long volatile_long;

/* Global variables for memory patterns */
int global_array[100];
struct ComplexStruct {
    int a;
    long b;
    short c[4];
    int d;
};

static struct ComplexStruct global_struct;

/* Pattern 1: ZERO_EXTRACT destination */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int seed) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int middle: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = seed;
    
    /* Store into specific bitfield - may generate ZERO_EXTRACT */
    u.bits.middle = (seed >> 4) & 0xFF;
    u.bits.high = (seed >> 8) & 0xFFFF;
    
    /* Complex bitfield manipulation in loop */
    for (int i = 0; i < (seed & 0x3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
    }
    
    use_int(u.full);
    
    /* Another approach: explicit bit masking */
    unsigned int value = seed;
    unsigned int mask = 0x0000FF00;
    value = (value & ~mask) | ((seed * 2) & mask);
    sink(value);
}

/* Pattern 2: STRICT_LOW_PART destination */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile int seed) {
    /* Assigning smaller types to larger ones */
    int big_val = seed * 1000;
    short small_val = (short)(seed + 1);
    
    /* This may generate STRICT_LOW_PART */
    big_val = (big_val & ~0xFFFF) | (small_val & 0xFFFF);
    use_int(big_val);
    
    /* Using pointer casting */
    long long_val = seed * 1000000L;
    int *p_int = (int*)&long_val;
    *p_int = seed;  /* May generate STRICT_LOW_PART for 32-bit on 64-bit */
    
    /* In a loop with volatile control */
    for (volatile int i = 0; i < (seed & 0x7); i++) {
        int temp = long_val;
        short *ps = (short*)&temp;
        ps[1] = (short)(i * 100);  /* Modify high part of int */
        long_val = temp;
    }
    
    use_long(long_val);
}

/* Pattern 3: SUBREG destination */
__attribute__((noinline, noipa))
void test_subreg(volatile int seed) {
    /* Type punning with different sizes */
    long long big_array[4];
    int *int_ptr = (int*)&big_array[seed & 0x3];
    
    /* Store through int pointer into long long array */
    *int_ptr = seed * 3;
    use_ptr(int_ptr);
    
    /* Array access with sub-word types */
    int data[10];
    short *short_ptr = (short*)data;
    volatile_index = seed % 20;
    short_ptr[volatile_index] = (short)seed;
    
    /* Structure with mixed types */
    struct Mixed {
        char a;
        int b;
        short c;
        long d;
    } mix;
    
    mix.b = seed;
    mix.c = (short)(seed >> 1);
    
    /* Access through different type pointers */
    char *char_ptr = (char*)&mix;
    char_ptr[4] = (seed >> 8) & 0xFF;  /* May access part of int b */
    
    /* Complex nested access */
    for (int i = 0; i < (seed & 0x3); i++) {
        int *p = (int*)(char_ptr + i * 2);
        *p = seed + i;
    }
    
    use_int(mix.b);
}

/* Pattern 4: Complex MEM destination with addressing modes */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int seed) {
    /* Complex addressing modes */
    int *ptr = &global_array[0];
    volatile_index = seed % 50;
    
    /* Store with index computation */
    ptr[volatile_index * 2] = seed;
    ptr[volatile_index * 2 + 1] = seed * 2;
    
    /* Structure member access with offset */
    struct ComplexStruct *sptr = &global_struct;
    sptr->a = seed;
    sptr->c[(seed >> 2) & 0x3] = (short)seed;
    
    /* Pointer arithmetic with multiple steps */
    int *p = &sptr->d;
    for (int i = 0; i < (seed & 0x3); i++) {
        *(p + i) = seed + i * 100;
    }
    
    /* Global variable with computed address */
    extern int extern_global;
    int *global_ptr = &extern_global + (seed & 0xF);
    *global_ptr = seed;
    
    /* Stack array with volatile index */
    int local_array[20];
    int idx = volatile_index % 20;
    local_array[idx] = seed * 3;
    local_array[idx + 1] = seed * 4;
    
    use_int(local_array[idx]);
}

/* Pattern 5: Combined patterns in complex control flow */
__attribute__((noinline, noipa))
void test_combined(volatile int seed) {
    int result = 0;
    
    /* Switch with different patterns */
    switch (seed & 0x3) {
        case 0: {
            /* ZERO_EXTRACT pattern */
            union {
                unsigned int val;
                struct {
                    unsigned int a: 10;
                    unsigned int b: 10;
                    unsigned int c: 12;
                } fields;
            } u;
            u.val = seed;
            u.fields.b = (seed >> 5) & 0x3FF;
            result = u.val;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART pattern */
            long x = seed * 1000L;
            int *ip = (int*)&x;
            *ip = seed + 100;
            result = (int)x;
            break;
        }
        case 2: {
            /* SUBREG pattern */
            long long data[2];
            short *sp = (short*)data;
            sp[volatile_index & 0x3] = (short)seed;
            result = (int)data[0];
            break;
        }
        case 3: {
            /* MEM pattern */
            struct {
                int a;
                int b[3];
            } s;
            int *p = &s.a + (seed & 0x1);
            *p = seed * 2;
            result = s.a;
            break;
        }
    }
    
    /* Loop with mixed operations */
    for (volatile int i = 0; i < (seed & 0x7); i++) {
        int temp = result;
        
        /* Alternate between patterns */
        if (i & 0x1) {
            /* Bitfield store */
            temp = (temp & ~0xFF00) | ((seed + i) << 8);
        } else {
            /* Sub-word store */
            char *cp = (char*)&temp;
            cp[i & 0x3] = (seed >> (i * 2)) & 0xFF;
        }
        
        result ^= temp;
    }
    
    sink(result);
}

/* Main driver */
int main(int argc, char **argv) {
    /* Initialize volatile seed from various sources */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    volatile_int = volatile_seed;
    volatile_short = (short)(volatile_seed >> 4);
    volatile_long = volatile_seed * 1000L;
    volatile_index = (volatile_seed >> 8) & 0xFF;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    
    /* Run all pattern tests */
    test_zero_extract(volatile_seed);
    test_strict_low_part(volatile_seed + 1);
    test_subreg(volatile_seed + 2);
    test_complex_mem(volatile_seed + 3);
    test_combined(volatile_seed + 4);
    
    /* Create checksum from global state */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum ^= global_array[i * 10];
    }
    checksum ^= global_struct.a;
    
    printf("Result: %d\n", checksum);
    return checksum & 0x7F;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { volatile int dummy = x; (void)dummy; }
void use_long(long x) { volatile long dummy = x; (void)dummy; }
void use_ptr(void* x) { volatile void* dummy = x; (void)dummy; }
void sink(int x) { volatile int dummy = x; (void)dummy; }
int extern_global = 0;
