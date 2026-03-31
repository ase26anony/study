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
struct S { int a; int b; int c; } global_struct;

/* Pattern 1: ZERO_EXTRACT destination through bitfield operations */
__attribute__((noinline, noipa))
void test_zero_extract(int seed) {
    /* Use union with bitfields for explicit ZERO_EXTRACT pattern */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = seed;
    /* This assignment to bitfield may generate ZERO_EXTRACT destination */
    u.bits.mid = volatile_int & 0xFF;
    
    /* Complex control flow to keep it alive */
    for (int i = 0; i < (volatile_seed & 3); i++) {
        u.bits.high ^= (volatile_int >> (i * 4)) & 0xF;
    }
    
    use_int(u.full);
    
    /* Alternative: bitwise operations that mask specific bits */
    unsigned int val = seed;
    unsigned int mask = 0xFF00;
    unsigned int insert = volatile_int & 0xFF;
    /* Pattern: (val & ~mask) | (insert << 8) */
    val = (val & ~mask) | ((insert << 8) & mask);
    
    sink(val);
}

/* Pattern 2: STRICT_LOW_PART destination through partial word assignment */
__attribute__((noinline, noipa))
void test_strict_low_part(int seed) {
    int large = seed;
    short small = volatile_short;
    
    /* Assignment that only affects low 16 bits */
    /* May generate STRICT_LOW_PART destination */
    *(short*)&large = small;
    
    /* Alternative with arithmetic */
    int another = seed;
    another = (another & ~0xFFFF) | (small & 0xFFFF);
    
    /* In a loop to create more RTL contexts */
    for (int i = 0; i < (volatile_index & 1); i++) {
        another ^= (volatile_int << 16);
    }
    
    use_int(another);
    
    /* Using different sized types */
    long long big = seed;
    int *p = (int*)&big;
    *p = volatile_int;  /* Only affects part of 'big' */
    
    use_long(big);
}

/* Pattern 3: SUBREG destination through type punning and sub-word access */
__attribute__((noinline, noipa))
void test_subreg(int seed) {
    /* Array access with type conversion */
    int array[4] = {seed, seed + 1, seed + 2, seed + 3};
    volatile int idx = volatile_index & 3;
    
    /* Access as different type - may generate SUBREG */
    short *ps = (short*)&array[idx];
    *ps = volatile_short;
    
    /* Another SUBREG pattern: accessing part of larger type */
    long long big_val = volatile_long;
    int *int_ptr = (int*)&big_val;
    if (volatile_seed & 1) {
        int_ptr[0] = volatile_int;  /* First 32 bits */
    } else {
        int_ptr[1] = volatile_int;  /* Second 32 bits */
    }
    
    use_long(big_val);
    
    /* Structure field access with pointer arithmetic */
    struct Point { int x; int y; int z; } point;
    point.x = seed;
    point.y = seed * 2;
    
    char *byte_ptr = (char*)&point;
    byte_ptr[volatile_index & 7] = volatile_int & 0xFF;
    
    use_int(point.x + point.y);
}

/* Pattern 4: Complex MEM destination with non-trivial addressing */
__attribute__((noinline, noipa))
void test_complex_mem(int seed) {
    /* Complex addressing mode: global + index + offset */
    int *ptr = &global_array[volatile_index % 50];
    *ptr = volatile_int;
    
    /* More complex: pointer arithmetic with multiple operations */
    struct S local_struct;
    int *field_ptr = &local_struct.a + (volatile_seed & 2);
    *field_ptr = seed;
    
    /* Even more complex: computed address with scaling */
    int *complex_ptr = (int*)((char*)global_array + 
                             (volatile_index * sizeof(int)) % 200);
    *complex_ptr = volatile_int ^ seed;
    
    /* Structure with pointer chasing */
    struct Node {
        int value;
        struct Node *next;
    } node1, node2;
    
    node1.value = seed;
    node2.value = seed * 2;
    node1.next = &node2;
    
    struct Node *current = &node1;
    if (volatile_seed & 1) {
        current = current->next;
    }
    current->value = volatile_int;
    
    use_int(node1.value + node2.value);
    
    /* Array with volatile index in loop */
    int local_array[10];
    for (int i = 0; i < (volatile_seed & 7); i++) {
        int idx = (volatile_index + i) % 10;
        local_array[idx] = volatile_int + i;
    }
    
    /* Compute checksum to keep values alive */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += local_array[i];
    }
    sink(sum);
}

/* Pattern 5: Mixed patterns in complex control flow */
__attribute__((noinline, noipa))
void test_mixed_patterns(int seed) {
    int result = seed;
    
    /* Switch with different patterns in each case */
    switch (volatile_seed & 3) {
        case 0: {
            /* ZERO_EXTRACT-like */
            union {
                unsigned int val;
                struct {
                    unsigned int a:10;
                    unsigned int b:10;
                    unsigned int c:12;
                } fields;
            } u;
            u.val = result;
            u.fields.b = volatile_int & 0x3FF;
            result = u.val;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART-like */
            long long big = result;
            int *p = (int*)&big;
            p[volatile_index & 1] = volatile_int;
            result = (int)big;
            break;
        }
        case 2: {
            /* SUBREG-like */
            int arr[2] = {result, result * 2};
            short *sp = (short*)arr;
            sp[volatile_index & 3] = volatile_short;
            result = arr[0] + arr[1];
            break;
        }
        case 3: {
            /* Complex MEM */
            struct { int x; int y; } point;
            int *ptr = (volatile_seed & 1) ? &point.x : &point.y;
            *ptr = volatile_int + result;
            result = point.x + point.y;
            break;
        }
    }
    
    /* Loop with varying patterns */
    for (int i = 0; i < (volatile_seed & 3); i++) {
        if (i & 1) {
            /* Partial store */
            char *cptr = (char*)&result;
            cptr[i % 4] = volatile_int >> (i * 8);
        } else {
            /* Bitfield operation */
            result ^= (1 << (volatile_int & 31));
        }
    }
    
    sink(result);
}

/* Main function that drives all tests */
int main(int argc, char *argv[]) {
    /* Initialize volatile variables */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    volatile_index = volatile_seed ^ 0x12345678;
    volatile_short = volatile_seed & 0xFFFF;
    volatile_int = volatile_seed;
    volatile_long = (long)volatile_seed * 1000;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i + volatile_seed;
    }
    global_struct.a = volatile_seed;
    global_struct.b = volatile_seed * 2;
    global_struct.c = volatile_seed * 3;
    
    int seed = volatile_seed;
    
    /* Call all test functions */
    test_zero_extract(seed);
    test_strict_low_part(seed);
    test_subreg(seed);
    test_complex_mem(seed);
    test_mixed_patterns(seed);
    
    /* Compute final checksum from globals */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= global_array[i];
    }
    checksum ^= global_struct.a;
    checksum ^= global_struct.b;
    checksum ^= global_struct.c;
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy external references */
void use_int(int x) { volatile_seed = x; }
void use_long(long x) { volatile_long = x; }
void use_ptr(void* x) { (void)x; }
void sink(int x) { volatile_int = x; }
