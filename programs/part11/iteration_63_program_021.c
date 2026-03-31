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
struct ComplexStruct {
    int a;
    int b;
    long long c;
    short d;
} global_struct;

/* Pattern 1: ZERO_EXTRACT destination */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int seed) {
    /* Using union with bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = seed;
    
    /* Store into specific bitfield - may generate ZERO_EXTRACT destination */
    u.bits.mid = (volatile_short & 0xFF);
    u.bits.high = (volatile_int & 0xFFFF);
    
    /* Complex bitfield manipulation */
    for (int i = 0; i < (seed & 3); i++) {
        u.bits.low = (u.bits.low + volatile_int) & 0xFF;
        u.bits.mid = (u.bits.mid ^ volatile_short) & 0xFF;
    }
    
    use_int(u.full);
    sink(u.bits.low | u.bits.mid << 8 | u.bits.high << 16);
}

/* Pattern 2: STRICT_LOW_PART destination */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile int seed) {
    int val = seed;
    short s_val = volatile_short;
    
    /* Assign short to int - may generate STRICT_LOW_PART */
    val = (val & ~0xFFFF) | (s_val & 0xFFFF);
    
    /* Pointer casting approach */
    int val2 = seed * 2;
    *(short*)&val2 = volatile_short;
    
    /* Loop with strict low part assignments */
    for (int i = 0; i < (volatile_index & 3); i++) {
        int temp = val;
        *(short*)&temp = (volatile_short + i) & 0xFFFF;
        val ^= temp;
    }
    
    use_int(val);
    use_int(val2);
    sink(val + val2);
}

/* Pattern 3: SUBREG destination */
__attribute__((noinline, noipa))
void test_subreg(volatile int seed) {
    /* Type punning with different sizes */
    long long big_val = (long long)seed * 1000LL;
    int *p_int = (int*)&big_val;
    
    /* Store into part of long long - may generate SUBREG */
    p_int[0] = volatile_int;
    p_int[1] = volatile_int ^ seed;
    
    /* Array with sub-word access */
    int array[4] = {seed, seed+1, seed+2, seed+3};
    short *ps = (short*)array;
    
    for (int i = 0; i < (volatile_index & 7); i++) {
        ps[i] = (volatile_short + i) & 0x7FFF;
    }
    
    /* Structure member access with casting */
    struct Mixed {
        char a;
        short b;
        int c;
    } m;
    m.c = seed;
    short *pb = (short*)&m.c;
    pb[0] = volatile_short;
    
    use_long(big_val);
    use_int(array[0] + array[3]);
    sink(m.c);
}

/* Pattern 4: Complex MEM destination with addressing modes */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int seed) {
    /* Complex pointer arithmetic */
    int *ptr = &global_array[volatile_index];
    
    for (int i = 0; i < (seed & 7); i++) {
        ptr[i] = volatile_int + i;
        ptr[i * 2] = volatile_int ^ i;
    }
    
    /* Structure with offset */
    struct ComplexStruct local_struct;
    local_struct.a = seed;
    local_struct.b = volatile_int;
    
    int *struct_ptr = &local_struct.a;
    struct_ptr[volatile_index & 1] = volatile_int * 2;
    
    /* Computed address with multiple operations */
    int *addr = &global_array[0] + (volatile_index * 3) / 2;
    *addr = volatile_int;
    
    /* Nested addressing */
    int **pptr = &ptr;
    (*pptr)[(volatile_index & 3)] = seed;
    
    use_ptr(ptr);
    use_int(local_struct.a + local_struct.b);
    sink(*addr + ptr[0]);
}

/* Pattern 5: Combined patterns in complex control flow */
__attribute__((noinline, noipa))
void test_combined_patterns(volatile int seed) {
    int result = 0;
    
    /* Switch with different patterns */
    switch (seed & 3) {
        case 0: {
            /* ZERO_EXTRACT pattern */
            union {
                unsigned int val;
                struct {
                    unsigned int a:10;
                    unsigned int b:10;
                    unsigned int c:12;
                } f;
            } u;
            u.val = seed;
            u.f.b = volatile_short & 0x3FF;
            result = u.val;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART pattern */
            int x = seed * 3;
            *(short*)&x = volatile_short;
            result = x;
            break;
        }
        case 2: {
            /* SUBREG pattern */
            long long ll = seed;
            int *p = (int*)&ll;
            p[1] = volatile_int;
            result = (int)(ll >> 32);
            break;
        }
        case 3: {
            /* Complex MEM pattern */
            int *p = &global_array[volatile_index & 15];
            *p = volatile_int;
            result = *p;
            break;
        }
    }
    
    /* Loop with mixed operations */
    for (int i = 0; i < (volatile_index & 3); i++) {
        int temp = result;
        
        /* Alternate between patterns */
        if (i & 1) {
            /* SUBREG-like */
            short *ps = (short*)&temp;
            ps[0] = (volatile_short + i) & 0x7FFF;
        } else {
            /* STRICT_LOW_PART-like */
            temp = (temp & ~0xFFFF) | ((volatile_short + i) & 0xFFFF);
        }
        
        result ^= temp;
    }
    
    sink(result);
}

/* Main driver */
int main(int argc, char *argv[]) {
    /* Initialize volatile variables */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    volatile_short = (volatile_seed >> 8) & 0xFFFF;
    volatile_int = volatile_seed ^ 0x12345678;
    volatile_long = (long)volatile_seed * 1000L;
    volatile_index = (volatile_seed >> 16) & 0xFF;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i + volatile_seed;
    }
    
    global_struct.a = volatile_seed;
    global_struct.b = volatile_int;
    global_struct.c = volatile_long;
    global_struct.d = volatile_short;
    
    /* Execute all pattern tests */
    test_zero_extract(volatile_seed);
    test_strict_low_part(volatile_seed);
    test_subreg(volatile_seed);
    test_complex_mem(volatile_seed);
    test_combined_patterns(volatile_seed);
    
    /* Create checksum to prevent optimization */
    int checksum = global_array[volatile_index & 99] 
                   + global_struct.a 
                   + (int)(global_struct.c & 0xFFFFFFFF);
    
    printf("Result: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { volatile int dummy = x; (void)dummy; }
void use_long(long x) { volatile long dummy = x; (void)dummy; }
void use_ptr(void* x) { volatile void* dummy = x; (void)dummy; }
void sink(int x) { volatile int dummy = x; (void)dummy; }
