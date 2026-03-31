/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Global variables for memory patterns */
int global_array[1024];
struct compound {
    int a;
    long b;
    short c;
    char d;
} global_struct;

/* Pattern 1: ZERO_EXTRACT destination */
NOINLINE void test_zero_extract(volatile int seed) {
    /* Using bitfields in unions */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = seed;
    /* Assignment to bitfield may generate ZERO_EXTRACT */
    u.bits.mid = volatile_input() & 0xFF;
    u.bits.high = (volatile_input() >> 8) & 0xFFFF;
    
    /* Complex bitfield manipulation */
    volatile int src = seed;
    unsigned int dest = 0;
    dest = (dest & ~0xFF00) | ((src & 0xFF) << 8);
    dest = (dest & ~0xFF0000) | ((src & 0xFF00) << 8);
    
    use(u.full);
    use(dest);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(volatile int seed) {
    /* Assignment to low part of larger integer */
    long long big = seed * 1000LL;
    short s = volatile_input() & 0x7FFF;
    
    /* This may generate STRICT_LOW_PART */
    *(short*)&big = s;
    
    /* Another pattern using masking */
    int val = seed;
    short low_part = volatile_input() & 0xFFFF;
    val = (val & ~0xFFFF) | (low_part & 0xFFFF);
    
    /* Pointer casting to access low part */
    long lval = seed * 100L;
    int* p = (int*)&lval;
    *p = volatile_input();
    
    use(big);
    use(val);
    use(lval);
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(volatile int seed) {
    /* Type punning through pointers */
    long long big_array[4];
    volatile int idx = seed & 3;
    
    /* Access sub-word parts */
    int* p_int = (int*)&big_array[idx];
    *p_int = volatile_input();
    
    short* p_short = (short*)&big_array[1];
    p_short[1] = volatile_input() & 0xFFFF;
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.ll = seed * 1000LL;
    /* Access subregions */
    short* ps = (short*)&m.ll;
    ps[2] = volatile_input() & 0xFFFF;
    
    /* Array with byte access */
    int arr[8];
    char* byte_ptr = (char*)arr;
    byte_ptr[seed & 31] = volatile_input() & 0xFF;
    
    use(big_array[0]);
    use(m.ll);
    use(arr[0]);
}

/* Pattern 4: Complex MEM destinations */
NOINLINE void test_complex_mem(volatile int seed) {
    volatile int offset = seed & 1023;
    volatile int idx = seed & 7;
    
    /* Complex addressing modes */
    int* ptr1 = &global_array[offset + idx * 2];
    *ptr1 = volatile_input();
    
    /* Structure member with offset */
    struct compound* sptr = &global_struct;
    int* ptr2 = &sptr->a + offset;
    *ptr2 = volatile_input();
    
    /* Pointer arithmetic with scaling */
    short* base = (short*)global_array;
    short* ptr3 = base + (offset * 3) / 2;
    *ptr3 = volatile_input() & 0xFFFF;
    
    /* Multi-dimensional access */
    int matrix[8][8];
    volatile int row = seed & 7;
    volatile int col = seed & 7;
    int* cell = &matrix[row][col];
    *cell = volatile_input();
    
    /* Indirect through pointer array */
    int* ptr_array[4];
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &global_array[i * 64];
    }
    *ptr_array[idx] = volatile_input();
    
    use(*ptr1);
    use(*ptr2);
    use(matrix[0][0]);
}

/* Pattern 5: Combined patterns in loops */
NOINLINE void test_combined_patterns(volatile int seed) {
    volatile int limit = (seed & 7) + 1;
    int result = 0;
    
    for (int i = 0; i < limit; i++) {
        volatile int val = seed + i;
        
        /* Mix different patterns */
        union {
            unsigned int full;
            struct {
                unsigned int low: 10;
                unsigned int high: 22;
            } bits;
        } u;
        
        u.full = result;
        /* ZERO_EXTRACT pattern */
        u.bits.low = val & 0x3FF;
        
        /* SUBREG pattern */
        long long buffer[2];
        int* p = (int*)&buffer[i & 1];
        *p = val;
        
        /* MEM pattern with complex address */
        global_array[(val + i) & 1023] = val;
        
        /* STRICT_LOW_PART pattern */
        int temp = result;
        short low = val & 0xFFFF;
        temp = (temp & ~0xFFFF) | (low & 0xFFFF);
        
        result = u.full + *p + global_array[0] + temp;
    }
    
    use(result);
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
    
    /* Initialize globals */
    memset(global_array, 0, sizeof(global_array));
    global_struct.a = seed;
    global_struct.b = seed * 100L;
    global_struct.c = seed & 0xFFFF;
    global_struct.d = seed & 0xFF;
    
    /* Run all pattern tests */
    test_zero_extract(seed);
    test_strict_low_part(seed + 1);
    test_subreg(seed + 2);
    test_complex_mem(seed + 3);
    test_combined_patterns(seed + 4);
    
    /* Create checksum to prevent optimization */
    int checksum = global_array[seed & 1023] + 
                   global_struct.a + 
                   (global_struct.c & 0xFFFF);
    
    printf("Result: %d\n", checksum);
    return 0;
}

/* Dummy definitions to satisfy linker */
void use(int x) {
    /* Empty but prevents dead code elimination */
    global_array[0] ^= x;
}

void sink(void* p) {
    /* Empty */
}

int volatile_input(void) {
    static int counter = 0;
    return ++counter;
}
