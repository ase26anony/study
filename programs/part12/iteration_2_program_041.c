/* reload_test.c - Program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    char c;
    long l;
    float f;
};

/* Volatile variables to prevent optimization */
volatile int volatile_flag = 1;
volatile int volatile_counter = 0;

/* Target function with high register pressure */
__attribute__((noinline))
static long complex_reload_function(int N, int *checksum) {
    /* Declare many variables to exceed available registers */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    long l1 = 1000, l2 = 2000, l3 = 3000, l4 = 4000;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for secondary reloads */
    struct Packed packed_data;
    packed_data.d = 3.14159;
    packed_data.i = 42;
    packed_data.c = 'X';
    packed_data.l = 999999;
    packed_data.f = 2.718f;
    
    /* Volatile pointer to packed struct */
    volatile struct Packed *volatile_packed = &packed_data;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 17 + j * 13) % 97;
        }
    }
    
    /* Main computation loop - creates many live variables */
    for (int iter = 0; iter < N; iter++) {
        volatile_counter++;
        
        /* Complex addressing modes - forces address reloads */
        int idx1 = (iter * 7) % 128;
        int idx2 = (iter * 13) % 128;
        
        /* Array access with computed indices */
        int temp = arr[idx1][idx2];
        arr[idx2][idx1] = arr[idx1][idx2] + 1;
        
        /* Inline assembly with conflicting constraints */
        /* Force input reloads with "r" constraints */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r" (a)
            : "r" (temp)
            : "cc"
        );
        
        /* Another asm with tied operand */
        asm volatile (
            "imull %1, %0\n\t"
            : "+r" (b)
            : "r" (a), "0" (b)
            : "cc"
        );
        
        /* Mixed-type computations to use all variables */
        d1 = d1 * f1 + d2;
        d2 = d2 * f2 + d3;
        d3 = d3 * f3 + d4;
        d4 = d4 * f4 + d1;
        
        f1 = f1 + (float)d1;
        f2 = f2 + (float)d2;
        f3 = f3 + (float)d3;
        f4 = f4 + (float)d4;
        
        /* Long integer computations */
        l1 = l1 * l2 + l3;
        l2 = l2 * l3 + l4;
        l3 = l3 * l4 + l1;
        l4 = l4 * l1 + l2;
        
        /* Conditional block for optional reloads */
        if (volatile_flag) {
            /* Use different variables inside conditional */
            c = c * d + e;
            e = e * f + g;
            g = g * h + c;
            h = h * a + b;
            
            /* Access packed struct through volatile pointer */
            int packed_i = volatile_packed->i;
            float packed_f = volatile_packed->f;
            
            /* More inline asm with memory constraints */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+m" (arr[iter % 128][0])
                : "r" (packed_i)
                : "%eax", "cc"
            );
            
            f1 = f1 * packed_f;
        } else {
            /* Alternative path with different variable usage */
            a = b + c * 2;
            b = c + d * 3;
        }
        
        /* Chain computations to keep variables live */
        a = b + c;
        b = c + d;
        c = d + e;
        d = e + f;
        e = f + g;
        f = g + h;
        g = h + a;
        h = a + b;
        
        /* Force spill/reload with another asm */
        asm volatile (
            "movq %1, %%rax\n\t"
            "addq %%rax, %0\n\t"
            : "+r" (l1)
            : "r" (l2)
            : "%rax", "cc"
        );
        
        /* Access array with complex addressing */
        arr[(a + idx1) % 128][(b + idx2) % 128] += c;
    }
    
    /* Compute checksum using all variables */
    long total = 0;
    total += a + b + c + d + e + f + g + h;
    total += (long)(f1 + f2 + f3 + f4);
    total += (long)(d1 + d2 + d3 + d4);
    total += l1 + l2 + l3 + l4;
    
    /* Add array elements to checksum */
    for (int i = 0; i < 128; i += 8) {
        for (int j = 0; j < 128; j += 8) {
            total += arr[i][j];
        }
    }
    
    /* Add packed struct data */
    total += (long)volatile_packed->d;
    total += volatile_packed->i;
    total += volatile_packed->l;
    
    *checksum = (int)(total % 1000000);
    return total;
}

/* Another function to increase compilation complexity */
__attribute__((noinline))
static void additional_pressure(int N) {
    double da[8] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8};
    float fa[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    long la[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    
    for (int i = 0; i < N; i++) {
        /* Mix all arrays */
        for (int j = 0; j < 8; j++) {
            da[j] = da[j] * 1.01 + fa[j];
            fa[j] = fa[j] * 1.1f + (float)la[j];
            la[j] = la[j] + (long)da[j];
        }
        
        /* Inline asm with multiple constraints */
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "addsd %2, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m" (da[i % 8])
            : "r" (da[(i + 1) % 8]), "r" (fa[i % 8])
            : "%xmm0", "cc"
        );
    }
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    if (N < 10) N = 10;
    if (N > 10000) N = 10000;
    
    srand(time(NULL));
    
    /* Set volatile flag randomly */
    volatile_flag = rand() % 2;
    
    int checksum1 = 0, checksum2 = 0;
    
    /* Call functions to create reload scenarios */
    long result1 = complex_reload_function(N, &checksum1);
    additional_pressure(N / 2);
    
    /* Second call with different parameters */
    volatile_flag = !volatile_flag;
    long result2 = complex_reload_function(N / 2, &checksum2);
    
    /* Final computation to use results */
    long final_result = result1 + result2 + checksum1 + checksum2;
    
    printf("Result: %ld (checksums: %d, %d)\n", 
           final_result, checksum1, checksum2);
    
    return (final_result > 0) ? 0 : 1;
}
