/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_seed = 42;
volatile double global_accumulator = 0.0;

/* Function 1: Deeply nested loops with many live ranges */
void nested_loop_pressure(int *data, int size) {
    int i, j, k, l;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int temp1, temp2, temp3, temp4;
    double fp1, fp2, fp3, fp4;
    
    /* Outer loops create many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        temp1 = data[i];
        for (j = 0; j < size / 8; j++) {
            temp2 = data[j] * 2;
            fp1 = sin(temp1 * 0.01) * cos(temp2 * 0.01);
            
            for (k = 0; k < size / 16; k++) {
                temp3 = data[k] + temp1 - temp2;
                fp2 = exp(fp1 * 0.001) * tan(temp3 * 0.001);
                
                for (l = 0; l < size / 32; l++) {
                    temp4 = data[l] ^ temp3;
                    fp3 = log(fabs(fp2) + 1.0) * sqrt(temp4);
                    fp4 = fp3 * fp2 * fp1;
                    
                    sum1 += (int)(fp4 * 1000);
                    sum2 += temp4;
                }
                sum3 += temp3;
            }
            sum4 += temp2;
        }
        global_accumulator += sum1 * 0.000001;
    }
    
    /* Force register pressure with many simultaneous variables */
    asm volatile("" : "+r"(sum1), "+r"(sum2), "+r"(sum3), "+r"(sum4) : : "memory");
    data[0] = sum1 + sum2 + sum3 + sum4;
}

/* Function 2: Complex control flow with switch and early returns */
int complex_control_flow(int *data, int size, int mode) {
    int result = 0;
    int i, j;
    
    switch (mode % SWITCH_CASES) {
        case 0: {
            int a = data[0], b = data[1], c = data[2];
            for (i = 0; i < size; i++) {
                if (i % 3 == 0) return a + b;
                if (i % 5 == 0) break;
                a += data[i] * 2;
            }
            result = a + b + c;
            break;
        }
        case 1: {
            double x = 0.0, y = 0.0, z = 0.0;
            for (i = 0; i < size; i += 2) {
                x += sin(data[i] * 0.01);
                y += cos(data[i+1] * 0.01);
                if (x > y) continue;
                z += x * y;
            }
            result = (int)(z * 100);
            break;
        }
        case 2:
        case 3:  /* Fall-through case */
        {
            long l1 = 0, l2 = 0, l3 = 0, l4 = 0;
            for (i = 0; i < size; i++) {
                l1 += data[i];
                if (i % 2 == 0) l2 += data[i] * 3;
                if (i % 3 == 0) l3 += data[i] * 5;
                if (i % 4 == 0) l4 += data[i] * 7;
            }
            result = (int)((l1 + l2 + l3 + l4) / 4);
            break;
        }
        case 4: {
            /* Early return with many live variables */
            int t1 = data[0], t2 = data[1], t3 = data[2];
            int t4 = data[3], t5 = data[4], t6 = data[5];
            if (t1 > t2 && t3 > t4 && t5 > t6)
                return t1 + t3 + t5;
            result = t2 + t4 + t6;
            break;
        }
        case 5: {
            /* Nested switch inside loop */
            for (i = 0; i < size; i++) {
                switch (data[i] % 5) {
                    case 0: result += data[i] * 2; break;
                    case 1: result += data[i] * 3; break;
                    case 2: result += data[i] * 4; break;
                    case 3: result += data[i] * 5; break;
                    case 4: result += data[i] * 6; break;
                }
            }
            break;
        }
        case 6 ... 14: {  /* Range of cases */
            /* Mixed data types to stress different register classes */
            char c1, c2, c3;
            short s1, s2, s3;
            int i1, i2, i3;
            float f1, f2, f3;
            double d1, d2, d3;
            
            c1 = (char)(data[0] & 0xFF);
            c2 = (char)(data[1] & 0xFF);
            c3 = (char)(data[2] & 0xFF);
            
            s1 = (short)(data[3] & 0xFFFF);
            s2 = (short)(data[4] & 0xFFFF);
            s3 = (short)(data[5] & 0xFFFF);
            
            i1 = data[6];
            i2 = data[7];
            i3 = data[8];
            
            f1 = (float)data[9] * 0.1f;
            f2 = (float)data[10] * 0.2f;
            f3 = (float)data[11] * 0.3f;
            
            d1 = (double)data[12] * 0.01;
            d2 = (double)data[13] * 0.02;
            d3 = (double)data[14] * 0.03;
            
            /* Complex expression keeping all variables live */
            result = (int)(c1 + c2 + c3 + s1 + s2 + s3 + i1 + i2 + i3 + 
                          f1 + f2 + f3 + d1 + d2 + d3);
            break;
        }
        default:
            result = -1;
    }
    
    return result;
}

/* Function 3: Inline assembly with explicit register constraints */
void asm_register_pressure(int *data, int size) {
    int i;
    int a, b, c, d, e, f;
    long long la, lb, lc;
    double da, db, dc;
    
    for (i = 0; i < size; i += 8) {
        /* Force specific register allocations */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %%eax, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a) : "r" (data[i]) : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "addl $100, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b) : "r" (data[i+1]) : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "subl $50, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (c) : "r" (data[i+2]) : "%ecx", "memory"
        );
        
        /* Compete for same registers */
        asm volatile (
            "movq %1, %%rax\n\t"
            "addq $1000, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r" (la) : "r" ((long long)data[i+3]) : "%rax", "memory"
        );
        
        asm volatile (
            "movq %1, %%rbx\n\t"
            "subq $500, %%rbx\n\t"
            "movq %%rbx, %0\n\t"
            : "=r" (lb) : "r" ((long long)data[i+4]) : "%rbx", "memory"
        );
        
        /* Floating point register pressure */
        da = (double)data[i+5];
        db = (double)data[i+6];
        
        asm volatile (
            "addsd %1, %0\n\t"
            "mulsd %2, %0\n\t"
            : "+x" (da) : "x" (db), "x" (da) : "memory"
        );
        
        /* Memory clobber to force spills */
        asm volatile("" : : : "memory");
        
        data[i] = a + b + c + (int)la + (int)lb + (int)da;
    }
}

/* Function 4: Many function arguments to stress calling convention */
int many_arguments(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   double f1, double f2, double f3, double f4, double f5) {
    /* Force all arguments to stay live */
    int sum_int = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    double sum_double = f1 + f2 + f3 + f4 + f5;
    
    /* Complex expression using all arguments */
    for (int i = 0; i < 10; i++) {
        sum_int += (int)(sum_double * i);
        sum_double += sin(sum_int * 0.01);
    }
    
    return sum_int + (int)sum_double;
}

/* Function 5: Pointer aliasing to prevent optimization */
void pointer_aliasing_test(int *data, int size) {
    int *ptr1 = data;
    int *ptr2 = data + size/2;
    int *ptr3 = data + size/4;
    int *ptr4 = data + 3*size/4;
    
    int i, j, k;
    int accum1 = 0, accum2 = 0, accum3 = 0, accum4 = 0;
    
    /* Create aliasing that confuses the optimizer */
    for (i = 0; i < size/8; i++) {
        ptr1[i] = i * 2;
        ptr2[i] = ptr1[i] + 1;  /* May alias with ptr1 */
        
        for (j = 0; j < size/16; j++) {
            ptr3[j] = ptr1[i] + ptr2[j];
            ptr4[j] = ptr3[j] * 2;
            
            /* Keep many variables live across loop iterations */
            accum1 += ptr1[i];
            accum2 += ptr2[j];
            accum3 += ptr3[j];
            accum4 += ptr4[j];
        }
    }
    
    /* Force all accumulators to be used */
    data[0] = accum1 + accum2 + accum3 + accum4;
}

/* Function 6: Vector-like operations using multiple registers */
void vector_operations(int *data, int size) {
    int i;
    int v0, v1, v2, v3, v4, v5, v6, v7;
    int w0, w1, w2, w3, w4, w5, w6, w7;
    
    for (i = 0; i < size - 16; i += 8) {
        /* Load 8 values - simulates vector load */
        v0 = data[i];
        v1 = data[i+1];
        v2 = data[i+2];
        v3 = data[i+3];
        v4 = data[i+4];
        v5 = data[i+5];
        v6 = data[i+6];
        v7 = data[i+7];
        
        /* Process all 8 values simultaneously */
        w0 = v0 * 3 + 1;
        w1 = v1 * 5 + 2;
        w2 = v2 * 7 + 3;
        w3 = v3 * 11 + 4;
        w4 = v4 * 13 + 5;
        w5 = v5 * 17 + 6;
        w6 = v6 * 19 + 7;
        w7 = v7 * 23 + 8;
        
        /* Cross-dependent operations */
        w0 += w1;
        w1 += w2;
        w2 += w3;
        w3 += w4;
        w4 += w5;
        w5 += w6;
        w6 += w7;
        w7 += w0;
        
        /* Store results */
        data[i] = w0;
        data[i+1] = w1;
        data[i+2] = w2;
        data[i+3] = w3;
        data[i+4] = w4;
        data[i+5] = w5;
        data[i+6] = w6;
        data[i+7] = w7;
    }
}

/* Main function that orchestrates all tests */
int main() {
    int *data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int i, j;
    long long checksum = 0;
    clock_t start, end;
    
    if (!data || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
        data2[i] = rand() % 1000;
    }
    
    printf("Starting register pressure tests...\n");
    start = clock();
    
    /* Warm-up phase for profile feedback */
    for (j = 0; j < ITERATIONS/10; j++) {
        nested_loop_pressure(data, ARRAY_SIZE/10);
        asm volatile("" ::: "memory");  /* Memory barrier */
    }
    
    /* Main test sequence */
    for (j = 0; j < ITERATIONS; j++) {
        /* Test 1: Nested loops */
        nested_loop_pressure(data, ARRAY_SIZE);
        checksum += data[0];
        
        /* Memory barrier between tests */
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex control flow */
        int cf_result = complex_control_flow(data2, ARRAY_SIZE, j);
        checksum += cf_result;
        
        /* Test 3: Inline assembly */
        asm_register_pressure(data, ARRAY_SIZE);
        checksum += data[ARRAY_SIZE/2];
        
        /* Test 4: Many arguments */
        int arg_result = many_arguments(
            data[0], data[1], data[2], data[3], data[4],
            data[5], data[6], data[7], data[8], data[9],
            data[10] * 0.1, data[11] * 0.2, data[12] * 0.3,
            data[13] * 0.4, data[14] * 0.5
        );
        checksum += arg_result;
        
        /* Test 5: Pointer aliasing */
        pointer_aliasing_test(data2, ARRAY_SIZE);
        checksum += data2[0];
        
        /* Test 6: Vector operations */
        vector_operations(data, ARRAY_SIZE);
        checksum += data[ARRAY_SIZE-1];
        
        /* Alternate between datasets */
        if (j % 2 == 0) {
            int *temp = data;
            data = data2;
            data2 = temp;
        }
    }
    
    end = clock();
    
    printf("Tests completed.\n");
    printf("Final checksum: %lld\n", checksum);
    printf("Time elapsed: %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    printf("Global accumulator: %f\n", global_accumulator);
    
    /* Verify with a simple computation */
    int verify = 0;
    for (i = 0; i < 100; i++) {
        verify += data[i] + data2[i];
    }
    printf("Verification sum: %d\n", verify);
    
    free(data);
    free(data2);
    
    return 0;
}
