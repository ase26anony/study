#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    float f;
    long l;
    char c;
    short s;
};

/* Volatile variable to prevent optimization of conditionals */
volatile int volatile_flag = 1;

/* Target function that will induce many reloads */
__attribute__((noinline))
unsigned long long induce_reloads(int N, int seed) {
    /* Large set of scalar variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    
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
    
    l1 = rand() * 1000L; l2 = rand() * 1000L; l3 = rand() * 1000L;
    l4 = rand() * 1000L; l5 = rand() * 1000L; l6 = rand() * 1000L;
    l7 = rand() * 1000L; l8 = rand() * 1000L;
    
    /* Initialize array */
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            arr[i][j] = (i * 200 + j) % 1000;
        }
    }
    
    /* Initialize packed structs */
    p1.d = d1; p1.i = a1; p1.f = f1; p1.l = l1; p1.c = a1 % 256; p1.s = a1 % 65536;
    p2.d = d2; p2.i = a2; p2.f = f2; p2.l = l2; p2.c = a2 % 256; p2.s = a2 % 65536;
    
    /* Volatile pointer to packed struct to force memory accesses */
    volatile struct Packed *volatile_p = &p1;
    
    /* Main computation loop - creates complex data dependencies */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            /* Complex array addressing - forces address register reloads */
            int idx1 = (i * j) % 199;
            int idx2 = (i + j) % 199;
            
            /* Use scalars in computation chain */
            a1 = a2 + a3;
            a2 = a3 * a4;
            a3 = a4 - a5;
            a4 = a5 + a6;
            a5 = a6 ^ a7;
            a6 = a7 | a8;
            a7 = a8 & a9;
            a8 = a9 << 2;
            a9 = a10 >> 1;
            a10 = a1 + i;
            
            /* Float computations */
            f1 = f2 + f3;
            f2 = f3 * f4;
            f3 = f4 - f5;
            f4 = f5 + f6;
            f5 = f6 * f7;
            f6 = f7 - f8;
            f7 = f8 + f1;
            f8 = f1 * 1.1f;
            
            /* Double computations */
            d1 = d2 + d3;
            d2 = d3 * d4;
            d3 = d4 - d5;
            d4 = d5 + d6;
            d5 = d6 * 1.01;
            d6 = d1 - 0.5;
            
            /* Long computations */
            l1 = l2 + l3;
            l2 = l3 * l4;
            l3 = l4 - l5;
            l4 = l5 + l6;
            l5 = l6 ^ l7;
            l6 = l7 | l8;
            l7 = l8 << 3;
            l8 = l1 >> 2;
            
            /* Inline assembly with conflicting constraints */
            /* Force input/output reloads with tied operands */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r"(a1)
                : "r"(a2), "0"(a3)
                : "cc"
            );
            
            /* Another asm with different constraints */
            asm volatile (
                "imul %0, %1\n\t"
                : "+r"(a4)
                : "r"(a5)
                : "cc"
            );
            
            /* Float asm with constraints */
            float ftmp;
            asm volatile (
                "addss %1, %0\n\t"
                : "=x"(ftmp)
                : "x"(f1), "0"(f2)
            );
            f3 = ftmp;
            
            /* Double asm with constraints */
            double dtmp;
            asm volatile (
                "addsd %1, %0\n\t"
                : "=x"(dtmp)
                : "x"(d1), "0"(d2)
            );
            d3 = dtmp;
            
            /* Array operation with complex addressing - forces address reloads */
            arr[idx1][idx2] = arr[idx2][idx1] + a1;
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different subset of variables inside conditional */
                int tmp = a6 + a7 + a8;
                float ftmp2 = f4 + f5 + f6;
                double dtmp2 = d4 + d5 + d6;
                
                /* More inline asm in conditional path */
                asm volatile (
                    "sub %0, %1, %2\n\t"
                    : "=r"(tmp)
                    : "r"(a9), "r"(a10)
                    : "cc"
                );
                
                arr[i][j] = tmp + (int)ftmp2 + (int)dtmp2;
            } else {
                /* Alternative path with different variables */
                long ltmp = l1 + l2 + l3;
                arr[j][i] = (int)ltmp;
            }
            
            /* Access packed struct through volatile pointer - forces unaligned reloads */
            int packed_val = volatile_p->i;
            float packed_float = volatile_p->f;
            
            /* Use packed values in computation */
            a1 ^= packed_val;
            f1 += packed_float;
            
            /* Switch between structs */
            if (j % 2) {
                volatile_p = &p1;
            } else {
                volatile_p = &p2;
            }
            
            /* Update packed struct members */
            volatile_p->i = a1;
            volatile_p->f = f1;
        }
        
        /* Additional computations between outer loop iterations */
        a1 = a1 * 2 - a2;
        a2 = a2 + a3 * 3;
        a3 = a3 - a4 / 2;
        a4 = a4 ^ a5;
        
        f1 = f1 * 1.5f - f2;
        f2 = f2 + f3 * 0.5f;
        
        d1 = d1 * 1.1 - d2;
        d2 = d2 + d3 * 0.9;
        
        l1 = l1 << 1 | l2;
        l2 = l2 + l3 >> 1;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    checksum += (unsigned long long)(f1 * 1000) + (unsigned long long)(f2 * 1000);
    checksum += (unsigned long long)(f3 * 1000) + (unsigned long long)(f4 * 1000);
    checksum += (unsigned long long)(d1 * 1000) + (unsigned long long)(d2 * 1000);
    checksum += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
    
    /* Add array elements to checksum */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            checksum += arr[i][j];
        }
    }
    
    /* Add packed struct values */
    checksum += p1.i + p2.i;
    checksum += (unsigned long long)(p1.f * 1000) + (unsigned long long)(p2.f * 1000);
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    printf("Starting reload induction test with N=%d, seed=%d\n", N, seed);
    
    unsigned long long result = induce_reloads(N, seed);
    
    printf("Result checksum: %llu\n", result);
    
    return 0;
}
