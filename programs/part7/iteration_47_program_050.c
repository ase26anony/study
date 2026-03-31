/* Basic delimiter coverage header */
#ifndef DELIMITERS_H
#define DELIMITERS_H

/* Parentheses cases */
void simple_function(int param);
int (*function_pointer)(int, char);
void (*complex_ptr)(int (*)(void), char *);

/* Brackets cases */
int array_simple[10];
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

#endif /* DELIMITERS_H */
