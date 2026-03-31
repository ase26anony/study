/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);

/* Volatile seed to prevent compile-time optimization */
static volatile int seed = 0;

/* Pattern 1: ZERO_EXTRACT destination through bitfield operations */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int input) {
    /* Use union with bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } data;
    
    data.full = 0x12345678;
    
    /* Store into bitfield - may generate ZERO_EXTRACT destination */
    data.bits.mid = input & 0xFF;
    
    /* Complex bitfield manipulation in loop */
    for (int i = 0; i < (input & 3); i++) {
        data.bits.low = (data.bits.low + i) & 0xFF;
        data.bits.high = (data.bits.high ^ (input >> i)) & 0xFFFF;
    }
    
    use(data.full);
}

/* Pattern 2: STRICT_LOW_PART destination through partial word stores */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile short input) {
    int value = 0xDEADBEEF;
    
    /* Store short into int - may generate STRICT_LOW_PART */
    value = (value & ~0xFFFF) | (input & 0xFFFF);
    
    /* Alternative: pointer casting approach */
    if (input > 0) {
        *(short*)&value = input;
    }
    
    /* Multiple partial stores */
    for (int i = 0; i < 2; i++) {
        char* byte_ptr = (char*)&value + i;
        *byte_ptr = (input >> (i * 8)) & 0xFF;
    }
    
    use(value);
}

/* Pattern 3: SUBREG destination through type punning */
__attribute__((noinline, noipa))
void test_subreg(volatile int input) {
    /* Array with sub-word access */
    int array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    
    /* Access via different type pointer - may generate SUBREG */
    short* ps = (short*)&array[input & 3];
    *ps = input & 0xFFFF;
    
    /* Long long to int conversion */
    long long big = 0x123456789ABCDEF0LL;
    int* p = (int*)&big;
    p[input & 1] = input;
    
    /* Structure with mixed types */
    struct Mixed {
        char c;
        short s;
        int i;
        long long ll;
    } mix;
    
    mix.s = input & 0xFFFF;
    mix.i = input;
    
    use(array[0] + array[1] + (int)big + mix.i);
}

/* Pattern 4: Complex MEM_P destination with addressing modes */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int index) {
    /* Global-like static variable */
    static int globals[10] = {0};
    
    /* Complex addressing computation */
    int* ptr = &globals[(index * 7 + 3) % 10];
    *ptr = index;
    
    /* Structure with pointer arithmetic */
    struct Point {
        int x, y, z;
    } points[5];
    
    /* Compute address with offset */
    int* field_ptr = &points[index % 5].y + (index & 1);
    *field_ptr = index * 2;
    
    /* Multi-dimensional array with computed index */
    int matrix[4][4];
    int (*row_ptr)[4] = &matrix[index % 4];
    row_ptr[0][index & 3] = index * 3;
    
    /* Prevent optimization */
    sink(globals);
    sink(points);
    sink(matrix);
}

/* Pattern 5: Combined patterns in complex control flow */
__attribute__((noinline, noipa))
int test_combined(volatile int input) {
    int result = 0;
    
    /* Switch with different patterns */
    switch (input & 3) {
        case 0: {
            /* ZERO_EXTRACT-like */
            union {
                unsigned int val;
                struct {
                    unsigned int a: 10;
                    unsigned int b: 10;
                    unsigned int c: 12;
                } fields;
            } u;
            u.val = input;
            u.fields.b = (input * 3) & 0x3FF;
            result += u.val;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART-like */
            long value = 0x1234567890ABCDEF;
            *(int*)&value = input;
            result += (int)value;
            break;
        }
        case 2: {
            /* SUBREG-like */
            char buffer[8];
            *(short*)(buffer + (input & 6)) = input & 0xFFFF;
            result += buffer[0];
            break;
        }
        case 3: {
            /* MEM_P with complex address */
            int* heap_ptr = (int*)malloc(sizeof(int) * 4);
            if (heap_ptr) {
                heap_ptr[input & 3] = input;
                result += heap_ptr[0];
                free(heap_ptr);
            }
            break;
        }
    }
    
    return result;
}

/* Main driver with volatile control flow */
int main(int argc, char** argv) {
    /* Initialize volatile seed */
    seed = (argc > 1) ? atoi(argv[1]) : (int)time(NULL);
    
    int checksum = 0;
    
    /* Call pattern functions with volatile inputs */
    test_zero_extract(seed);
    checksum += seed & 0xFF;
    
    test_strict_low_part(seed & 0xFFFF);
    checksum += (seed >> 8) & 0xFF;
    
    test_subreg(seed ^ 0x55AA55AA);
    checksum += (seed >> 16) & 0xFF;
    
    test_complex_mem(seed % 100);
    checksum += seed % 256;
    
    /* Combined test with loop */
    for (volatile int i = 0; i < (seed & 7); i++) {
        checksum += test_combined(seed + i);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return checksum & 1;
}

/* Dummy external function definitions */
void use(int x) {
    static volatile int sink;
    sink = x;
}

void sink(void* p) {
    static volatile void* vsink;
    vsink = p;
}
