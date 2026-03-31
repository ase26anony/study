/* test_resources.c - Generate RTL patterns for resource.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Volatile variables to prevent compile-time optimization */
static volatile int volatile_seed;
static volatile int volatile_index;
static volatile short volatile_short;
static volatile int volatile_int;
static volatile long volatile_long;

/* Global variables for memory patterns */
int global_array[16];
struct ComplexStruct {
    int a;
    int b;
    long c;
    short d;
} global_struct;

/* ===== Pattern 1: ZERO_EXTRACT destination ===== */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int seed) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union BitFieldUnion {
        unsigned int full;
        struct {
            unsigned int low_bits : 8;
            unsigned int mid_bits : 8;
            unsigned int high_bits : 16;
        } parts;
    } u;
    
    /* Initialize with volatile to prevent constant propagation */
    u.full = seed & 0xFFFFFFFF;
    
    /* Assignment to bitfield - may generate ZERO_EXTRACT destination */
    u.parts.mid_bits = (seed >> 8) & 0xFF;
    
    /* More complex bitfield manipulation */
    if (seed & 1) {
        u.parts.low_bits = (seed >> 16) & 0xFF;
    } else {
        u.parts.high_bits = (seed >> 24) & 0xFFFF;
    }
    
    /* Use result to keep computation live */
    use_int(u.full);
    
    /* Alternative: Manual bitfield using bitwise operations */
    int manual = seed;
    /* Store into specific bits - may generate ZERO_EXTRACT */
    manual = (manual & ~0xFF00) | ((seed * 2) & 0xFF00);
    
    /* In loop to increase chances */
    for (int i = 0; i < (seed & 3); i++) {
        manual = (manual & ~(0xF << (i * 4))) | 
                ((seed >> (i * 4)) & (0xF << (i * 4)));
    }
    
    sink(manual);
}

/* ===== Pattern 2: STRICT_LOW_PART destination ===== */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile int seed) {
    /* Assigning to smaller type within larger container */
    int large = seed;
    
    /* Cast to short pointer - may generate STRICT_LOW_PART */
    short *short_ptr = (short*)&large;
    *short_ptr = volatile_short;
    
    /* Alternative with explicit masking */
    int another = seed * 3;
    /* Only modify low 16 bits */
    another = (another & ~0xFFFF) | (volatile_short & 0xFFFF);
    
    /* Use union for type punning */
    union {
        int full;
        struct {
            short low;
            short high;
        } halves;
    } pun;
    
    pun.full = seed;
    pun.halves.low = volatile_short;  /* STRICT_LOW_PART candidate */
    
    /* In conditional to create varied control flow */
    for (int i = 0; i < (seed & 7); i++) {
        if (i & 1) {
            *(short*)((char*)&another + 2) = volatile_short;
        } else {
            another = (another & ~0xFFFF) | (volatile_short & 0xFFFF);
        }
    }
    
    use_int(large + another + pun.full);
}

/* ===== Pattern 3: SUBREG destination ===== */
__attribute__((noinline, noipa))
void test_subreg(volatile int seed) {
    /* Type punning between different sizes */
    long long big_value = (long long)seed * 1000;
    
    /* Access part of long long as int - may generate SUBREG */
    int *int_part = (int*)&big_value;
    *int_part = volatile_int;
    
    /* Array with sub-word access */
    int array[4] = {seed, seed + 1, seed + 2, seed + 3};
    
    /* Access as short through pointer arithmetic */
    short *short_view = (short*)array;
    short_view[volatile_index & 3] = volatile_short;
    
    /* Structure with mixed types */
    struct Mixed {
        char a;
        short b;
        int c;
        long d;
    } mixed;
    
    mixed.d = seed;
    /* Access part of long as int */
    *(int*)((char*)&mixed.d + sizeof(int)) = volatile_int;
    
    /* Complex pointer arithmetic */
    char *byte_ptr = (char*)array;
    int offset = volatile_index & 7;
    *(short*)(byte_ptr + offset) = volatile_short;
    
    /* Use results */
    use_long(big_value);
    use_int(array[0] + array[1]);
    sink(mixed.c);
}

/* ===== Pattern 4: Complex MEM destination ===== */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int seed) {
    /* Complex addressing modes */
    int *ptr;
    
    /* Pointer arithmetic with volatile index */
    ptr = &global_array[volatile_index & 15];
    *ptr = volatile_int;
    
    /* Structure member access with offset */
    struct ComplexStruct local_struct;
    int *member_ptr;
    
    if (seed & 1) {
        member_ptr = &local_struct.a;
    } else if (seed & 2) {
        member_ptr = &local_struct.b;
    } else {
        member_ptr = (int*)&local_struct.c;
    }
    
    *member_ptr = volatile_int;
    
    /* More complex addressing: base + index + displacement */
    for (int i = 0; i < (seed & 3); i++) {
        int *complex_ptr = &global_array[i] + (volatile_index & 3);
        *complex_ptr = volatile_int + i;
    }
    
    /* Global structure with member access */
    short *d_ptr = &global_struct.d;
    *d_ptr = volatile_short;
    
    /* Pointer to pointer with dereference */
    int **pptr = (int**)&ptr;
    **pptr = volatile_int;
    
    /* Use results */
    use_int(global_array[0]);
    use_ptr(&local_struct);
}

/* ===== Combined test with all patterns ===== */
__attribute__((noinline, noipa))
int run_all_tests(volatile int seed) {
    int result = 0;
    
    /* Initialize volatile variables */
    volatile_index = (seed >> 8) & 15;
    volatile_short = (short)(seed >> 16);
    volatile_int = seed * 3;
    volatile_long = (long)seed * 100;
    
    /* Initialize globals */
    memset(global_array, 0, sizeof(global_array));
    global_struct.a = seed;
    global_struct.b = seed + 1;
    global_struct.c = seed * 2;
    global_struct.d = (short)seed;
    
    /* Run each test pattern */
    test_zero_extract(seed);
    result += volatile_int;
    
    test_strict_low_part(seed + 1);
    result += volatile_short;
    
    test_subreg(seed + 2);
    result += global_array[0];
    
    test_complex_mem(seed + 3);
    result += global_struct.a;
    
    /* Additional mixed pattern */
    if (seed & 4) {
        /* Combined pattern in one function */
        union {
            long long ll;
            int parts[2];
        } u;
        u.ll = volatile_long;
        u.parts[volatile_index & 1] = volatile_int;  /* SUBREG + MEM */
        
        /* Bitfield in structure */
        struct {
            unsigned int field : 12;
            unsigned int : 4;
            unsigned int next : 16;
        } bf;
        bf.field = volatile_int & 0xFFF;  /* ZERO_EXTRACT */
        bf.next = volatile_short;         /* STRICT_LOW_PART */
        
        result += u.parts[0] + bf.field;
    }
    
    return result;
}

/* External function declarations (never defined) */
void use_int(int x) { /* Empty in real use, prevents optimization */ }
void use_long(long x) { /* Empty */ }
void use_ptr(void* x) { /* Empty */ }
void sink(int x) { /* Empty */ }

int main(int argc, char **argv) {
    /* Use command line argument or time for seed */
    volatile int seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    volatile_seed = seed;
    
    /* Run tests multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        total += run_all_tests(seed + i * 1000);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
