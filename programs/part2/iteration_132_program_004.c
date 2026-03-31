/* mcf_test.c - Test program to trigger min-cost flow debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define WARMUP 10

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(int *data, int size) {
    int i, j, k, l, m;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0;
    
    /* Outer loops create many overlapping live ranges */
    for (i = 0; i < size / 10; i++) {
        tmp1 = data[i] * 2;
        for (j = 0; j < size / 20; j++) {
            tmp2 = data[j] + tmp1;
            for (k = 0; k < size / 40; k++) {
                tmp3 = data[k] - tmp2;
                sum1 += tmp3;
                for (l = 0; l < size / 80; l++) {
                    tmp4 = data[l] ^ tmp3;
                    sum2 += tmp4;
                    for (m = 0; m < size / 160; m++) {
                        tmp5 = data[m] | tmp4;
                        sum3 += tmp5;
                        acc1 += sin(tmp5 * 0.01);
                    }
                    acc2 += cos(tmp4 * 0.01);
                }
                acc3 += tan(tmp3 * 0.01);
            }
            sum4 += tmp2 * tmp1;
        }
        sum5 += tmp1 * i;
    }
    
    /* Force all values to be used */
    global_counter += sum1 + sum2 + sum3 + sum4 + sum5;
    global_accumulator += acc1 + acc2 + acc3;
}

/* Function 2: Complex control flow with switch and early returns */
int test_complex_cfg(int *data, int size) {
    int result = 0;
    int i, j;
    
    for (i = 0; i < size; i++) {
        /* Multiple nested if-else chains */
        if (data[i] < 0) {
            if (data[i] < -100) {
                result -= 10;
                if (data[i] < -200) return result; /* Early return */
            } else {
                result -= 1;
            }
        } else if (data[i] > 0) {
            if (data[i] > 100) {
                result += 10;
                if (data[i] > 200) {
                    /* Nested switch inside if */
                    switch (data[i] % 10) {
                        case 0: result += 100; break;
                        case 1: result += 50; break;
                        case 2: result += 25; break;
                        case 3: result += 12; break;
                        case 4: result += 6; break;
                        case 5: result += 3; break;
                        case 6: result += 1; break;
                        case 7: result -= 1; break;
                        case 8: result -= 3; break;
                        case 9: result -= 6; break;
                        default: result -= 12; break;
                    }
                }
            } else {
                result += 1;
            }
        }
        
        /* Loop with break/continue at different levels */
        for (j = 0; j < 5; j++) {
            if (data[i] % 2 == 0) {
                if (j == 2) break;
                result += j;
            } else {
                if (j == 3) continue;
                result -= j;
            }
            
            /* Another level of nesting */
            switch (j) {
                case 0: result *= 2; break;
                case 1: result /= 2; break;
                case 2: result <<= 1; break;
                case 3: result >>= 1; break;
                case 4: result ^= 0xFF; break;
            }
        }
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_asm_constraints(int *data, int size) {
    int i;
    int a, b, c, d, e, f;
    long long la, lb, lc;
    double da, db, dc;
    
    for (i = 0; i < size; i += 8) {
        /* Force use of specific registers */
        a = data[i];
        b = data[i+1];
        c = data[i+2];
        d = data[i+3];
        e = data[i+4];
        f = data[i+5];
        
        /* Compete for EAX/RAX register */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (b), "r" (c)
            : "%eax", "memory"
        );
        
        /* Compete for EBX/RBX register */
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull %2, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b)
            : "r" (c), "r" (d)
            : "%ebx", "memory"
        );
        
        /* Use multiple register constraints */
        asm volatile (
            "movq %1, %%rax\n\t"
            "movq %2, %%rbx\n\t"
            "addq %%rbx, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r" (la)
            : "r" ((long long)a), "r" ((long long)b)
            : "%rax", "%rbx", "memory"
        );
        
        /* Floating point register pressure */
        da = (double)a;
        db = (double)b;
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "addsd %2, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=x" (dc)
            : "x" (da), "x" (db)
            : "%xmm0", "memory"
        );
        
        data[i] = a + b + (int)la + (int)dc;
    }
}

/* Function 4: Mixed data types and many function arguments */
double test_mixed_types(char c, short s, int i, long l, float f, double d,
                       int *p1, int *p2, int *p3, int *p4, int *p5) {
    /* Different register classes for different types */
    char c1 = c + 1;
    short s1 = s * 2;
    int i1 = i / 3;
    long l1 = l - 4;
    float f1 = f * 1.5f;
    double d1 = d / 2.0;
    
    /* Pointer arithmetic stresses address registers */
    int sum = *p1 + *p2 + *p3 + *p4 + *p5;
    int diff = *p1 - *p2 + *p3 - *p4 + *p5;
    int prod = (*p1) * (*p2) * (*p3);
    
    /* Mixed type computations */
    double result = (double)c1 + (double)s1 + (double)i1 + (double)l1 +
                   (double)f1 + d1 + (double)sum + (double)diff + (double)prod;
    
    /* Complex expression with many intermediates */
    result = sin(result) * cos(result) + tan(result * 0.01) -
            exp(result * 0.001) * log(fabs(result) + 1.0);
    
    return result;
}

/* Function 5: Vector operations using SIMD registers */
void test_vector_ops(int *data, int size) {
    int i;
    int v1[4], v2[4], v3[4], v4[4];
    float fv1[4], fv2[4], fv3[4];
    double dv1[2], dv2[2], dv3[2];
    
    for (i = 0; i < size - 16; i += 4) {
        /* Load vectors */
        v1[0] = data[i];     v1[1] = data[i+1];   v1[2] = data[i+2];   v1[3] = data[i+3];
        v2[0] = data[i+4];   v2[1] = data[i+5];   v2[2] = data[i+6];   v2[3] = data[i+7];
        v3[0] = data[i+8];   v3[1] = data[i+9];   v3[2] = data[i+10];  v3[3] = data[i+11];
        v4[0] = data[i+12];  v4[1] = data[i+13];  v4[2] = data[i+14];  v4[3] = data[i+15];
        
        /* SIMD-like operations */
        for (int j = 0; j < 4; j++) {
            v1[j] = v1[j] + v2[j] * v3[j] - v4[j];
            fv1[j] = (float)v1[j] * 0.5f;
            fv2[j] = (float)v2[j] * 1.5f;
            fv3[j] = fv1[j] + fv2[j] * 2.0f - (float)v3[j];
        }
        
        /* Double precision vector */
        dv1[0] = (double)v1[0]; dv1[1] = (double)v1[1];
        dv2[0] = (double)v2[0]; dv2[1] = (double)v2[1];
        dv3[0] = dv1[0] * dv2[0] + dv1[1] * dv2[1];
        dv3[1] = dv1[0] / (dv2[0] + 1.0) - dv1[1] / (dv2[1] + 1.0);
        
        /* Store results back */
        data[i] = v1[0] + (int)fv3[0] + (int)dv3[0];
        data[i+1] = v1[1] + (int)fv3[1] + (int)dv3[1];
    }
}

/* Function 6: Variable scoping that extends live ranges */
long test_extended_liveranges(int *data, int size) {
    /* Variables declared at function scope but used in nested blocks */
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    long total = 0;
    int *ptr1, *ptr2, *ptr3;
    
    ptr1 = &a;
    ptr2 = &b;
    ptr3 = &c;
    
    for (int i = 0; i < size; i++) {
        /* Keep variables alive through pointer aliasing */
        *ptr1 = data[i];
        *ptr2 = data[i] * 2;
        *ptr3 = data[i] / 2;
        
        if (i % 3 == 0) {
            d = *ptr1 + *ptr2;
            e = *ptr2 - *ptr3;
            f = *ptr1 * *ptr3;
        } else if (i % 3 == 1) {
            g = d * e;
            h = f + g;
        } else {
            /* Deeply nested block */
            {
                int x = g + h;
                int y = x * d;
                int z = y / (e + 1);
                total += z;
                
                /* Pointer update creates anti-dependencies */
                ptr1 = &x;
                ptr2 = &y;
                ptr3 = &z;
            }
        }
        
        /* Use all variables to prevent dead store elimination */
        total += a + b + c + d + e + f + g + h;
    }
    
    return total;
}

/* Main function that orchestrates all tests */
int main() {
    int *data1, *data2, *data3;
    int i, j;
    long checksum = 0;
    double result;
    
    /* Allocate and initialize data */
    data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000 - 500;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 256;
    }
    
    printf("Starting register pressure stress test...\n");
    
    /* Warm-up phase for profile feedback */
    for (j = 0; j < WARMUP; j++) {
        test_nested_loops(data1, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    /* Main test iterations */
    for (j = 0; j < ITERATIONS; j++) {
        /* Test 1: Nested loops */
        test_nested_loops(data1, ARRAY_SIZE / 10);
        checksum += global_counter;
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex CFG */
        checksum += test_complex_cfg(data2, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        test_asm_constraints(data3, ARRAY_SIZE);
        for (i = 0; i < 100; i++) checksum += data3[i];
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed types with many arguments */
        result = test_mixed_types(
            (char)(j % 256), (short)j, j, (long)j * 10,
            (float)j * 0.1f, (double)j * 0.01,
            &data1[0], &data1[100], &data1[200],
            &data1[300], &data1[400]
        );
        checksum += (long)result;
        asm volatile("" ::: "memory");
        
        /* Test 5: Vector operations */
        test_vector_ops(data2, ARRAY_SIZE);
        for (i = 0; i < 100; i++) checksum += data2[i];
        asm volatile("" ::: "memory");
        
        /* Test 6: Extended live ranges */
        checksum += test_extended_liveranges(data1, ARRAY_SIZE / 5);
        asm volatile("" ::: "memory");
    }
    
    printf("Final checksum: %ld\n", checksum);
    printf("Global accumulator: %f\n", global_accumulator);
    
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
