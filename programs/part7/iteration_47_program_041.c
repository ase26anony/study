/* Basic delimiter coverage for gengtype parser testing */
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
struct SimpleStruct {
    int id;
    char name[50];
};

union DataUnion {
    int i;
    float f;
    char str[20];
};

enum Color { RED, GREEN, BLUE };

/* Mixed delimiters */
typedef struct {
    int (*compare)(const void *, const void *);
    void (*print)(char format[], ...);
} Operations;

#endif /* DELIMITERS_H */
