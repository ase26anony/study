#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 128
#define ITERS 16

struct DataNode {
    int data[SIZE];
    double weights[SIZE];
    struct DataNode *next;
};

/* Non-inline helper to force register usage for arguments */
__attribute__((noinline)) 
int compute_checksum(int a, int b, int c, int d, int e, float f, double g) {
    volatile int sink;
    int result = (a * b) + (c / (d ? d : 1)) - e;
    result += (int)(f * 100.0f);
    result += (int)(g * 10.0);
    
    /* Force store with register pointer */
    int *ptr = &result;
    asm volatile("" : "+r"(ptr) : : "memory");
    sink = *ptr;
    
    return result;
}

int main() {
    /* Initialize multi-dimensional arrays */
    int matrix[SIZE][SIZE];
    double dbl_arr[SIZE][SIZE];
    char char_arr[SIZE][SIZE];
    struct DataNode nodes[4];
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            dbl_arr[i][j] = (i * 0.1) + (j * 0.01);
            char_arr[i][j] = (i + j) & 0xFF;
        }
    }
    
    for (int n = 0; n < 4; n++) {
        for (int i = 0; i < SIZE; i++) {
            nodes[n].data[i] = (n * SIZE) + i;
            nodes[n].weights[i] = (n * 0.5) + (i * 0.01);
        }
        nodes[n].next = (n < 3) ? &nodes[n + 1] : NULL;
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix, dbl_arr, char_arr, nodes) \
                      map(tofrom: result)
    {
        /* Create massive register pressure with many live scalars */
        register int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
        register float f0, f1, f2, f3, f4, f5;
        register double d0, d1, d2, d3, d4;
        register char c0, c1, c2, c3;
        volatile int vsink;
        volatile double vdsink;
        
        /* Initialize from arrays with complex addressing */
        r0 = matrix[(0 * SIZE + 1) % SIZE][2];
        r1 = matrix[3][(4 * 5 + 6) % SIZE];
        r2 = nodes[0].data[7] + nodes[1].data[8];
        r3 = matrix[9][10] * matrix[11][12];
        r4 = (int)dbl_arr[13][14];
        r5 = char_arr[15][16] * 256;
        r6 = nodes[2].data[17] - nodes[3].data[18];
        r7 = matrix[19][20] / (matrix[21][22] ? matrix[21][22] : 1);
        r8 = (int)(nodes[0].weights[23] * 100.0);
        r9 = matrix[24][25] | matrix[26][27];
        
        f0 = (float)dbl_arr[28][29];
        f1 = (float)matrix[30][31] * 0.01f;
        f2 = f0 + f1;
        f3 = (float)char_arr[32][33];
        f4 = f2 * f3;
        f5 = (float)nodes[1].weights[34];
        
        d0 = dbl_arr[35][36];
        d1 = (double)matrix[37][38];
        d2 = d0 * d1;
        d3 = nodes[2].weights[39];
        d4 = d2 + d3;
        
        c0 = char_arr[40][41];
        c1 = char_arr[42][43];
        c2 = c0 + c1;
        c3 = char_arr[44][45] - c2;
        
        /* Complex nested loops with register pressure */
        for (int i = 0; i < ITERS; i++) {
            for (int j = 0; j < ITERS; j++) {
                /* Complex array indexing forcing address reloads */
                int idx1 = (i * SIZE + j * 3 + r0) % SIZE;
                int idx2 = (j * SIZE + i * 7 + r1) % SIZE;
                int idx3 = (i * 11 + j * 13) % SIZE;
                
                /* Force output reloads with pointer dereference */
                int *ptr1 = &matrix[idx1][idx2];
                double *ptr2 = &dbl_arr[idx2][idx1];
                
                /* Inline assembly with register constraints */
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %3, %4, %5"
                    : "=r"(r0), "+r"(r1), "+r"(r2)
                    : "r"(r3), "r"(r4), "r"(r5)
                    : "cc"
                );
                
                /* Mixed-type operations forcing mode conversions */
                f0 = (float)r6 + f1;
                d0 = (double)f0 * d1;
                
                /* Secondary reload triggers: move between register classes */
                #ifdef __aarch64__
                asm volatile (
                    "fmov %s0, %w1\n\t"
                    : "=w"(f2)
                    : "r"(r7)
                );
                #else
                /* x86 version forcing FP register usage */
                asm volatile (
                    "movd %0, %1\n\t"
                    : "=x"(f2)
                    : "r"(r7)
                );
                #endif
                
                /* More inline assembly with memory constraints */
                asm volatile (
                    "mov %0, %1\n\t"
                    "add %2, %3, %4"
                    : "=r"(r8), "=m"(*ptr1)
                    : "r"(r9), "r"(c0), "r"(c1)
                    : "memory"
                );
                
                /* Volatile stores forcing output reloads */
                vsink = r0 + r1 + r2;
                vdsink = d0 * d1 + d2;
                
                /* Complex chain keeping all variables live */
                r3 = r4 * r5 + r6;
                r4 = r7 / (r8 ? r8 : 1);
                r5 = r9 & r0;
                r6 = r1 | r2;
                r7 = r3 ^ r4;
                
                f3 = f4 * f5 + f0;
                f4 = f1 - f2;
                f5 = f3 / (f4 != 0.0f ? f4 : 1.0f);
                
                d2 = d3 * d4 + d0;
                d3 = d1 - d2;
                d4 = d3 * 2.0;
                
                c2 = c3 + c0;
                c3 = c1 - c2;
                
                /* Assignment with computed index - forces address reload */
                matrix[(i * 3 + j * 5) % SIZE][(j * 7 + i * 11) % SIZE] = r3;
                dbl_arr[idx3][(idx1 + idx2) % SIZE] = d2;
                char_arr[(idx2 * 2) % SIZE][(idx1 * 3) % SIZE] = c2;
                
                /* Pointer chain access */
                struct DataNode *node_ptr = &nodes[i % 4];
                int data_idx = (j * 17 + i * 19) % SIZE;
                r9 = node_ptr->data[data_idx];
                d0 = node_ptr->weights[data_idx];
                
                if (node_ptr->next) {
                    f0 = (float)node_ptr->next->data[(data_idx + 1) % SIZE];
                }
            }
            
            /* Function call forcing register arguments */
            int checksum = compute_checksum(r0, r1, r2, r3, r4, f0, d0);
            result += checksum;
            
            /* More operations to maintain live ranges */
            asm volatile (
                "imul %0, %1, %2\n\t"
                : "=r"(r0)
                : "r"(r5), "r"(r6)
            );
            
            #ifdef __x86_64__
            /* x86 specific constraint for secondary reload */
            asm volatile (
                "cvtsi2sd %1, %0\n\t"
                : "=x"(d1)
                : "r"(r7)
            );
            #endif
        }
        
        /* Final volatile store with complex address */
        int final_idx = (r0 * r1 + r2 * r3) % SIZE;
        vsink = matrix[final_idx][(final_idx * 2) % SIZE];
        vdsink = dbl_arr[(final_idx * 3) % SIZE][(final_idx * 5) % SIZE];
    }
    
    printf("Result: %d\n", result);
    return 0;
}
