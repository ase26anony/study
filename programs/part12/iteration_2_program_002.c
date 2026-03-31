#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and potential secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    float f;
    long l;
};

/* Volatile variable to prevent optimization and create conditional paths */
volatile int global_flag = 1;

/* Function to induce register pressure and various reload types */
void induce_reloads(int N, int *checksum) {
    /* Declare many scalar variables to exceed available registers */
    int a1 = rand() % 100, a2 = rand() % 100, a3 = rand() % 100, a4 = rand() % 100;
    int a5 = rand() % 100, a6 = rand() % 100, a7 = rand() % 100, a8 = rand() % 100;
    float f1 = rand() / (float)RAND_MAX, f2 = rand() / (float)RAND_MAX;
    float f3 = rand() / (float)RAND_MAX, f4 = rand() / (float)RAND_MAX;
    double d1 = rand() / (double)RAND_MAX, d2 = rand() / (double)RAND_MAX;
    double d3 = rand() / (double)RAND_MAX, d4 = rand() / (double)RAND_MAX;
    long l1 = rand() % 1000, l2 = rand() % 1000, l3 = rand() % 1000, l4 = rand() % 1000;
    
    /* Multi-dimensional array for address reloads */
    int arr[200][200];
    for (int i = 0; i < 200; i++)
        for (int j = 0; j < 200; j++)
            arr[i][j] = i * 200 + j;
    
    /* Packed struct with volatile pointer */
    struct Packed packed;
    packed.d = 3.14159;
    packed.i = 42;
    packed.f = 2.71828f;
    packed.l = 123456789;
    volatile struct Packed *volatile_packed = &packed;
    
    /* Nested loops with complex array accesses */
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            /* Complex addressing that may require address reloads */
            int temp = arr[i][j] + arr[j][i];
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with "r" constraints */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r"(a1)
                : "r"(a2), "0"(a3)
                : "cc"
            );
            
            /* Another asm with memory constraint to force spills */
            asm volatile (
                "mov %0, %1\n\t"
                : "=r"(a4)
                : "m"(arr[i][j])
                : 
            );
            
            /* Use all scalar variables in computation chain */
            f1 = f2 + f3;
            d1 = d2 * d3 + d4;
            l1 = l2 + l3 - l4;
            
            /* Conditional block for optional reloads */
            if (global_flag) {
                /* Different computation path using different variables */
                a5 = a6 * a7 + a8;
                f4 = f1 * 2.0f;
                d4 = d1 / 2.0;
                
                /* Inline asm with tied operand in conditional path */
                asm volatile (
                    "sub %0, %1, %2\n\t"
                    : "=r"(a5)
                    : "r"(a6), "0"(a7)
                    : "cc"
                );
            } else {
                /* Alternative path */
                a8 = a1 + a2;
                f3 = f4 * 3.0f;
            }
            
            /* Access packed struct through volatile pointer */
            double packed_d = volatile_packed->d;
            int packed_i = volatile_packed->i;
            
            /* More arithmetic to keep variables live */
            a2 = a3 + a4;
            a3 = a5 - a6;
            f2 = f3 * f4;
            d2 = d3 + d4;
            l2 = l3 ^ l4;
            
            /* Array update with swapped indices */
            arr[i][j] = arr[j][i] + temp + a1;
            
            /* Another inline asm with specific register constraints (x86) */
            #ifdef __x86_64__
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(a7)
                : "r"(a8)
                : "cc"
            );
            #endif
            
            /* Use packed struct members */
            packed_d += d1;
            packed_i += a1;
            
            /* Store back to packed struct */
            volatile_packed->d = packed_d;
            volatile_packed->i = packed_i;
        }
        
        /* Update checksum with all variables */
        *checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
        *checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
        *checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4;
        *checksum += (int)l1 + (int)l2 + (int)l3 + (int)l4;
        *checksum += arr[i][i % 200];
    }
    
    /* Final computation using all variables */
    int final_result = 
        a1 * a2 + a3 * a4 - a5 * a6 + a7 * a8 +
        (int)(f1 * 100) + (int)(f2 * 100) + (int)(f3 * 100) + (int)(f4 * 100) +
        (int)(d1 * 100) + (int)(d2 * 100) + (int)(d3 * 100) + (int)(d4 * 100) +
        (int)l1 + (int)l2 + (int)l3 + (int)l4;
    
    *checksum += final_result;
}

/* Another function to create more reload contexts */
void additional_reload_context(int N, int *checksum) {
    /* Different set of variables */
    double dd1 = 1.0, dd2 = 2.0, dd3 = 3.0, dd4 = 4.0;
    float ff1 = 1.0f, ff2 = 2.0f, ff3 = 3.0f, ff4 = 4.0f;
    int ii1 = 1, ii2 = 2, ii3 = 3, ii4 = 4, ii5 = 5, ii6 = 6, ii7 = 7, ii8 = 8;
    
    /* Array with non-constant access pattern */
    int arr2[100][100];
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr2[i][j] = i * j;
        }
    }
    
    for (int k = 0; k < N; k++) {
        /* Complex computation with all variables */
        dd1 = dd2 * dd3 + dd4;
        dd2 = dd3 / dd4 - dd1;
        ff1 = ff2 + ff3 * ff4;
        ff2 = ff3 - ff4 / ff1;
        
        /* Inline asm with memory operand constraints */
        asm volatile (
            "mov %0, %1\n\t"
            : "=r"(ii1)
            : "m"(arr2[k % 100][(k * 7) % 100])
            :
        );
        
        /* More asm with tied operands */
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=r"(ii2)
            : "r"(ii3), "0"(ii4)
            : "cc"
        );
        
        /* Update checksum */
        *checksum += (int)dd1 + (int)dd2 + (int)dd3 + (int)dd4;
        *checksum += (int)ff1 + (int)ff2 + (int)ff3 + (int)ff4;
        *checksum += ii1 + ii2 + ii3 + ii4 + ii5 + ii6 + ii7 + ii8;
        *checksum += arr2[k % 100][(k * 13) % 100];
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    /* Use command-line argument for loop bound */
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    if (N < 10) N = 10;
    if (N > 1000) N = 1000;
    
    int checksum1 = 0, checksum2 = 0;
    
    /* Call functions to induce reloads */
    induce_reloads(N, &checksum1);
    additional_reload_context(N / 2, &checksum2);
    
    /* Final result to prevent optimization */
    int final_checksum = checksum1 + checksum2;
    printf("Checksum: %d\n", final_checksum);
    
    return final_checksum != 0 ? 0 : 1;
}
