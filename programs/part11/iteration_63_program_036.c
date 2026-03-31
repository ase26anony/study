/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
struct S {
    int a;
    int b;
    int c;
} global_struct;

/* ===== Pattern 1: ZERO_EXTRACT destination ===== */
__attribute__((noinline, noipa))
void test_zero_extract(void) {
    /* Pattern 1a: Using bitfields in unions */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = volatile_seed;
    /* This assignment to a bitfield may generate ZERO_EXTRACT */
    u.bits.mid = volatile_int & 0xFF;
    use_int(u.full);
    
    /* Pattern 1b: Explicit bitfield extraction with masking */
    unsigned int val = volatile_seed;
    unsigned int mask = 0xFF00;
    unsigned int insert = volatile_int & 0xFF;
    
    /* This pattern: store into masked portion of val */
    val = (val & ~mask) | ((insert << 8) & mask);
    use_int(val);
    
    /* Pattern 1c: Nested bitfields in struct */
    struct {
        unsigned int field1:4;
        unsigned int field2:12;
        unsigned int field3:16;
    } bit_struct;
    
    bit_struct.field1 = volatile_int & 0xF;
    bit_struct.field2 = (volatile_int >> 4) & 0xFFF;
    bit_struct.field3 = volatile_int & 0xFFFF;
    use_int(*(int*)&bit_struct);
}

/* ===== Pattern 2: STRICT_LOW_PART destination ===== */
__attribute__((noinline, noipa))
void test_strict_low_part(void) {
    /* Pattern 2a: Assigning short to int with masking */
    int i = volatile_seed;
    short s = volatile_short;
    
    /* This may generate STRICT_LOW_PART */
    i = (i & ~0xFFFF) | (s & 0xFFFF);
    use_int(i);
    
    /* Pattern 2b: Pointer casting to access low part */
    long long big = volatile_long;
    int *p = (int*)&big;
    *p = volatile_int;  /* Store into low part of big */
    use_long(big);
    
    /* Pattern 2c: Using char array to modify low bytes */
    int val = volatile_seed;
    unsigned char *bytes = (unsigned char*)&val;
    bytes[0] = volatile_int & 0xFF;
    bytes[1] = (volatile_int >> 8) & 0xFF;
    use_int(val);
    
    /* Pattern 2d: Explicit low-part preservation */
    unsigned int x = volatile_seed;
    unsigned int low_part = volatile_int & 0xFFFF;
    x = (x & 0xFFFF0000) | low_part;
    use_int(x);
}

/* ===== Pattern 3: SUBREG destination ===== */
__attribute__((noinline, noipa))
void test_subreg(void) {
    /* Pattern 3a: Type punning with different sizes */
    long long big_var = volatile_long;
    int *int_ptr = (int*)&big_var;
    
    /* Access different parts of the long long */
    int_ptr[0] = volatile_int;          /* Low 32 bits */
    int_ptr[1] = volatile_int ^ 0x1234; /* High 32 bits */
    use_long(big_var);
    
    /* Pattern 3b: Array access with type conversion */
    int array[4] = {0};
    volatile int idx = volatile_index & 3;
    short *short_ptr = (short*)&array[idx];
    
    /* Store short into int array - may generate SUBREG */
    *short_ptr = volatile_short;
    use_int(array[idx]);
    
    /* Pattern 3c: Structure field access with casting */
    struct Mixed {
        char c;
        short s;
        int i;
        long long ll;
    } mixed;
    
    mixed.ll = volatile_long;
    /* Access part of the long long through different type */
    int *from_ll = (int*)&mixed.ll;
    from_ll[1] = volatile_int;  /* Modify high part */
    use_long(mixed.ll);
    
    /* Pattern 3d: Union with different sized members */
    union U {
        long long ll;
        struct {
            int low;
            int high;
        } parts;
    } u;
    
    u.ll = volatile_long;
    u.parts.high = volatile_int ^ 0xABCD;
    use_long(u.ll);
}

/* ===== Pattern 4: Complex MEM destinations ===== */
__attribute__((noinline, noipa))
void test_complex_mem(void) {
    /* Pattern 4a: Computed pointer with offset */
    struct S local_struct;
    volatile int off = volatile_index & 1;
    int *ptr = &local_struct.a + off;
    *ptr = volatile_int;  /* Complex addressing mode */
    use_int(local_struct.a + local_struct.b);
    
    /* Pattern 4b: Global array with variable index */
    int idx = volatile_index % 100;
    global_array[idx] = volatile_int;
    global_array[idx + 1] = volatile_int ^ 0x55;
    use_int(global_array[idx]);
    
    /* Pattern 4c: Pointer arithmetic in loop */
    int *arr = global_array;
    for (int i = 0; i < (volatile_index & 3); i++) {
        *(arr + i + (volatile_seed & 1)) = volatile_int + i;
    }
    use_int(arr[0]);
    
    /* Pattern 4d: Nested structure access */
    struct Outer {
        struct Inner {
            int x;
            int y;
        } inner[2];
        int z;
    } outer;
    
    int n = volatile_index & 1;
    outer.inner[n].x = volatile_int;
    outer.inner[n].y = volatile_int * 2;
    use_int(outer.inner[0].x + outer.inner[1].y);
    
    /* Pattern 4e: Multi-dimensional array */
    int matrix[3][3];
    int row = volatile_index % 3;
    int col = (volatile_index / 3) % 3;
    matrix[row][col] = volatile_int;
    use_int(matrix[0][0] + matrix[2][2]);
}

/* ===== Combined patterns in control flow ===== */
__attribute__((noinline, noipa))
void test_combined_patterns(void) {
    int result = 0;
    
    /* Use volatile conditions to create control flow */
    if (volatile_seed & 1) {
        /* Mix patterns in conditional branches */
        union {
            uint32_t full;
            struct {
                uint16_t low;
                uint16_t high;
            } parts;
        } u;
        u.full = volatile_int;
        u.parts.low = volatile_short;  /* Could be STRICT_LOW_PART or ZERO_EXTRACT */
        result += u.full;
    } else {
        /* Different pattern in else branch */
        long long big = volatile_long;
        int *p = (int*)&big;
        p[volatile_index & 1] = volatile_int;  /* SUBREG + complex MEM */
        result += (int)big;
    }
    
    /* Loop with pattern variations */
    for (int i = 0; i < (volatile_seed & 3); i++) {
        int temp = volatile_int + i;
        
        /* Alternate between patterns based on loop index */
        if (i & 1) {
            /* Bitfield pattern */
            temp = (temp & ~0xFF) | (volatile_short & 0xFF);
        } else {
            /* Pointer pattern */
            short *sp = (short*)&temp;
            sp[0] = volatile_short;
        }
        result += temp;
    }
    
    use_int(result);
}

/* ===== Main test driver ===== */
int main(int argc, char **argv) {
    /* Initialize volatile variables */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : 12345;
    volatile_index = volatile_seed ^ 0x5555;
    volatile_short = volatile_seed & 0xFFFF;
    volatile_int = volatile_seed * 3;
    volatile_long = (long)volatile_seed * 1000;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    global_struct.a = 1;
    global_struct.b = 2;
    global_struct.c = 3;
    
    /* Execute all pattern tests */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_combined_patterns();
    
    /* Create a checksum from all operations */
    int checksum = 0;
    checksum += global_array[volatile_index % 100];
    checksum += global_struct.a + global_struct.b + global_struct.c;
    checksum += volatile_int;
    checksum += (int)volatile_long;
    
    printf("Result: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions to satisfy external references */
void use_int(int x) { sink(x); }
void use_long(long x) { sink((int)x); }
void use_ptr(void *p) { sink((int)(intptr_t)p); }
void sink(int x) { volatile int dummy = x; (void)dummy; }
