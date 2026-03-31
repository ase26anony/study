/* reload_stress_test.c
 * A program designed to trigger complex reload scenarios in GCC's reload pass
 * Specifically targets uncovered lines in reload.cc (1381-1399)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define DIM 16

/* Struct to create complex addressing patterns */
typedef struct {
    int data[SIZE];
    double matrix[DIM][DIM];
    char *ptr;
    volatile int volatile_member;
} ComplexStruct;

/* Non-inline helper to force register usage for arguments */
__attribute__((noinline)) 
double compute_helper(double a, double b, int c, float d, long e) {
    volatile double result;
    /* Force register-to-register moves with mixed types */
    asm volatile ("# Helper computation begin" ::: "memory");
    result = a * b + c / (d + 1.0) - e;
    asm volatile ("# Helper computation end" ::: "memory");
    return result;
}

/* Another helper with specific register constraints */
__attribute__((noinline))
int register_pressure_helper(int a, int b, int c, int d, int e, 
                             int f, int g, int h, int i, int j) {
    /* Force many live registers */
    int sum = a + b + c + d + e + f + g + h + i + j;
    
    /* Inline asm with specific constraints to force reloads */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4"
        : "=r"(sum)
        : "r"(a), "r"(b), "r"(c), "r"(sum)
        : "cc"
    );
    
    return sum;
}

int main() {
    /* Initialize complex data structures */
    ComplexStruct cs1, cs2;
    int multi_array[DIM][DIM][DIM];
    double result_matrix[DIM][DIM];
    volatile int global_sink = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        cs1.data[i] = i * 3;
        cs2.data[i] = i * 7;
    }
    
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            cs1.matrix[i][j] = (i + j) * 1.5;
            cs2.matrix[i][j] = (i - j) * 2.5;
            for (int k = 0; k < DIM; k++) {
                multi_array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    cs1.ptr = (char*)cs1.data;
    cs2.ptr = (char*)cs2.data;
    
    int checksum = 0;
    
    /* OpenMP target region - creates device code that needs register allocation */
    #pragma omp target map(to: cs1, cs2, multi_array) map(from: result_matrix, checksum) \
                       map(tofrom: global_sink)
    {
        /* Create massive register pressure with many local variables */
        register int r0 asm("r0") = cs1.data[0];  /* Hint at register allocation */
        int r1 = cs1.data[1];
        int r2 = cs1.data[2];
        int r3 = cs1.data[3];
        int r4 = cs1.data[4];
        int r5 = cs1.data[5];
        int r6 = cs1.data[6];
        int r7 = cs1.data[7];
        int r8 = cs1.data[8];
        int r9 = cs1.data[9];
        int r10 = cs1.data[10];
        int r11 = cs1.data[11];
        int r12 = cs1.data[12];
        int r13 = cs1.data[13];
        int r14 = cs1.data[14];
        int r15 = cs1.data[15];
        
        /* Mixed types for mode conversion reloads */
        float f0 = cs1.matrix[0][0];
        float f1 = cs1.matrix[0][1];
        float f2 = cs1.matrix[1][0];
        float f3 = cs1.matrix[1][1];
        double d0 = cs2.matrix[0][0];
        double d1 = cs2.matrix[0][1];
        double d2 = cs2.matrix[1][0];
        double d3 = cs2.matrix[1][1];
        char c0 = cs1.ptr[0];
        char c1 = cs1.ptr[1];
        char c2 = cs1.ptr[2];
        char c3 = cs1.ptr[3];
        
        /* Long live range chain computation */
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                /* Complex addressing mode - forces address reloads */
                int idx = (i * DIM + j * 3) % SIZE;
                int idx2 = (i * 7 + j * 11) % SIZE;
                int idx3 = (i * 13 + j * 17) % DIM;
                
                /* Chain computations keeping many variables live */
                int temp1 = r0 + r1 + r2 + r3;
                int temp2 = r4 + r5 + r6 + r7;
                int temp3 = r8 + r9 + r10 + r11;
                int temp4 = r12 + r13 + r14 + r15;
                
                /* Mixed type computations forcing mode conversions */
                double mixed1 = d0 + f0 + c0 + temp1;
                double mixed2 = d1 + f1 + c1 + temp2;
                float mixed3 = f2 + c2 + temp3;
                float mixed4 = f3 + c3 + temp4;
                
                /* Complex array access with multiple dimensions */
                int arr_val = multi_array[i][j][idx3] + 
                             multi_array[idx3][i][j] +
                             multi_array[j][idx3][i];
                
                /* Struct member access with pointer chain */
                int struct_val = cs1.data[idx] + cs2.data[idx2];
                
                /* Inline assembly with register constraints */
                int asm_result;
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "mul %0, %0, %3\n\t"
                    "sub %0, %0, %4"
                    : "=r"(asm_result)
                    : "r"(temp1), "r"(temp2), "r"(arr_val), "r"(struct_val)
                    : "cc"
                );
                
                /* Force output reload with volatile store */
                volatile int volatile_sink;
                volatile_sink = asm_result;
                global_sink += volatile_sink;
                
                /* More inline asm with memory constraints */
                double fp_result;
                asm volatile (
                    "fadd %d0, %d1, %d2\n\t"
                    "fmul %d0, %d0, %d3"
                    : "=w"(fp_result)  /* 'w' constraint for FP/SIMD reg on AArch64 */
                    : "w"(mixed1), "w"(mixed2), "w"(d2)
                    : 
                );
                
                /* Assignment to array element with computed index - forces out reload */
                result_matrix[i][j] = fp_result + mixed3 + mixed4;
                
                /* Call helper function forcing register usage for arguments */
                double helper_result = compute_helper(
                    mixed1, mixed2, asm_result, mixed3, arr_val
                );
                
                /* Update many live variables to extend their live ranges */
                r0 = r0 ^ temp1;
                r1 = r1 ^ temp2;
                r2 = r2 ^ temp3;
                r3 = r3 ^ temp4;
                f0 = f0 + mixed3;
                f1 = f1 + mixed4;
                d0 = d0 + helper_result;
                d1 = d1 + fp_result;
                
                /* Secondary reload trigger: move between register classes */
                int int_from_fp;
                asm volatile (
                    "# Simulate cross-register-class move\n\t"
                    "fmov %w0, %s1"
                    : "=r"(int_from_fp)
                    : "w"(f0)  /* 'w' for FP, 'r' for general */
                    :
                );
                
                c0 = c0 + (char)int_from_fp;
                
                /* Complex addressing with struct pointer */
                int* data_ptr = cs1.data;
                data_ptr[idx] = data_ptr[idx] + asm_result;
                
                /* Another complex store */
                cs2.matrix[i][j] = result_matrix[i][j] * 0.5;
                
                /* Register pressure helper with many arguments */
                int reg_pressure_result = register_pressure_helper(
                    r0, r1, r2, r3, r4, r5, r6, r7, r8, r9
                );
                
                checksum += reg_pressure_result + int_from_fp + volatile_sink;
            }
        }
        
        /* Additional stress: unrolled loop with many temporaries */
        for (int i = 0; i < 8; i++) {
            /* Create many temporary variables in unrolled iterations */
            int t0 = cs1.data[i*2];
            int t1 = cs1.data[i*2+1];
            int t2 = cs2.data[i*2];
            int t3 = cs2.data[i*2+1];
            float ft0 = cs1.matrix[i][0];
            float ft1 = cs1.matrix[i][1];
            double dt0 = cs2.matrix[i][0];
            double dt1 = cs2.matrix[i][1];
            
            /* Chain them together */
            int chain1 = t0 + t1 + t2 + t3;
            float chain2 = ft0 + ft1 + chain1;
            double chain3 = dt0 + dt1 + chain2;
            
            /* Force store with complex addressing */
            result_matrix[i % DIM][(i+1) % DIM] += chain3;
            
            /* Volatile store to force output reload */
            volatile double volatile_double;
            volatile_double = chain3;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global sink: %d\n", global_sink);
    
    /* Verify some results */
    double total = 0;
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            total += result_matrix[i][j];
        }
    }
    printf("Result matrix total: %f\n", total);
    
    return 0;
}
