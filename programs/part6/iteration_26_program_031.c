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

/* Complex structure to create varied addressing modes */
struct MixedData {
    int id;
    double value;
    float fval;
    char data[8];
    long long big;
};

/* Hot function 1: Creates high integer register pressure */
static int hot_function1(int *array, int size) {
    int sum = 0;
    int i, j;
    
    /* Outer loop creates values that inner loop uses */
    for (i = 0; i < size; i++) {
        /* Create many live variables with cheap-to-recompute expressions */
        int base = array[i] * 3;          /* Candidate for remat */
        int offset = (i & 0xF) + 5;       /* Candidate for remat */
        int mask = pure_compute(i, size); /* Pure function call */
        
        /* Inner loop with high register pressure */
        for (j = 0; j < 16; j++) {
            /* Use all variables in complex expressions */
            int temp1 = base + j * offset;
            int temp2 = mask ^ (j << 3);
            int temp3 = pure_compute(temp1, temp2);
            
            /* Force register pressure with many intermediate values */
            int a = temp1 * 2;
            int b = temp2 / 3;
            int c = temp3 + 7;
            int d = a ^ b;
            int e = c & 0xFF;
            int f = d | e;
            int g = f << 2;
            int h = g >> 1;
            
            /* Mix with array access */
            sum += array[(temp1 + temp2) % size] + h;
            
            /* Create control flow to inhibit CSE */
            if (j & 1) {
                sum += a;
            } else {
                sum -= b;
            }
            
            if (j % 3 == 0) {
                sum ^= c;
            }
        }
        
        /* Use goto to create non-trivial CFG */
        if (base > 1000) {
            goto skip_point;
        }
        
        /* More computations */
        sum += pure_compute(base, offset);
        
    skip_point:
        /* Empty label for goto target */
        ;
    }
    
    return sum;
}

/* Hot function 2: Mixes integer and floating point */
static double hot_function2(struct MixedData *data, int count) {
    double total = 0.0;
    int i;
    
    /* Use pragma to ensure optimization level */
    #pragma GCC optimize ("O2")
    for (i = 0; i < count; i++) {
        /* Create mixed-type register pressure */
        double base_val = data[i].value * 3.14;  /* Candidate for remat */
        float fval = data[i].fval * 2.0f;        /* Different mode */
        long long big = data[i].big;             /* Different mode */
        
        /* Complex addressing */
        int idx = (int)(base_val * 100) % count;
        double d1 = pure_double(base_val, fval);
        double d2 = pure_double(fval, base_val);
        
        /* Many intermediate floating point values */
        double t1 = d1 * d2;
        double t2 = t1 / (i + 1);
        double t3 = t2 + base_val;
        double t4 = t3 - fval;
        double t5 = t4 * 1.5;
        
        /* Mix with integer computations */
        int offset = ((int)big ^ i) & 0xFF;
        total += t5 * offset;
        
        /* Access array with complex index */
        if (idx > 0 && idx < count) {
            total += data[idx].value * (i % 8);
        }
        
        /* Switch statement for complex control flow */
        switch (i & 3) {
            case 0:
                total += d1;
                break;
            case 1:
                total -= d2;
                break;
            case 2:
                total *= 1.1;
                break;
            case 3:
                total /= 1.05;
                /* fall through */
            default:
                total += 1.0;
        }
    }
    
    return total;
}

/* Hot function 3: Uses inline assembly to force register references */
static int hot_function3(int *arr, int n) {
    int result = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Create value that's expensive but pure */
        int computed = pure_compute(arr[i], i);
        
        /* Use inline assembly to create artificial register pressure */
        int reg1, reg2, reg3;
        
        /* Force computed into a register and use it multiple times */
        asm volatile (
            "movl %1, %0\n\t"
            "imull $3, %0\n\t"
            : "=r" (reg1)
            : "r" (computed)
            : /* clobbers */
        );
        
        asm volatile (
            "movl %1, %0\n\t"
            "addl $7, %0\n\t"
            : "=r" (reg2)
            : "r" (computed)
            : /* clobbers */
        );
        
        asm volatile (
            "movl %1, %0\n\t"
            "xorl $0x55AA, %0\n\t"
            : "=r" (reg3)
            : "r" (computed)
            : /* clobbers */
        );
        
        /* Use all register values */
        result += reg1 + reg2 * 2 - reg3;
        
        /* Create more intermediate values */
        int t1 = reg1 ^ reg2;
        int t2 = reg2 & reg3;
        int t3 = reg1 | reg3;
        int t4 = t1 + t2;
        int t5 = t3 - t4;
        int t6 = t5 * 3;
        
        result += t6;
        
        /* Conditional that uses computed value again */
        if (computed > 1000) {
            result -= pure_compute(computed, i);
        }
    }
    
    return result;
}

/* Main function that orchestrates everything */
int main(void) {
    const int ARRAY_SIZE = 256;
    const int STRUCT_COUNT = 128;
    
    /* Initialize data */
    int *int_array = malloc(ARRAY_SIZE * sizeof(int));
    struct MixedData *mixed_data = malloc(STRUCT_COUNT * sizeof(struct MixedData));
    
    if (!int_array || !mixed_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 37 + 123) & 0xFFF;
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        mixed_data[i].id = i;
        mixed_data[i].value = sin(i * 0.1) * 100.0;
        mixed_data[i].fval = cos(i * 0.05) * 50.0f;
        mixed_data[i].big = (long long)i * 1000000LL;
        for (int j = 0; j < 8; j++) {
            mixed_data[i].data[j] = (i + j) & 0xFF;
        }
    }
    
    /* Call hot functions to trigger rematerialization */
    int result1 = hot_function1(int_array, ARRAY_SIZE);
    double result2 = hot_function2(mixed_data, STRUCT_COUNT);
    int result3 = hot_function3(int_array, ARRAY_SIZE);
    
    /* Combine results in a non-trivial way */
    int final_result = result1 + (int)result2 + result3;
    
    /* Print to prevent dead code elimination */
    printf("Final result: %d (int1=%d, double=%f, int3=%d)\n", 
           final_result, result1, result2, result3);
    
    /* Cleanup */
    free(int_array);
    free(mixed_data);
    
    return 0;
}
