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
static volatile short volatile_short;
static volatile int volatile_int;
static volatile long volatile_long;
static volatile int volatile_index;

/* Global variables for memory patterns */
int global_array[100];
struct Compound {
    int a;
    long b;
    short c;
    char d;
} global_struct;

/* Pattern 1: ZERO_EXTRACT destination */
__attribute__((noinline, noipa))
void test_zero_extract(int seed) {
    union BitfieldUnion {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    volatile int input = seed;
    
    /* Multiple assignments to bitfields - may generate ZERO_EXTRACT */
    u.bits.low = input & 0xFF;
    u.bits.mid = (input >> 8) & 0xFF;
    u.bits.high = (input >> 16) & 0xFFFF;
    
    /* Complex bitfield manipulation in loop */
    for (int i = 0; i < (seed & 3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
        u.bits.mid ^= (i << 4) & 0xFF;
    }
    
    sink(u.full);
}

/* Pattern 2: STRICT_LOW_PART destination */
__attribute__((noinline, noipa))
void test_strict_low_part(int seed) {
    volatile short vs = seed;
    int large_val = 0x12345678;
    
    /* Assign short to part of int - may generate STRICT_LOW_PART */
    if (seed & 1) {
        /* Using type punning */
        *(short*)&large_val = vs;
    } else {
        /* Using bitwise operations */
        large_val = (large_val & ~0xFFFF) | (vs & 0xFFFF);
    }
    
    /* In a loop with volatile control */
    for (int i = 0; i < (volatile_seed & 3); i++) {
        short temp = vs + i;
        *(short*)&large_val = temp;
    }
    
    sink(large_val);
}

/* Pattern 3: SUBREG destination */
__attribute__((noinline, noipa))
void test_subreg(int seed) {
    long long big_value = 0x1122334455667788LL;
    volatile int vi = seed;
    
    /* Access parts of larger type - may generate SUBREG */
    if (seed & 1) {
        int* p = (int*)&big_value;
        p[0] = vi;
        p[1] = vi ^ 0xFFFF;
    } else {
        short* ps = (short*)&big_value;
        for (int i = 0; i < 4; i++) {
            ps[i] = (vi + i) & 0xFFFF;
        }
    }
    
    /* Array with sub-word access */
    int array[4] = {0};
    short* array_ptr = (short*)array;
    for (int i = 0; i < (seed & 7); i++) {
        array_ptr[i] = (vi + i * 3) & 0xFFFF;
    }
    
    sink((int)big_value + array[0]);
}

/* Pattern 4: Complex MEM destination with addressing modes */
__attribute__((noinline, noipa))
void test_complex_mem(int seed) {
    volatile int idx = seed;
    
    /* Complex addressing modes */
    if (seed & 1) {
        /* Array with volatile index */
        global_array[idx % 100] = volatile_int;
        global_array[(idx + 1) % 100] = volatile_int ^ 0xAA;
    } else {
        /* Structure member with offset */
        struct Compound local_struct;
        int* ptr = &local_struct.a + (idx & 3);
        *ptr = volatile_int;
        
        /* Pointer arithmetic */
        char* cptr = (char*)&local_struct;
        cptr[idx & 15] = (char)volatile_int;
    }
    
    /* Nested addressing */
    struct Compound* struct_ptr = &global_struct;
    for (int i = 0; i < (volatile_seed & 2); i++) {
        struct_ptr->a = volatile_int + i;
        struct_ptr->c = volatile_short;
        
        /* Compute address with multiple operations */
        int* computed = (int*)((char*)struct_ptr + (idx * 4) % 16);
        *computed = volatile_int * 2;
    }
    
    sink(global_array[0] + global_struct.a);
}

/* Pattern 5: Combined patterns in complex control flow */
__attribute__((noinline, noipa))
void test_combined(int seed) {
    volatile int control = seed;
    int result = 0;
    
    /* Mixed patterns in switch */
    switch (control & 3) {
        case 0: {
            /* ZERO_EXTRACT pattern */
            union {
                unsigned int val;
                struct {
                    unsigned int a: 5;
                    unsigned int b: 11;
                    unsigned int c: 16;
                } fields;
            } u;
            u.fields.b = control & 0x7FF;
            result = u.val;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART pattern */
            long x = 0xFFFFFFFF;
            *(short*)&x = control & 0xFFFF;
            result = (int)x;
            break;
        }
        case 2: {
            /* SUBREG pattern */
            long long ll = 0;
            int* p = (int*)&ll;
            p[control & 1] = control;
            result = (int)ll;
            break;
        }
        case 3: {
            /* MEM pattern */
            int arr[4] = {0};
            arr[control & 3] = control;
            result = arr[0];
            break;
        }
    }
    
    /* Loop with varying patterns */
    for (int i = 0; i < (control & 7); i++) {
        if (i & 1) {
            /* Alternate between patterns */
            int temp = result;
            *(short*)&temp = (control + i) & 0xFFFF;
            result ^= temp;
        } else {
            /* Bitfield in loop */
            union {
                int full;
                struct { int low: 12; int high: 20; } bits;
            } u;
            u.full = result;
            u.bits.low = (u.bits.low + i) & 0xFFF;
            result = u.full;
        }
    }
    
    sink(result);
}

/* Main driver */
int main(int argc, char** argv) {
    /* Initialize volatile seed from various sources */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    volatile_short = volatile_seed & 0xFFFF;
    volatile_int = volatile_seed ^ 0x5555;
    volatile_long = volatile_seed * 3;
    volatile_index = (volatile_seed >> 3) & 0xF;
    
    int checksum = 0;
    
    /* Call each test function multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        int seed = volatile_seed + i * 100;
        
        test_zero_extract(seed);
        checksum += seed & 0xFF;
        
        test_strict_low_part(seed + 1);
        checksum += (seed + 1) & 0xFF;
        
        test_subreg(seed + 2);
        checksum += (seed + 2) & 0xFF;
        
        test_complex_mem(seed + 3);
        checksum += (seed + 3) & 0xFF;
        
        test_combined(seed + 4);
        checksum += (seed + 4) & 0xFF;
    }
    
    /* Use checksum to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions to satisfy linker (in real use, these would be external) */
void use_int(int x) { volatile int dummy = x; }
void use_long(long x) { volatile long dummy = x; }
void use_ptr(void* x) { volatile void* dummy = x; }
void sink(int x) { volatile int dummy = x; }
