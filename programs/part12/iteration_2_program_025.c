#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    char c;
    long l;
    float f;
};

/* Volatile flag for conditional execution */
volatile int volatile_flag = 1;

/* Target function with high register pressure */
__attribute__((noinline))
static long long induce_reloads(int N, int seed) {
    /* Declare many scalar variables to exceed available registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5;
    
    /* Multi-dimensional array for address reloads */
    int arr[200][200];
    
    /* Packed struct for unaligned accesses */
    struct Packed p1, p2;
    
    /* Initialize with non-constant values */
    srand(seed);
    a1 = rand() % 100; a2 = rand() % 100; a3 = rand() % 100; a4 = rand() % 100;
    a5 = rand() % 100; a6 = rand() % 100; a7 = rand() % 100; a8 = rand() % 100;
    a9 = rand() % 100; a10 = rand() % 100;
    
    f1 = (float)rand() / RAND_MAX; f2 = (float)rand() / RAND_MAX;
    f3 = (float)rand() / RAND_MAX; f4 = (float)rand() / RAND_MAX;
    f5 = (float)rand() / RAND_MAX; f6 = (float)rand() / RAND_MAX;
    f7 = (float)rand() / RAND_MAX; f8 = (float)rand() / RAND_MAX;
    
    d1 = (double)rand() / RAND_MAX; d2 = (double)rand() / RAND_MAX;
    d3 = (double)rand() / RAND_MAX; d4 = (double)rand() / RAND_MAX;
    d5 = (double)rand() / RAND_MAX; d6 = (double)rand() / RAND_MAX;
    
    l1 = rand() * 1000LL; l2 = rand() * 1000LL;
    l3 = rand() * 1000LL; l4 = rand() * 1000LL; l5 = rand() * 1000LL;
    
    /* Initialize array with non-constant pattern */
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            arr[i][j] = (i * 31 + j * 17) % 1000;
        }
    }
    
    /* Initialize packed structs */
    p1.d = d1; p1.i = a1; p1.c = (char)(a2 % 256); p1.l = l1; p1.f = f1;
    p2.d = d2; p2.i = a3; p2.c = (char)(a4 % 256); p2.l = l2; p2.f = f2;
    
    /* Main computation loop with high register pressure */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            /* Complex array access pattern requiring address reloads */
            int tmp1 = arr[i][j];
            int tmp2 = arr[j][i];
            int tmp3 = arr[i-1][j];
            int tmp4 = arr[i][j-1];
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with "r" constraints */
            asm volatile (
                "addl %1, %0\n\t"
                "addl %2, %0"
                : "+r" (tmp1)
                : "r" (tmp2), "r" (tmp3)
                : "cc"
            );
            
            /* Another asm with tied operand (output = input 0) */
            asm volatile (
                "imull %1, %0"
                : "+0" (tmp4)
                : "r" (tmp1)
                : "cc"
            );
            
            /* Mixed-type computations to keep all variables live */
            a1 = a2 + a3;
            a2 = a3 * a4;
            a3 = a4 - a5;
            a4 = a5 ^ a6;
            a5 = a6 | a7;
            a6 = a7 & a8;
            a7 = a8 + a9;
            a8 = a9 - a10;
            a9 = a10 * a1;
            a10 = a1 + a2;
            
            /* Float computations */
            f1 = f2 + f3;
            f2 = f3 * f4;
            f3 = f4 - f5;
            f4 = f5 * f6;
            f5 = f6 + f7;
            f6 = f7 - f8;
            f7 = f8 * f1;
            f8 = f1 + f2;
            
            /* Double computations with type mixing */
            d1 = d2 + (double)f1;
            d2 = d3 * (double)a1;
            d3 = d4 - d5;
            d4 = d5 * d6;
            d5 = d6 + (double)l1;
            d6 = d1 * d2;
            
            /* Long computations */
            l1 = l2 + a1;
            l2 = l3 * a2;
            l3 = l4 - a3;
            l4 = l5 ^ a4;
            l5 = l1 + a5;
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different subset of variables inside conditional */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "addl %2, %%eax\n\t"
                    "movl %%eax, %0"
                    : "=r" (tmp2)
                    : "r" (a6), "r" (a7)
                    : "eax", "cc"
                );
                
                /* Access packed struct through volatile pointer */
                volatile struct Packed *vp = &p1;
                int packed_val = vp->i;
                packed_val += vp->c;
                
                /* Force memory constraint in asm */
                asm volatile (
                    "addl %1, %0"
                    : "+r" (a8)
                    : "m" (vp->i)
                    : "cc"
                );
                
                a9 = packed_val + a8;
            } else {
                /* Alternative path with different variable usage */
                asm volatile (
                    "movq %1, %%rax\n\t"
                    "addq %2, %%rax\n\t"
                    "movq %%rax, %0"
                    : "=r" (l3)
                    : "r" (l4), "r" (l5)
                    : "rax", "cc"
                );
                
                a10 = (int)l3;
            }
            
            /* Update array with computed values */
            arr[i][j] = tmp1 + tmp4 + a1 + (int)f1;
            arr[j][i] = tmp2 + a2 + (int)d1;
            
            /* Access second packed struct */
            struct Packed *p_ptr = &p2;
            p_ptr->i = a3;
            p_ptr->f = f3;
            
            /* Another asm with memory output constraint */
            asm volatile (
                "movl %1, %0"
                : "=m" (p_ptr->c)
                : "r" (a4)
            );
        }
        
        /* Cross-iteration dependencies */
        a1 = a1 ^ arr[i][i % 100];
        f1 = f1 * (float)arr[(i+1) % 100][i % 100];
        d1 = d1 + (double)arr[i % 100][(i+2) % 100];
        l1 = l1 + arr[(i+3) % 100][i % 100];
    }
    
    /* Compute checksum to prevent optimization */
    long long checksum = 0;
    checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    checksum += (long long)(f1 * 1000) + (long long)(f2 * 1000) +
                (long long)(f3 * 1000) + (long long)(f4 * 1000);
    checksum += (long long)(d1 * 1000) + (long long)(d2 * 1000) +
                (long long)(d3 * 1000) + (long long)(d4 * 1000);
    checksum += l1 + l2 + l3 + l4 + l5;
    
    /* Add array elements to checksum */
    for (int i = 0; i < 100; i += 7) {
        for (int j = 0; j < 100; j += 11) {
            checksum += arr[i][j];
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    printf("Starting reload induction test with N=%d, seed=%d\n", N, seed);
    
    long long result = induce_reloads(N, seed);
    
    printf("Result checksum: %lld\n", result);
    
    return 0;
}
