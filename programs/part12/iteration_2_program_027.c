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
static volatile int volatile_flag = 0;

/* Target function that will induce many reloads */
__attribute__((noinline))
static long long induce_reloads(int N, int seed) {
    /* Declare many scalar variables to exceed register file */
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
            arr[i][j] = (i * 197 + j * 193) % 1000;
        }
    }
    
    /* Initialize packed structs */
    p1.d = d1; p1.i = a1; p1.c = (char)(a2 % 256); p1.l = l1; p1.f = f1;
    p2.d = d2; p2.i = a3; p2.c = (char)(a4 % 256); p2.l = l2; p2.f = f2;
    
    /* Volatile pointer to packed struct to prevent optimization */
    volatile struct Packed *volatile_p = &p1;
    
    /* Main computation loop - creates many live variables */
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            /* Complex array access pattern - forces address reloads */
            int temp = arr[i][j];
            arr[i][j] = arr[j][i] + a1;
            arr[j][i] = temp - a2;
            
            /* Chain computations to keep variables live */
            a1 = a2 + a3 * i - a4 / (j + 1);
            a2 = a3 ^ a5 | a6 & a7;
            a3 = a4 * a8 - a9 + a10;
            
            /* Floating point computations */
            f1 = f2 * f3 + f4 - f5 * (float)i;
            f2 = f3 / (f6 + 1.0f) * f7;
            f3 = f4 + f8 * (float)j - f1;
            
            /* Double computations */
            d1 = d2 * d3 - d4 / (double)(i + 1);
            d2 = d3 + d5 * (double)j - d6;
            d3 = d4 * d1 - d2 + d5;
            
            /* Long computations */
            l1 = l2 + l3 * i - l4 / (j + 1);
            l2 = l3 ^ l5 | l6 & l7;
            l3 = l4 * l8 - l1 + l2;
            
            /* Inline assembly with conflicting constraints */
            /* Force specific register allocation conflicts */
            __asm__ volatile (
                "add %0, %1, %2\n\t"
                : "=r"(a4) 
                : "r"(a5), "0"(a6)
                : /* clobbers */
            );
            
            /* Another asm with different constraints */
            __asm__ volatile (
                "mov %0, %1\n\t"
                "add %0, %0, %2\n\t"
                : "=&r"(a5), "+r"(a6)
                : "r"(a7)
                : /* clobbers */
            );
            
            /* Access packed struct through volatile pointer */
            /* This may require secondary reloads due to alignment */
            int packed_val = volatile_p->i;
            volatile_p->i = packed_val + i - j;
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different subset of variables conditionally */
                a9 = a10 * i + a8 / (j + 1);
                f7 = f8 * (float)i - f6;
                d5 = d6 * (double)j + d4;
                l7 = l8 ^ l6 | l5;
                
                /* More inline asm in conditional path */
                __asm__ volatile (
                    "sub %0, %1, %2\n\t"
                    : "=r"(a10)
                    : "r"(a9), "r"(i)
                    : /* clobbers */
                );
            } else {
                /* Alternative computation path */
                a8 = a7 + a6 * j - a5;
                f6 = f5 + f4 * (float)j - f3;
                d4 = d3 * (double)i - d2;
                l6 = l5 + l4 * j - l3;
            }
            
            /* Toggle volatile flag occasionally */
            if ((i * j) % 100 == 0) {
                volatile_flag = !volatile_flag;
            }
            
            /* More computations to increase register pressure */
            a7 = a1 + a2 - a3 * a4 / (a5 + 1);
            f5 = f1 + f2 - f3 * f4;
            d6 = d1 + d2 - d3 * d4;
            l5 = l1 + l2 - l3 * l4 / (l6 + 1);
            
            /* Use all variables in final computation to keep them live */
            a10 = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
            f8 = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
            d6 = d1 + d2 + d3 + d4 + d5 + d6;
            l8 = l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
        }
    }
    
    /* Compute checksum to prevent optimization */
    long long checksum = 0;
    checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    checksum += (long long)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8);
    checksum += (long long)(d1 + d2 + d3 + d4 + d5 + d6);
    checksum += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
    
    /* Add array elements to checksum */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            checksum += arr[i][j];
        }
    }
    
    /* Add packed struct values */
    checksum += (long long)volatile_p->i + (long long)volatile_p->l;
    
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
