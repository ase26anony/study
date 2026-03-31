/* test_early_remat.c - Test program to trigger early rematerialization */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Pure function attribute to encourage rematerialization */
static int __attribute__((const)) pure_compute(int x, int y) {
    return (x * 3 + y * 7) ^ 0x1234;
}

/* Another pure function with different mode */
static double __attribute__((const)) pure_double(double x, double y) {
    return x * 3.14159 + y * 2.71828;
}

/* Structure with mixed types to create varied register modes */
struct mixed_data {
    int id;
    double value;
    float fval;
    long long big;
    char* name;
};

/* Hot function 1: Creates high integer register pressure */
int hot_function1(struct mixed_data* data, int size) {
    int i, j;
    int result = 0;
    
    /* Outer loop with computations that could be rematerialized */
    for (i = 0; i < size; i++) {
        /* Create many live variables with cheap-to-recompute expressions */
        int base = data[i].id;
        int offset1 = base * 3 + 7;      /* Candidate for remat */
        int offset2 = base / 2 - 5;      /* Candidate for remat */
        int offset3 = base ^ 0xABCD;     /* Candidate for remat */
        
        /* Use inline assembly to create artificial register pressure */
        int temp1, temp2, temp3;
        asm volatile ("# Force register use %0" : "=r" (temp1) : "0" (offset1));
        asm volatile ("# Force register use %0" : "=r" (temp2) : "0" (offset2));
        asm volatile ("# Force register use %0" : "=r" (temp3) : "0" (offset3));
        
        /* Inner loop with high pressure - prevents keeping values in registers */
        for (j = 0; j < 16; j++) {
            /* Complex expression using all offsets multiple times */
            int val1 = pure_compute(offset1, j);
            int val2 = pure_compute(offset2, j);
            int val3 = pure_compute(offset3, j);
            
            /* Use all values in computation to keep them live */
            result += val1 * val2 - val3;
            
            /* More computations to increase pressure */
            int t1 = offset1 * j + offset2;
            int t2 = offset2 * j - offset3;
            int t3 = offset3 * j ^ offset1;
            
            /* Use goto to create non-trivial CFG */
            if ((t1 + t2 + t3) % 7 == 0) {
                goto special_case;
            }
            
            result += t1 + t2 * 2 + t3 / 3;
            continue;
            
        special_case:
            result -= t1 - t2 + t3;
        }
        
        /* Use switch to complicate control flow */
        switch (base % 4) {
            case 0:
                result += offset1 * 100;
                break;
            case 1:
                result += offset2 * 200;
                /* fall through */
            case 2:
                result += offset3 * 300;
                break;
            default:
                result += (offset1 + offset2 + offset3) * 400;
        }
    }
    
    return result;
}

/* Hot function 2: Mixed integer and floating point pressure */
double hot_function2(struct mixed_data* data, int size) {
    int i;
    double total = 0.0;
    
    /* Disable some optimizations for this function */
    #pragma GCC optimize ("O2")
    #pragma GCC push_options
    
    for (i = 0; i < size; i++) {
        /* Mixed type computations */
        double base_val = data[i].value;
        float fval = data[i].fval;
        
        /* Create values that are cheap to recompute but used multiple times */
        double scaled = base_val * 3.14159;      /* Candidate for remat */
        double offset = base_val + 2.71828;      /* Candidate for remat */
        float fscaled = fval * 2.0f;             /* Candidate for remat */
        
        /* Use register hint to increase pressure */
        register double r1 asm ("xmm0") = scaled;
        register double r2 asm ("xmm1") = offset;
        register float r3 asm ("xmm2") = fscaled;
        
        /* Complex expression using all values multiple times */
        double term1 = pure_double(r1, r2);
        double term2 = pure_double(r2, r1);
        float term3 = r3 * 1.5f + (float)r1;
        
        /* Pointer arithmetic with mixed types */
        char* ptr = data[i].name;
        if (ptr) {
            int len = 0;
            while (ptr[len] != '\0') {
                len++;
            }
            
            /* Use length in computation */
            total += term1 * len + term2 / (len + 1) + term3;
            
            /* More computations to use the values again */
            if (len > 10) {
                total += r1 * len - r2 * 2.0 + r3 * 3.0f;
            } else {
                total -= r1 / len + r2 * 0.5 - r3 * 2.0f;
            }
        }
        
        /* Array indexing with computed values */
        int idx = (int)(scaled * 100) % 256;
        double array[256];
        for (int j = 0; j < 256; j++) {
            array[j] = j * 0.01;
        }
        
        total += array[idx] * offset + array[255 - idx] * scaled;
        
        /* Nested conditionals to prevent optimization */
        if (i % 3 == 0) {
            if (total > 1000.0) {
                total *= 0.9;
            } else if (total < -1000.0) {
                total *= 1.1;
            } else {
                total += fscaled * 100.0f;
            }
        }
    }
    
    #pragma GCC pop_options
    return total;
}

/* Hot function 3: Extreme register pressure with all types */
long long hot_function3(struct mixed_data* data, int size) {
    long long grand_total = 0;
    int i, j, k;
    
    /* Triple nested loop for maximum pressure */
    for (i = 0; i < size; i += 2) {
        int base_i = data[i].id;
        double val_i = data[i].value;
        
        for (j = 0; j < 8; j++) {
            int mod_j = (base_i + j) % 256;
            double scale_j = val_i * j * 0.1;
            
            for (k = 0; k < 4; k++) {
                /* Many live variables with recomputable expressions */
                int a = mod_j * k + 7;
                int b = mod_j / (k + 1) - 3;
                int c = mod_j ^ (k * 0x1111);
                double d = scale_j * k * 0.5;
                double e = scale_j + k * 2.5;
                float f = (float)d * 1.5f;
                float g = (float)e * 0.75f;
                
                /* Use all variables multiple times in complex expression */
                long long sum = 0;
                sum += a * b * c;
                sum += (long long)(d * 1000.0);
                sum -= (long long)(e * 500.0);
                sum += (long long)(f * 200.0f);
                sum -= (long long)(g * 100.0f);
                
                /* More uses to prevent register allocation */
                int t = pure_compute(a, b);
                double u = pure_double(d, e);
                float v = f * g;
                
                sum += t * 2;
                sum += (long long)(u * 100.0);
                sum += (long long)(v * 50.0f);
                
                /* Conditional that uses all values */
                if ((a + b + c) % 2 == 0) {
                    sum += (long long)((d + e) * (f + g) * 10.0);
                } else {
                    sum -= (long long)((d - e) * (f - g) * 5.0);
                }
                
                grand_total += sum;
                
                /* Artificial register clobber to force spills */
                asm volatile ("# Clobber registers" : : :
                    "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                    "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                    "xmm12", "xmm13", "xmm14", "xmm15");
            }
        }
    }
    
    return grand_total;
}

int main() {
    const int SIZE = 100;
    struct mixed_data* data = malloc(SIZE * sizeof(struct mixed_data));
    int i;
    
    /* Initialize data */
    for (i = 0; i < SIZE; i++) {
        data[i].id = i * 3;
        data[i].value = sin(i * 0.1) * 100.0;
        data[i].fval = cos(i * 0.05) * 50.0f;
        data[i].big = (long long)i * i * 1000LL;
        
        /* Allocate and fill name string */
        data[i].name = malloc(20);
        snprintf(data[i].name, 20, "Item%d", i);
    }
    
    /* Call hot functions to trigger rematerialization */
    int result1 = hot_function1(data, SIZE);
    double result2 = hot_function2(data, SIZE);
    long long result3 = hot_function3(data, SIZE / 4);
    
    /* Final computation to use results and prevent elimination */
    long long final_result = result1 + (long long)result2 + result3;
    
    /* Print to prevent dead code elimination */
    printf("Final result: %lld\n", final_result);
    
    /* Cleanup */
    for (i = 0; i < SIZE; i++) {
        free(data[i].name);
    }
    free(data);
    
    return 0;
}
