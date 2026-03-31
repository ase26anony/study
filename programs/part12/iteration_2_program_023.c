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
};

/* Volatile variable to prevent optimization of conditionals */
volatile int volatile_flag = 1;

/* Target function that induces various types of reloads */
__attribute__((noinline))
unsigned long long induce_reloads(int N, int seed) {
    /* Large set of scalar variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5, l6;
    
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
    
    /* Initialize array */
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            arr[i][j] = (i * 200 + j) % 100;
        }
    }
    
    /* Initialize packed structs */
    p1.d = d1; p1.i = a1; p1.f = f1; p1.l = l1; p1.c = 'A';
    p2.d = d2; p2.i = a2; p2.f = f2; p2.l = l2; p2.c = 'B';
    
    /* Main computation loop - creates complex data dependencies */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            /* Complex addressing with non-constant indices */
            int idx1 = (i * j) % 199;
            int idx2 = (i + j) % 199;
            
            /* Force address reloads with array accesses */
            int temp1 = arr[idx1][idx2];
            int temp2 = arr[idx2][idx1];
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with "r" constraints */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(temp1)
                : "r"(temp2)
                : "cc"
            );
            
            /* Another asm with tied operand (forces reload) */
            asm volatile (
                "imull %1, %0\n\t"
                : "+r"(a1)
                : "r"(temp1), "0"(a1)
                : "cc"
            );
            
            /* Float/double operations to use FP registers */
            f1 = f1 * f2 + f3;
            d1 = d1 * d2 + d3;
            
            /* Long operations */
            l1 = l1 + l2 * l3;
            
            /* Chain computations to keep variables live */
            a2 = a1 + a3;
            a3 = a2 * a4;
            a4 = a3 - a5;
            a5 = a4 + a6;
            a6 = a5 * a7;
            a7 = a6 - a8;
            a8 = a7 + a9;
            a9 = a8 * a10;
            a10 = a9 - a1;
            
            /* More FP chaining */
            f2 = f1 + f4;
            f3 = f2 * f5;
            f4 = f3 - f6;
            f5 = f4 + f7;
            f6 = f5 * f8;
            
            d2 = d1 + d4;
            d3 = d2 * d5;
            d4 = d3 - d6;
            
            l2 = l1 + l4;
            l3 = l2 * l5;
            l4 = l3 - l6;
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different subset of variables inside conditional */
                asm volatile (
                    "subl %1, %0\n\t"
                    : "+r"(a2)
                    : "r"(a3)
                    : "cc"
                );
                
                f7 = f6 * 2.0f;
                d5 = d4 / 2.0;
                l5 = l4 >> 2;
                
                /* Access packed struct through volatile pointer */
                volatile struct Packed *vp = &p1;
                vp->i = a2;
                vp->f = f7;
            } else {
                /* Alternative path with different variables */
                asm volatile (
                    "addl %1, %0\n\t"
                    : "+r"(a8)
                    : "r"(a9)
                    : "cc"
                );
                
                f8 = f7 / 2.0f;
                d6 = d5 * 2.0;
                l6 = l5 << 2;
                
                volatile struct Packed *vp = &p2;
                vp->i = a8;
                vp->f = f8;
            }
            
            /* Store result back to array with complex addressing */
            arr[i][j] = a1 + a2 + a3 + temp1;
            
            /* Access packed struct members (unaligned, may need secondary reload) */
            p1.d = d1 + p2.d;
            p1.i = a1 + p2.i;
            p1.f = f1 + p2.f;
            p1.l = l1 + p2.l;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    checksum += (unsigned long long)(f1 * 1000) + (unsigned long long)(f2 * 1000);
    checksum += (unsigned long long)(d1 * 1000) + (unsigned long long)(d2 * 1000);
    checksum += l1 + l2 + l3 + l4 + l5 + l6;
    
    /* Add array elements to checksum */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            checksum += arr[i][j];
        }
    }
    
    /* Add packed struct members */
    checksum += (unsigned long long)p1.d + p1.i + (unsigned long long)p1.f + p1.l;
    checksum += (unsigned long long)p2.d + p2.i + (unsigned long long)p2.f + p2.l;
    
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
