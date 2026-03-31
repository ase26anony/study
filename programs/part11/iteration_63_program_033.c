/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
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
int global_array[100];
struct ComplexStruct {
    int a;
    int b;
    long c;
    short d;
} global_struct;

/* Pattern 1: ZERO_EXTRACT destination */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int seed) {
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
    
    /* Store into specific bitfield - may generate ZERO_EXTRACT */
    u.bits.mid = (unsigned char)(seed * 3);
    u.bits.high = (unsigned short)(seed * 5);
    
    /* Complex bitfield manipulation */
    for (int i = 0; i < (seed & 3); i++) {
        u.bits.low = (u.bits.low + i) & 0xFF;
    }
    
    use_int(u.full);
    sink(u.bits.mid);
}

/* Pattern 2: STRICT_LOW_PART destination */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile int seed) {
    int large_val = seed * 1000;
    short small_val = (short)(seed * 37);
    
    /* Assignment that modifies only low bits */
    /* May generate STRICT_LOW_PART in RTL */
    large_val = (large_val & ~0xFFFF) | (small_val & 0xFFFF);
    
    /* Pointer casting approach */
    int another_val = seed * 2000;
    *(short*)&another_val = volatile_short;
    
    /* In a loop to increase chances */
    for (int i = 0; i < (seed & 7); i++) {
        int temp = another_val + i;
        *(short*)&temp = (short)(small_val + i);
        use_int(temp);
    }
    
    use_int(large_val);
    use_int(another_val);
}

/* Pattern 3: SUBREG destination */
__attribute__((noinline, noipa))
void test_subreg(volatile int seed) {
    /* Type punning with different sizes */
    long long big_value = (long long)seed * 1000000LL;
    
    /* Access parts of larger object - may generate SUBREG */
    int* p_int = (int*)&big_value;
    p_int[0] = volatile_int;
    p_int[1] = volatile_int * 2;
    
    /* Array with sub-word access */
    int array[4] = {0};
    volatile int idx = seed & 3;
    
    /* Access via short pointer */
    short* ps = (short*)&array[idx];
    *ps = volatile_short;
    
    /* Structure with mixed types */
    struct Mixed {
        char c;
        short s;
        int i;
        long l;
    } m;
    
    m.l = volatile_long;
    *(short*)((char*)&m.l + 2) = volatile_short;  /* SUBREG of MEM */
    
    use_long(big_value);
    use_int(array[idx]);
    sink(m.l);
}

/* Pattern 4: Complex MEM destination with addressing modes */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int seed) {
    /* Complex addressing modes */
    int* ptr = &global_array[volatile_index];
    
    /* Store with index computation */
    for (int i = 0; i < (seed & 15); i++) {
        ptr[i * 2] = volatile_int + i;
        global_array[(i + volatile_index) % 100] = i * 3;
    }
    
    /* Structure member access with offset */
    struct ComplexStruct local_struct;
    int* member_ptr = &local_struct.a + (seed & 1);
    *member_ptr = volatile_int;
    
    /* Pointer arithmetic with scaling */
    char* byte_ptr = (char*)global_array;
    byte_ptr[volatile_index * sizeof(int) + 1] = (char)seed;
    
    /* Global structure access */
    global_struct.d = volatile_short;
    (&global_struct.c)[0] = volatile_long;  /* Array-like access to struct member */
    
    use_ptr(ptr);
    use_int(local_struct.a);
}

/* Pattern 5: Combined patterns in control flow */
__attribute__((noinline, noipa))
void test_combined(volatile int seed) {
    int result = 0;
    
    /* Complex control flow with different patterns */
    for (int i = 0; i < (seed & 31); i++) {
        if (i & 1) {
            /* ZERO_EXTRACT-like pattern */
            union {
                unsigned int val;
                struct {
                    unsigned int low: 12;
                    unsigned int high: 20;
                } parts;
            } u;
            u.val = result;
            u.parts.low = (u.parts.low + i) & 0xFFF;
            result = u.val;
        } else if (i & 2) {
            /* STRICT_LOW_PART-like pattern */
            int temp = result;
            short s = (short)(i * 7);
            temp = (temp & ~0xFFFF) | (s & 0xFFFF);
            result += temp;
        } else {
            /* SUBREG-like pattern */
            long long big = result;
            int* p = (int*)&big;
            p[0] += i;
            result += (int)big;
        }
        
        /* MEM pattern in loop */
        global_array[i % 100] = result;
    }
    
    sink(result);
}

/* Main driver */
int main(int argc, char** argv) {
    /* Initialize volatile variables */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    volatile_index = (volatile_seed >> 3) & 99;
    volatile_short = (short)(volatile_seed * 13);
    volatile_int = volatile_seed * 17;
    volatile_long = volatile_seed * 1000L;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    global_struct.a = 1;
    global_struct.b = 2;
    global_struct.c = 3;
    global_struct.d = 4;
    
    /* Execute all pattern tests */
    test_zero_extract(volatile_seed);
    test_strict_low_part(volatile_seed);
    test_subreg(volatile_seed);
    test_complex_mem(volatile_seed);
    test_combined(volatile_seed);
    
    /* Compute and print checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= global_array[i];
    }
    checksum ^= global_struct.a;
    checksum ^= global_struct.b;
    checksum ^= (int)global_struct.c;
    checksum ^= global_struct.d;
    
    printf("Checksum: %d\n", checksum);
    return checksum & 255;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { volatile int dummy = x; (void)dummy; }
void use_long(long x) { volatile long dummy = x; (void)dummy; }
void use_ptr(void* x) { volatile void* dummy = x; (void)dummy; }
void sink(int x) { volatile int dummy = x; (void)dummy; }
