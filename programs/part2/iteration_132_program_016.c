/* mcf_trigger.c - Program to trigger GCC's min-cost flow debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_barrier = 0;

/* Complex expression with many temporaries */
int complex_expression(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many intermediate values requiring registers */
    int t1 = a * b + c;
    int t2 = d * e - f;
    int t3 = g * h + a;
    int t4 = t1 * t2 - t3;
    int t5 = t2 * t3 + t1;
    int t6 = t3 * t1 - t2;
    int t7 = t4 * t5 + t6;
    int t8 = t5 * t6 - t4;
    int t9 = t6 * t4 + t5;
    int t10 = t7 * t8 - t9;
    
    /* Deeply nested conditionals extending live ranges */
    if (t10 > 1000) {
        int t11 = t10 * 2;
        if (t11 < 5000) {
            int t12 = t11 + t10;
            if (t12 % 2 == 0) {
                int t13 = t12 * 3;
                return t13 - t10;
            } else {
                int t14 = t12 / 2;
                return t14 + t10;
            }
        } else {
            int t15 = t11 / 3;
            return t15 * t10;
        }
    }
    return t10;
}

/* Function with switch creating complex CFG */
int switch_complex(int value, int* data, int size) {
    int result = 0;
    int i, j;
    
    /* Variables declared at function scope but used in nested blocks */
    int accumulator1 = 0;
    int accumulator2 = 0;
    int accumulator3 = 0;
    int* ptr1 = data;
    int* ptr2 = data + size/2;
    
    for (i = 0; i < size; i++) {
        /* Multiple live ranges across switch */
        int current = data[i];
        int modifier = i * 2;
        
        /* Large switch with fall-through cases */
        switch (value % SWITCH_CASES) {
            case 0:
                accumulator1 += current;
                /* Fall through */
            case 1:
                accumulator1 -= modifier;
                break;
            case 2:
                accumulator2 = current * 2;
                /* Fall through */
            case 3:
            case 4:
                accumulator2 += modifier;
                break;
            case 5:
                accumulator3 = current / 2;
                /* Fall through */
            case 6:
            case 7:
            case 8:
                accumulator3 *= modifier;
                break;
            case 9:
                *ptr1++ = current;
                /* Fall through */
            case 10:
                *ptr2++ = modifier;
                break;
            case 11:
                result += accumulator1;
                /* Fall through */
            case 12:
                result += accumulator2;
                /* Fall through */
            case 13:
                result += accumulator3;
                break;
            case 14:
                result = result * 2 - current;
                break;
            default:
                result = 0;
        }
        
        /* Nested loop with break/continue */
        for (j = 0; j < 10; j++) {
            if (j == current % 5) {
                accumulator1 += j;
                continue;
            }
            if (j == 7 && accumulator2 > 1000) {
                accumulator2 /= 2;
                break;
            }
            accumulator3 += j * current;
        }
    }
    
    /* Mix accumulators */
    return result + accumulator1 - accumulator2 + accumulator3;
}

/* Function with inline assembly creating register pressure */
void asm_register_pressure(int* input, int* output, int size) {
    int i;
    int reg_a, reg_b, reg_c, reg_d, reg_e, reg_f;
    
    for (i = 0; i < size; i += 8) {
        /* Multiple asm statements with fixed register constraints */
        /* Competing for EAX/RAX */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (reg_a)
            : "r" (input[i])
            : "%eax", "memory"
        );
        
        /* Competing for EBX/RBX */
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull $2, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (reg_b)
            : "r" (input[i+1])
            : "%ebx", "memory"
        );
        
        /* More register constraints */
        asm volatile (
            "movl %1, %%ecx\n\t"
            "subl $5, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (reg_c)
            : "r" (input[i+2])
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "xorl $0xFF, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (reg_d)
            : "r" (input[i+3])
            : "%edx", "memory"
        );
        
        /* Complex expression using all constrained registers */
        reg_e = reg_a + reg_b - reg_c * reg_d;
        
        /* Another asm clobbering multiple registers */
        asm volatile (
            "movl %1, %%esi\n\t"
            "movl %2, %%edi\n\t"
            "addl %%esi, %%edi\n\t"
            "movl %%edi, %0\n\t"
            : "=r" (reg_f)
            : "r" (reg_e), "r" (input[i+4])
            : "%esi", "%edi", "memory"
        );
        
        /* Store results, keeping variables alive */
        output[i] = reg_a;
        output[i+1] = reg_b;
        output[i+2] = reg_c;
        output[i+3] = reg_d;
        output[i+4] = reg_e;
        output[i+5] = reg_f;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
}

/* Function with mixed data types stressing register classes */
double mixed_data_types(char* cdata, short* sdata, int* idata, 
                        float* fdata, double* ddata, int size) {
    double result = 0.0;
    int i;
    
    /* Variables of different types requiring different registers */
    char c1, c2;
    short s1, s2;
    int i1, i2, i3, i4, i5, i6;
    float f1, f2, f3;
    double d1, d2, d3, d4;
    
    /* Keep variables alive across complex control flow */
    c1 = cdata[0];
    s1 = sdata[0];
    i1 = idata[0];
    f1 = fdata[0];
    d1 = ddata[0];
    
    for (i = 1; i < size; i++) {
        /* Complex addressing modes */
        c2 = cdata[i * 2 % size];
        s2 = sdata[i * 3 % size];
        i2 = idata[i * 5 % size];
        f2 = fdata[i * 7 % size];
        d2 = ddata[i * 11 % size];
        
        /* Mixed type computations */
        i3 = (int)c1 * (int)c2;
        i4 = (int)s1 + (int)s2;
        i5 = i1 * i2;
        f3 = f1 * f2;
        d3 = d1 + d2;
        
        /* Type conversions requiring moves between register classes */
        d4 = (double)i3 + (double)i4 + (double)i5 + (double)f3 + d3;
        
        /* Conditional updates extending live ranges */
        if (d4 > result) {
            result = d4;
            /* Update variables that stay alive */
            c1 = c2;
            s1 = s2;
            i1 = i2;
            f1 = f2;
            d1 = d2;
        } else if (d4 < result / 2) {
            result /= 2;
            /* Different update path */
            c1 = ~c2;
            s1 = s2 * 2;
            i1 = i2 / 2;
            f1 = f2 * 0.5f;
            d1 = d2 - 1.0;
        }
        
        /* Nested loop with early exit */
        for (int j = 0; j < 5; j++) {
            if (j == 3 && result > 1000.0) {
                result = sqrt(result);
                break;
            }
            result += 0.1 * j;
        }
    }
    
    return result;
}

/* Function with many parameters and local variables */
int many_parameters(int a, int b, int c, int d, int e, int f, int g, int h,
                    int i, int j, int k, int l, int m, int n, int o, int p) {
    /* Many local variables competing for registers */
    int v1 = a + b;
    int v2 = c - d;
    int v3 = e * f;
    int v4 = g / h;
    int v5 = i % j;
    int v6 = k & l;
    int v7 = m | n;
    int v8 = o ^ p;
    
    int v9 = v1 + v2;
    int v10 = v3 - v4;
    int v11 = v5 * v6;
    int v12 = v7 / v8;
    
    int v13 = v9 & v10;
    int v14 = v11 | v12;
    int v15 = v13 ^ v14;
    
    /* Complex control flow with early returns */
    if (v15 > 1000) {
        return v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15;
    }
    
    if (v15 < -1000) {
        return v2 + v4 + v6 + v8 + v10 + v12 + v14;
    }
    
    /* Deep nesting */
    for (int x = 0; x < 100; x++) {
        if (x % 3 == 0) {
            v1 += x;
            for (int y = 0; y < 50; y++) {
                if (y % 5 == 0) {
                    v2 -= y;
                    if (x + y > 100) {
                        v3 *= 2;
                        goto early_exit;
                    }
                }
            }
        }
    }
    
early_exit:
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

/* Main test driver */
int main() {
    int i, j;
    long checksum = 0;
    
    /* Allocate and initialize large arrays */
    int* data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* cdata = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short* sdata = (short*)malloc(ARRAY_SIZE * sizeof(short));
    float* fdata = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* ddata = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    srand(global_seed);
    
    /* Initialize with random data */
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = 0;
        cdata[i] = rand() % 256;
        sdata[i] = rand() % 10000;
        fdata[i] = (float)rand() / RAND_MAX * 100.0f;
        ddata[i] = (double)rand() / RAND_MAX * 1000.0;
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up iterations for profile feedback */
    for (j = 0; j < 5; j++) {
        for (i = 0; i < ARRAY_SIZE/100; i++) {
            checksum += complex_expression(
                data1[i], data1[i+1], data1[i+2], data1[i+3],
                data1[i+4], data1[i+5], data1[i+6], data1[i+7]
            );
        }
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    /* Test 1: Complex expressions */
    printf("Test 1: Complex expressions...\n");
    for (i = 0; i < ITERATIONS; i++) {
        int idx = i % (ARRAY_SIZE - 8);
        checksum += complex_expression(
            data1[idx], data1[idx+1], data1[idx+2], data1[idx+3],
            data1[idx+4], data1[idx+5], data1[idx+6], data1[idx+7]
        );
    }
    
    /* Test 2: Switch with complex CFG */
    printf("Test 2: Switch complex CFG...\n");
    for (i = 0; i < ITERATIONS/10; i++) {
        checksum += switch_complex(i, data1, ARRAY_SIZE/10);
    }
    
    /* Test 3: Inline assembly register pressure */
    printf("Test 3: Inline assembly...\n");
    asm_register_pressure(data1, data2, ARRAY_SIZE/2);
    for (i = 0; i < ARRAY_SIZE/2; i++) {
        checksum += data2[i];
    }
    
    /* Test 4: Mixed data types */
    printf("Test 4: Mixed data types...\n");
    double dresult = mixed_data_types(cdata, sdata, data1, fdata, ddata, ARRAY_SIZE/10);
    checksum += (long)dresult;
    
    /* Test 5: Many parameters */
    printf("Test 5: Many parameters...\n");
    for (i = 0; i < ITERATIONS; i++) {
        checksum += many_parameters(
            i, i+1, i+2, i+3, i+4, i+5, i+6, i+7,
            i+8, i+9, i+10, i+11, i+12, i+13, i+14, i+15
        );
    }
    
    /* Final computation */
    printf("Final checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(cdata);
    free(sdata);
    free(fdata);
    free(ddata);
    
    return 0;
}
