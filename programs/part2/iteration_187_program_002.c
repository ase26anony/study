#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function declarations from lib.c */
extern void lib_function1(int iterations);
extern void lib_function2(int iterations);
extern void lib_function3(int iterations);
extern void lib_function4(int iterations);
extern int lib_calculate(int a, int b);
extern void lib_hot_path(int iterations);
extern void lib_cold_path(void);

/* Local function declarations */
static void process_data(int *data, int size, int multiplier);
static void analyze_results(double *results, int count);
static void generate_report(int mode);
static void handle_mode1(void);
static void handle_mode2(void);
static void handle_mode3(void);
static void mixed_execution(int iterations);

/* Global variables for different execution paths */
static int global_counter = 0;
static double global_sum = 0.0;

/* Function with varying execution frequency based on mode */
void process_data(int *data, int size, int multiplier) {
    if (data == NULL || size <= 0) {
        return;
    }
    
    for (int i = 0; i < size; i++) {
        data[i] = i * multiplier;
        global_sum += data[i];
        
        /* Conditional execution - some paths hotter than others */
        if (i % 3 == 0) {
            data[i] += 100;  /* Hot path */
        } else if (i % 7 == 0) {
            data[i] -= 50;   /* Cold path */
        }
    }
    
    global_counter += size;
}

/* Function with nested loops for more coverage */
void analyze_results(double *results, int count) {
    if (results == NULL || count <= 0) {
        return;
    }
    
    double min = results[0];
    double max = results[0];
    double sum = 0.0;
    
    for (int i = 0; i < count; i++) {
        sum += results[i];
        
        if (results[i] < min) {
            min = results[i];
        }
        
        if (results[i] > max) {
            max = results[i];
        }
        
        /* Different execution frequencies */
        if (i % 2 == 0) {
            results[i] *= 1.1;  /* Frequently executed */
        } else if (i % 5 == 0) {
            results[i] /= 1.05; /* Less frequently executed */
        }
    }
    
    printf("Analysis: min=%.2f, max=%.2f, avg=%.2f\n", 
           min, max, sum / count);
}

/* Function that generates different reports based on mode */
void generate_report(int mode) {
    static int report_count = 0;
    report_count++;
    
    switch (mode) {
        case 1:
            printf("Report %d: Mode 1 - Detailed analysis\n", report_count);
            for (int i = 0; i < 10; i++) {
                printf("  Item %d: value = %d\n", i, i * 10);
            }
            break;
            
        case 2:
            printf("Report %d: Mode 2 - Summary only\n", report_count);
            printf("  Total items: %d\n", 5);
            break;
            
        case 3:
            printf("Report %d: Mode 3 - Debug info\n", report_count);
            printf("  Counter: %d, Sum: %.2f\n", global_counter, global_sum);
            break;
            
        default:
            printf("Report %d: Unknown mode %d\n", report_count, mode);
            break;
    }
}

/* Mode-specific handlers with different execution patterns */
void handle_mode1(void) {
    printf("=== MODE 1: Intensive computation ===\n");
    
    int data[100];
    double results[50];
    
    /* Execute many times for hot paths */
    for (int i = 0; i < 1000; i++) {
        process_data(data, 100, i % 10 + 1);
        
        if (i % 100 == 0) {
            for (int j = 0; j < 50; j++) {
                results[j] = data[j % 100] * 0.5;
            }
            analyze_results(results, 50);
        }
    }
    
    /* Call library functions */
    lib_function1(500);
    lib_function2(200);
    lib_hot_path(1000);
    
    generate_report(1);
}

void handle_mode2(void) {
    printf("=== MODE 2: Moderate computation ===\n");
    
    int data[50];
    double results[25];
    
    /* Execute fewer times */
    for (int i = 0; i < 100; i++) {
        process_data(data, 50, i % 5 + 1);
        
        if (i % 20 == 0) {
            for (int j = 0; j < 25; j++) {
                results[j] = data[j % 50] * 0.3;
            }
            analyze_results(results, 25);
        }
    }
    
    /* Different library function mix */
    lib_function3(100);
    lib_function4(50);
    lib_cold_path();
    
    generate_report(2);
}

void handle_mode3(void) {
    printf("=== MODE 3: Light computation ===\n");
    
    int data[20];
    
    /* Minimal execution */
    for (int i = 0; i < 10; i++) {
        process_data(data, 20, i + 1);
    }
    
    /* Mixed library calls */
    lib_function1(50);
    lib_function4(30);
    
    generate_report(3);
}

void mixed_execution(int iterations) {
    /* Mixed execution pattern */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            int val = lib_calculate(i, i * 2);
            global_sum += val;
        } else if (i % 7 == 0) {
            lib_cold_path();
        }
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;  /* Default mode */
    
    /* Parse command line argument for mode */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (mode < 1 || mode > 3) {
            mode = 1;
        }
    }
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Reset globals */
    global_counter = 0;
    global_sum = 0.0;
    
    /* Execute based on mode */
    switch (mode) {
        case 1:
            handle_mode1();
            mixed_execution(500);
            break;
            
        case 2:
            handle_mode2();
            mixed_execution(100);
            break;
            
        case 3:
            handle_mode3();
            mixed_execution(50);
            break;
    }
    
    printf("Execution complete. Mode: %d, Counter: %d, Sum: %.2f\n",
           mode, global_counter, global_sum);
    
    return 0;
}
