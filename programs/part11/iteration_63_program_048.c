/* test_resource_patterns.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - SET with ZERO_EXTRACT destination
 * - SET with STRICT_LOW_PART destination  
 * - SET with SUBREG destination
 * - SET with complex MEM destination
 */

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

/* ========== Pattern 1: ZERO_EXTRACT destination ========== */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int seed) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union BitfieldUnion {
        unsigned int full;
        struct {
            unsigned int low16 : 16;
            unsigned int high16 : 16;
        } parts;
    } u;
    
    /* Initialize with volatile to prevent constant propagation */
    u.full = seed * 0x12345678;
    
    /* Assignment to bitfield - may generate ZERO_EXTRACT */
    u.parts.low16 = (unsigned short)(seed + 0xABCD);
    u.parts.high16 = (unsigned short)(seed * 0x55AA);
    
    /* More complex bitfield manipulation */
    unsigned int mask = (seed & 0xFF) | 0x100;
    u.full = (u.full & ~mask) | ((seed * 0x89ABCDEF) & mask);
    
    /* Use result to keep computation live */
    use_int(u.full);
    sink(u.parts.low16 + u.parts.high16);
}

/* ========== Pattern 2: STRICT_LOW_PART destination ========== */
__attribute__((noinline, noipa)) 
void test_strict_low_part(volatile int seed) {
    int large_val = seed * 0x87654321;
    short small_val = (short)(seed + 0x1234);
    
    /* Assignment that modifies only low bits - may generate STRICT_LOW_PART */
    /* Method 1: Direct assignment through pointer */
    *(short*)&large_val = small_val;
    
    /* Method 2: Bitwise operation preserving high bits */
    large_val = (large_val & ~0xFFFF) | (small_val & 0xFFFF);
    
    /* Method 3: In loop with volatile control */
    for (int i = 0; i < (seed & 3) + 1; i++) {
        short temp = (short)(small_val + i * 0x100);
        *(short*)&large_val = temp;
        use_int(large_val);
    }
    
    /* Mixed size operations */
    long long_val = seed * 0x12345678L;
    int int_val = seed + 0xABCD;
    
    /* Cast assignment that might use STRICT_LOW_PART */
    *(int*)&long_val = int_val;
    
    use_long(long_val);
    sink(large_val);
}

/* ========== Pattern 3: SUBREG destination ========== */
__attribute__((noinline, noipa))
void test_subreg(volatile int seed) {
    /* Array access with type punning */
    int array[4] = {seed, seed + 1, seed + 2, seed + 3};
    volatile int idx = seed & 3;
    
    /* Access sub-word through pointer - may generate SUBREG */
    short* ps = (short*)&array[idx];
    *ps = (short)(seed * 0x55AA);
    
    /* Another sub-word access */
    char* pc = (char*)&array[(idx + 1) & 3];
    pc[1] = (char)(seed & 0xFF);
    
    /* Large type with sub-word access */
    long long big_val = seed * 0x1122334455667788LL;
    int* p_int = (int*)&big_val;
    
    /* Assignment to part of larger object */
    p_int[0] = seed + 0x1111;
    p_int[1] = seed + 0x2222;
    
    /* Structure with mixed types */
    struct Mixed {
        long a;
        int b;
        short c;
        char d;
    } m;
    
    m.a = seed * 0x12345678L;
    m.b = seed + 0xABCD;
    m.c = (short)(seed & 0xFFFF);
    m.d = (char)(seed & 0xFF);
    
    /* Access structure members through different type pointers */
    short* p_short = (short*)&m;
    p_short[2] = (short)(seed * 0x3333);
    
    use_ptr(array);
    use_long(big_val);
    sink(m.b + m.c);
}

/* ========== Pattern 4: Complex MEM destination ========== */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int seed) {
    /* Complex addressing modes */
    int* ptr;
    
    /* Array with volatile index */
    ptr = &global_array[volatile_index % 50];
    *ptr = seed + 0x1111;
    
    /* Pointer arithmetic */
    ptr = global_array + (seed & 0x1F) + (volatile_index & 0x1F);
    *ptr = seed * 0x2222;
    
    /* Structure member access with offset */
    struct ComplexStruct local_struct;
    int* member_ptr;
    
    /* Access with computed offset */
    member_ptr = &local_struct.a + (seed & 1);
    *member_ptr = seed + 0x3333;
    
    /* More complex: pointer to pointer */
    int** pptr = &ptr;
    **pptr = seed + 0x4444;
    
    /* Even more complex: array of pointers */
    int* ptr_array[10];
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = &global_array[i * 5 + (seed & 3)];
    }
    
    /* Store through computed pointer array */
    int idx2 = (seed * 7) % 10;
    *ptr_array[idx2] = seed + 0x5555;
    
    /* Global structure access */
    global_struct.a = seed + 0x6666;
    global_struct.b = seed * 0x7777;
    global_struct.d = (short)(seed & 0x8888);
    
    /* Use results */
    use_int(*ptr);
    use_int(global_struct.a);
    sink(local_struct.a);
}

/* ========== Combined test with control flow ========== */
__attribute__((noinline, noipa))
int test_combined(volatile int seed) {
    int result = 0;
    
    /* Vary control flow based on volatile input */
    if (seed & 0x01) {
        test_zero_extract(seed);
        result += 1;
    }
    
    if (seed & 0x02) {
        test_strict_low_part(seed + 1);
        result += 2;
    }
    
    /* Loop with volatile bound */
    for (int i = 0; i < (seed & 0x03) + 1; i++) {
        test_subreg(seed + i);
        result += 4;
    }
    
    if (seed & 0x04) {
        test_complex_mem(seed + 2);
        result += 8;
    }
    
    /* Nested conditionals */
    switch (seed & 0x07) {
        case 0:
            test_zero_extract(seed * 3);
            break;
        case 1:
            test_strict_low_part(seed * 5);
            break;
        case 2:
        case 3:
            test_subreg(seed * 7);
            break;
        default:
            test_complex_mem(seed * 11);
            break;
    }
    
    return result;
}

/* ========== Main function ========== */
int main(int argc, char** argv) {
    /* Initialize volatile variables */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    volatile_index = volatile_seed % 100;
    volatile_short = (short)(volatile_seed & 0xFFFF);
    volatile_int = volatile_seed * 0x1234567;
    volatile_long = volatile_seed * 0x12345678L;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i + volatile_seed;
    }
    
    global_struct.a = volatile_seed;
    global_struct.b = volatile_seed * 2;
    global_struct.c = volatile_seed * 3L;
    global_struct.d = (short)(volatile_seed & 0x7FFF);
    
    /* Run tests */
    int result = 0;
    
    result += test_combined(volatile_seed);
    
    /* Additional direct calls with different seeds */
    result += test_combined(volatile_seed + 1000);
    result += test_combined(volatile_seed + 2000);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}

/* Dummy definitions for external functions */
void use_int(int x) { volatile int dummy = x; (void)dummy; }
void use_long(long x) { volatile long dummy = x; (void)dummy; }
void use_ptr(void* x) { volatile void* dummy = x; (void)dummy; }
void sink(int x) { volatile int dummy = x; (void)dummy; }
