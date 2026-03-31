/* simple.h - Basic delimiter coverage */
#ifndef SIMPLE_H
#define SIMPLE_H

/* Trigger '(' case with function prototypes */
void simple_func(int arg);
int calculate(int x, int y);
char* get_string(void);

/* Trigger '[' case with array declarations */
extern int numbers[10];
float matrix[3][4];
char buffer[256];

/* Trigger '{' case with structure definitions */
struct Point {
    int x;
    int y;
};

union Data {
    int i;
    float f;
    char str[20];
};

/* Trigger default case with various non-delimiter characters */
typedef unsigned long size_t;
static const volatile int global_counter = 0;
enum Color { RED, GREEN, BLUE };

#endif /* SIMPLE_H */
