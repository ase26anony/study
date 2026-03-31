#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 128
#define ITERS 10

/* Struct to create pointer chains */
struct DataNode {
    int data[SIZE];
    struct DataNode *next;
    double values[SIZE/2];
};

/* Non-inline helper to force register usage for arguments */
__attribute__((noinline)) 
int helper_func(int a, int b, int c, int d, int e, float f, double g) {
    volatile int sink;
    sink = a + b - c * d + e;
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c) : : "memory");
    return sink + (int)(f * g);
}

int main() {
    /* Initialize complex data structures */
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    double dmatrix[SIZE][SIZE/2];
    struct DataNode nodes[4];
    volatile int vsink1, vsink2;  /* Volatile sinks for output reloads */
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * SIZE + j;
            matrix2[i][j] = i * SIZE - j;
        }
        for (int j = 0; j < SIZE/2; j++) {
            dmatrix[i][j] = i * 0.5 + j * 0.25;
        }
    }
    
    for (int n = 0; n < 4; n++) {
        nodes[n].next = (n < 3) ? &nodes[n+1] : &nodes[0];
        for (int i = 0; i < SIZE; i++) {
            nodes[n].data[i] = n * SIZE + i;
            if (i < SIZE/2) {
                nodes[n].values[i] = n * 1.5 + i * 0.75;
            }
        }
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix1, matrix2, dmatrix, nodes) \
                      map(tofrom: result)
    {
        /* Create register pressure with many local variables */
        register int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
        register int r10, r11, r12, r13, r14, r15;
        float f0, f1, f2, f3, f4, f5, f6, f7;
        double d0, d1, d2, d3, d4;
        char c0, c1, c2, c3, c4;
        int tmp1, tmp2, tmp3, tmp4, tmp5;
        
        /* Initialize from mapped arrays with complex addressing */
        r0 = matrix1[0][0];
        r1 = matrix2[0][0];
        f0 = dmatrix[0][0];
        d0 = nodes[0].values[0];
        c0 = nodes[0].data[0] & 0xFF;
        
        /* Chain computations to keep variables live */
        for (int iter = 0; iter < ITERS; iter++) {
            /* Complex array indexing forcing address reloads */
            int idx1 = (iter * 17 + r0) % SIZE;
            int idx2 = (iter * 23 + r1) % SIZE;
            int idx3 = (iter * 37 + idx1 * idx2) % (SIZE/2);
            
            /* Multi-dimensional access with complex expression */
            tmp1 = matrix1[idx1][idx2] + matrix2[idx2][idx1];
            tmp2 = matrix1[(idx1*3 + idx2) % SIZE][(idx2*5 + iter) % SIZE];
            
            /* Pointer chain access - forces base address computation */
            struct DataNode *node_ptr = &nodes[iter % 4];
            for (int chain = 0; chain < 3; chain++) {
                /* Complex addressing mode */
                tmp3 = node_ptr->data[(idx1 + chain * idx2) % SIZE];
                tmp4 = node_ptr->next->data[(idx2 + chain * iter) % SIZE];
                
                /* Mixed-type computation */
                d1 = node_ptr->values[chain] * f0 + d0;
                f1 = (float)d1 + tmp3 * 0.5f;
                
                /* Inline assembly with register constraints */
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %3, %4, %5"
                    : "=r"(r2), "+r"(r3), "+r"(r4)
                    : "r"(tmp3), "r"(tmp4), "r"(iter)
                    : "cc"
                );
                
                /* More arithmetic to chain variables */
                r5 = r2 * r3 - r4;
                r6 = r5 + tmp3 / (iter + 1);
                r7 = r6 ^ (tmp4 << 2);
                r8 = r7 & 0xFFFF;
                r9 = r8 | (iter << 16);
                
                /* Floating-point operations */
                f2 = f1 * 1.5f + (float)r5;
                f3 = f2 - f0 * 0.75f;
                f4 = f3 / (f1 + 1.0f);
                
                /* Double precision */
                d2 = d1 * 2.0 + (double)f2;
                d3 = d2 - d0 * 1.25;
                d4 = d3 / (d1 + 0.5);
                
                /* Character operations */
                c1 = (char)(r5 & 0xFF);
                c2 = c1 + (char)iter;
                c3 = c2 * c0;
                c4 = c3 - (char)(tmp3 & 0xFF);
                
                /* More register pressure variables */
                r10 = r9 + c1;
                r11 = r10 * r8 - r7;
                r12 = r11 >> (iter % 8);
                r13 = r12 ^ r6;
                r14 = r13 + r5 * 3;
                r15 = r14 - r4 / 2;
                
                /* Force output reloads with volatile assignments */
                vsink1 = r15;  /* Output to volatile - needs store */
                vsink2 = (int)d4;  /* Another volatile output */
                
                /* Assignment to array element with computed index */
                matrix1[(iter + chain) % SIZE][idx1] = r15;
                
                /* Mixed-type inline asm for potential secondary reloads */
                int ival = r15;
                float fval;
                /* This might need secondary reload on some archs */
                asm volatile (
                    "fmov %s0, %w1"
                    : "=w"(fval)
                    : "r"(ival)
                );
                f5 = fval * f4;
                
                /* Another asm with memory constraint */
                int memtmp;
                asm volatile (
                    "ldr %0, [%1]\n\t"
                    "add %0, %0, #1\n\t"
                    "str %0, [%1]"
                    : "=r"(memtmp)
                    : "r"(&tmp5)
                    : "memory"
                );
                
                /* Call helper function - forces calling convention handling */
                tmp5 = helper_func(r10, r11, r12, r13, r14, f5, d4);
                
                /* Update node pointer for next iteration */
                node_ptr = node_ptr->next;
            }
            
            /* Accumulate result */
            result += r15 + (int)f5 + tmp5;
            
            /* Rotate register values to keep them live */
            int rot_tmp = r0;
            r0 = r1; r1 = r2; r2 = r3; r3 = r4; r4 = r5;
            r5 = r6; r6 = r7; r7 = r8; r8 = r9; r9 = r10;
            r10 = r11; r11 = r12; r12 = r13; r13 = r14; r14 = r15;
            r15 = rot_tmp;
            
            float frot = f0;
            f0 = f1; f1 = f2; f2 = f3; f3 = f4; f4 = f5;
            f5 = f6; f6 = f7; f7 = frot;
            
            double drot = d0;
            d0 = d1; d1 = d2; d2 = d3; d3 = d4; d4 = drot;
            
            char crot = c0;
            c0 = c1; c1 = c2; c2 = c3; c3 = c4; c4 = crot;
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}
