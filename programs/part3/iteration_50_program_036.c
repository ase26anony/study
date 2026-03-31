#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Compiler hints to influence optimization */
#pragma GCC optimize("O3")
#define RESTRICT __restrict

/* Mixed data types for offset calculation stress */
struct Heterogeneous {
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
};

/* __attribute__((noinline)) helper to create another find_inc context */
__attribute__((noinline)) 
static double process_subset(int* RESTRICT arr, int n) {
    double sum = 0.0;
    int* p = arr;
    int* end = arr + n;
    
    /* Pattern 1: Simple post-increment in while loop */
    while (p < end) {
        sum += *p++;
    }
    
    /* Pattern 2: Post-decrement with reset */
    p = end - 1;
    while (p >= arr) {
        sum += *p--;
    }
    
    return sum;
}

/* Another noinline function with different access patterns */
__attribute__((noinline))
static float traverse_structs(struct Heterogeneous* RESTRICT arr, int n) {
    float total = 0.0f;
    struct Heterogeneous* ptr = arr;
    
    /* Access different struct members with constant offsets */
    for (int i = 0; i < n; i++) {
        total += ptr->f;          /* Base + constant offset */
        total += (float)ptr->i;   /* Different constant offset */
        ptr++;                    /* Post-increment of struct pointer */
    }
    
    return total;
}

int main(void) {
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    struct Heterogeneous arr_s[100];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < 100; i++) {
        arr_s[i].c = (char)(rand() % 256);
        arr_s[i].s = (short)(rand() % 1000);
        arr_s[i].i = rand() % 1000;
        arr_s[i].l = rand() % 1000;
        arr_s[i].f = (float)(rand() % 1000) / 10.0f;
        arr_s[i].d = (double)(rand() % 1000) / 10.0;
    }
    
    double checksum = 0.0;
    
    /* ===== PATTERN 1: Multiple base registers with zero offset ===== */
    {
        /* Different pointer variables accessing with zero offset */
        int* p1 = &arr_i[0];
        int* p2 = &arr_i[50];
        float* q1 = &arr_f[0];
        double* r1 = &arr_d[0];
        
        /* Force [reg + 0] addressing */
        checksum += *p1;      /* Direct dereference - should be [reg + 0] */
        checksum += *p2;      /* Another base register */
        checksum += *q1;      /* Different type */
        checksum += *r1;      /* Another type */
    }
    
    /* ===== PATTERN 2: Post-increment in for loop ===== */
    {
        register int* p = arr_i;  /* register hint for allocation */
        int sum = 0;
        
        #pragma GCC unroll 4
        for (int i = 0; i < 100; i++) {
            sum += *p++;          /* Post-increment access */
        }
        checksum += sum;
        
        /* Function call to obscure aliasing */
        checksum += sin(checksum);
    }
    
    /* ===== PATTERN 3: Post-decrement in while loop ===== */
    {
        float* q = &arr_f[99];
        float sum = 0.0f;
        
        while (q >= arr_f) {
            sum += *q--;          /* Post-decrement access */
        }
        checksum += sum;
        
        /* Another function call as compiler barrier */
        checksum += cos(checksum);
    }
    
    /* ===== PATTERN 4: Nested loops with pointer reset ===== */
    {
        double* base = arr_d;
        double outer_sum = 0.0;
        
        /* Outer loop resets pointer */
        for (int outer = 0; outer < 4; outer++) {
            double* ptr = base + (outer * 25);
            double inner_sum = 0.0;
            
            /* Inner loop uses pointer arithmetic */
            for (int inner = 0; inner < 25; inner++) {
                inner_sum += *(ptr + inner);  /* Pointer + index */
            }
            outer_sum += inner_sum;
        }
        checksum += outer_sum;
    }
    
    /* ===== PATTERN 5: Mixed pointer and index variables ===== */
    {
        int sum1 = 0, sum2 = 0;
        int* base_ptr = arr_i;
        
        /* Mix pointer and index in same loop */
        for (int idx = 0; idx < 100; idx++) {
            /* Two different access patterns */
            sum1 += *(base_ptr + idx);      /* Base + index */
            sum2 += arr_i[idx];             /* Array indexing */
        }
        checksum += sum1 + sum2;
        
        /* Inline assembly to clobber memory */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* ===== PATTERN 6: Compound expressions in loop conditions ===== */
    {
        int index = 0;
        int sum = 0;
        
        /* arr[index++] in condition */
        while (index < 100) {
            sum += arr_i[index++];          /* Post-increment in array access */
        }
        checksum += sum;
        
        index = 99;
        sum = 0;
        /* arr[--index] in update */
        for (int i = 0; i < 100; i++) {
            sum += arr_i[index];
            --index;                        /* Pre-decrement */
        }
        checksum += sum;
    }
    
    /* ===== PATTERN 7: Struct array traversal ===== */
    {
        float struct_sum = 0.0f;
        struct Heterogeneous* sptr = arr_s;
        
        /* Access struct members with constant offsets */
        for (int i = 0; i < 50; i++) {
            struct_sum += sptr->f;          /* Base + offset for float */
            struct_sum += (float)sptr->i;   /* Base + different offset */
            sptr++;                         /* Struct pointer increment */
        }
        checksum += struct_sum;
        
        /* Function call between pointer operations */
        checksum += sqrt(fabs(checksum));
    }
    
    /* ===== PATTERN 8: Call helper functions ===== */
    checksum += process_subset(arr_i, 100);
    checksum += traverse_structs(arr_s, 50);
    
    /* ===== PATTERN 9: Complex expression with multiple increments ===== */
    {
        int* p1 = arr_i;
        int* p2 = arr_i + 50;
        int complex_sum = 0;
        
        for (int i = 0; i < 50; i++) {
            /* Multiple increments in one statement */
            complex_sum += *p1++ + *p2++;
        }
        checksum += complex_sum;
    }
    
    /* ===== PATTERN 10: Different sized types ===== */
    {
        char* cp = (char*)arr_i;
        short* sp = (short*)arr_i;
        int* ip = arr_i;
        
        /* Access same memory with different type pointers */
        for (int i = 0; i < 50; i++) {
            checksum += cp[i] + sp[i] + ip[i];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    return (int)checksum % 256;
}
