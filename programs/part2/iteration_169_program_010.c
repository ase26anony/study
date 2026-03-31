/* test.c - Source file with intentional issues for testing */
#include <stdio.h>

/* Deliberate warning: unused parameter */
int unused_function(int unused_param) {
    return 0;
}

/* Deliberate warning: implicit declaration */
void implicit_declaration_test() {
    printf("Testing\n");
}

/* Function with potential error if -Werror is used */
int main(int argc, char *argv[]) {
    int x; /* Uninitialized variable warning */
    
    if (argc > 1) {
        printf("Arguments: %d\n", argc);
    }
    
    /* This will cause a warning about unused variable */
    int y = 0;
    
    /* Return statement to avoid -Wreturn-type warning */
    return 0;
}

/* Missing semicolon to cause syntax error in one version */
int bad_function() 
{
    return 1 /* Missing semicolon */
}
