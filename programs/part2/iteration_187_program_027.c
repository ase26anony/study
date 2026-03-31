#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations for functions in lib.c */
void hot_function_a(int iterations);
void hot_function_b(int iterations);
void cold_function_c(void);
void medium_function_d(int iterations);
void rarely_called_e(void);
void path_selector(int mode);

/* Functions in main.c */
void main_hot_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        if (i % 100 == 0) {
            printf("Main hot: %d\n", i);
        }
    }
}

void main_cold_function(void) {
    printf("Main cold function executed\n");
}

void main_medium_function(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (i % 50 == 0) {
            printf("Main medium: %d\n", i);
        }
    }
    printf("Main medium sum: %d\n", sum);
}

void main_rare_function(void) {
    static int call_count = 0;
    call_count++;
    printf("Main rare function called %d times\n", call_count);
}

void execute_mode_1(void) {
    printf("=== MODE 1: Heavy execution ===\n");
    
    /* Hot paths */
    main_hot_function(10000);
    hot_function_a(5000);
    hot_function_b(3000);
    
    /* Medium paths */
    main_medium_function(1000);
    medium_function_d(500);
    
    /* Cold paths (executed once) */
    main_cold_function();
    cold_function_c();
    
    /* Rare path (executed few times) */
    for (int i = 0; i < 3; i++) {
        main_rare_function();
    }
    
    rarely_called_e();
}

void execute_mode_2(void) {
    printf("=== MODE 2: Light execution ===\n");
    
    /* Different execution pattern */
    main_hot_function(1000);  /* Less iterations */
    hot_function_a(200);
    hot_function_b(100);
    
    /* More medium, less hot */
    main_medium_function(2000);
    medium_function_d(1000);
    
    /* Execute cold paths more */
    for (int i = 0; i < 5; i++) {
        main_cold_function();
        cold_function_c();
    }
    
    /* Rare path executed more */
    for (int i = 0; i < 10; i++) {
        main_rare_function();
    }
    
    rarely_called_e();
    rarely_called_e();  /* Called twice */
}

void execute_mode_3(void) {
    printf("=== MODE 3: Mixed execution ===\n");
    
    /* Very hot */
    main_hot_function(20000);
    
    /* Some medium */
    main_medium_function(500);
    
    /* Skip some cold functions entirely */
    printf("Skipping some cold functions in mode 3\n");
    
    /* Call rare function once */
    main_rare_function();
}

int main(int argc, char *argv[]) {
    int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode %d\n", mode);
    
    switch (mode) {
        case 1:
            execute_mode_1();
            break;
        case 2:
            execute_mode_2();
            break;
        case 3:
            execute_mode_3();
            break;
        default:
            printf("Unknown mode %d, using mode 1\n", mode);
            execute_mode_1();
            break;
    }
    
    /* Always execute path selector */
    path_selector(mode);
    
    return 0;
}
