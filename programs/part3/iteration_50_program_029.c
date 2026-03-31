/* auto-inc-dec-test.c */
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

/* Restrict-qualified helper function */
__attribute__((noinline, optimize("O3")))
static double process_restricted(int* __restrict p1, float* __restrict p2, 
                                 struct S* __restrict p3, int n) {
    double sum = 0.0;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; i++) {
        sum += p1[i];      /* [reg + 0] addressing for p1 */
        sum += p2[i];      /* [reg + 0] addressing for p2 */
    }
    
    /* Pattern 2: Mixed pointer/index arithmetic */
    int* ptr = p1;
    for (int i = 0; i < n; i++) {
        sum += *(ptr + i);  /* Force base+index calculation */
        ptr = p1;           /* Reset pointer - forces re-evaluation */
    }
    
    return sum;
}

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O3"), noinline))
static void mixed_access_patterns(int* arr_i, float* arr_f, 
                                  struct S* arr_s, int n) {
    register int* rp1 asm ("r12") = arr_i;  /* Hint for register allocation */
    register float* rp2 asm ("r13") = arr_f;
    
    /* Pattern 3: Post-increment in loop with register hints */
    for (int i = 0; i < n; i++) {
        *rp1++ += i;        /* Post-increment on register variable */
        *rp2++ += i * 0.5f;
        
        /* Compiler barrier via function call */
        if (i % 16 == 0) {
            volatile double x = sin(i * 0.1);
            (void)x;
        }
    }
}

int main(void) {
    /* Declare arrays of different types and alignments */
    int arr_i[200] __attribute__((aligned(32)));
    float arr_f[200] __attribute__((aligned(16)));
    double arr_d[200];
    struct S arr_s[100];
    char arr_c[400];
    short arr_sh[200];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 100.0;
        if (i < 100) {
            arr_s[i].c = rand() % 256;
            arr_s[i].s = rand() % 1000;
            arr_s[i].a = rand() % 1000;
            arr_s[i].b = (float)(rand() % 1000) / 10.0f;
            arr_s[i].d = (double)(rand() % 1000) / 100.0;
            arr_s[i].ll = (long long)rand() * rand();
        }
        arr_c[i] = rand() % 256;
        arr_sh[i] = rand() % 1000;
    }
    
    double total_sum = 0.0;
    
    /* PATTERN 1: Simple post-increment with multiple base registers */
    {
        int* p = arr_i;
        float* q = arr_f;
        double* r = arr_d;
        
        #pragma GCC unroll 4
        for (int i = 0; i < 100; i++) {
            total_sum += *p++;      /* Should trigger find_inc for [reg+0] */
            total_sum += *q++;      /* Different base register */
            total_sum += *r++;      /* Another base register */
            
            /* Access with zero offset via different pointer */
            int* p2 = p;
            total_sum += *p2;       /* [reg + 0] addressing */
        }
    }
    
    /* Compiler barrier */
    volatile double barrier = cos(total_sum);
    (void)barrier;
    
    /* PATTERN 2: Post-decrement in while loop */
    {
        float* q = &arr_f[99];
        int counter = 100;
        
        while (counter-- > 0) {
            total_sum += *q--;      /* Post-decrement */
            
            /* Compound expression with index modification */
            arr_i[counter] += (int)total_sum;
        }
    }
    
    /* PATTERN 3: Nested loops with pointer reset */
    for (int outer = 0; outer < 10; outer++) {
        struct S* ptr = arr_s;
        
        /* Inner loop with structure field access (base + constant offset) */
        for (int inner = 0; inner < 50; inner++) {
            /* Access different fields - each is base + offset (0 for first field) */
            total_sum += ptr->a;    /* Base + 0 offset after alignment */
            total_sum += ptr->b;
            total_sum += ptr->d;
            
            ptr++;  /* Post-increment */
            
            /* Inline assembly to clobber memory */
            asm volatile("" ::: "memory");
        }
        
        /* Switch to different pointer type */
        char* cp = arr_c;
        for (int i = 0; i < 100; i++) {
            total_sum += cp[i];     /* Array indexing */
            cp = arr_c;             /* Reset pointer - forces address recalculation */
        }
    }
    
    /* PATTERN 4: Mixed pointer and index arithmetic */
    {
        int* base = arr_i;
        register int idx asm ("r14") = 0;  /* Register hint */
        
        for (idx = 0; idx < 100; idx++) {
            /* Multiple ways to access same memory */
            total_sum += *(base + idx);          /* Base + index */
            total_sum += base[idx];              /* Array indexing */
            
            int* alias = base + idx;
            total_sum += *alias;                 /* [reg + 0] */
            
            /* Function call between accesses */
            if (idx % 8 == 0) {
                total_sum += sqrt(fabs(total_sum));
            }
        }
    }
    
    /* PATTERN 5: Complex expression with side effects */
    {
        int index = 0;
        for (int i = 0; i < 100; ) {
            /* Compound expression modifying index */
            total_sum += arr_i[index++] * 2;
            total_sum += arr_i[--index] * 3;
            i++;
            index++;
            
            /* Multiple pointer aliases */
            int* p1 = &arr_i[i];
            int* p2 = p1;
            int* p3 = p2;
            total_sum += *p1 + *p2 + *p3;  /* Same [reg + 0] address */
        }
    }
    
    /* Call helper functions */
    mixed_access_patterns(arr_i, arr_f, arr_s, 50);
    total_sum += process_restricted(arr_i, arr_f, arr_s, 50);
    
    /* PATTERN 6: Different data sizes and alignments */
    {
        /* Misaligned access */
        char* misaligned = (char*)arr_i + 1;
        for (int i = 0; i < 100; i++) {
            total_sum += misaligned[i];
        }
        
        /* Properly aligned access */
        int* aligned = (int*)(((uintptr_t)arr_i + 15) & ~15);
        for (int i = 0; i < 50; i++) {
            total_sum += aligned[i];
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %f\n", total_sum);
    
    return (int)total_sum % 256;
}
