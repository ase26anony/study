/* Basic delimiter coverage for gengtype parser */
#ifndef DELIMITERS_H
#define DELIMITERS_H

/* Parentheses cases */
void simple_function(int param);
int (*function_pointer)(double, char);
void (*complex_ptr)(int (*)(void), char *[]);

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
    int i;
    float f;
    char str[20];
};

enum Color {
    RED,
    GREEN,
    BLUE = 255
};

/* Mixed delimiters */
typedef struct {
    int (*compare)(const void *, const void *);
    void (*process)(int data[], int size);
} Operations;

#endif /* DELIMITERS_H */
