/* Basic delimiter test cases for gengtype coverage */
#ifndef DELIMITERS_H
#define DELIMITERS_H

/* Parentheses cases */
void simple_function(int param);
int (*function_pointer)(int, char);
void (*complex_func_ptr)(int (*)(double), char *);

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

/* Mixed delimiters */
typedef struct {
    int (*compare)(const void *, const void *);
    void (*process)(int data[], int size);
} Operations;

#endif /* DELIMITERS_H */
