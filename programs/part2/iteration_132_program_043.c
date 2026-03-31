/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int vol_global_counter = 0;
volatile double vol_global_sum = 0.0;

/* Complex expression with many intermediate values */
int complex_expression_test(int *data, int size) {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int result = 0;
    
    /* Declare at function scope but use in nested loops */
    int temp1, temp2, temp3, temp4, temp5;
    double fp_temp1, fp_temp2, fp_temp3;
    
    for (int idx = 0; idx < size; idx++) {
        /* Deeply nested calculations creating many live ranges */
        a = data[idx] * 2;
        b = a + idx;
        c = b * 3;
        d = c - a;
        e = d / 2;
        f = e * e;
        g = f % 17;
        h = g << 2;
        i = h >> 1;
        j = i | 0xFF;
        k = j & 0x0F;
        l = k ^ idx;
        m = l * l;
        n = m + a + b + c;
        o = n - d - e;
        p = o * f;
        q = p / (g + 1);
        r = q % 19;
        s = r << 3;
        t = s >> 2;
        
        /* Floating point calculations to stress different register classes */
        fp_temp1 = sin(idx * 0.01);
        fp_temp2 = cos(idx * 0.02);
        fp_temp3 = fp_temp1 * fp_temp2;
        
        /* Keep variables alive across complex control flow */
        temp1 = a + b;
        temp2 = c + d;
        
        /* Complex control flow with early returns */
        if (idx % 7 == 0) {
            temp3 = e + f;
            if (idx % 11 == 0) {
                temp4 = g + h;
                if (idx % 13 == 0) {
                    temp5 = i + j;
                    result += temp1 + temp2 + temp3 + temp4 + temp5;
                    continue;
                }
                result += temp1 + temp2 + temp3 + temp4;
                continue;
            }
            result += temp1 + temp2 + temp3;
            continue;
        }
        
        /* Switch statement with many cases */
        switch (idx % SWITCH_CASES) {
            case 0: result += t; break;
            case 1: result += s; break;
            case 2: result += r; break;
            case 3: result += q; break;
            case 4: result += p; break;
            case 5: result += o; break;
            case 6: result += n; break;
            case 7: result += m; break;
            case 8: result += l; break;
            case 9: result += k; break;
            case 10: result += j; break;
            case 11: result += i; break;
            case 12: result += h; break;
            case 13: result += g; break;
            case 14: result += f; break;
            default: result += a + b + c + d + e;
        }
        
        /* Use volatile to force memory access */
        vol_global_counter++;
        vol_global_sum += fp_temp3;
    }
    
    return result;
}

/* Function with inline assembly and register constraints */
void asm_register_pressure_test(int *data, int size) {
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    long long r11, r12, r13, r14, r15;
    
    for (int i = 0; i < size; i += 10) {
        /* Multiple inline asm statements competing for registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (r1)
            : "r" (data[i])
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull $3, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (r2)
            : "r" (data[i+1])
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "shrl $2, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (r3)
            : "r" (data[i+2])
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movq %1, %%rax\n\t"
            "addq $100, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r" (r11)
            : "r" ((long long)data[i+3])
            : "%rax", "memory"
        );
        
        asm volatile (
            "movq %1, %%rbx\n\t"
            "subq $50, %%rbx\n\t"
            "movq %%rbx, %0\n\t"
            : "=r" (r12)
            : "r" ((long long)data[i+4])
            : "%rbx", "memory"
        );
        
        /* Complex expression using all the asm results */
        r4 = r1 + r2;
        r5 = r3 * r4;
        r6 = r5 - data[i];
        r7 = r6 / (r2 + 1);
        r8 = r7 % 256;
        r9 = r8 << 3;
        r10 = r9 >> 1;
        
        r13 = r11 + r12;
        r14 = r13 * 2;
        r15 = r14 - r11;
        
        /* Force spilling with memory clobber */
        asm volatile ("" ::: "memory");
        
        /* Use results to prevent dead code elimination */
        vol_global_counter += r10;
        vol_global_sum += (double)r15;
    }
}

/* Function with irreducible control flow */
int irreducible_cfg_test(int *data, int size) {
    int result = 0;
    int i = 0;
    
    /* Create irreducible control flow with computed goto */
    void *labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    while (i < size) {
        int selector = data[i] % 10;
        goto *labels[selector];
        
    label0:
        result += data[i] * 2;
        i++;
        if (i % 3 == 0) goto label5;
        continue;
        
    label1:
        result += data[i] + 1;
        i += 2;
        if (i % 4 == 0) goto label8;
        continue;
        
    label2:
        result += data[i] - 1;
        i += 3;
        if (i % 5 == 0) goto label3;
        continue;
        
    label3:
        result += data[i] * 3;
        i++;
        if (i % 6 == 0) goto label9;
        continue;
        
    label4:
        result += data[i] / 2;
        i += 4;
        if (i % 7 == 0) goto label1;
        continue;
        
    label5:
        result += data[i] % 13;
        i += 2;
        if (i % 8 == 0) goto label2;
        continue;
        
    label6:
        result += data[i] << 1;
        i++;
        if (i % 9 == 0) goto label4;
        continue;
        
    label7:
        result += data[i] >> 1;
        i += 3;
        if (i % 10 == 0) goto label6;
        continue;
        
    label8:
        result += data[i] | 0x0F;
        i += 2;
        if (i % 11 == 0) goto label7;
        continue;
        
    label9:
        result += data[i] & 0xF0;
        i++;
        if (i % 12 == 0) goto label0;
        continue;
    }
    
    return result;
}

/* Function with many function calls in loops */
double function_call_pressure_test(double *data, int size) {
    double sum = 0.0;
    double temp[10];
    
    for (int i = 0; i < size; i++) {
        /* Many function calls creating call-clobbered conflicts */
        temp[0] = sin(data[i]);
        temp[1] = cos(data[i]);
        temp[2] = exp(data[i] * 0.01);
        temp[3] = log(fabs(data[i]) + 1.0);
        temp[4] = sqrt(fabs(data[i]));
        temp[5] = pow(data[i], 2.0);
        temp[6] = atan(data[i]);
        temp[7] = tan(data[i] * 0.1);
        temp[8] = asin(fmod(fabs(data[i]), 0.99));
        temp[9] = acos(fmod(fabs(data[i]), 0.99));
        
        /* Complex expression using all results */
        double subsum = 0.0;
        for (int j = 0; j < 10; j++) {
            subsum += temp[j];
        }
        
        sum += subsum;
        
        /* Early return in nested loop */
        if (i > 0 && i % 100 == 0) {
            if (sum > 1000.0) {
                return sum;
            }
        }
    }
    
    return sum;
}

/* Function with mixed data types */
long long mixed_type_test(char *cdata, short *sdata, int *idata, 
                          float *fdata, double *ddata, int size) {
    long long result = 0;
    int int_temp[20];
    float float_temp[20];
    double double_temp[20];
    
    for (int i = 0; i < size; i++) {
        /* Mixed type calculations */
        int_temp[0] = (int)cdata[i] * 2;
        int_temp[1] = (int)sdata[i] + 1;
        int_temp[2] = idata[i] - 1;
        int_temp[3] = (int)(fdata[i] * 10.0f);
        int_temp[4] = (int)(ddata[i] * 5.0);
        
        float_temp[0] = (float)cdata[i] * 0.5f;
        float_temp[1] = (float)sdata[i] * 0.25f;
        float_temp[2] = (float)idata[i] * 0.1f;
        float_temp[3] = fdata[i] * 2.0f;
        float_temp[4] = (float)ddata[i];
        
        double_temp[0] = (double)cdata[i] * 0.2;
        double_temp[1] = (double)sdata[i] * 0.4;
        double_temp[2] = (double)idata[i] * 0.6;
        double_temp[3] = (double)fdata[i];
        double_temp[4] = ddata[i] * 3.0;
        
        /* Complex nested loops */
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                int_temp[5 + j * 3 + k] = int_temp[j] * int_temp[k];
                float_temp[5 + j * 3 + k] = float_temp[j] + float_temp[k];
                double_temp[5 + j * 3 + k] = double_temp[j] - double_temp[k];
            }
        }
        
        /* Use all temporaries */
        long long ll_temp = 0;
        for (int j = 0; j < 20; j++) {
            ll_temp += int_temp[j];
            ll_temp += (long long)float_temp[j];
            ll_temp += (long long)double_temp[j];
        }
        
        result += ll_temp;
        
        /* Break and continue at different nesting levels */
        if (i % 7 == 0) {
            for (int j = 0; j < 3; j++) {
                if ((i + j) % 11 == 0) {
                    break;
                }
                if ((i + j) % 13 == 0) {
                    continue;
                }
                result += j * 1000;
            }
        }
    }
    
    return result;
}

/* Function with many arguments to stress register/stack passing */
int many_argument_test(int a1, int a2, int a3, int a4, int a5,
                       int a6, int a7, int a8, int a9, int a10,
                       int a11, int a12, int a13, int a14, int a15) {
    /* Complex calculation using all arguments */
    int b1 = a1 * a2;
    int b2 = a3 + a4;
    int b3 = a5 - a6;
    int b4 = a7 / (a8 + 1);
    int b5 = a9 % (a10 + 1);
    int b6 = a11 << a12;
    int b7 = a13 >> a14;
    int b8 = a15 ^ a1;
    
    int c1 = b1 + b2;
    int c2 = b3 * b4;
    int c3 = b5 | b6;
    int c4 = b7 & b8;
    
    int d1 = c1 - c2;
    int d2 = c3 + c4;
    
    int e1 = d1 * d2;
    int e2 = d1 / (d2 + 1);
    
    return e1 + e2 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
}

/* Main test driver */
int main() {
    /* Initialize large arrays with random data */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    char *char_data = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short *short_data = (short*)malloc(ARRAY_SIZE * sizeof(short));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        double_data[i] = (rand() % 1000) * 0.01;
        char_data[i] = rand() % 256;
        short_data[i] = rand() % 1000;
        float_data[i] = (rand() % 1000) * 0.01f;
    }
    
    int total_result = 0;
    double total_double_result = 0.0;
    long long total_mixed_result = 0;
    
    /* Warm-up iterations for profile feedback */
    for (int warmup = 0; warmup < 10; warmup++) {
        total_result += complex_expression_test(int_data, ARRAY_SIZE / 10);
    }
    
    /* Memory barrier between functions */
    asm volatile("" ::: "memory");
    
    /* Run all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Complex expressions */
        total_result += complex_expression_test(int_data, ARRAY_SIZE / 5);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 2: Inline assembly register pressure */
        asm_register_pressure_test(int_data, ARRAY_SIZE / 10);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 3: Irreducible control flow */
        total_result += irreducible_cfg_test(int_data, ARRAY_SIZE / 8);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 4: Function call pressure */
        total_double_result += function_call_pressure_test(double_data, ARRAY_SIZE / 20);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 5: Mixed data types */
        total_mixed_result += mixed_type_test(char_data, short_data, int_data, 
                                              float_data, double_data, ARRAY_SIZE / 25);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 6: Many arguments */
        total_result += many_argument_test(
            int_data[iter % ARRAY_SIZE],
            int_data[(iter + 1) % ARRAY_SIZE],
            int_data[(iter + 2) % ARRAY_SIZE],
            int_data[(iter + 3) % ARRAY_SIZE],
            int_data[(iter + 4) % ARRAY_SIZE],
            int_data[(iter + 5) % ARRAY_SIZE],
            int_data[(iter + 6) % ARRAY_SIZE],
            int_data[(iter + 7) % ARRAY_SIZE],
            int_data[(iter + 8) % ARRAY_SIZE],
            int_data[(iter + 9) % ARRAY_SIZE],
            int_data[(iter + 10) % ARRAY_SIZE],
            int_data[(iter + 11) % ARRAY_SIZE],
            int_data[(iter + 12) % ARRAY_SIZE],
            int_data[(iter + 13) % ARRAY_SIZE],
            int_data[(iter + 14) % ARRAY_SIZE]
        );
    }
    
    /* Compute final checksum */
    unsigned long long final_checksum = (unsigned long long)total_result;
    final_checksum += (unsigned long long)total_double_result;
    final_checksum += total_mixed_result;
    final_checksum += (unsigned long long)vol_global_counter;
    final_checksum += (unsigned long long)vol_global_sum;
    
    printf("Final checksum: %llu\n", final_checksum);
    printf("Global counter: %d\n", vol_global_counter);
    printf("Global sum: %f\n", vol_global_sum);
    
    /* Cleanup */
    free(int_data);
    free(double_data);
    free(char_data);
    free(short_data);
    free(float_data);
    
    return 0;
}
