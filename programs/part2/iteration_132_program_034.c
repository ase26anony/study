/* test_mcf_coverage.c
 * Designed to trigger GCC's min-cost flow solver debug output
 * Compile with: gcc -O2 -fdump-ira-all -fdump-ira-details -fdump-rtl-all -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Global volatile variables to extend live ranges */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Complex function with deeply nested loops and many live ranges */
int complex_loop_pattern(int *data, int size) {
    int sum = 0;
    int product = 1;
    int temp1, temp2, temp3, temp4, temp5;
    double fp_sum = 0.0;
    double fp_temp1, fp_temp2, fp_temp3;
    
    /* Outer loop with multiple induction variables */
    for (int i = 0; i < size; i++) {
        /* Inner loop with complex expressions */
        for (int j = 0; j < 10; j++) {
            /* Many intermediate values requiring registers */
            temp1 = data[i] * j;
            temp2 = temp1 + i;
            temp3 = temp2 * temp2;
            temp4 = temp3 - j;
            temp5 = temp4 / (i + 1);
            
            /* Floating point operations mixing with integer */
            fp_temp1 = (double)temp5;
            fp_temp2 = sin(fp_temp1);
            fp_temp3 = cos(fp_temp1);
            fp_sum += fp_temp2 * fp_temp3;
            
            /* Multiple conditionals creating control flow */
            if (temp5 > 1000) {
                sum += temp5;
                if (sum > 10000) {
                    product *= (temp5 % 100);
                    if (product == 0) {
                        product = 1;
                        continue; /* Creates complex CFG */
                    }
                }
            } else if (temp5 < -1000) {
                sum -= temp5;
                break; /* Early exit from inner loop */
            }
            
            /* More register pressure */
            temp1 = temp5 * 2;
            temp2 = temp1 + 3;
            temp3 = temp2 * 4;
            temp4 = temp3 - 5;
            temp5 = temp4 / 6;
        }
        
        /* Function call within loop creates call-clobbered conflicts */
        global_counter += i;
        global_accumulator += fp_sum;
    }
    
    return sum + (int)fp_sum + product;
}

/* Function with complex switch statement creating many basic blocks */
int switch_pattern(int value) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0: {
            int a = value * 2;
            int b = a + 1;
            result = b * 3;
            /* Fall through */
        }
        case 1: {
            double c = sin(value);
            double d = cos(value);
            result += (int)(c * d * 1000);
            break;
        }
        case 2: {
            result = value << 2;
            /* Fall through */
        }
        case 3:
        case 4: {
            result = value >> 1;
            int e = result * 5;
            result = e % 7;
            break;
        }
        case 5: {
            for (int i = 0; i < 5; i++) {
                result += value * i;
                if (result > 100) break;
            }
            break;
        }
        case 6: {
            result = value ^ 0xFF;
            break;
        }
        case 7: {
            result = value & 0x0F;
            /* Fall through */
        }
        case 8: {
            result |= 0xF0;
            break;
        }
        case 9: {
            result = ~value;
            break;
        }
        case 10: {
            result = abs(value);
            break;
        }
        case 11: {
            result = value * value;
            break;
        }
        case 12: {
            result = sqrt(abs(value));
            break;
        }
        case 13: {
            result = value % 13;
            break;
        }
        case 14: {
            result = value + 14;
            break;
        }
        default: {
            result = -value;
            break;
        }
    }
    
    return result;
}

/* Function with inline assembly creating register pressure */
void asm_intensive_operations(int *input, int *output, int size) {
    int a, b, c, d, e, f;
    int *ptr = input;
    
    for (int i = 0; i < size; i++) {
        /* Multiple inline asm statements with fixed register constraints */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull $37, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (*ptr)
            : "%eax"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "addl $42, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b)
            : "r" (a)
            : "%ebx"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "xorl $0x55, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (c)
            : "r" (b)
            : "%ecx"
        );
        
        /* Memory clobber forces spills */
        asm volatile (
            "movl %1, %%edx\n\t"
            "shrl $2, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (d)
            : "r" (c)
            : "%edx", "memory"
        );
        
        /* More operations to increase pressure */
        e = d * 3;
        f = e + i;
        
        /* Complex expression with many temporaries */
        output[i] = (a + b) * (c - d) / (e + 1) + (f % 100);
        
        ptr++;
        
        /* Prevent optimization across iterations */
        asm volatile("" ::: "memory");
    }
}

/* Function with mixed data types stressing register classes */
double mixed_type_operations(char *chars, short *shorts, int *ints, 
                            float *floats, double *doubles, int size) {
    double total = 0.0;
    long long big_temp = 0;
    
    for (int i = 0; i < size; i++) {
        /* Operations on different data types */
        char c = chars[i];
        short s = shorts[i];
        int n = ints[i];
        float f = floats[i];
        double d = doubles[i];
        
        /* Many intermediate values of different types */
        int temp1 = c * 2;
        short temp2 = s + temp1;
        long temp3 = n * temp2;
        float temp4 = f * temp3;
        double temp5 = d + temp4;
        
        /* Complex expression mixing types */
        total += temp5 * i;
        big_temp += (long long)temp3 * temp1;
        
        /* Vector-like operations */
        if (i % 4 == 0) {
            total += sin(temp5) * cos(temp5);
        }
        
        /* Address calculations with multiple indexing */
        int idx1 = i * 2;
        int idx2 = i / 2;
        int idx3 = i % 100;
        
        /* Stress address reload pass */
        chars[idx1 % size] = temp1 % 256;
        shorts[idx2 % size] = temp2;
        ints[idx3] = temp3 % 10000;
        floats[i] = temp4;
        doubles[i] = temp5;
    }
    
    return total + (double)big_temp;
}

/* Function with many arguments to stress register/stack passing */
int many_arguments(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   double f1, double f2, double f3, double f4) {
    /* Complex expression using all arguments */
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    double product = f1 * f2 * f3 * f4;
    
    /* Many local variables */
    int b1 = sum * 2;
    int b2 = b1 + a1;
    int b3 = b2 * a2;
    int b4 = b3 - a3;
    int b5 = b4 / (a4 + 1);
    int b6 = b5 % a5;
    int b7 = b6 ^ a6;
    int b8 = b7 & a7;
    int b9 = b8 | a8;
    int b10 = b9 << 2;
    
    /* Nested loops with these variables */
    for (int i = 0; i < 10; i++) {
        b1 += i;
        for (int j = 0; j < 5; j++) {
            b2 += j;
            if (j % 2 == 0) {
                b3 *= (i + j);
                continue;
            } else {
                b4 -= (i - j);
                if (b4 < 0) break;
            }
        }
        b5 = b1 + b2 + b3 + b4;
    }
    
    return b5 + b6 + b7 + b8 + b9 + b10 + (int)product;
}

/* Main function that orchestrates all tests */
int main() {
    /* Initialize large arrays with random data */
    int *int_data = malloc(ARRAY_SIZE * sizeof(int));
    int *output_data = malloc(ARRAY_SIZE * sizeof(int));
    char *char_data = malloc(ARRAY_SIZE * sizeof(char));
    short *short_data = malloc(ARRAY_SIZE * sizeof(short));
    float *float_data = malloc(ARRAY_SIZE * sizeof(float));
    double *double_data = malloc(ARRAY_SIZE * sizeof(double));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        char_data[i] = rand() % 256;
        short_data[i] = rand() % 1000;
        float_data[i] = (float)rand() / RAND_MAX;
        double_data[i] = (double)rand() / RAND_MAX;
    }
    
    int total_result = 0;
    double fp_result = 0.0;
    
    /* Warm-up iterations for profile feedback */
    for (int warmup = 0; warmup < 10; warmup++) {
        total_result += complex_loop_pattern(int_data, 100);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    /* Main test iterations */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Complex loop pattern */
        total_result += complex_loop_pattern(int_data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");
        
        /* Test 2: Switch pattern */
        for (int i = 0; i < 100; i++) {
            total_result += switch_pattern(int_data[i % ARRAY_SIZE]);
        }
        asm volatile("" ::: "memory");
        
        /* Test 3: Assembly intensive operations */
        asm_intensive_operations(int_data, output_data, ARRAY_SIZE / 20);
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed type operations */
        fp_result += mixed_type_operations(char_data, short_data, int_data,
                                          float_data, double_data, ARRAY_SIZE / 50);
        asm volatile("" ::: "memory");
        
        /* Test 5: Many arguments function */
        total_result += many_arguments(
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
            double_data[iter % ARRAY_SIZE],
            double_data[(iter + 1) % ARRAY_SIZE],
            double_data[(iter + 2) % ARRAY_SIZE],
            double_data[(iter + 3) % ARRAY_SIZE]
        );
        asm volatile("" ::: "memory");
    }
    
    /* Compute final checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_data[i];
        checksum += char_data[i];
        checksum += short_data[i];
        checksum += (unsigned long long)(float_data[i] * 1000);
        checksum += (unsigned long long)(double_data[i] * 1000);
    }
    
    checksum += (unsigned long long)total_result;
    checksum += (unsigned long long)fp_result;
    
    printf("Final checksum: %llu\n", checksum);
    printf("Total result: %d\n", total_result);
    printf("FP result: %f\n", fp_result);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %f\n", global_accumulator);
    
    /* Cleanup */
    free(int_data);
    free(output_data);
    free(char_data);
    free(short_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
