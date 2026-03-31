#include <stdio.h>
#include <stdlib.h>
#include "lib1.h"
#include "lib2.h"

void hot_function(int iterations) {
    for (int i = 0; i < iterations; i++) {
        // Hot block - executed many times
        volatile int x = i * 2;
    }
}

void cold_function(int value) {
    if (value < 0) {
        printf("Negative value: %d\n", value);
    } else if (value == 0) {
        printf("Zero value\n");
    } else if (value < 100) {
        printf("Small positive: %d\n", value);
    } else {
        printf("Large positive: %d\n", value);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <run_id>\n", argv[0]);
        return 1;
    }
    
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("Failed to open input file");
        return 1;
    }
    
    int mode, iterations;
    if (fscanf(f, "%d %d", &mode, &iterations) != 2) {
        fprintf(stderr, "Invalid input format\n");
        fclose(f);
        return 1;
    }
    fclose(f);
    
    // Vary execution paths based on input
    switch (mode % 4) {
        case 0:
            hot_function(iterations);
            lib1_function_a(iterations / 2);
            break;
        case 1:
            cold_function(mode);
            lib2_function_a();
            break;
        case 2:
            hot_function(iterations * 2);
            lib1_function_b();
            lib2_function_b(mode);
            break;
        case 3:
            cold_function(-mode);
            lib1_function_a(10);
            lib2_function_a();
            break;
    }
    
    // Additional branching for more coverage
    if (iterations > 1000) {
        for (int i = 0; i < 100; i++) {
            // Another hot block
            volatile int y = i * i;
        }
    } else if (iterations < 10) {
        printf("Very few iterations: %d\n", iterations);
    }
    
    return 0;
}
