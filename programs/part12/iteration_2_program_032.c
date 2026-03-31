/* reload_coverage.c - Program to exercise GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and potential secondary reloads */
struct __attribute__((packed)) PackedStruct {
    double d;
    int i;
    float f;
    long l;
    char c;
    short s;
};

/* Volatile flag for conditional execution */
volatile int volatile_flag = 0;

/* Target function with high register pressure */
__attribute__((noinline))
static long long induce_reloads(int N, int seed) {
    /* Declare many scalar variables to exceed available registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5;
    
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
    
    l1 = rand() * 100LL; l2 = rand() * 100LL; l3 = rand() * 100LL;
    l4 = rand() * 100LL; l5 = rand() * 100LL;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = i * 1000 + j;
        }
    }
    
    /* Packed struct accessed through volatile pointer */
    struct PackedStruct ps;
    ps.d = d1; ps.i = a1; ps.f = f1; ps.l = l1; ps.c = 'A'; ps.s = 42;
    volatile struct PackedStruct *volatile_ps = &ps;
    
    long long checksum = 0;
    
    /* Main computation loop with high register pressure */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            /* Complex array access with computed indices - forces address reloads */
            int idx1 = (i * j) % 127;
            int idx2 = (i + j) % 127;
            
            /* Chain computations to keep variables live */
            a1 = a2 + a3;
            a2 = a3 * a4;
            a3 = a4 - a5;
            a4 = a5 ^ a6;
            a5 = a6 | a7;
            a6 = a7 & a8;
            a7 = a8 << 2;
            a8 = a9 >> 1;
            a9 = a10 + i;
            a10 = a1 * j;
            
            f1 = f2 + f3;
            f2 = f3 * f4;
            f3 = f4 - f5;
            f4 = f5 * f6;
            f5 = f6 + f7;
            f6 = f7 * f8;
            f7 = f8 - f1;
            f8 = f1 * 1.5f;
            
            d1 = d2 + d3;
            d2 = d3 * d4;
            d3 = d4 - d5;
            d4 = d5 * d6;
            d5 = d6 + d1;
            d6 = d1 * 2.0;
            
            l1 = l2 + l3;
            l2 = l3 * l4;
            l3 = l4 - l5;
            l4 = l5 ^ l1;
            l5 = l1 | (i * j);
            
            /* Inline assembly with conflicting constraints to force reloads */
            /* Tied operand constraint (output tied to input) */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(a1)  /* read-write operand */
                : "r"(a2)   /* input operand */
                : "cc"
            );
            
            /* Different register class constraints */
            asm volatile (
                "mov %1, %0\n\t"
                : "=r"(a3)
                : "r"(a4)
            );
            
            /* Memory constraint forcing spill/reload */
            asm volatile (
                "addl %1, %0\n\t"
                : "+m"(arr[idx1][idx2])  /* memory operand */
                : "r"(a5)                /* register operand */
                : "cc"
            );
            
            /* Float/double operations with constraints */
            asm volatile (
                "addss %1, %0\n\t"
                : "+x"(f1)
                : "x"(f2)
            );
            
            asm volatile (
                "addsd %1, %0\n\t"
                : "+x"(d1)
                : "x"(d2)
            );
            
            /* Conditional block for optional reloads */
            if (volatile_flag || (i % 13 == 0)) {
                /* Use different subset of variables here */
                int t1 = a6 + a7;
                int t2 = a8 * a9;
                float tf = f3 + f4;
                double td = d3 * d4;
                
                /* More inline assembly in conditional path */
                asm volatile (
                    "imull %1, %0\n\t"
                    : "+r"(t1)
                    : "r"(t2)
                    : "cc"
                );
                
                a10 = t1 + t2;
                f8 = tf * 2.0f;
                d6 = td / 2.0;
                
                /* Access packed struct through volatile pointer */
                volatile_ps->i = t1;
                volatile_ps->f = tf;
            }
            
            /* Array manipulation with complex addressing */
            arr[j][i] = arr[i][j] + a1;
            arr[i+1][j-1] = arr[i-1][j+1] * 2;
            
            /* Update checksum with many variables */
            checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
            checksum += (long long)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8);
            checksum += (long long)(d1 + d2 + d3 + d4 + d5 + d6);
            checksum += l1 + l2 + l3 + l4 + l5;
            checksum += arr[i][j];
        }
        
        /* Occasionally update volatile flag */
        if (i % 100 == 0) {
            volatile_flag = i & 1;
        }
    }
    
    /* Final computation using all variables */
    long long final_result = 
        (long long)a1 * a2 * a3 * a4 * a5 +
        (long long)(f1 * f2 * f3 * 1000) +
        (long long)(d1 * d2 * 1000) +
        l1 * l2 * l3 +
        checksum;
    
    return final_result;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    printf("Running reload coverage test with N=%d, seed=%d\n", N, seed);
    
    long long result = induce_reloads(N, seed);
    
    printf("Result: %lld\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("Zero result (unlikely)\n");
    }
    
    return 0;
}
