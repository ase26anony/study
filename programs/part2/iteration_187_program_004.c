#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations for functions in lib.c */
void hot_function_a(int iterations);
void hot_function_b(int iterations);
void cold_function_c(void);
void medium_function_d(int iterations);
void rarely_called_e(void);
void path_dependent_f(int mode);

/* Local functions */
static void local_hot_func(int n) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            printf("Even iteration: %d\n", i);
        } else {
            printf("Odd iteration: %d\n", i);
        }
    }
}

static void local_cold_func(void) {
    printf("This function is rarely called\n");
    for (int i = 0; i < 3; i++) {
        printf("Cold loop iteration %d\n", i);
    }
}

static void local_medium_func(int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += i;
        if (sum > 100) {
            printf("Sum exceeded 100 at iteration %d\n", i);
            break;
        }
    }
    printf("Final sum: %d\n", sum);
}

void process_mode_1(void) {
    printf("=== Processing Mode 1 (High Activity) ===\n");
    
    /* Hot paths */
    hot_function_a(1000);
    hot_function_b(500);
    
    /* Medium activity */
    medium_function_d(100);
    local_medium_func(50);
    
    /* Some cold paths */
    cold_function_c();
    rarely_called_e();
    
    /* Path dependent with hot branch */
    path_dependent_f(1);
    
    /* Local hot function */
    local_hot_func(200);
}

void process_mode_2(void) {
    printf("=== Processing Mode 2 (Mixed Activity) ===\n");
    
    /* Different mix of activities */
    hot_function_a(100);  /* Less than mode 1 */
    hot_function_b(200);  /* More than mode 1 */
    
    /* More medium activity */
    medium_function_d(200);
    local_medium_func(100);
    
    /* Call cold functions more often */
    cold_function_c();
    cold_function_c();  /* Extra call */
    rarely_called_e();
    
    /* Path dependent with cold branch */
    path_dependent_f(2);
    
    /* Local functions with different frequencies */
    local_hot_func(50);
    local_cold_func();  /* Called in mode 2 but not mode 1 */
}

void process_mode_3(void) {
    printf("=== Processing Mode 3 (Low Activity) ===\n");
    
    /* Minimal activity */
    hot_function_a(10);
    hot_function_b(5);
    
    /* Mostly cold paths */
    cold_function_c();
    rarely_called_e();
    rarely_called_e();  /* Extra call */
    
    /* Path dependent with rare branch */
    path_dependent_f(3);
    
    /* Local cold function */
    local_cold_func();
}

int main(int argc, char *argv[]) {
    int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode %d\n", mode);
    
    switch (mode) {
        case 1:
            process_mode_1();
            break;
        case 2:
            process_mode_2();
            break;
        case 3:
            process_mode_3();
            break;
        default:
            printf("Unknown mode %d, using mode 1\n", mode);
            process_mode_1();
            break;
    }
    
    /* Always execute this common path */
    printf("Common cleanup path executed\n");
    for (int i = 0; i < 10; i++) {
        if (i < 5) {
            printf("Early cleanup iteration %d\n", i);
        } else {
            printf("Late cleanup iteration %d\n", i);
        }
    }
    
    return 0;
}
