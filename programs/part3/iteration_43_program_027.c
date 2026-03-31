/* gcc -O2 -m32 -fno-strict-aliasing -funroll-loops -fdump-rtl-all -c this_file.c */

#include <stddef.h>

/* Function A: Focus on ZERO_EXTRACT and MEM */
static __attribute__((noinline)) 
void pattern_a(volatile int *counter) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct S {
        volatile unsigned int f1:5;
        volatile unsigned int f2:7;
        volatile unsigned int f3:12;
    } s;
    
    /* Array with complex addressing for MEM */
    int arr[10][10];
    volatile int idx1 = *counter % 10;
    volatile int idx2 = (*counter + 1) % 10;
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    s.f1 = idx1 & 0x1F;
    s.f2 = idx2 & 0x7F;
    
    /* MEM pattern with complex addressing */
    volatile int v = arr[idx1][idx2];
    
    /* Combine: use bit-field value in memory access */
    arr[s.f1][s.f2] = v + 1;
    
    /* Prevent dead code elimination */
    *counter += s.f1 + s.f2;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG */
static __attribute__((noinline))
void pattern_b(volatile int *counter) {
    volatile char c = (char)(*counter & 0xFF);
    volatile short s = (short)(*counter & 0xFFFF);
    volatile int i = *counter;
    
    /* STRICT_LOW_PART pattern: inline assembly modifying byte part */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)   /* =q constraint for byte-addressable register */
        : "0"(c)
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different-sized accesses */
    short *ps = (short*)&i;
    *ps = s;  /* This generates SUBREG in RTL */
    
    /* More SUBREG: access through char pointer */
    char *pc = (char*)&i;
    pc[1] = c;
    
    /* STRICT_LOW_PART with short */
    asm volatile (
        "incw %0\n\t"
        : "=r"(s)
        : "0"(s)
        : "cc"
    );
    
    *counter = i + s + c;
}

/* Function C: Complex expression mixing patterns */
static __attribute__((noinline))
void pattern_c(volatile int *counter) {
    /* Mixed-size array for SUBREG/MEM combinations */
    volatile long long big_array[5];
    volatile int small_array[10];
    
    /* Struct with bit-fields at different offsets */
    struct Mixed {
        volatile unsigned int a:3;
        volatile unsigned int b:9;
        volatile unsigned int c:20;
    } m;
    
    volatile int idx = *counter % 5;
    
    /* Complex addressing with ternary operator */
    volatile long long *ptr = (idx & 1) ? &big_array[idx] : 
                              (volatile long long*)&small_array[idx * 2];
    
    /* ZERO_EXTRACT with computed value */
    m.a = (*counter >> 0) & 0x7;
    m.b = (*counter >> 3) & 0x1FF;
    m.c = (*counter >> 12) & 0xFFFFF;
    
    /* MEM with pointer arithmetic */
    volatile int val = *((volatile int*)ptr + m.a);
    
    /* SUBREG through cast and assignment */
    volatile short *sptr = (volatile short*)ptr;
    sptr[1] = (short)(val + m.b);
    
    /* Final assignment that could involve multiple transformations */
    *counter = m.c + val + (int)(*ptr & 0xFFFFFFFF);
}

/* Helper to force MEM with complex addressing modes */
static __attribute__((noinline))
void complex_mem_access(volatile int *arr, int size, volatile int *counter) {
    for (volatile int i = 0; i < size; i++) {
        /* Multi-dimensional addressing simulation */
        volatile int *p = arr + i + *counter;
        volatile int *q = p + (i * 2);
        
        /* Chain of MEM accesses */
        volatile int x = *p;
        volatile int y = *(q - 1);
        *p = x + y;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(x), "r"(y) : "memory");
    }
}

int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int iterations = (argc > 1) ? 10 : 5;  /* Prevent infinite loops */
    
    /* Initialize some data */
    volatile int data_array[100];
    for (volatile int i = 0; i < 100; i++) {
        data_array[i] = i;
    }
    
    /* Main loop combining all patterns */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call each pattern function */
        pattern_a(&counter);
        pattern_b(&counter);
        pattern_c(&counter);
        
        /* Force complex MEM patterns */
        complex_mem_access(data_array, 20, &counter);
        
        /* Mix in some direct bit-field operations */
        struct {
            volatile unsigned int low:4;
            volatile unsigned int high:28;
        } bits;
        bits.low = counter & 0xF;
        bits.high = counter >> 4;
        
        /* More SUBREG through type punning */
        volatile int tmp = counter;
        volatile char *byte_ptr = (volatile char*)&tmp;
        for (volatile int j = 0; j < 4; j++) {
            byte_ptr[j] += j;
        }
        counter = tmp + bits.high;
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(counter) : "memory");
    }
    
    /* Final dummy result */
    volatile int result = counter;
    return result & 1;  /* Ensure program has valid return */
}
