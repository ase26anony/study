/* Basic delimiter test cases for gengtype parser coverage */
#ifndef DELIMITERS_H
#define DELIMITERS_H

/* Parentheses cases */
void simple_function(int param);
int (*function_pointer)(double, char);
void (*complex_ptr)(int (*)(void), char *);

/* Brackets cases */
extern int array_1d[10];
float matrix_2d[5][20];
char *string_array[] = {"hello", "world"};

/* Braces cases */
struct SimpleStruct {
    int id;
    double value;
};

union DataUnion {
    int int_val;
    float float_val;
    char str_val[20];
};

enum Color {
    RED,
    GREEN,
    BLUE
};

/* Mixed delimiters */
struct Mixed {
    int (*compare)(const void *, const void *);
    void (*handlers[5])(int);
    char data[100];
};

#endif /* DELIMITERS_H */
