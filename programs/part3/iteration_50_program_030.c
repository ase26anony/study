#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")

/* Heterogeneous structure with different alignments */
struct S {
    char c;
    short s;
    int a;
    float b;
    double d;
    long long ll;
} __attribute__((packed));

/* __attribute__((noinline)) helper to create another context */
__attribute__((noinline)) 
static long long process_subset(int* base, int size) {
    register int* restrict p = base;
    long long sum = 0;
    
    /* Pattern with [reg + 0] addressing */
    for (int i = 0; i < size; i++) {
        sum += *p;  /* Should become [reg + 0] */
        p = &base[i];  /* Reset pointer with zero offset */
    }
    
    return sum;
}

/* Another noinline helper with mixed access patterns */
__attribute__((noinline))
static double process_structs(struct S* arr, int count) {
    register struct S* restrict ptr = arr;
    double total = 0.0;
    
    /* Loop with structure field accesses (base + constant offset) */
    for (int i = 0; i < count; i++) {
        total += ptr->a;    /* First field, offset 0 */
        total += ptr->b;
        total += ptr->d;
        ptr++;  /* Post-increment */
    }
    
    return total;
}

int main(void) {
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    char arr_c[200];
    short arr_s[200];
    struct S arr_struct[100];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
        arr_c[i] = (char)(rand() % 256);
        arr_s[i] = (short)(rand() % 1000);
    }
    
    for (int i = 0; i < 100; i++) {
        arr_struct[i].c = (char)(rand() % 256);
        arr_struct[i].s = (short)(rand() % 1000);
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (float)(rand() % 1000) / 10.0f;
        arr_struct[i].d = (double)(rand() % 1000) / 10.0;
        arr_struct[i].ll = (long long)rand() * rand();
    }
    
    long long checksum = 0;
    double fp_checksum = 0.0;
    
    /* PATTERN 1: Simple post-increment with int pointer */
    {
        register int* restrict p = arr_i;
        #pragma GCC unroll 4
        for (int i = 0; i < 100; i++) {
            checksum += *p++;  /* Post-increment in access */
        }
        
        /* Compiler barrier with math function */
        fp_checksum += sin(checksum % 1000 * 0.001);
    }
    
    /* PATTERN 2: Post-decrement with float pointer */
    {
        register float* restrict q = &arr_f[99];
        int count = 100;
        while (count-- > 0) {
            fp_checksum += *q--;  /* Post-decrement */
            
            /* Insert occasional zero-offset access */
            if (count % 10 == 0) {
                fp_checksum += *(&arr_f[50]);  /* [reg + 0] pattern */
            }
        }
        
        /* Another compiler barrier */
        fp_checksum += cos(fp_checksum);
    }
    
    /* PATTERN 3: Mixed pointer and index arithmetic */
    {
        double* base = arr_d;
        register int idx = 0;
        
        for (int i = 0; i < 100; i++) {
            /* Multiple pointer aliases with zero offset */
            double* p1 = &base[idx];
            double* p2 = &arr_d[i];
            
            fp_checksum += *p1;  /* Should be [reg + 0] */
            fp_checksum += *p2;
            
            /* Compound expression with index modification */
            idx = (idx + 1) % 100;
            fp_checksum += arr_d[idx++];  /* arr[index++] */
        }
        
        /* Inline assembly to clobber memory */
        asm volatile("" ::: "memory");
    }
    
    /* PATTERN 4: Nested loops with pointer reset */
    {
        char* ptr_c = arr_c;
        short* ptr_s = arr_s;
        
        for (int outer = 0; outer < 10; outer++) {
            /* Inner loop with pointer stepping */
            register char* p = ptr_c;
            for (int inner = 0; inner < 20; inner++) {
                checksum += *p;  /* Memory access */
                p++;  /* Separate increment - should combine */
            }
            
            /* Switch to different pointer/type */
            register short* q = ptr_s;
            for (int inner = 0; inner < 20; inner++) {
                checksum += *q++;
            }
            
            /* Reset pointers for next iteration */
            ptr_c = &arr_c[outer * 20];
            ptr_s = &arr_s[outer * 20];
            
            /* Function call between pointer ops */
            fp_checksum += sqrt(fabs(fp_checksum));
        }
    }
    
    /* PATTERN 5: Structure array traversal */
    {
        struct S* ptr = arr_struct;
        #pragma GCC unroll 2
        for (int i = 0; i < 50; i++) {
            /* Access different structure fields */
            checksum += ptr->a;    /* First field - offset 0 */
            fp_checksum += ptr->b;
            fp_checksum += ptr->d;
            
            /* Pre-increment alternative */
            ++ptr;
            
            /* Access with negative index */
            if (i > 0) {
                checksum += (ptr - 1)->ll;
            }
        }
    }
    
    /* PATTERN 6: Complex expression with array indexing */
    {
        int index = 99;
        while (index >= 0) {
            /* arr[--index] pattern */
            checksum += arr_i[--index];
            
            /* Mixed with pointer arithmetic */
            if (index % 2 == 0) {
                checksum += *(arr_i + index);  /* *(base + index) */
            }
            
            /* Another zero-offset access */
            int* zero_ptr = &arr_i[index];
            checksum += *zero_ptr;  /* [reg + 0] */
        }
    }
    
    /* Call helper functions for additional contexts */
    checksum += process_subset(arr_i, 50);
    fp_checksum += process_structs(arr_struct, 25);
    
    /* Final computation to prevent elimination */
    double final_result = (double)checksum + fp_checksum;
    
    /* Use result to prevent dead code elimination */
    if (final_result > 0.0) {
        printf("Checksum: %f\n", final_result);
    } else {
        printf("Zero result\n");
    }
    
    return 0;
}
