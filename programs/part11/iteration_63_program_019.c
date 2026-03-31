/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Volatile seed to prevent compile-time optimization */
static volatile int seed = 0;

/* Global variables for memory patterns */
int global_array[100];
struct Complex {
    int a;
    int b;
    long c;
    short d;
} global_struct;

/* Pattern 1: ZERO_EXTRACT destination */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int input) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = seed;
    /* This assignment to a bitfield may generate ZERO_EXTRACT */
    u.bits.mid = input & 0xFF;
    
    /* Complex control flow to keep it alive */
    for (int i = 0; i < (input & 3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
    }
    
    use_int(u.full);
    sink(u.bits.high);
}

/* Pattern 2: STRICT_LOW_PART destination */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile short input) {
    int val = seed * 3;
    
    /* Assigning short to int may generate STRICT_LOW_PART */
    /* Using pointer cast to force low-part assignment */
    short *ps = (short*)&val;
    *ps = input;
    
    /* Alternative: explicit masking */
    int val2 = seed;
    val2 = (val2 & ~0xFFFF) | (input & 0xFFFF);
    
    /* Mix with control flow */
    if (input > 0) {
        ps = (short*)&val2;
        ps[1] = input >> 8;  /* High part of short */
    }
    
    use_int(val);
    use_int(val2);
}

/* Pattern 3: SUBREG destination */
__attribute__((noinline, noipa))
void test_subreg(volatile int input) {
    /* Type punning with different sizes */
    long long big_val = (long long)seed * 1000LL;
    
    /* Accessing parts of larger object */
    int *p_int = (int*)&big_val;
    *p_int = input;  /* May generate SUBREG */
    
    /* Array with sub-word access */
    int array[4] = {seed, seed+1, seed+2, seed+3};
    short *p_short = (short*)array;
    
    for (int i = 0; i < (input & 7); i++) {
        p_short[i] = (input + i) & 0xFFFF;  /* SUBREG patterns */
    }
    
    /* Structure with mixed types */
    struct Mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = input;
    m.s = input >> 16;
    
    use_long(big_val);
    use_int(array[input & 3]);
    use_int(m.i);
}

/* Pattern 4: Complex MEM destination with addressing modes */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int index) {
    /* Complex addressing modes */
    int *ptr = &global_array[index % 50];
    *ptr = seed + index;  /* MEM with index */
    
    /* Structure member access with offset */
    struct Complex *sptr = &global_struct;
    sptr->b = index * 2;
    
    /* Pointer arithmetic */
    int *base = global_array;
    int offset = (index * 3) % 40;
    base[offset + 5] = seed - index;
    
    /* Multi-dimensional access */
    int matrix[10][10];
    for (int i = 0; i < (index & 3); i++) {
        for (int j = 0; j < (index & 3); j++) {
            matrix[i][j] = seed + i * 10 + j;
        }
    }
    
    /* Indirect through pointer array */
    int *ptr_array[5];
    for (int i = 0; i < 5; i++) {
        ptr_array[i] = &global_array[i * 10];
    }
    *ptr_array[index % 5] = index;
    
    use_ptr(ptr);
    use_int(sptr->a);
    use_int(matrix[0][0]);
}

/* Pattern 5: Combined patterns in loop */
__attribute__((noinline, noipa))
void test_combined(volatile int iterations) {
    union {
        unsigned int full;
        struct {
            unsigned int low: 12;
            unsigned int high: 20;
        } bits;
    } data;
    
    int results[10] = {0};
    
    for (int i = 0; i < (iterations % 10) + 1; i++) {
        /* Mix different patterns */
        data.full = seed + i;
        
        /* ZERO_EXTRACT pattern */
        data.bits.low = (i * 7) & 0xFFF;
        
        /* SUBREG pattern */
        short *ps = (short*)&data.full;
        ps[1] = (i * 13) & 0xFFFF;  /* High part */
        
        /* STRICT_LOW_PART-like */
        int temp = results[i % 5];
        temp = (temp & ~0xFFFF) | (i & 0xFFFF);
        results[i % 5] = temp;
        
        /* MEM pattern */
        global_array[i] = data.full + temp;
    }
    
    /* Use results to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += results[i];
    }
    use_int(sum);
}

/* Main driver */
int main(int argc, char *argv[]) {
    /* Initialize volatile seed */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) & 0xFFFF;
    }
    
    volatile int input1 = seed * 3;
    volatile short input2 = seed & 0xFFFF;
    volatile int input3 = seed ^ 0x1234;
    volatile int input4 = seed + 100;
    volatile int input5 = seed % 20;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * i;
    }
    global_struct.a = seed;
    global_struct.b = seed * 2;
    global_struct.c = seed * 100LL;
    global_struct.d = seed & 0x7FFF;
    
    /* Execute all patterns */
    test_zero_extract(input1);
    test_strict_low_part(input2);
    test_subreg(input3);
    test_complex_mem(input4);
    test_combined(input5);
    
    /* Create checksum of results */
    int checksum = 0;
    for (int i = 0; i < 50; i++) {
        checksum ^= global_array[i];
    }
    checksum ^= global_struct.a;
    checksum ^= global_struct.b;
    checksum ^= (int)global_struct.c;
    checksum ^= global_struct.d;
    
    printf("Result checksum: %d\n", checksum);
    printf("Seed was: %d\n", seed);
    
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_long(long x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void sink(int x) { (void)x; }
