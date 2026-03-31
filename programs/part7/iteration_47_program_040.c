/* Basic delimiter coverage for gengtype parser */
#ifndef DELIMITERS_H
#define DELIMITERS_H

/* Parentheses cases */
void simple_function(int param);
int (*function_pointer)(double, char);
void (*complex_ptr)(int (*)(void), char *[]);

/* Brackets cases */
int array_1d[10];
float matrix[5][5];
char *string_array[] = {"hello", "world"};

/* Braces cases */
struct Point {
    int x;
    int y;
};

union Data {
    int i;
    float f;
    char str[20];
};

enum Color { RED, GREEN, BLUE };

#endif /* DELIMITERS_H */
