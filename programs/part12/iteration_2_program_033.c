/* reload_coverage.c - Program to trigger GCC reload pass initialization */
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

/* Volatile variables to prevent optimization */
volatile int g_flag = 0;
volatile int g_volatile_counter = 0;

/* Target function that creates register pressure and various reloads */
__attribute__((noinline))
static long create_reloads(int N, int seed) {
    /* Large set of scalar variables to exceed available registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    
    /* Multi-dimensional array for address reloads */
    int arr[200][200];
    
    /* Packed struct for unaligned accesses */
    struct Packed packed_data[10];
    
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
    
    /* Initialize array with pattern */
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            arr[i][j] = (i * 197 + j * 193) % 1000;
        }
    }
    
    /* Initialize packed structs */
    for (int i = 0; i < 10; i++) {
        packed_data[i].d = (double)rand() / RAND_MAX;
        packed_data[i].i = rand() % 1000;
        packed_data[i].f = (float)rand() / RAND_MAX;
        packed_data[i].l = rand() * 1000L;
        packed_data[i].c = rand() % 256;
    }
    
    /* Main computation loop - creates register pressure and various reloads */
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            /* Complex array access pattern - forces address reloads */
            int temp = arr[i][j];
            arr[i][j] = arr[j][i] + a1;
            arr[j][i] = temp - a2;
            
            /* Chain of arithmetic operations - keeps many variables live */
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
            
            /* Floating point chains */
            f1 = f2 + f3;
            f2 = f3 * f4;
            f3 = f4 - f5;
            f4 = f5 + f6;
            f5 = f6 * f7;
            f6 = f7 - f8;
            f7 = f8 + f1;
            f8 = f1 * f2;
            
            /* Double precision chains */
            d1 = d2 + d3;
            d2 = d3 * d4;
            d3 = d4 - d5;
            d4 = d5 + d6;
            d5 = d6 * d1;
            d6 = d1 - d2;
            
            /* Long integer chains */
            l1 = l2 + l3;
            l2 = l3 * l4;
            l3 = l4 - l5;
            l4 = l5 ^ l6;
            l5 = l6 | l7;
            l6 = l7 & l8;
            l7 = l8 + l1;
            l8 = l1 - l2;
            
            /* Inline assembly with conflicting constraints - forces reloads */
            /* Tied operand constraint (output tied to input) */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r"(a1)
                : "r"(a2), "0"(a3)
                : "cc"
            );
            
            /* Different register class constraints */
            asm volatile (
                "imul %0, %1\n\t"
                : "+r"(a4)
                : "r"(a5)
                : "cc"
            );
            
            /* Memory constraint forcing spill/reload */
            asm volatile (
                "addl %1, %0\n\t"
                : "+m"(arr[i][j])
                : "r"(a6)
                : "cc"
            );
            
            /* Conditional block for optional reloads */
            if (g_flag) {
                /* Use different subset of variables */
                volatile int* volatile_ptr = &g_volatile_counter;
                *volatile_ptr += a7 + a8;
                
                /* More arithmetic in conditional path */
                f1 = f2 * 2.0f;
                d1 = d2 / 2.0;
                l1 = l2 << 2;
            } else {
                /* Alternative path with different variables */
                a9 = a10 * 3;
                f3 = f4 * 3.0f;
                d3 = d4 / 3.0;
                l3 = l4 >> 1;
            }
            
            /* Access packed struct through volatile pointer - forces secondary reloads */
            volatile struct Packed* volatile_packed = &packed_data[i % 10];
            volatile_packed->i += a1;
            volatile_packed->f += f1;
            
            /* More arithmetic mixing types */
            a1 = (int)(f1 * 100.0f);
            f1 = (float)(d1 * 2.0);
            d1 = (double)(l1 % 1000);
            l1 = (long)(a1 * 1000);
            
            /* Cross-type dependencies */
            a2 = a3 + (int)f2;
            f2 = f3 + (float)a4;
            d2 = d3 + (double)l2;
            l2 = l3 + (long)a5;
            
            /* Prevent loop elimination */
            g_volatile_counter++;
        }
    }
    
    /* Compute checksum using all variables to prevent dead code elimination */
    long checksum = 0;
    checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    checksum += (long)(f1 * 1000) + (long)(f2 * 1000) + (long)(f3 * 1000) + 
                (long)(f4 * 1000) + (long)(f5 * 1000) + (long)(f6 * 1000) + 
                (long)(f7 * 1000) + (long)(f8 * 1000);
    checksum += (long)(d1 * 1000) + (long)(d2 * 1000) + (long)(d3 * 1000) + 
                (long)(d4 * 1000) + (long)(d5 * 1000) + (long)(d6 * 1000);
    checksum += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
    
    /* Add array checksum */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            checksum += arr[i][j];
        }
    }
    
    /* Add packed struct checksum */
    for (int i = 0; i < 10; i++) {
        checksum += packed_data[i].i;
        checksum += (long)(packed_data[i].f * 1000);
        checksum += (long)(packed_data[i].d * 1000);
        checksum += packed_data[i].l;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    printf("Running reload coverage test with N=%d, seed=%d\n", N, seed);
    
    /* Call the reload-intensive function multiple times */
    long total_checksum = 0;
    for (int iter = 0; iter < 3; iter++) {
        long result = create_reloads(N, seed + iter);
        total_checksum += result;
        printf("Iteration %d: checksum = %ld\n", iter, result);
        
        /* Toggle flag to create different execution paths */
        g_flag = !g_flag;
    }
    
    printf("Total checksum: %ld\n", total_checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return 0;
}
