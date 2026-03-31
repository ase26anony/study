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
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Complex structure to force register pressure */
struct DataPacket {
    int id;
    double values[8];
    float weights[4];
    long timestamp;
    char metadata[32];
    short flags[16];
};

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(struct DataPacket *data, int size) {
    int i, j, k, l;
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    float prod1 = 1.0f, prod2 = 1.0f, prod3 = 1.0f;
    long acc1 = 0, acc2 = 0, acc3 = 0;
    char temp_char;
    short temp_short;
    
    /* Outer loops create many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        sum1 += data[i].values[0] * data[i].weights[0];
        prod1 *= data[i].weights[1];
        acc1 += data[i].timestamp;
        
        for (j = 0; j < 8; j++) {
            sum2 += data[i].values[j] * sin(data[i].values[j]);
            prod2 *= cos(data[i].weights[j % 4]);
            
            for (k = 0; k < 4; k++) {
                sum3 += data[i].values[k] * data[i].values[7 - k];
                prod3 *= data[i].weights[k] + 1.0f;
                
                for (l = 0; l < 2; l++) {
                    /* Complex expression with many intermediates */
                    double t1 = data[i].values[j] * data[i].values[k];
                    double t2 = sin(t1) * cos(t1);
                    float t3 = data[i].weights[l] * data[i].weights[3 - l];
                    long t4 = data[i].timestamp + i + j + k + l;
                    
                    acc2 += (long)(t1 * t2 * t3) + t4;
                    acc3 += (long)(t2 * 1000.0);
                    
                    /* Force register pressure with mixed types */
                    temp_char = (char)(t1 * 255);
                    temp_short = (short)(t2 * 32767);
                    data[i].flags[l * 2] = temp_short;
                    data[i].metadata[k] = temp_char;
                }
            }
        }
        
        /* Early return in some cases to create complex CFG */
        if (i % 100 == 0 && sum1 > 1000.0) {
            return;
        }
        
        if (i % 50 == 0 && prod1 < 0.0001f) {
            continue;
        }
    }
    
    global_accumulator += sum1 + sum2 + sum3;
    global_counter += (int)(acc1 + acc2 + acc3);
}

/* Function 2: Complex switch statement with fall-through */
int test_complex_switch(int value, struct DataPacket *data) {
    int result = 0;
    double temp[8];
    float ftemp[4];
    
    /* Multiple intermediate values in registers */
    double d1 = data[value % ARRAY_SIZE].values[0];
    double d2 = data[value % ARRAY_SIZE].values[1];
    double d3 = data[value % ARRAY_SIZE].values[2];
    float f1 = data[value % ARRAY_SIZE].weights[0];
    float f2 = data[value % ARRAY_SIZE].weights[1];
    long l1 = data[value % ARRAY_SIZE].timestamp;
    
    switch (value % SWITCH_CASES) {
        case 0:
            result = (int)(d1 * 100.0);
            /* Fall through */
        case 1:
            result += (int)(d2 * 50.0);
            temp[0] = d1 + d2;
            break;
        case 2:
            result = (int)(f1 * 1000.0f);
            ftemp[0] = f1 * f2;
            /* Fall through */
        case 3:
            result += (int)(f2 * 500.0f);
            temp[1] = d3 * 2.0;
            break;
        case 4:
            result = (int)l1;
            temp[2] = sin(d1) * cos(d2);
            /* Fall through */
        case 5:
        case 6:
            result += value * 2;
            ftemp[1] = f1 + f2;
            temp[3] = d1 * d2 * d3;
            break;
        case 7:
            result = (int)(d1 * d2 * 10000.0);
            /* Fall through */
        case 8:
            result += (int)(f1 * f2 * 1000.0f);
            temp[4] = exp(d1);
            break;
        case 9:
            result = value * value;
            ftemp[2] = sqrtf(f1 * f1 + f2 * f2);
            /* Fall through */
        case 10:
            result += (int)(log(fabs(d1) + 1.0) * 100.0);
            temp[5] = d2 * d2 * d2;
            break;
        case 11:
            result = (int)(sin(d1) * cos(d2) * 1000.0);
            /* Fall through */
        case 12:
            result += (int)(tan(d3) * 500.0);
            ftemp[3] = f1 * 2.0f - f2;
            temp[6] = d1 / (d2 + 1.0);
            break;
        case 13:
            result = (int)(l1 % 1000);
            temp[7] = d1 + d2 + d3;
            /* Fall through */
        case 14:
            result += value % 100;
            break;
        default:
            result = -1;
            break;
    }
    
    /* Use all temporaries to keep them alive */
    for (int i = 0; i < 8; i++) {
        result += (int)(temp[i] * 10.0);
    }
    for (int i = 0; i < 4; i++) {
        result += (int)(ftemp[i] * 100.0f);
    }
    
    return result;
}

/* Function 3: Inline assembly with explicit register constraints */
void test_asm_register_pressure(int *array, int size) {
    int a, b, c, d, e, f, g, h;
    int *ptr = array;
    
    /* Multiple asm statements competing for registers */
    for (int i = 0; i < size; i += 8) {
        /* Force use of specific registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            "movl %2, %%ebx\n\t"
            "imull %%ebx, %%eax\n\t"
            : "=r" (a)
            : "r" (ptr[i]), "r" (ptr[i+1])
            : "%eax", "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "subl %%ecx, %0\n\t"
            "movl %2, %%edx\n\t"
            "xorl %%edx, %%ecx\n\t"
            : "=r" (b)
            : "r" (ptr[i+2]), "r" (ptr[i+3])
            : "%ecx", "%edx", "memory"
        );
        
        asm volatile (
            "movl %1, %%esi\n\t"
            "orl %%esi, %0\n\t"
            "movl %2, %%edi\n\t"
            "andl %%edi, %%esi\n\t"
            : "=r" (c)
            : "r" (ptr[i+4]), "r" (ptr[i+5])
            : "%esi", "%edi", "memory"
        );
        
        asm volatile (
            "movl %1, %%r8d\n\t"
            "shll $3, %%r8d\n\t"
            "addl %%r8d, %0\n\t"
            : "=r" (d)
            : "r" (ptr[i+6])
            : "%r8", "memory"
        );
        
        asm volatile (
            "movl %1, %%r9d\n\t"
            "rorl $5, %%r9d\n\t"
            "movl %%r9d, %0\n\t"
            : "=r" (e)
            : "r" (ptr[i+7])
            : "%r9", "memory"
        );
        
        /* Complex expression using all results */
        f = (a * b) + (c ^ d) - (e << 2);
        g = (a + b) * (c - d) / (e + 1);
        h = (a | b) & (c ^ d) | (e & 0xFF);
        
        /* Store results creating more register pressure */
        ptr[i] = f;
        ptr[i+1] = g;
        ptr[i+2] = h;
        ptr[i+3] = a + b + c;
        ptr[i+4] = d * e;
        ptr[i+5] = f ^ g ^ h;
        ptr[i+6] = (a << 3) | (b >> 2);
        ptr[i+7] = c + d - e;
        
        /* Memory barrier to force spills */
        asm volatile("" ::: "memory");
    }
    
    global_counter += a + b + c + d + e + f + g + h;
}

/* Function 4: Irreducible control flow with computed goto */
void test_irreducible_cfg(int *array, int size) {
    static void *labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    int i = 0;
    int state = 0;
    int a = 0, b = 0, c = 0, d = 0, e = 0;
    double f = 0.0, g = 0.0, h = 0.0;
    
    /* Create irreducible control flow */
    while (i < size) {
        int idx = array[i] % 10;
        goto *labels[idx];
        
    label0:
        a += array[i] * 2;
        f += sin(a * 0.01);
        i++;
        if (i % 100 == 0) goto label5;
        continue;
        
    label1:
        b += array[i] * 3;
        g += cos(b * 0.01);
        i++;
        if (i % 200 == 0) goto label8;
        continue;
        
    label2:
        c += array[i] * 5;
        h += tan(c * 0.001);
        i++;
        if (i % 150 == 0) goto label0;
        continue;
        
    label3:
        d += array[i] * 7;
        f -= d * 0.1;
        i++;
        if (i % 250 == 0) goto label2;
        continue;
        
    label4:
        e += array[i] * 11;
        g *= 1.0 + e * 0.001;
        i++;
        if (i % 300 == 0) goto label7;
        continue;
        
    label5:
        a -= array[i];
        f = sqrt(fabs(f));
        i++;
        if (i % 350 == 0) goto label1;
        continue;
        
    label6:
        b ^= array[i];
        g = exp(g * 0.1);
        i++;
        if (i % 400 == 0) goto label4;
        continue;
        
    label7:
        c |= array[i];
        h = log(fabs(h) + 1.0);
        i++;
        if (i % 450 == 0) goto label3;
        continue;
        
    label8:
        d &= array[i];
        f += g * h;
        i++;
        if (i % 500 == 0) goto label6;
        continue;
        
    label9:
        e = ~array[i];
        g -= f * 0.5;
        i++;
        if (i % 550 == 0) goto label9;
        continue;
    }
    
    global_accumulator += f + g + h;
    global_counter += a + b + c + d + e;
}

/* Function 5: Many function arguments forcing register/stack pressure */
double test_many_arguments(double a1, double a2, double a3, double a4, double a5,
                          double a6, double a7, double a8, double a9, double a10,
                          float f1, float f2, float f3, float f4, int i1, int i2,
                          int i3, int i4, int i5, long l1, long l2) {
    /* Complex expressions using all arguments */
    double sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    double prod = a1 * a2 * a3 * a4 * a5;
    double diff = a6 - a7 - a8 - a9 - a10;
    
    float fsum = f1 + f2 + f3 + f4;
    float fprod = f1 * f2 * f3 * f4;
    
    int isum = i1 + i2 + i3 + i4 + i5;
    int iprod = i1 * i2 * i3;
    
    long lsum = l1 + l2;
    long ldiff = l1 - l2;
    
    /* Nested calculations creating register pressure */
    for (int i = 0; i < 100; i++) {
        sum += sin(a1 * i) * cos(a2 * i);
        prod *= 1.0 + fabs(a3 * 0.01);
        diff -= tan(a4 * 0.001) * exp(a5 * 0.0001);
        
        fsum += sqrtf(f1 * f1 + f2 * f2);
        fprod *= 1.0f + f3 * f4 * 0.0001f;
        
        isum += (i1 ^ i2) | (i3 & i4);
        iprod *= (i5 + i) % 256;
        
        lsum += (l1 << (i % 8)) | (l2 >> (i % 8));
        ldiff -= (l1 * i) / (l2 + 1);
    }
    
    /* Return complex expression using all locals */
    return sum + prod + diff + fsum + fprod + isum + iprod + lsum + ldiff;
}

/* Function 6: Pointer aliasing preventing optimization */
void test_pointer_aliasing(struct DataPacket *data1, struct DataPacket *data2, int size) {
    /* Create aliases */
    struct DataPacket *alias1 = data1;
    struct DataPacket *alias2 = data2;
    struct DataPacket *alias3 = data1 + size/2;
    
    int *int_ptr1 = (int *)data1;
    int *int_ptr2 = (int *)data2;
    double *double_ptr = (double *)data1;
    float *float_ptr = (float *)data2;
    
    /* Complex loop with aliased accesses */
    for (int i = 0; i < size; i++) {
        /* Multiple aliased writes */
        alias1[i].id = i;
        alias3[i/2].id = i * 2;
        
        /* Aliased reads and writes creating dependencies */
        double_ptr[i] = sin(double_ptr[i]) * cos(double_ptr[size - i - 1]);
        float_ptr[i] = sqrtf(fabs(float_ptr[i]) + float_ptr[size - i - 1]);
        
        /* Integer computations with aliasing */
        int_ptr1[i] = int_ptr1[i] ^ int_ptr2[size - i - 1];
        int_ptr2[i] = int_ptr1[i] + int_ptr2[i] * 3;
        
        /* More complex expressions */
        for (int j = 0; j < 4; j++) {
            alias1[i].values[j] = alias1[i].values[j] * 
                                 alias2[i].weights[j % 2] +
                                 alias3[i/2].values[7 - j];
            alias2[i].weights[j] = alias2[i].weights[j] +
                                  alias1[i].values[j] * 0.1f;
        }
        
        /* Volatile access to force memory operations */
        global_counter += alias1[i].id;
        global_accumulator += alias2[i].values[0];
    }
}

int main() {
    /* Initialize large arrays */
    struct DataPacket *data = malloc(ARRAY_SIZE * sizeof(struct DataPacket));
    int *int_array = malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i].id = i;
        data[i].timestamp = rand();
        for (int j = 0; j < 8; j++) {
            data[i].values[j] = (double)rand() / RAND_MAX * 100.0 - 50.0;
        }
        for (int j = 0; j < 4; j++) {
            data[i].weights[j] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        }
        for (int j = 0; j < 16; j++) {
            data[i].flags[j] = rand() % 1000;
        }
        int_array[i] = rand() % 10000;
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up iterations for profile feedback */
    for (int warmup = 0; warmup < 5; warmup++) {
        test_nested_loops(data, 1000);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    /* Main test iterations */
    long total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Nested loops */
        test_nested_loops(data, ARRAY_SIZE / (iter % 10 + 1));
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex switch */
        int switch_result = 0;
        for (int i = 0; i < 1000; i++) {
            switch_result += test_complex_switch(int_array[i] + iter, data);
        }
        total_result += switch_result;
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        test_asm_register_pressure(int_array, ARRAY_SIZE / 2);
        asm volatile("" ::: "memory");
        
        /* Test 4: Irreducible CFG */
        test_irreducible_cfg(int_array, ARRAY_SIZE / 4);
        asm volatile("" ::: "memory");
        
        /* Test 5: Many arguments */
        double arg_result = test_many_arguments(
            1.0 + iter * 0.1, 2.0 + iter * 0.2, 3.0 + iter * 0.3,
            4.0 + iter * 0.4, 5.0 + iter * 0.5, 6.0 + iter * 0.6,
            7.0 + iter * 0.7, 8.0 + iter * 0.8, 9.0 + iter * 0.9,
            10.0 + iter * 1.0,
            1.1f + iter * 0.01f, 2.2f + iter * 0.02f,
            3.3f + iter * 0.03f, 4.4f + iter * 0.04f,
            iter * 11, iter * 12, iter * 13, iter * 14, iter * 15,
            iter * 1000L, iter * 2000L
        );
        total_result += (long)arg_result;
        asm volatile("" ::: "memory");
        
        /* Test 6: Pointer aliasing */
        test_pointer_aliasing(data, data + ARRAY_SIZE/2, ARRAY_SIZE/4);
        asm volatile("" ::: "memory");
        
        /* Modify data to prevent optimization */
        for (int i = iter % 100; i < ARRAY_SIZE; i += 100) {
            data[i].values[0] += 0.001;
            int_array[i] ^= iter;
        }
    }
    
    /* Compute final checksum */
    long checksum = total_result + global_counter + (long)global_accumulator;
    for (int i = 0; i < ARRAY_SIZE; i += 100) {
        checksum += data[i].id + (long)data[i].values[0] + int_array[i];
    }
    
    printf("Test completed. Checksum: %ld\n", checksum);
    printf("Global counter: %d, Global accumulator: %f\n", 
           global_counter, global_accumulator);
    
    free(data);
    free(int_array);
    
    return 0;
}
