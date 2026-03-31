/* Basic delimiter patterns to trigger all three cases */
#ifndef DELIMITERS_BASIC_H
#define DELIMITERS_BASIC_H

/* Parentheses cases */
void simple_function(int param);
int (*function_pointer)(double, char);
void (*complex_func_ptr)(int (*)(char), float);

/* Brackets cases */
int simple_array[10];
float matrix[5][10];
char *string_array[] = {"hello", "world"};

/* Braces cases */
struct SimpleStruct {
    int id;
    char name[50];
};

union SimpleUnion {
    int int_val;
    float float_val;
    char char_val;
};

enum SimpleEnum {
    VALUE1,
    VALUE2,
    VALUE3
};

/* Mixed delimiters in one declaration */
struct Mixed {
    int (*compare)(const void *, const void *);
    void (*handlers[5])(int);
    char data[100];
};

#endif /* DELIMITERS_BASIC_H */
