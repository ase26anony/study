/* test_resource_marking.c
 * Designed to generate RTL SET destinations with:
 * - ZERO_EXTRACT
 * - STRICT_LOW_PART  
 * - SUBREG
 * - Complex MEM patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Global variables for memory patterns */
int global_array[256];
struct compound {
    int a;
    int b;
    long long c;
    int d;
} global_struct;

/* Pattern 1: ZERO_EXTRACT destination via bitfields */
NOINLINE void test_zero_extract(int seed) {
    volatile int v = seed;
    
    /* Union with bitfields - likely to generate ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = v * 0x12345678;
    
    /* Store into bitfield - destination may be ZERO_EXTRACT */
    u.bits.mid = (v & 0xFF) ^ 0x55;
    u.bits.high = (v >> 8) & 0xFFFF;
    
    /* Force usage */
    use_int(u.full);
    
    /* Another approach: explicit bitfield in struct */
    struct {
        unsigned int field1:10;
        unsigned int field2:10;
        unsigned int field3:12;
    } s;
    
    s.field1 = v & 0x3FF;
    s.field2 = (v >> 10) & 0x3FF;
    s.field3 = (v >> 20) & 0xFFF;
    
    /* Cast to int to use */
    int result = *(int*)&s;
    use_int(result);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(int seed) {
    volatile short vs = seed & 0xFFFF;
    volatile int vi = seed;
    
    /* Assignment to low part of larger integer */
    int val = vi * 0x87654321;
    
    /* These may generate STRICT_LOW_PART */
    val = (val & ~0xFFFF) | (vs & 0xFFFF);
    
    /* Pointer cast to short */
    int another = vi ^ 0x12345678;
    *(short*)&another = vs;
    
    /* In loop to increase chances */
    for (int i = 0; i < (vs & 0x3); i++) {
        int temp = vi + i;
        *(short*)&temp = (vs + i) & 0xFFFF;
        use_int(temp);
    }
    
    use_int(val);
    use_int(another);
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(int seed) {
    volatile int v = seed;
    
    /* Type punning between different sizes */
    long long big = (long long)v * 0x1122334455667788LL;
    
    /* Access parts - may generate SUBREG */
    int* p1 = (int*)&big;
    *p1 = v ^ 0xABCDEF01;
    
    short* p2 = (short*)&big;
    p2[1] = (v >> 16) & 0xFFFF;
    p2[3] = (v >> 8) & 0xFFFF;
    
    /* Array with sub-word access */
    int arr[4] = {v, v+1, v+2, v+3};
    short* ps = (short*)arr;
    
    for (int i = 0; i < (v & 0x7); i++) {
        ps[i] = (v + i * 0x1111) & 0xFFFF;
    }
    
    /* Union for sub-register access */
    union {
        long long ll;
        struct {
            int low;
            int high;
        } parts;
    } u;
    
    u.ll = big;
    u.parts.high = v * 0x33333333;
    
    use_long(big);
    use_int(arr[0]);
    use_long(u.ll);
}

/* Pattern 4: Complex MEM destinations */
NOINLINE void test_complex_mem(int seed) {
    volatile int idx = seed & 0xFF;
    volatile int offset = (seed >> 8) & 0x3F;
    
    /* Complex addressing modes */
    int* ptr1 = &global_array[idx + offset];
    *ptr1 = seed ^ 0x12345678;
    
    /* Pointer arithmetic */
    int* ptr2 = global_array + idx * 2 + offset;
    *ptr2 = *ptr2 + seed;
    
    /* Structure member with offset */
    struct compound* sptr = &global_struct;
    int* member_ptr = &sptr->d + offset;
    *member_ptr = seed;
    
    /* Multi-dimensional calculation */
    int matrix[8][8];
    for (int i = 0; i < (idx & 0x7); i++) {
        for (int j = 0; j < (offset & 0x7); j++) {
            matrix[i][j] = seed + i * 8 + j;
        }
    }
    
    /* Indirect through pointer array */
    int* ptr_array[4];
    ptr_array[0] = &global_array[0];
    ptr_array[1] = &global_array[64];
    ptr_array[2] = &global_array[128];
    ptr_array[3] = &global_array[192];
    
    int* final_ptr = ptr_array[idx & 0x3] + offset;
    *final_ptr = *final_ptr * 2 + 1;
    
    /* Force usage */
    use_int(global_array[0]);
    use_int(sptr->a);
    use_int(matrix[0][0]);
}

/* Pattern 5: Combined patterns in control flow */
NOINLINE void test_combined(int seed) {
    volatile int v = seed;
    volatile int cond = v & 1;
    
    /* Mixed patterns in branches */
    if (cond) {
        /* ZERO_EXTRACT pattern */
        union {
            unsigned int val;
            struct {
                unsigned int a:5;
                unsigned int b:11;
                unsigned int c:16;
            } fields;
        } u;
        
        u.val = v;
        u.fields.b = (v >> 5) & 0x7FF;
        use_int(u.val);
    } else {
        /* STRICT_LOW_PART pattern */
        int x = v * 3;
        *(short*)&x = (v >> 4) & 0xFFFF;
        use_int(x);
    }
    
    /* Loop with SUBREG pattern */
    long long accumulator = 0;
    for (int i = 0; i < (v & 0xF); i++) {
        long long temp = v + i;
        *(int*)&temp = i * 0x11111111;
        accumulator += temp;
    }
    
    /* Complex MEM in switch */
    switch (v & 0x3) {
        case 0:
            global_array[(v >> 2) & 0xFF] = v;
            break;
        case 1:
            *(int*)((char*)&global_struct + (v & 0xF)) = v;
            break;
        case 2:
            {
                int* p = &global_array[0] + (v & 0x7F);
                *p = *p ^ v;
            }
            break;
        case 3:
            {
                short* sp = (short*)&global_array[v & 0x7F];
                *sp = v & 0xFFFF;
            }
            break;
    }
    
    use_long(accumulator);
}

/* Main driver */
int main(int argc, char** argv) {
    /* Volatile seed from command line or timer */
    int seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) ^ (getpid() << 16);
    }
    
    volatile int vseed = seed;
    
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 0x01010101;
    }
    
    global_struct.a = vseed;
    global_struct.b = vseed * 2;
    global_struct.c = (long long)vseed * 0x100000001LL;
    global_struct.d = vseed ^ 0xF0F0F0F0;
    
    /* Call pattern generators */
    test_zero_extract(vseed);
    test_strict_low_part(vseed + 1);
    test_subreg(vseed + 2);
    test_complex_mem(vseed + 3);
    test_combined(vseed + 4);
    
    /* Create checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 256; i += 16) {
        checksum ^= global_array[i];
    }
    
    checksum ^= global_struct.a;
    checksum ^= global_struct.b;
    checksum ^= (int)global_struct.c;
    checksum ^= (int)(global_struct.c >> 32);
    checksum ^= global_struct.d;
    
    printf("Result: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) {
    volatile static int sink;
    sink = x;
}

void use_long(long x) {
    volatile static long sink;
    sink = x;
}

void use_ptr(void* x) {
    volatile static void* sink;
    sink = x;
}
