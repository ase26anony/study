/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define NUM_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Complex expression with many intermediate values */
int complex_expression(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many intermediate calculations requiring registers */
    int t1 = a * b + c;
    int t2 = d * e - f;
    int t3 = g * h + a;
    int t4 = t1 * t2 - t3;
    int t5 = t2 * t3 + t1;
    int t6 = t3 * t4 - t2;
    int t7 = t4 * t5 + t3;
    int t8 = t5 * t6 - t4;
    int t9 = t6 * t7 + t5;
    int t10 = t7 * t8 - t6;
    
    /* Deeply nested conditionals creating complex CFG */
    if (t1 > t2) {
        if (t3 < t4) {
            if (t5 == t6) {
                return t7 + t8;
            } else if (t6 > t7) {
                return t8 - t9;
            } else {
                return t9 * t10;
            }
        } else {
            if (t7 != t8) {
                return t10 / t1;
            } else {
                return t1 % t2;
            }
        }
    } else {
        if (t9 <= t10) {
            if (t2 >= t3) {
                return t4 | t5;
            } else {
                return t6 & t7;
            }
        } else {
            return t8 ^ t9;
        }
    }
}

/* Function with switch statement creating many basic blocks */
int switch_complex(int value) {
    int result = 0;
    
    switch (value % NUM_CASES) {
        case 0:
            result = value * 2;
            /* Fall through */
        case 1:
            result += value / 3;
            break;
        case 2:
            result = value << 2;
            if (result > 1000) {
                result >>= 1;
            }
            break;
        case 3:
            result = value >> 1;
            /* Fall through */
        case 4:
            result *= 3;
            break;
        case 5:
            result = value & 0xFF;
            if (result < 128) {
                result |= 0x80;
            }
            break;
        case 6:
            result = value | 0x3F;
            /* Fall through */
        case 7:
            result ^= 0x55;
            break;
        case 8:
            result = ~value;
            break;
        case 9:
            result = value + 100;
            if (result > 200) {
                result -= 50;
            }
            break;
        case 10:
            result = value - 50;
            /* Fall through */
        case 11:
            result *= 7;
            break;
        case 12:
            result = value % 13;
            break;
        case 13:
            result = value / 2;
            /* Fall through */
        case 14:
            result += 42;
            break;
        default:
            result = -1;
    }
    
    return result;
}

/* Function with inline assembly creating register pressure */
void asm_register_pressure(int *array, int size) {
    int i, a, b, c, d, e, f, g, h;
    
    for (i = 0; i < size; i += 8) {
        /* Multiple asm statements with fixed register constraints */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (array[i]), "r" (array[i+1])
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull %2, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b)
            : "r" (array[i+2]), "r" (array[i+3])
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "xorl %2, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (c)
            : "r" (array[i+4]), "r" (array[i+5])
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "orl %2, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (d)
            : "r" (array[i+6]), "r" (array[i+7])
            : "%edx", "memory"
        );
        
        /* Force register spilling by using all results */
        e = a + b;
        f = c - d;
        g = e * f;
        h = g / (a + 1);
        
        /* Store results back, creating more register pressure */
        array[i] = a;
        array[i+1] = b;
        array[i+2] = c;
        array[i+3] = d;
        array[i+4] = e;
        array[i+5] = f;
        array[i+6] = g;
        array[i+7] = h;
    }
}

/* Function with deeply nested loops and mixed data types */
double nested_loops_mixed_types(float *farray, short *sarray, char *carray, int size) {
    double total = 0.0;
    int i, j, k;
    long long big_temp;
    float float_temp;
    short short_temp;
    char char_temp;
    
    /* Outer loop */
    for (i = 0; i < size; i += 10) {
        float_temp = farray[i];
        
        /* Middle loop */
        for (j = 0; j < 5; j++) {
            short_temp = sarray[i + j];
            
            /* Inner loop - creates many live ranges */
            for (k = 0; k < 3; k++) {
                char_temp = carray[i + j + k];
                
                /* Complex expression with mixed types */
                big_temp = (long long)(float_temp * 1000) + 
                          (short_temp * 100) + 
                          (char_temp * 10);
                
                /* Conditional with early continue */
                if (big_temp % 2 == 0) {
                    total += (double)big_temp / 1000.0;
                    continue;
                }
                
                /* More calculations keeping variables alive */
                float_temp += 0.1f;
                short_temp += k;
                char_temp -= j;
                
                /* Another conditional with break */
                if (total > 1000000.0) {
                    break;
                }
            }
            
            /* Early return from middle loop */
            if (short_temp < 0) {
                return total;
            }
        }
        
        /* Update global volatile to prevent optimization */
        global_accumulator += total;
    }
    
    return total;
}

/* Function with many function calls in loops */
int many_function_calls(int *data, int size) {
    int sum = 0;
    int i, j;
    
    for (i = 0; i < size; i++) {
        /* Multiple function calls with different arguments */
        sum += complex_expression(
            data[i], 
            data[i+1], 
            data[i+2], 
            data[i+3],
            data[i+4],
            data[i+5],
            data[i+6],
            data[i+7]
        );
        
        /* Call switch function */
        sum += switch_complex(data[i]);
        
        /* Update global */
        global_counter++;
        
        /* Nested loop with more calls */
        for (j = 0; j < 3; j++) {
            if (j % 2 == 0) {
                sum += complex_expression(
                    i, j, sum, data[i],
                    global_counter, 0, 0, 0
                );
            }
        }
    }
    
    return sum;
}

/* Function with computed goto (GCC extension) */
void computed_goto_test(int *array, int size) {
    static void *labels[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5, &&label6, &&label7,
        &&label8, &&label9, &&label10, &&label11
    };
    
    int i = 0;
    int result = 0;
    
    start:
    if (i >= size) goto end;
    
    /* Computed goto creates irreducible control flow */
    goto *labels[array[i] % 12];
    
    label0:
    result += array[i] * 2;
    i++;
    goto start;
    
    label1:
    result += array[i] / 3;
    i++;
    goto start;
    
    label2:
    result += array[i] << 1;
    i++;
    goto start;
    
    label3:
    result += array[i] >> 2;
    i++;
    goto start;
    
    label4:
    result += array[i] & 0xF;
    i++;
    goto start;
    
    label5:
    result += array[i] | 0xA;
    i++;
    goto start;
    
    label6:
    result += array[i] ^ 0x5;
    i++;
    goto start;
    
    label7:
    result += ~array[i];
    i++;
    goto start;
    
    label8:
    result += array[i] + 100;
    i++;
    goto start;
    
    label9:
    result += array[i] - 50;
    i++;
    goto start;
    
    label10:
    result += array[i] % 7;
    i++;
    goto start;
    
    label11:
    result += array[i] * array[i];
    i++;
    goto start;
    
    end:
    array[0] = result;
}

/* Function with many arguments to stress register/stack passing */
int many_arguments(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   int a11, int a12, int a13, int a14, int a15) {
    /* Use all arguments in complex ways */
    int t1 = a1 + a2 + a3;
    int t2 = a4 * a5 - a6;
    int t3 = a7 / a8 + a9;
    int t4 = a10 ^ a11 | a12;
    int t5 = a13 & a14 << a15;
    
    /* Deep nesting */
    if (t1 > t2) {
        if (t3 < t4) {
            return t1 + t3 + t5;
        } else {
            return t2 - t4 + t5;
        }
    } else {
        if (t5 != 0) {
            return t3 * t4 / t5;
        } else {
            return t1 | t2 | t3 | t4;
        }
    }
}

/* Main test driver */
int main() {
    int i, j;
    unsigned long long checksum = 0;
    
    /* Allocate large arrays with different types */
    int *int_array = malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = malloc(ARRAY_SIZE * sizeof(float));
    short *short_array = malloc(ARRAY_SIZE * sizeof(short));
    char *char_array = malloc(ARRAY_SIZE * sizeof(char));
    
    /* Initialize with random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = rand() % 1000;
        float_array[i] = (float)(rand() % 1000) / 10.0f;
        short_array[i] = (short)(rand() % 1000);
        char_array[i] = (char)(rand() % 256);
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up iterations for profile feedback */
    for (j = 0; j < 5; j++) {
        for (i = 0; i < 100; i++) {
            checksum += complex_expression(
                int_array[i], int_array[i+1], int_array[i+2], int_array[i+3],
                int_array[i+4], int_array[i+5], int_array[i+6], int_array[i+7]
            );
        }
        
        /* Memory barrier to prevent optimization across iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Test 1: Complex expressions */
    printf("Test 1: Complex expressions...\n");
    for (i = 0; i < ITERATIONS; i++) {
        int idx = i % (ARRAY_SIZE - 8);
        checksum += complex_expression(
            int_array[idx], int_array[idx+1], int_array[idx+2], int_array[idx+3],
            int_array[idx+4], int_array[idx+5], int_array[idx+6], int_array[idx+7]
        );
    }
    
    /* Test 2: Switch statements */
    printf("Test 2: Switch statements...\n");
    for (i = 0; i < ITERATIONS * 10; i++) {
        checksum += switch_complex(int_array[i % ARRAY_SIZE]);
    }
    
    /* Test 3: Inline assembly */
    printf("Test 3: Inline assembly...\n");
    asm_register_pressure(int_array, ARRAY_SIZE / 8);
    checksum += int_array[0];
    
    /* Test 4: Nested loops with mixed types */
    printf("Test 4: Nested loops with mixed types...\n");
    double loop_result = nested_loops_mixed_types(
        float_array, short_array, char_array, ARRAY_SIZE / 10
    );
    checksum += (unsigned long long)loop_result;
    
    /* Test 5: Many function calls */
    printf("Test 5: Many function calls...\n");
    checksum += many_function_calls(int_array, ARRAY_SIZE / 10);
    
    /* Test 6: Computed goto */
    printf("Test 6: Computed goto...\n");
    computed_goto_test(int_array, ARRAY_SIZE / 10);
    checksum += int_array[0];
    
    /* Test 7: Many arguments */
    printf("Test 7: Many arguments...\n");
    for (i = 0; i < ITERATIONS; i++) {
        checksum += many_arguments(
            int_array[i], int_array[i+1], int_array[i+2], int_array[i+3], int_array[i+4],
            int_array[i+5], int_array[i+6], int_array[i+7], int_array[i+8], int_array[i+9],
            int_array[i+10], int_array[i+11], int_array[i+12], int_array[i+13], int_array[i+14]
        );
    }
    
    /* Final verification */
    printf("Final checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(short_array);
    free(char_array);
    
    return 0;
}
