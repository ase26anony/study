/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic (mark_referenced_resources in resource.cc).
 * The goal is to cover lines 282-290 that handle ZERO_EXTRACT,
 * STRICT_LOW_PART, SUBREG, and MEM expressions.
 *
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -fdump-rtl-all -c test_resource_coverage.c
 * Or for more aggressive optimization: gcc -O3 -m32 -funroll-loops -fno-strict-aliasing -c test_resource_coverage.c
 */

#include <stddef.h>

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_a(volatile int *base, int idx1, int idx2) 
{
    /* Struct with volatile bit-field to generate ZERO_EXTRACT */
    struct S { 
        volatile unsigned int f1:5; 
        volatile unsigned int f2:3;
        volatile unsigned int f3:8;
    } s;
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    s.f1 = 1;
    s.f2 = idx1 & 0x7;
    s.f3 = (idx1 + idx2) & 0xFF;
    
    /* MEM pattern with complex addressing */
    volatile int *ptr = base + idx1 * 8 + idx2;
    volatile int val = *ptr;
    
    /* Combine both: MEM addressing with bit-field result */
    s.f1 = (*ptr) & 0x1F;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(s.f1), "r"(val) : "memory");
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_b(volatile int *data) 
{
    /* Use char/short types to encourage QI/HI modes */
    volatile char c = 0;
    volatile short s = 0;
    volatile int i = 0;
    
    /* STRICT_LOW_PART pattern: inline assembly modifying only part of register */
    /* Byte operation on char */
    asm volatile("addb $1, %0" : "=q"(c) : "0"(c) : "cc");
    
    /* Word operation on short */
    asm volatile("addw $1, %0" : "=r"(s) : "0"(s) : "cc");
    
    /* SUBREG pattern: type punning with mixed-size accesses */
    int temp = *data;
    
    /* Access int as short (SUBREG from SImode to HImode) */
    short *ps = (short*)&temp;
    *ps = (*ps) + 1;
    
    /* Access int as char (SUBREG from SImode to QImode) */
    char *pc = (char*)&temp;
    pc[1] = pc[0] + 2;
    
    /* Another SUBREG pattern: store short in int array */
    ((short*)data)[1] = s;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(c), "r"(s), "r"(temp) : "memory");
}

/* Function C: Complex expression mixing multiple patterns */
static void __attribute__((noinline))
pattern_c(volatile int *arr, int i, int j) 
{
    /* Struct with volatile bit-fields at different positions */
    struct BitFields {
        volatile unsigned int a:3;
        volatile unsigned int b:5;
        volatile unsigned int c:12;
        volatile unsigned int d:8;
    } bf;
    
    /* Complex addressing with multiple indices (MEM pattern) */
    volatile int *ptr1 = arr + i * 16 + j;
    volatile int *ptr2 = arr + j * 8 + i;
    
    /* Ternary operator selecting address, then bit-field assignment */
    volatile int *selected = (i > j) ? ptr1 : ptr2;
    
    /* ZERO_EXTRACT from memory load */
    bf.a = (*selected) & 0x7;
    bf.b = (*selected >> 3) & 0x1F;
    
    /* More complex: bit-field from pointer arithmetic result */
    bf.c = (*(selected + 1)) & 0xFFF;
    
    /* SUBREG: access part of larger type */
    long long big = 0x123456789ABCDEF0LL;
    int *pint = (int*)&big;
    bf.d = pint[1] & 0xFF;  /* Access high part of long long */
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bf.a), "r"(bf.b), "r"(bf.c), "r"(bf.d) : "memory");
}

/* Helper to create more complex MEM addressing patterns */
static void __attribute__((noinline))
mem_intensive(volatile int *arr, int size) 
{
    /* Multi-dimensional array access in loop - complex MEM patterns */
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - 1; j++) {
            /* Complex addressing with multiple indices */
            volatile int val = arr[i * size + j] 
                             + arr[(i + 1) * size + j] 
                             + arr[i * size + (j + 1)];
            
            /* Store with offset */
            arr[i * size + j] = val / 3;
        }
    }
}

int main(int argc, char **argv) 
{
    /* Use argc to bound loops, preventing infinite loops in analysis */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Volatile counters to prevent optimization */
    volatile int counter = 0;
    volatile int sum = 0;
    
    /* Arrays with volatile elements to force MEM patterns */
    volatile int array[100];
    volatile int matrix[10][10];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Main loop calling pattern functions */
    for (volatile int iter = 0; iter < iterations; iter++) {
        int idx1 = counter % 10;
        int idx2 = (counter * 3) % 10;
        
        /* Call pattern functions with volatile-derived arguments */
        pattern_a((int*)array, idx1, idx2);
        pattern_b((int*)&array[counter % 50]);
        pattern_c((int*)array, idx1, idx2);
        
        /* MEM-intensive operations */
        mem_intensive((int*)array, 10);
        
        /* Mix in matrix access for more complex MEM patterns */
        volatile int val = matrix[idx1][idx2];
        sum += val;
        
        counter++;
    }
    
    /* Final dummy operation using results */
    volatile int result = sum + counter;
    
    /* The program doesn't need correct runtime semantics,
     * but we return something to make compilation happy */
    return (result > 0) ? 0 : 1;
}
