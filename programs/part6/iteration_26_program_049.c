/* test_early_remat.c - Test program to trigger GCC's early rematerialization */
/* Compile with: gcc -O2 -c test_early_remat.c -fdump-rtl-early_remat -da */
/* Or for coverage: gcc -O2 -c test_early_remat.c -fprofile-arcs -ftest-coverage */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Pure function attribute to encourage rematerialization */
static int __attribute__((const)) pure_helper(int x, int y) {
    return (x * 3 + y * 7) & 0xFF;
}

/* Another pure function with different mode */
static double __attribute__((const)) pure_double(double x, double y) {
    return x * 3.14159 + y * 2.71828;
}

/* Structure with mixed types to create varied register modes */
struct MixedData {
    int id;
    double value;
    float fval;
    char* name;
    long counter;
};

/* Hot function 1: Creates high integer register pressure */
static long hot_function1(struct MixedData* data, int size) {
    long total = 0;
    int i, j;
    
    /* Outer loop creates values that inner loop needs but can't keep in registers */
    for (i = 0; i < size; i++) {
        /* These computations are cheap but create register pressure */
        int base = pure_helper(data[i].id, i);
        int offset = (base * 3) / 2;
        int scale = (offset + 7) & 0xF;
        
        /* Inner loop with high pressure - prevents keeping values in registers */
        for (j = 0; j < 8; j++) {
            /* Multiple uses of 'base' that can't all stay in registers */
            int val1 = base + j * scale;
            int val2 = base * j + offset;
            int val3 = pure_helper(base, j);  /* Candidate for remat */
            
            /* Complex expression using all values */
            total += val1 * val2 - val3;
            
            /* Force non-linear control flow with goto */
            if (val1 > 100) {
                goto skip_point;
            }
            
            /* More computations */
            val2 = pure_helper(val2, val3);  /* Another candidate */
            total += val2;
            
        skip_point:
            /* Continue with more computations */
            val3 = pure_helper(val1, val2);
            total += val3;
        }
    }
    
    return total;
}

/* Hot function 2: Mixes integer and floating point modes */
static double hot_function2(struct MixedData* data, int size) {
    double result = 0.0;
    int i;
    
    /* Use pragma to ensure optimization level */
    #pragma GCC optimize ("O2")
    for (i = 0; i < size; i++) {
        /* Create floating point register pressure */
        double dbase = pure_double(data[i].value, i);
        float fbase = (float)dbase;
        
        /* Integer computations intermixed */
        int ibase = data[i].id;
        long lbase = data[i].counter;
        
        /* Multiple uses of dbase that might need rematerialization */
        double temp1 = dbase * 2.0;
        double temp2 = dbase / 3.0;
        double temp3 = pure_double(dbase, temp1);  /* Candidate */
        
        /* Complex expression with mixed modes */
        result += temp1 * temp2 - temp3 + ibase + lbase;
        
        /* Array indexing creates address computations */
        int idx = (ibase * 3 + (int)fbase) % size;
        if (idx >= 0 && idx < size) {
            result += data[idx].value * fbase;
        }
        
        /* Use inline assembly to force specific register usage */
        asm volatile ("# Force register usage %0" : : "r" (ibase));
    }
    
    return result;
}

/* Hot function 3: Complex control flow with switch/goto */
static int hot_function3(int* array, int size) {
    int sum = 0;
    int i = 0;
    
    /* Non-trivial loop with goto-based control flow */
    loop_start:
    if (i >= size) goto loop_end;
    
    /* Create value used multiple times */
    int base_val = pure_helper(array[i], i);
    int scaled = base_val * 3;
    
    /* Switch creates complex CFG */
    switch (base_val & 0x3) {
        case 0:
            sum += base_val + scaled;
            /* Fall through */
        case 1:
            sum += pure_helper(scaled, base_val);  /* Candidate */
            i += 2;
            goto loop_start;
        case 2:
            sum += base_val * 2;
            i += 1;
            goto loop_start;
        case 3:
            sum += scaled / 2;
            i += 3;
            goto loop_start;
    }
    
    loop_end:
    
    /* Another loop with register pressure */
    for (int j = 0; j < 4; j++) {
        /* Recompute base_val in different way */
        int new_base = pure_helper(sum, j);
        int temp1 = new_base + j;
        int temp2 = new_base * j;
        
        /* Use both temps multiple times */
        sum += temp1 * temp2;
        sum -= pure_helper(temp1, temp2);  /* Candidate */
        sum += temp1 + temp2;
    }
    
    return sum;
}

/* Hot function 4: Uses register variables to increase pressure */
static long hot_function4(struct MixedData* data, int size) {
    register long r1 asm ("r12");  /* Hint to use specific register */
    register int r2 asm ("r13");
    register double r3 asm ("xmm0");
    
    r1 = 0;
    
    for (int i = 0; i < size; i++) {
        /* Force many values to compete for registers */
        int a = data[i].id;
        int b = pure_helper(a, i);
        int c = b * 3 + a;
        int d = pure_helper(c, b);  /* Candidate */
        int e = d + c;
        int f = pure_helper(e, d);  /* Candidate */
        
        /* Use all variables in complex expression */
        r2 = a + b - c + d - e + f;
        
        /* Floating point computations intermixed */
        r3 = data[i].value;
        double g = pure_double(r3, i);
        double h = g * 2.0;
        double k = pure_double(h, g);  /* Candidate */
        
        /* Mix integer and float */
        r1 += r2 + (long)(g + h + k);
        
        /* Array access with complex addressing */
        int idx = (a * b) % size;
        if (idx != i) {
            r1 += data[idx].counter;
        }
    }
    
    return r1;
}

/* Main function that orchestrates everything */
int main(int argc, char** argv) {
    const int SIZE = 100;
    struct MixedData* data;
    long result1, result3, result4;
    double result2;
    
    /* Allocate and initialize data */
    data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    if (!data) return 1;
    
    for (int i = 0; i < SIZE; i++) {
        data[i].id = i;
        data[i].value = sin(i * 0.1) * 100.0;
        data[i].fval = (float)data[i].value;
        data[i].name = "test";
        data[i].counter = i * i;
    }
    
    /* Create integer array for function3 */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i * 3 + 7;
    }
    
    /* Call hot functions to trigger rematerialization */
    result1 = hot_function1(data, SIZE);
    result2 = hot_function2(data, SIZE);
    result3 = hot_function3(int_array, SIZE);
    result4 = hot_function4(data, SIZE);
    
    /* Combine results in non-trivial way to prevent optimization */
    long final_result = result1 + (long)result2 + result3 + result4;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %ld\n", final_result);
    
    /* Cleanup */
    free(data);
    free(int_array);
    
    return 0;
}
