/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions targeting specific gcov-tool options */
__attribute__((noinline)) static void cold_function_1(void);
__attribute__((noinline)) static void cold_function_2(void);
__attribute__((hot)) void hot_loop_function(int iterations);
__attribute__((noinline)) void medium_work_function(int mode);
__attribute__((noinline)) int data_processing(int* data, int size, int threshold);
__attribute__((noinline)) void branch_heavy_function(int seed, int* counter);
__attribute__((cold)) void rarely_called_validator(int value);

/* Global counters for tracking execution */
static int global_hot_counter = 0;
static int global_cold_counter = 0;

/* Different implementations in separate compilation units would have same names */
__attribute__((noinline)) void process_data_a(int* arr, int n);
__attribute__((noinline)) void process_data_b(int* arr, int n);

/* ====== Functions targeting -f (function-level summary) ====== */

/* Function 1: Complex branching based on input */
__attribute__((noinline)) 
void func_for_f_summary_1(int mode, int* result) {
    *result = 0;
    if (mode % 3 == 0) {
        for (int i = 0; i < 10; i++) {
            *result += i * mode;
        }
    } else if (mode % 3 == 1) {
        int j = 0;
        while (j < 5) {
            *result -= j * mode;
            j++;
        }
    } else {
        *result = mode * 100;
    }
    
    /* Nested conditionals */
    if (*result > 1000) {
        *result /= 2;
    } else if (*result < -500) {
        *result *= -1;
    } else {
        *result += 42;
    }
}

/* Function 2: Switch statement with fallthrough */
__attribute__((noinline))
int func_for_f_summary_2(char op, int a, int b) {
    int res = 0;
    switch (op) {
        case 'A':
            res = a + b;
            /* Fall through */
        case 'B':
            res += a * b;
            break;
        case 'C':
            res = a - b;
            if (res < 0) res = -res;
            break;
        case 'D':
            res = (a > b) ? a : b;
            break;
        default:
            res = 0;
    }
    return res;
}

/* Function 3: Recursive-like pattern */
__attribute__((noinline))
void func_for_f_summary_3(int depth, int* accum) {
    if (depth <= 0) return;
    
    *accum += depth;
    
    for (int i = 0; i < depth % 5; i++) {
        *accum += i;
        if (i % 2 == 0) {
            *accum *= 2;
        } else {
            *accum /= 2;
        }
    }
    
    func_for_f_summary_3(depth - 1, accum);
}

/* Function 4: Matrix operations */
__attribute__((noinline))
void func_for_f_summary_4(int size, int fill) {
    int matrix[10][10];
    int sum = 0;
    
    for (int i = 0; i < size && i < 10; i++) {
        for (int j = 0; j < size && j < 10; j++) {
            matrix[i][j] = (i * j + fill) % 100;
            sum += matrix[i][j];
            
            if (matrix[i][j] > 50) {
                sum -= 25;
            }
        }
    }
    
    global_hot_counter += sum;
}

/* Function 5: String processing */
__attribute__((noinline))
int func_for_f_summary_5(const char* str, int len) {
    int vowels = 0, consonants = 0;
    
    for (int i = 0; i < len && str[i] != '\0'; i++) {
        char c = str[i];
        if (c >= 'A' && c <= 'Z') c += 32;
        
        if (c >= 'a' && c <= 'z') {
            if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
                vowels++;
            } else {
                consonants++;
            }
        }
    }
    
    return vowels * 10 + consonants;
}

/* ====== Functions targeting -h (hot-only) and -t (threshold) ====== */

__attribute__((hot))
void hot_loop_function(int iterations) {
    int local_sum = 0;
    
    /* This loop runs many times - will be "hot" */
    for (int i = 0; i < iterations; i++) {
        local_sum += i * i;
        
        if (i % 100 == 0) {
            local_sum /= 2;
        }
        
        /* Nested loop for extra heat */
        for (int j = 0; j < 10; j++) {
            local_sum += j;
        }
    }
    
    global_hot_counter += local_sum;
}

__attribute__((cold))
static void cold_function_1(void) {
    /* This function is marked cold and rarely called */
    global_cold_counter++;
    
    /* Minimal work */
    int x = 0;
    for (int i = 0; i < 3; i++) {
        x += i;
    }
}

__attribute__((cold))
static void cold_function_2(void) {
    /* Another cold function */
    if (global_cold_counter % 2 == 0) {
        global_cold_counter *= 2;
    } else {
        global_cold_counter += 1;
    }
}

/* ====== Functions for -o (object-level) - would be in separate files ====== */

/* In a real multi-file setup, these would be in different .c files */
__attribute__((noinline))
void process_data_a(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2 + 1;
        if (arr[i] > 1000) arr[i] = 1000;
    }
}

__attribute__((noinline))
void process_data_b(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] / 2 - 1;
        if (arr[i] < 0) arr[i] = 0;
    }
}

/* ====== Core processing functions ====== */

__attribute__((noinline))
void medium_work_function(int mode) {
    int buffer[100];
    int sum = 0;
    
    /* Fill buffer differently based on mode */
    for (int i = 0; i < 100; i++) {
        if (mode == 1) {
            buffer[i] = i * 3;
        } else if (mode == 2) {
            buffer[i] = i * i;
        } else {
            buffer[i] = i;
        }
    }
    
    /* Process buffer */
    for (int i = 0; i < 100; i++) {
        sum += buffer[i];
        if (buffer[i] % 2 == 0) {
            sum += 5;
        } else {
            sum -= 3;
        }
    }
    
    global_hot_counter += sum % 1000;
}

__attribute__((noinline))
int data_processing(int* data, int size, int threshold) {
    int positive = 0, negative = 0;
    
    for (int i = 0; i < size; i++) {
        if (data[i] > threshold) {
            positive++;
            data[i] = threshold;
        } else if (data[i] < -threshold) {
            negative++;
            data[i] = -threshold;
        } else {
            data[i] = 0;
        }
    }
    
    return positive - negative;
}

__attribute__((noinline))
void branch_heavy_function(int seed, int* counter) {
    srand(seed);
    int local = 0;
    
    for (int i = 0; i < 50; i++) {
        int r = rand() % 100;
        
        if (r < 20) {
            local += r * 2;
        } else if (r < 40) {
            local -= r;
        } else if (r < 60) {
            local *= (r % 5) + 1;
        } else if (r < 80) {
            local /= (r % 3) + 1;
        } else {
            local = local ^ r;
        }
    }
    
    *counter += local;
}

__attribute__((cold))
void rarely_called_validator(int value) {
    /* Only called under specific conditions */
    if (value > 10000) {
        printf("Value exceeds normal range: %d\n", value);
    }
}

/* ====== Main program with configurable execution paths ====== */

int main(int argc, char** argv) {
    int mode = 1;
    int seed = 42;
    int iterations = 1000;
    int data_size = 500;
    char algorithm = 'A';
    
    /* Parse command line arguments for different execution modes */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            data_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            algorithm = argv[++i][0];
        }
    }
    
    /* Initialize random seed for branch variability */
    srand(seed);
    
    /* Allocate and initialize data array */
    int* data = malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < data_size; i++) {
        data[i] = (rand() % 2000) - 1000;  /* Values between -1000 and 1000 */
    }
    
    /* Execute different code paths based on mode */
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Call function-level summary targets */
    func_for_f_summary_1(mode, &result1);
    result2 = func_for_f_summary_2(algorithm, mode, seed);
    func_for_f_summary_3(mode % 10, &result3);
    func_for_f_summary_4(mode % 10, seed % 100);
    
    char test_str[] = "TestingGcovToolFunctionSummary";
    int str_result = func_for_f_summary_5(test_str, sizeof(test_str));
    
    /* Hot/Cold function execution based on iterations */
    if (iterations > 500) {
        hot_loop_function(iterations);
    } else {
        cold_function_1();
    }
    
    if (mode % 7 == 0) {
        cold_function_2();
    }
    
    /* Medium work - always called */
    medium_work_function(mode);
    
    /* Process data with different algorithms */
    if (algorithm == 'A' || algorithm == 'B') {
        process_data_a(data, data_size / 2);
    } else {
        process_data_b(data, data_size / 2);
    }
    
    /* Branch-heavy execution */
    int branch_counter = 0;
    branch_heavy_function(seed, &branch_counter);
    
    /* Data processing with threshold */
    int proc_result = data_processing(data + data_size/2, data_size/2, 500);
    
    /* Rare validation */
    if (global_hot_counter > 1000000) {
        rarely_called_validator(global_hot_counter);
    }
    
    /* Calculate final checksum for verification */
    unsigned long long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    checksum += result1 + result2 + result3 + str_result;
    checksum += global_hot_counter + global_cold_counter;
    checksum += branch_counter + proc_result;
    
    printf("Result: %llu (mode=%d, seed=%d, iterations=%d, algorithm=%c)\n",
           checksum, mode, seed, iterations, algorithm);
    
    free(data);
    return 0;
}
