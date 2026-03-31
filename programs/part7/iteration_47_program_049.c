/* Basic delimiter testing header for gengtype coverage */
#ifndef DELIMITERS_H
#define DELIMITERS_H

/* Parentheses cases */
void simple_function(int param);
int (*function_pointer)(double, char);
void (*complex_func_ptr)(int (*)(char), float);

/* Brackets cases */
extern int array_1d[10];
float matrix_2d[5][10];
char *string_array[] = {"hello", "world"};

/* Braces cases */
struct SimpleStruct {
    int id;
    char name[50];
};

union DataUnion {
    int int_val;
    float float_val;
    char str_val[100];
};

enum Color {
    RED,
    GREEN,
    BLUE
};

/* Mixed delimiters */
typedef struct Node {
    int value;
    struct Node *next;
} Node_t;

#endif /* DELIMITERS_H */
