/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -c test_resource_coverage.c
 */

#include <stddef.h>

/* Function A: ZERO_EXTRACT and MEM patterns */
static __attribute__((noinline)) 
void func_a(volatile int *arr, int idx1, int idx2) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct S { 
        volatile unsigned int f1:5; 
        volatile unsigned int f2:3;
        volatile unsigned int pad:24;
    } s;
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    s.f1 = idx1 & 0x1F;
    s.f2 = idx2 & 0x07;
    
    /* MEM pattern with complex addressing */
    volatile int *ptr = arr + idx1 * 8 + idx2;
    volatile int val = *ptr;
    
    /* Combine: use bit-field value in memory access */
    *(arr + s.f1) = val + s.f2;
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static __attribute__((noinline))
void func_b(volatile int *base) {
    /* Use char/short types for STRICT_LOW_PART */
    volatile char c = 42;
    volatile short s = 1000;
    volatile int i = 0x12345678;
    
    /* STRICT_LOW_PART pattern: inline assembly modifying low byte */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c) 
        : "0"(c) 
        : "cc"
    );
    
    /* Another STRICT_LOW_PART with short */
    asm volatile (
        "addw $100, %0\n\t"
        : "=r"(s)
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different sizes */
    short *ps = (short*)&i;
    *ps = s;  /* This generates SUBREG in RTL */
    
    /* More SUBREG: access int as char array */
    char *pc = (char*)&i;
    pc[1] = c;
    
    /* Store result to memory */
    *base = i + s + c;
}

/* Function C: Mixed patterns with ternary operator */
static __attribute__((noinline))
void func_c(volatile int *arr, int idx, int cond) {
    /* Struct with volatile bit-fields */
    struct T {
        volatile unsigned int flag:1;
        volatile unsigned int value:7;
        volatile unsigned int data:24;
    } t;
    
    /* Initialize */
    t.flag = cond & 1;
    t.value = idx & 0x7F;
    t.data = idx * 100;
    
    /* Complex expression with ternary selecting address */
    volatile int *addr = cond ? 
        (arr + t.value) :           /* MEM with bit-field offset */
        (arr + (t.data >> 3));      /* MEM with shifted bit-field */
    
    /* Assignment that may involve multiple RTL transformations */
    volatile int temp = *addr;
    
    /* Use bit-field in calculation */
    temp += (t.flag ? t.value : 0);
    
    /* Write back with different offset */
    *(addr + (t.value & 3)) = temp;
}

/* Helper with array indexing for MEM patterns */
static __attribute__((noinline))
void func_mem_complex(volatile int arr[10][10], int i, int j) {
    /* Multi-dimensional array access - complex MEM addressing */
    volatile int v1 = arr[i][j];
    volatile int v2 = arr[j][i];
    
    /* Pointer arithmetic with multiple indices */
    volatile int *p = &arr[0][0];
    p += i * 10 + j;
    volatile int v3 = *p;
    
    /* Chain of memory accesses */
    volatile int v4 = *(p + 5);
    volatile int v5 = *(p - 3);
    
    /* Use all values to prevent elimination */
    arr[i][j] = v1 + v2 + v3 + v4 + v5;
}

/* Main function to drive all patterns */
int main(int argc, char **argv) {
    volatile int iteration_counter = 0;
    volatile int max_iterations = (argc > 1) ? 10 : 5;  /* Bound loops */
    volatile int result_sum = 0;
    
    /* Arrays for MEM patterns */
    volatile int array1[100];
    volatile int array2[10][10];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        array1[i] = i;
    }
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            array2[i][j] = i * 10 + j;
        }
    }
    
    /* Main loop to trigger resource tracking */
    while (iteration_counter < max_iterations) {
        int idx1 = iteration_counter;
        int idx2 = iteration_counter * 2;
        int cond = iteration_counter % 2;
        
        /* Call pattern functions */
        func_a(array1, idx1, idx2);
        func_b(&array1[idx1 % 50]);
        func_c(array1, idx1, cond);
        func_mem_complex(array2, idx1 % 10, idx2 % 10);
        
        /* Complex addressing mode in main loop */
        volatile int *complex_ptr = array1 + 
                                   (idx1 * 3) + 
                                   (idx2 >> 1) + 
                                   (cond ? 5 : 10);
        volatile int complex_val = *complex_ptr;
        
        /* Update result to prevent dead code elimination */
        result_sum += complex_val + idx1 + idx2;
        
        iteration_counter++;
    }
    
    /* Additional SUBREG pattern with 64-bit on 32-bit target */
    if (sizeof(long long) > sizeof(int)) {
        volatile long long ll = 0x123456789ABCDEF0LL;
        volatile int *p32 = (volatile int*)&ll;
        volatile short *p16 = (volatile short*)&ll;
        volatile char *p8 = (volatile char*)&ll;
        
        /* Mixed-size accesses generating SUBREG */
        *p32 = result_sum;
        p16[2] = (short)result_sum;
        p8[6] = (char)result_sum;
        
        /* Use the value */
        result_sum += (int)(ll >> 32);
    }
    
    /* Final volatile store to ensure all operations complete */
    volatile int final_result = result_sum;
    
    /* The program doesn't need to run correctly, just compile */
    /* return final_result; */  /* Commented to avoid runtime issues */
    return 0;
}

/* Additional function to force more ZERO_EXTRACT patterns */
static __attribute__((noinline))
void extra_bitfield_patterns(void) {
    /* Multiple volatile bit-fields in union */
    union U {
        struct {
            volatile unsigned int a:4;
            volatile unsigned int b:4;
            volatile unsigned int c:4;
            volatile unsigned int d:4;
            volatile unsigned int e:16;
        } bits;
        volatile unsigned int full;
    } u;
    
    /* Series of bit-field assignments */
    u.bits.a = 1;
    u.bits.b = 2;
    u.bits.c = 3;
    u.bits.d = 4;
    u.bits.e = 0xABCD;
    
    /* Use in calculation */
    u.full = u.full + (u.bits.a << 16);
    
    /* More complex: bit-field in condition */
    if (u.bits.b > 1) {
        u.bits.c = u.bits.d;
    }
    
    /* Bit-field as array index (will be converted) */
    volatile int arr[16];
    arr[u.bits.a] = u.bits.b;
}
