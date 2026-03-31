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
volatile int volatile_seed;
volatile int volatile_idx;
volatile short volatile_short;
volatile int volatile_int;
volatile long volatile_long;

/* Global variables for memory patterns */
int global_array[100];
struct ComplexStruct {
    int a;
    int b;
    long long c;
    short d;
} global_struct;

/* Pattern 1: Generate SET with ZERO_EXTRACT destination */
__attribute__((noinline, noipa))
void test_zero_extract(void) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union BitFieldUnion {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } parts;
    } u;
    
    /* Volatile operations to prevent optimization */
    u.full = volatile_seed;
    
    /* This assignment to bitfield may generate ZERO_EXTRACT */
    u.parts.mid = volatile_int & 0xFF;
    
    /* Complex bitfield manipulation */
    if (volatile_seed & 1) {
        u.parts.low = (volatile_int >> 8) & 0xFF;
    } else {
        u.parts.high = (volatile_int >> 16) & 0xFFFF;
    }
    
    /* Use the result */
    sink(u.full);
    
    /* Another approach: explicit bitfield in struct */
    struct {
        unsigned int field1:10;
        unsigned int field2:10;
        unsigned int field3:12;
    } s;
    
    s.field1 = volatile_int & 0x3FF;
    s.field2 = (volatile_int >> 10) & 0x3FF;
    s.field3 = (volatile_int >> 20) & 0xFFF;
    
    sink(*(int*)&s);
}

/* Pattern 2: Generate SET with STRICT_LOW_PART destination */
__attribute__((noinline, noipa))
void test_strict_low_part(void) {
    int large_val = volatile_seed;
    short small_val = volatile_short;
    
    /* Assignment that modifies only low part */
    /* May generate STRICT_LOW_PART in RTL */
    *(short*)&large_val = small_val;
    
    sink(large_val);
    
    /* Another pattern using type punning */
    long long big = volatile_long;
    int medium = volatile_int;
    
    /* Cast to smaller type pointer */
    *(int*)&big = medium;
    
    sink((int)big);
    
    /* Using arithmetic to preserve high bits */
    int val = volatile_seed;
    short s = volatile_short;
    
    /* This may generate STRICT_LOW_PART */
    val = (val & ~0xFFFF) | (s & 0xFFFF);
    
    sink(val);
}

/* Pattern 3: Generate SET with SUBREG destination */
__attribute__((noinline, noipa))
void test_subreg(void) {
    /* Array access with type punning */
    int array[4] = {0};
    volatile_idx = volatile_idx & 3; /* Ensure in bounds */
    
    /* Access sub-word through pointer cast */
    short *ps = (short*)&array[volatile_idx];
    *ps = volatile_short;
    
    sink(array[volatile_idx]);
    
    /* Long long to int access */
    long long big_array[2];
    int *p_int = (int*)&big_array[1];
    *p_int = volatile_int;
    
    sink((int)big_array[1]);
    
    /* Structure field access */
    struct Mixed {
        char a;
        short b;
        int c;
        long long d;
    } m;
    
    /* Access different sized members */
    m.b = volatile_short;
    m.c = volatile_int;
    
    sink(m.c);
    
    /* Union with different sized members */
    union SizeUnion {
        long long ll;
        int i[2];
        short s[4];
    } u;
    
    u.ll = volatile_long;
    u.s[2] = volatile_short;  /* SUBREG access */
    
    sink(u.i[1]);
}

/* Pattern 4: Generate SET with complex MEM destination */
__attribute__((noinline, noipa))
void test_complex_mem(void) {
    /* Complex addressing modes */
    int *ptr = &global_array[volatile_idx];
    *ptr = volatile_int;
    
    sink(*ptr);
    
    /* Structure with pointer arithmetic */
    struct ComplexStruct *sptr = &global_struct;
    int *field_ptr = &sptr->a + (volatile_idx & 1);
    *field_ptr = volatile_int;
    
    sink(sptr->a);
    
    /* Multi-dimensional array */
    int matrix[10][10];
    int (*row_ptr)[10] = &matrix[volatile_idx % 10];
    (*row_ptr)[volatile_idx % 10] = volatile_int;
    
    sink(matrix[0][0]);
    
    /* Pointer to pointer dereference */
    int val = volatile_int;
    int *p1 = &val;
    int **p2 = &p1;
    **p2 = volatile_seed;
    
    sink(val);
    
    /* Computed goto style address (label as value not used, but similar pattern) */
    int *dynamic_ptr = (int*)((char*)&global_array + volatile_idx * sizeof(int));
    *dynamic_ptr = volatile_int;
    
    sink(global_array[0]);
}

/* Pattern 5: Combined patterns in loops */
__attribute__((noinline, noipa))
void test_combined_patterns(void) {
    int result = 0;
    
    /* Loop with multiple patterns */
    for (int i = 0; i < (volatile_seed & 0xF); i++) {
        /* Mix different patterns */
        if (i & 1) {
            /* ZERO_EXTRACT pattern */
            union {
                unsigned int full;
                struct {
                    unsigned int low:16;
                    unsigned int high:16;
                } half;
            } u;
            u.full = result;
            u.half.low = volatile_int + i;
            result = u.full;
        } else {
            /* STRICT_LOW_PART pattern */
            short s = volatile_short + i;
            result = (result & ~0xFFFF) | (s & 0xFFFF);
        }
        
        /* SUBREG pattern occasionally */
        if ((i % 3) == 0) {
            long long temp = result;
            *(short*)&temp = volatile_short;
            result = (int)temp;
        }
        
        /* Complex MEM pattern */
        global_array[i % 10] = result;
    }
    
    sink(result);
}

/* Main driver */
int main(int argc, char *argv[]) {
    /* Initialize volatile seeds */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    volatile_idx = volatile_seed ^ 0x1234;
    volatile_short = (short)(volatile_seed >> 16);
    volatile_int = volatile_seed ^ 0x5678;
    volatile_long = (long)volatile_seed * 1234567;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    global_struct.a = 1;
    global_struct.b = 2;
    global_struct.c = 3;
    global_struct.d = 4;
    
    /* Execute all pattern tests */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_combined_patterns();
    
    /* Create checksum from results */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum ^= global_array[i];
    }
    checksum ^= global_struct.a;
    checksum ^= global_struct.b;
    checksum ^= (int)global_struct.c;
    checksum ^= global_struct.d;
    
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_long(long x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void sink(int x) { (void)x; }
