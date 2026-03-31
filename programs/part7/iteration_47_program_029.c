/* Basic delimiter coverage for gengtype parser */
#ifndef DELIMITERS_H
#define DELIMITERS_H

/* Parentheses cases */
void simple_function(int param);
int (*function_pointer)(double, char);
void (*complex_func_ptr)(int (*)(void), char *);

/* Brackets cases */
extern int array_1d[10];
float matrix_2d[5][10];
char *string_array[] = {"hello", "world"};

/* Braces cases */
struct SimpleStruct {
    int id;
    double value;
};

union DataUnion {
    int int_val;
    float float_val;
    char char_val;
};

enum Color {
    RED,
    GREEN,
    BLUE
};

#endif /* DELIMITERS_H */
