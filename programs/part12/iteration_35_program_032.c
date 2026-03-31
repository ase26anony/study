#include <stdio.h>
#include <stdlib.h>

// Hot function - will be called many times
void hot_function_1(int iterations) {
    for (int i = 0; i < iterations; i++) {
        // Hot block - executed many times
        if (i % 1000 == 0) {
            printf(".");  // Occasional output
        }
    }
}

// Cold function - rarely called
void cold_function_1(void) {
    printf("[Cold Function 1]\n");
    // Multiple branches for coverage
    int x = rand() % 10;
    if (x < 3) {
        printf("Branch A\n");
    } else if (x < 7) {
        printf("Branch B\n");
    } else {
        printf("Branch C\n");
    }
}

// Function with nested conditions
void complex_function_1(int value) {
    switch (value % 4) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            if (value > 100) {
                printf("Large value\n");
            }
            break;
        case 3:
            printf("Case 3\n");
            for (int i = 0; i < 10; i++) {
                printf("%d ", i);
            }
            printf("\n");
            break;
    }
}
