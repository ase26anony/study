/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Global variables for memory patterns */
int global_array[256];
volatile int volatile_seed = 0;

/* Pattern 1: ZERO_EXTRACT destination */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int input) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low_bits: 8;
            unsigned int mid_bits: 8;
            unsigned int high_bits: 16;
        } parts;
    } data;
    
    volatile int v = input;
    
    /* Multiple assignments to bitfields */
    data.parts.low_bits = v & 0xFF;
    data.parts.mid_bits = (v >> 8) & 0xFF;
    data.parts.high_bits = (v >> 16) & 0xFFFF;
    
    /* Complex bitfield manipulation in loop */
    for (int i = 0; i < (v & 3); i++) {
        data.parts.low_bits ^= (v >> (i * 4)) & 0xF;
        data.parts.mid_bits |= (v >> (i * 2)) & 0x3;
    }
    
    sink(data.full);
}

/* Pattern 2: STRICT_LOW_PART destination */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile short input) {
    int value = 0x12345678;
    volatile short vs = input;
    
    /* Direct assignment to low part through pointer */
    *(short*)&value = vs;
    
    /* Multiple low-part assignments */
    for (int i = 0; i < (vs & 3); i++) {
        short temp = vs + i;
        *(short*)&value = temp;
    }
    
    /* Using union for low-part access */
    union {
        int full;
        struct {
            short low;
            short high;
        } halves;
    } u;
    
    u.full = 0x87654321;
    u.halves.low = vs;
    
    sink(value + u.full);
}

/* Pattern 3: SUBREG destination */
__attribute__((noinline, noipa))
void test_subreg(volatile int input) {
    long long big_value = 0x1122334455667788LL;
    volatile int vi = input;
    
    /* Access different subregisters of the long long */
    int* p32 = (int*)&big_value;
    p32[0] = vi;
    p32[1] = vi ^ 0x55555555;
    
    /* Array with sub-word access */
    int array[4] = {0};
    short* ps = (short*)array;
    
    for (int i = 0; i < (vi & 7); i++) {
        ps[i] = (vi >> (i * 2)) & 0xFFFF;
    }
    
    /* Structure with mixed sizes */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = vi;
    m.s = vi & 0xFFFF;
    *(short*)((char*)&m.i + 2) = (vi >> 16) & 0xFFFF;
    
    sink(p32[0] + array[0] + m.i);
}

/* Pattern 4: Complex MEM destination with addressing modes */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int index) {
    volatile int vi = index;
    
    /* Complex array indexing */
    int* ptr = &global_array[vi & 255];
    *ptr = vi;
    
    /* Pointer arithmetic with offset */
    int offset = (vi >> 4) & 31;
    int* ptr2 = ptr + offset;
    *ptr2 = vi ^ 0xAAAAAAAA;
    
    /* Structure with computed member access */
    struct point {
        int x;
        int y;
        int z;
    } pt;
    
    int* member_ptr = ((int*)&pt) + (vi & 2);
    *member_ptr = vi;
    
    /* Nested addressing */
    int** pp = &ptr;
    **pp = vi + 1;
    
    /* Loop with memory stores */
    for (int i = 0; i < (vi & 7); i++) {
        global_array[(vi + i) & 255] = vi * i;
    }
    
    sink(*ptr + *ptr2 + pt.x);
}

/* Pattern 5: Combined patterns in complex control flow */
__attribute__((noinline, noipa))
void test_combined(volatile int input) {
    volatile int v = input;
    int result = 0;
    
    /* Switch with different patterns */
    switch (v & 3) {
        case 0: {
            /* ZERO_EXTRACT pattern */
            union {
                unsigned int val;
                struct {
                    unsigned int a: 10;
                    unsigned int b: 10;
                    unsigned int c: 12;
                } bits;
            } u;
            u.val = v;
            u.bits.b = (v >> 5) & 0x3FF;
            result = u.val;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART pattern */
            long data = 0xFEDCBA9876543210L;
            *(int*)&data = v;
            result = (int)data;
            break;
        }
        case 2: {
            /* SUBREG pattern */
            double d = 3.14159;
            int* ip = (int*)&d;
            ip[1] = v;
            result = ip[0] + ip[1];
            break;
        }
        case 3: {
            /* Complex MEM pattern */
            struct {
                int header;
                int data[8];
                int footer;
            } block;
            
            int idx = v & 7;
            block.data[idx] = v;
            result = block.data[idx];
            break;
        }
    }
    
    sink(result);
}

/* Main driver with volatile control flow */
int main(int argc, char** argv) {
    /* Use volatile seed from command line or time */
    volatile int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    volatile_seed = seed;
    
    /* Call pattern generators with volatile inputs */
    test_zero_extract(seed);
    test_strict_low_part(seed & 0xFFFF);
    test_subreg(seed ^ 0x12345678);
    test_complex_mem(seed);
    test_combined(seed);
    
    /* Create checksum from global array */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= global_array[i];
    }
    
    printf("Result: %d\n", checksum);
    return 0;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { volatile_seed = x; }
void use_long(long x) { volatile_seed = x; }
void use_ptr(void* x) { volatile_seed = (int)(long)x; }
void sink(int x) { volatile_seed = x; }
