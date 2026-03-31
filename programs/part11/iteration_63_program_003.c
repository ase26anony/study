/* test_resource_patterns.c
 * Generates RTL patterns to cover specific SET destination cases in GCC's resource.cc
 * Compile with: gcc -O2 -fno-strict-aliasing -c test.c -o test.o
 * Or for more patterns: gcc -O3 -fno-tree-vectorize -fno-gcse -c test.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern int get_volatile_int(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile variables to prevent constant propagation */
volatile int volatile_seed;
volatile int volatile_index;
volatile short volatile_short;
volatile int volatile_int;

/* Pattern 1: ZERO_EXTRACT destination via bitfield operations */
NOINLINE void test_zero_extract(int seed) {
    /* Using union with bitfields */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    /* Volatile operations to prevent optimization */
    u.full = seed;
    u.bits.mid = (unsigned char)(seed * 3);  /* Should generate ZERO_EXTRACT */
    u.bits.high = (unsigned short)(seed * 5); /* Another ZERO_EXTRACT */
    
    /* Force usage */
    use_int(u.full);
    
    /* Alternative: Using bitwise operations */
    unsigned int val = seed;
    unsigned int mask = 0xFF00;
    unsigned int insert = (seed * 7) & 0xFF;
    
    /* This pattern may generate ZERO_EXTRACT */
    val = (val & ~mask) | ((insert << 8) & mask);
    use_int(val);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(int seed) {
    /* Assignment to low part of larger integer */
    int large = seed * 11;
    short small = (short)(seed * 13);
    
    /* This should generate STRICT_LOW_PART */
    large = (large & ~0xFFFF) | (small & 0xFFFF);
    use_int(large);
    
    /* Using pointer casting */
    long long big_val = seed * 17LL;
    int* p_int = (int*)&big_val;
    *p_int = seed * 19;  /* Modifies low part only */
    use_long(big_val);
    
    /* Direct assignment through cast */
    int val = seed * 23;
    *(short*)&val = (short)(seed * 29);
    use_int(val);
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(int seed) {
    /* Array with sub-word access */
    int array[4] = {seed, seed*2, seed*3, seed*4};
    volatile int idx = volatile_index & 3;
    
    /* Access via different type pointer */
    short* ps = (short*)&array[idx];
    *ps = (short)(seed * 31);  /* Should generate SUBREG */
    use_int(array[idx]);
    
    /* Type punning with different sizes */
    long long big = seed * 37LL;
    int* p = (int*)&big;
    p[1] = seed * 41;  /* Access high part (platform dependent) */
    use_long(big);
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = seed * 43;
    *(short*)((char*)&m + 1) = (short)(seed * 47);  /* Unaligned SUBREG */
    use_int(m.i);
}

/* Pattern 4: Complex MEM destination with addressing modes */
NOINLINE void test_complex_mem(int seed) {
    /* Structure with pointer arithmetic */
    struct S {
        int a;
        int b;
        int c;
        int d;
    } s;
    
    s.a = seed;
    s.b = seed * 53;
    
    volatile int off = volatile_index & 3;
    int* ptr = &s.a + off;  /* Computed address */
    *ptr = seed * 59;       /* Complex MEM destination */
    use_int(s.a + s.b + s.c + s.d);
    
    /* Global-like access via pointer */
    static int globals[10];
    volatile int idx2 = volatile_index % 10;
    
    /* Multiple addressing modes */
    int* addr1 = &globals[idx2];
    int* addr2 = globals + (idx2 * 2) % 10;
    int* addr3 = &globals[0] + (idx2 + 3) % 10;
    
    *addr1 = seed * 61;
    *addr2 = seed * 67;
    *addr3 = seed * 71;
    
    /* Sum to force usage */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += globals[i];
    }
    use_int(sum);
    
    /* Stack array with index calculation */
    int local_arr[8];
    for (int i = 0; i < 8; i++) {
        local_arr[(i + seed) & 7] = seed * (73 + i);
    }
    
    /* Access with complex addressing */
    int* complex_ptr = &local_arr[0] + ((seed * 79) & 7);
    *complex_ptr = seed * 83;
    use_ptr(complex_ptr);
}

/* Pattern 5: Combined patterns in loops */
NOINLINE void test_combined_patterns(int seed) {
    volatile int bound = (volatile_seed & 0xF) + 1;
    int result = seed;
    
    for (int i = 0; i < bound; i++) {
        /* Mix different patterns in loop */
        if (i & 1) {
            /* ZERO_EXTRACT pattern */
            unsigned int val = result;
            unsigned int field = (seed * (i + 1)) & 0xFF;
            int shift = (i * 8) % 24;
            unsigned int mask = 0xFF << shift;
            val = (val & ~mask) | ((field << shift) & mask);
            result ^= val;
        } else {
            /* STRICT_LOW_PART pattern */
            int temp = result;
            short low = (short)(seed * (i + 101));
            temp = (temp & ~0xFFFF) | (low & 0xFFFF);
            result += temp;
        }
        
        /* SUBREG pattern every 3 iterations */
        if (i % 3 == 0) {
            long long big = result * 97LL;
            int* p = (int*)&big;
            p[i & 1] = seed * (i + 103);
            result ^= (int)big;
        }
        
        /* Complex MEM pattern every 4 iterations */
        if (i % 4 == 0) {
            static int counters[5];
            volatile int idx = (volatile_index + i) % 5;
            int* mem_ptr = &counters[0] + idx + (i & 1);
            *mem_ptr = result * (i + 107);
            result += *mem_ptr;
        }
    }
    
    use_int(result);
}

/* Helper to get volatile input */
NOINLINE int get_input(void) {
    return volatile_seed ^ volatile_int ^ (int)volatile_short;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize volatile values */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    volatile_index = volatile_seed * 127;
    volatile_short = (short)(volatile_seed * 131);
    volatile_int = volatile_seed * 137;
    
    /* Get initial value */
    int seed = get_input();
    
    /* Run all pattern tests */
    test_zero_extract(seed);
    test_strict_low_part(seed * 139);
    test_subreg(seed * 149);
    test_complex_mem(seed * 151);
    test_combined_patterns(seed * 157);
    
    /* Final computation to prevent optimization */
    int final_result = 
        (volatile_seed & 0xFF) +
        (volatile_index % 100) +
        (volatile_short & 0xFF) +
        (volatile_int & 0xFF);
    
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;
}

/* Dummy definitions to satisfy linker (in real use would be external) */
void use_int(int x) { volatile int dummy = x; (void)dummy; }
void use_long(long x) { volatile long dummy = x; (void)dummy; }
void use_ptr(void* x) { volatile void* dummy = x; (void)dummy; }
