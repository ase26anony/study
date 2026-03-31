/* test_resource_marking.c */
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
int global_array[100];
struct Complex {
    int a;
    int b;
    long long c;
    int d;
} global_struct;

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Pattern 1: ZERO_EXTRACT destination */
NOINLINE void test_zero_extract(volatile int seed) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    volatile int input = seed;
    
    /* Multiple assignments to bitfields */
    u.bits.low = input & 0xFF;
    u.bits.mid = (input >> 8) & 0xFF;
    u.bits.high = (input >> 16) & 0xFFFF;
    
    /* Force computation with bitfields */
    int result = u.bits.low + u.bits.mid * 2 + u.bits.high;
    sink(result);
    
    /* Another pattern: explicit bit masking */
    unsigned int val = 0;
    unsigned int mask = 0xFF00FF00;
    unsigned int bits = input;
    
    /* This may generate ZERO_EXTRACT for the masked store */
    val = (val & ~mask) | (bits & mask);
    sink((int)val);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(volatile int seed) {
    volatile short s_input = (short)seed;
    volatile char c_input = (char)seed;
    
    /* Assigning smaller types to larger variables */
    int i = 0x12345678;
    i = (i & ~0xFFFF) | (s_input & 0xFFFF);
    sink(i);
    
    long long ll = 0x123456789ABCDEF0LL;
    ll = (ll & ~0xFF) | (c_input & 0xFF);
    sink((int)ll);
    
    /* Pointer casting approach */
    int val = 0x87654321;
    *(short*)&val = s_input;
    sink(val);
    
    /* In a loop to create more opportunities */
    for (int j = 0; j < (seed & 3); j++) {
        int temp = j * 1000;
        *(char*)&temp = c_input + j;
        sink(temp);
    }
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(volatile int seed) {
    volatile int idx = seed % 4;
    
    /* Array with sub-word access */
    int array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = i * 100 + seed;
    }
    
    /* Access through different type pointers */
    short* ps = (short*)&array[idx];
    *ps = (short)seed;
    sink(array[idx]);
    
    /* Type punning with unions */
    union {
        long long big;
        int parts[2];
        short words[4];
    } pun;
    
    pun.big = 0x1122334455667788LL;
    pun.parts[1] = seed;
    pun.words[0] = (short)seed;
    
    sink(pun.parts[0]);
    sink(pun.parts[1]);
    
    /* Complex structure access */
    struct Mixed {
        char a;
        int b;
        short c;
        long long d;
    } m;
    
    m.b = seed;
    m.c = (short)seed;
    
    /* Access subparts through pointers */
    int* pb = &m.b;
    *pb = seed * 2;
    sink(m.b);
}

/* Pattern 4: Complex MEM destinations */
NOINLINE void test_complex_mem(volatile int seed) {
    volatile int offset = seed % 10;
    volatile int index = seed % 20;
    
    /* Structure with pointer arithmetic */
    struct Point {
        int x, y, z;
    } points[10];
    
    struct Point* ptr = &points[offset];
    ptr->x = seed;
    ptr->y = seed * 2;
    ptr->z = seed * 3;
    
    sink(points[offset].x);
    
    /* Global variable with computed address */
    int* addr = &global_array[index] + offset;
    *addr = seed;
    sink(global_array[index + offset]);
    
    /* Complex addressing mode */
    global_struct.a = seed;
    global_struct.b = seed + 1;
    global_struct.c = (long long)seed * 100;
    global_struct.d = seed - 1;
    
    /* Pointer to structure member with offset */
    int* member_ptr = &global_struct.a;
    for (int i = 0; i < 4; i++) {
        member_ptr[i] = seed + i * 100;
    }
    
    sink(global_struct.d);
    
    /* Multi-dimensional array */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * 10 + j + seed;
        }
    }
    
    /* Complex computed address */
    int* mat_ptr = &matrix[offset % 4][offset % 3];
    *mat_ptr = seed * 7;
    sink(matrix[offset % 4][offset % 3]);
}

/* Pattern 5: Combined patterns in control flow */
NOINLINE void test_combined(volatile int seed) {
    volatile int mode = seed % 4;
    int result = 0;
    
    for (int i = 0; i < (seed & 7) + 1; i++) {
        switch (mode) {
            case 0: {
                /* ZERO_EXTRACT-like */
                union {
                    unsigned int val;
                    struct {
                        unsigned int a: 10;
                        unsigned int b: 10;
                        unsigned int c: 12;
                    } bits;
                } u;
                u.bits.a = (seed + i) & 0x3FF;
                u.bits.b = (seed * i) & 0x3FF;
                result += u.val;
                break;
            }
            case 1: {
                /* STRICT_LOW_PART-like */
                long long big = 0xFEDCBA9876543210LL;
                short s = (short)(seed + i);
                big = (big & ~0xFFFFLL) | (s & 0xFFFFLL);
                result += (int)big;
                break;
            }
            case 2: {
                /* SUBREG-like */
                int arr[4] = {1, 2, 3, 4};
                short* sp = (short*)arr;
                sp[i % 2] = (short)(seed + i);
                result += arr[i % 4];
                break;
            }
            case 3: {
                /* Complex MEM */
                struct Node {
                    int data;
                    struct Node* next;
                } nodes[5];
                
                int idx = (seed + i) % 5;
                nodes[idx].data = seed * i;
                result += nodes[idx].data;
                break;
            }
        }
        mode = (mode + 1) % 4;
    }
    
    sink(result);
}

/* Main driver */
int main(int argc, char** argv) {
    /* Use command line or time for volatile seed */
    volatile int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    printf("Testing with seed: %d\n", seed);
    
    /* Initialize globals */
    memset(global_array, 0, sizeof(global_array));
    memset(&global_struct, 0, sizeof(global_struct));
    
    /* Run all pattern tests */
    test_zero_extract(seed);
    test_strict_low_part(seed);
    test_subreg(seed);
    test_complex_mem(seed);
    test_combined(seed);
    
    /* Create a checksum from global state */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= global_array[i];
    }
    checksum ^= global_struct.a;
    checksum ^= global_struct.b;
    checksum ^= (int)global_struct.c;
    checksum ^= global_struct.d;
    
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_long(long x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void sink(int x) { (void)x; }
