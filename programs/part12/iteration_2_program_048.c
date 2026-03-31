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

/* Volatile variable to prevent optimization and create conditional paths */
static volatile int volatile_flag = 0;

/* Function with high register pressure and complex operations */
void trigger_reloads(int N, int *checksum) {
    /* Declare many variables to exceed available registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5;
    
    /* Initialize with non-constant values */
    a1 = N * 1; a2 = N * 2; a3 = N * 3; a4 = N * 4; a5 = N * 5;
    a6 = N * 6; a7 = N * 7; a8 = N * 8; a9 = N * 9; a10 = N * 10;
    
    f1 = N * 1.1f; f2 = N * 2.2f; f3 = N * 3.3f; f4 = N * 4.4f;
    f5 = N * 5.5f; f6 = N * 6.6f; f7 = N * 7.7f; f8 = N * 8.8f;
    
    d1 = N * 1.11; d2 = N * 2.22; d3 = N * 3.33; 
    d4 = N * 4.44; d5 = N * 5.55; d6 = N * 6.66;
    
    l1 = N * 100L; l2 = N * 200L; l3 = N * 300L; 
    l4 = N * 400L; l5 = N * 500L;
    
    /* Multi-dimensional array for address reloads */
    int arr[150][150];
    
    /* Packed struct accessed through volatile pointer */
    struct Packed p;
    struct Packed *volatile pptr = &p;
    
    /* Initialize array */
    for (int i = 0; i < 150; i++) {
        for (int j = 0; j < 150; j++) {
            arr[i][j] = i * 1000 + j;
        }
    }
    
    /* Complex nested loops with many live variables */
    for (int i = 1; i < N && i < 149; i++) {
        for (int j = 1; j < N && j < 149; j++) {
            /* Create complex addressing modes */
            int idx1 = (i * j) % 149;
            int idx2 = (i + j) % 149;
            
            /* Force address reloads with computed indices */
            int temp = arr[idx1][idx2];
            arr[idx2][idx1] = arr[i][j] + temp;
            
            /* Chain arithmetic operations to keep variables live */
            a1 = a2 + a3;
            a4 = a5 * a6;
            a7 = a8 - a9;
            a10 = a1 + a4;
            
            f1 = f2 + f3;
            f4 = f5 * f6;
            f7 = f8 - f1;
            
            d1 = d2 + d3;
            d4 = d5 * d6;
            d1 = d4 - d2;
            
            l1 = l2 + l3;
            l4 = l5 * l1;
            l2 = l4 - l3;
            
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
            
            /* Float/double asm operations */
            asm volatile (
                "addss %0, %1\n\t"
                : "=x"(f1)
                : "x"(f2), "0"(f3)
            );
            
            /* Access packed struct through volatile pointer */
            pptr->i = a1 + a2;
            pptr->d = d1 + d2;
            pptr->f = f1 + f2;
            pptr->l = l1 + l2;
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different subset of variables */
                int t1 = a3 + a7;
                float t2 = f3 + f7;
                double t3 = d3 + d5;
                long t4 = l3 + l5;
                
                /* More asm in conditional path */
                asm volatile (
                    "sub %0, %1, %2\n\t"
                    : "=r"(t1)
                    : "r"(a8), "r"(a9)
                    : "cc"
                );
                
                arr[i][j] += t1 + (int)t2 + (int)t3 + t4;
            } else {
                /* Alternative path with different variables */
                int t5 = a4 + a8;
                float t6 = f4 + f8;
                double t7 = d4 + d6;
                long t8 = l4 + l1;
                
                asm volatile (
                    "and %0, %1, %2\n\t"
                    : "=r"(t5)
                    : "r"(a9), "r"(a10)
                    : "cc"
                );
                
                arr[i][j] += t5 + (int)t6 + (int)t7 + t8;
            }
            
            /* More arithmetic to create data dependencies */
            a2 = a1 * a10;
            a3 = a2 / (a4 ? a4 : 1);
            a5 = a3 + a7;
            a6 = a5 - a8;
            a9 = a6 * a2;
            
            f2 = f1 * f8;
            f3 = f2 / (f4 ? f4 : 1.0f);
            f5 = f3 + f7;
            f6 = f5 - f1;
            
            d2 = d1 * d6;
            d3 = d2 / (d4 ? d4 : 1.0);
            d5 = d3 + d1;
            
            l2 = l1 * l5;
            l3 = l2 / (l4 ? l4 : 1L);
            l5 = l3 + l1;
        }
    }
    
    /* Compute checksum using all variables */
    *checksum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    *checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8;
    *checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6;
    *checksum += (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
    
    /* Add array elements to checksum */
    for (int i = 0; i < 150 && i < N + 10; i++) {
        for (int j = 0; j < 150 && j < N + 10; j++) {
            *checksum += arr[i][j];
        }
    }
    
    /* Add packed struct members */
    *checksum += pptr->i + (int)pptr->d + (int)pptr->f + pptr->l;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int checksum = 0;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
    /* Toggle volatile flag */
    volatile_flag = rand() % 2;
    
    /* Call function that triggers reloads */
    trigger_reloads(N, &checksum);
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
