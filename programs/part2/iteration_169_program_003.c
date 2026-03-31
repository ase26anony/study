/* test.c - Contains intentional warnings and errors for testing */
#include <stdio.h>

/* Deliberate warning: unused parameter */
int unused_function(int unused_param) {
    return 0;
}

/* Deliberate warning: implicit declaration */
void implicit_declaration_test() {
    printf("Testing warnings\n");
}

/* Function with potential format warning */
void format_warning_test() {
    int x = 5;
    printf(x);  /* Wrong format specifier - will trigger warning */
}

int main(int argc, char *argv[]) {
    /* Trigger unused variable warning */
    int unused_variable;
    
    /* Conditional compilation to test different paths */
    #ifdef TEST_ERROR
    This will cause a compilation error when TEST_ERROR is defined
    #endif
    
    printf("Test program\n");
    return 0;
}
