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
    short c;
    char d;
    int e[4];
} global_struct;

/* Pattern 1: ZERO_EXTRACT destination via bitfield operations */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int seed) {
    /* Using union with bitfields */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = seed;
    /* This assignment to bitfield may generate ZERO_EXTRACT */
    u.bits.mid = (seed >> 4) & 0xFF;
    u.bits.high = (seed >> 8) & 0xFFFF;
    
    /* Force computation to prevent optimization */
    volatile int temp = u.full;
    for (int i = 0; i < (seed & 0x3); i++) {
        temp ^= u.bits.low << i;
    }
    sink(temp);
    
    /* Another pattern: explicit bitfield extraction */
    unsigned int value = seed * 3;
    /* Store into masked portion - may generate ZERO_EXTRACT */
    value = (value & ~0xFF00) | ((seed & 0xFF) << 8);
    sink(value);
}

/* Pattern 2: STRICT_LOW_PART destination via partial word stores */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile int seed) {
    /* Assign short to int with masking */
    int large = seed * 1000;
    short small = (short)(seed + 1);
    
    /* This may generate STRICT_LOW_PART */
    large = (large & ~0xFFFF) | (small & 0xFFFF);
    
    /* Use in loop to keep it alive */
    int sum = 0;
    for (int i = 0; i < (seed & 0x7); i++) {
        sum += large + i;
    }
    sink(sum);
    
    /* Pointer casting approach */
    long long big_val = seed * 1000000LL;
    int* ptr = (int*)&big_val;
    /* Store to low part of long long */
    *ptr = seed + 100;
    
    /* Another pattern with type punning */
    struct {
        int whole;
    } container;
    container.whole = seed;
    short* sp = (short*)&container.whole;
    *sp = (short)(seed ^ 0x55AA);  /* May generate STRICT_LOW_PART */
    
    sink(container.whole);
}

/* Pattern 3: SUBREG destination via sub-word accesses */
__attribute__((noinline, noipa))
void test_subreg(volatile int seed) {
    /* Array with sub-word access */
    int array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = seed + i * 100;
    }
    
    /* Access via short pointer - may generate SUBREG */
    short* short_ptr = (short*)&array[seed & 0x3];
    *short_ptr = (short)(seed ^ 0x1234);
    
    /* Type punning between different sizes */
    long long big = seed * 1000000000LL;
    int* int_ptr = (int*)&big;
    /* Store to part of long long - may use SUBREG */
    int_ptr[1] = seed + 999;  /* High part on little-endian */
    
    /* Structure with mixed types */
    struct Mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.ll = big;
    /* Access subparts */
    short* sptr = (short*)&m.ll;
    sptr[2] = (short)seed;  /* May generate SUBREG */
    
    /* Use results */
    sink(array[0] + m.i);
}

/* Pattern 4: Complex MEM destinations with addressing modes */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int seed, volatile int index) {
    /* Complex addressing with structure */
    struct Nested {
        int data[10];
        struct {
            int x;
            int y;
        } point;
        char padding[7];
    } nested;
    
    /* Store with complex address calculation */
    int* ptr = &nested.data[index & 0x7];
    *ptr = seed * 2;  /* MEM with index */
    
    /* Pointer arithmetic with volatile offset */
    volatile int offset = index & 0x3;
    int* addr = &global_array[10] + offset;
    *addr = seed + offset;
    
    /* Structure member access with computation */
    global_struct.e[(index + 1) & 0x3] = seed * 3;
    
    /* Multi-dimensional access */
    int matrix[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = seed + i * 10 + j;
        }
    }
    
    /* Store with 2D indexing */
    int* elem = &matrix[index & 0x3][(index >> 2) & 0x3];
    *elem = seed ^ 0xABCD;
    
    /* Use all results */
    sink(nested.data[0] + global_array[10] + matrix[0][0]);
}

/* Pattern 5: Combined patterns in loops */
__attribute__((noinline, noipa))
void test_combined_patterns(volatile int seed) {
    int result = 0;
    
    /* Loop with mixed operations */
    for (int i = 0; i < (seed & 0xF) + 1; i++) {
        /* ZERO_EXTRACT pattern */
        unsigned int val = seed + i;
        val = (val & ~0xF0) | ((i & 0xF) << 4);
        
        /* STRICT_LOW_PART pattern */
        long big_val = val * 100L;
        int* ip = (int*)&big_val;
        ip[0] = val + i;  /* Little-endian low part */
        
        /* SUBREG pattern */
        short* sp = (short*)&big_val;
        sp[1] = (short)(val ^ i);  /* Access high short */
        
        /* Complex MEM pattern */
        global_array[i % 10] = val + big_val;
        
        result += val + big_val;
    }
    
    sink(result);
}

/* Main driver */
int main(int argc, char** argv) {
    /* Initialize volatile seeds */
    volatile_seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    volatile_index = (volatile_seed * 13) % 100;
    volatile_short = (short)(volatile_seed ^ 0x55AA);
    volatile_int = volatile_seed * 7;
    volatile_long = volatile_seed * 1000000L;
    
    /* Initialize globals */
    memset(global_array, 0, sizeof(global_array));
    memset(&global_struct, 0, sizeof(global_struct));
    
    /* Run all pattern tests */
    test_zero_extract(volatile_seed);
    test_strict_low_part(volatile_seed);
    test_subreg(volatile_seed);
    test_complex_mem(volatile_seed, volatile_index);
    test_combined_patterns(volatile_seed);
    
    /* Create checksum of results */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum ^= global_array[i];
    }
    checksum ^= global_struct.e[0];
    checksum ^= volatile_int;
    
    printf("Result checksum: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_long(long x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void sink(int x) { (void)x; }
