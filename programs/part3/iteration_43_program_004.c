/* resource_patterns.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stddef.h>

/* Function A: ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_zero_extract_mem(volatile int *counter) 
{
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct S { 
        volatile unsigned int f1:5; 
        volatile unsigned int f2:7;
        volatile unsigned int f3:10;
    } s;
    
    /* Array with complex addressing for MEM */
    volatile int arr[10][10];
    volatile int *ptr;
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    s.f1 = (*counter) & 0x1F;
    s.f2 = ((*counter) >> 5) & 0x7F;
    s.f3 = ((*counter) >> 12) & 0x3FF;
    
    /* MEM pattern with complex addressing */
    int i = (*counter) % 10;
    int j = ((*counter) * 3) % 10;
    
    /* Multiple MEM accesses with pointer arithmetic */
    ptr = &arr[0][0];
    ptr += i * 10 + j;  /* Complex addressing calculation */
    
    /* Force MEM reference through volatile */
    volatile int v = *ptr;
    
    /* Another MEM with base+index addressing */
    v = arr[i][j];
    
    /* Update counter through MEM */
    *counter = v + 1;
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_strict_low_part_subreg(volatile int *counter) 
{
    /* Use char/short types for STRICT_LOW_PART */
    volatile char c = (*counter) & 0xFF;
    volatile short s = (*counter) & 0xFFFF;
    
    /* STRICT_LOW_PART pattern: inline assembly modifying low part */
    /* Byte operation on char */
    asm volatile (
        "addb $1, %0"
        : "=q"(c)    /* =q constraint for byte-addressable register */
        : "0"(c)     /* Matching input constraint */
        : "cc"
    );
    
    /* Word operation on short */
    asm volatile (
        "addw $2, %0"
        : "=r"(s)    /* Word register */
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG pattern: type punning between different sizes */
    int i = *counter;
    
    /* Access int as short (SUBREG from SImode to HImode) */
    short *ps = (short*)&i;
    *ps = (short)(i & 0xFFFF);  /* Write to low part */
    
    /* Access int as char (SUBREG from SImode to QImode) */
    char *pc = (char*)&i;
    pc[1] = (char)((i >> 8) & 0xFF);  /* Write to second byte */
    
    /* Mixed-size operations to force SUBREG usage */
    long long ll = (long long)i * 100;
    int *pi = (int*)&ll;
    i = pi[0] + pi[1];  /* Access 64-bit as two 32-bit parts */
    
    *counter = i + c + s;
}

/* Function C: Mixed patterns with ternary operator */
static void __attribute__((noinline))
pattern_mixed_complex(volatile int *counter) 
{
    /* Struct with bit-fields at different positions */
    struct T {
        volatile unsigned int a:3;
        volatile unsigned int b:9;
        volatile unsigned int c:20;
    } t1, t2;
    
    /* Array for MEM patterns */
    static volatile int buffer[100];
    
    /* Initialize */
    t1.a = (*counter) & 0x7;
    t1.b = ((*counter) >> 3) & 0x1FF;
    t1.c = ((*counter) >> 12) & 0xFFFFF;
    
    /* Complex expression with ternary selecting address */
    volatile int *addr;
    int idx = *counter;
    
    /* Ternary that could generate interesting RTL */
    addr = (idx & 1) ? 
           (volatile int*)&t1.a :  /* Bit-field address */
           (volatile int*)&buffer[idx % 100];  /* Array element address */
    
    /* Force evaluation through volatile pointer */
    volatile int temp = *addr;
    
    /* Assignment that could involve multiple transformations */
    t2.a = (temp >> 0) & 0x7;
    t2.b = (temp >> 3) & 0x1FF;
    t2.c = (temp >> 12) & 0xFFFFF;
    
    /* Update array through complex indexing (MEM pattern) */
    int i = (idx * 7) % 10;
    int j = (idx * 13) % 10;
    buffer[(i * 10 + j) % 100] = t2.c;
    
    *counter = idx + 1;
}

/* Helper with pointer arithmetic for MEM patterns */
static void __attribute__((noinline))
complex_mem_access(volatile int *arr, int size, volatile int *counter)
{
    /* Multiple indices for complex addressing */
    int idx1 = (*counter) % size;
    int idx2 = (*counter * 3) % size;
    int idx3 = (*counter * 7) % size;
    
    /* Chain of MEM accesses with pointer arithmetic */
    volatile int *p1 = arr + idx1;
    volatile int *p2 = p1 + idx2;
    volatile int *p3 = p2 + idx3;
    
    /* Force MEM references */
    volatile int v1 = *p1;
    volatile int v2 = *p2;
    volatile int v3 = *p3;
    
    /* Update through pointer */
    *p3 = v1 + v2 + v3;
    
    *counter = (*counter) + v3;
}

int main(int argc, char **argv) 
{
    volatile int counter = 0;
    volatile int sum = 0;
    
    /* Use argc to bound loops for analysis */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Initialize array for MEM patterns */
    volatile int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i * 3;
    }
    
    /* Main loop to generate patterns repeatedly */
    for (volatile int iter = 0; iter < iterations; iter++) {
        /* Call each pattern function */
        pattern_zero_extract_mem(&counter);
        
        pattern_strict_low_part_subreg(&counter);
        
        pattern_mixed_complex(&counter);
        
        /* Additional complex MEM access */
        complex_mem_access(data, 100, &counter);
        
        /* Accumulate to prevent dead code elimination */
        sum += counter;
        
        /* Force side effects */
        if (counter > 1000) {
            counter = 0;  /* Reset to prevent overflow in analysis */
        }
    }
    
    /* Final dummy use of results */
    volatile int result = sum;
    
    /* Prevent optimization of entire program */
    asm volatile ("" : : "r"(result));
    
    return 0;
}
