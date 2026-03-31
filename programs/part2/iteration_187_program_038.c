#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

/* Function with high execution frequency */
void hot_function_1(int iterations) {
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
}

/* Function with medium execution frequency */
void medium_function(int mode) {
    int count = (mode == 1) ? 100 : 50;
    for (int i = 0; i < count; i++) {
        if (i % 3 == 0) {
            call_lib_function_a(i);
        } else if (i % 3 == 1) {
            call_lib_function_b(i);
        } else {
            call_lib_function_c(i);
        }
    }
}

/* Function with low execution frequency */
void cold_function(int mode) {
    if (mode == 1) {
        printf("Cold path A executed\n");
        rarely_called_function();
    } else {
        printf("Cold path B executed\n");
    }
}

/* Rarely called function */
void rarely_called_function(void) {
    printf("This function is rarely called\n");
}

/* Another hot function */
void hot_function_2(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (sum > 1000) {
            sum = 0;
        }
    }
    printf("Final sum: %d\n", sum);
}

/* Function with conditional paths */
void conditional_function(int mode) {
    switch (mode) {
        case 1:
            printf("Mode 1 - frequent path\n");
            break;
        case 2:
            printf("Mode 2 - less frequent path\n");
            break;
        case 3:
            printf("Mode 3 - rare path\n");
            break;
        default:
            printf("Default path\n");
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode %d\n", mode);
    
    /* Vary execution based on mode to create different profiles */
    if (mode == 1) {
        /* High frequency execution */
        hot_function_1(1000);
        hot_function_2(500);
        medium_function(1);
        conditional_function(1);
        cold_function(1);  /* Rare call in mode 1 */
    } else if (mode == 2) {
        /* Medium frequency execution */
        hot_function_1(500);
        hot_function_2(250);
        medium_function(2);
        conditional_function(2);
        cold_function(2);  /* Different cold path */
    } else if (mode == 3) {
        /* Low frequency with different paths */
        hot_function_1(100);
        medium_function(3);
        conditional_function(3);  /* Rare path */
        cold_function(3);
    } else {
        /* Default execution */
        hot_function_1(200);
        medium_function(0);
        conditional_function(99);  /* Default path */
    }
    
    /* Call library functions */
    process_data(mode * 10);
    
    return 0;
}
