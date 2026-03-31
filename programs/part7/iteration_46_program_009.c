/* simple.h - Basic delimiter cases for coverage */

#ifndef SIMPLE_H
#define SIMPLE_H

/* Trigger '(' case with function prototypes */
void simple_func(int arg);
int calculate(int a, int b, int c);
char* get_string(void);

/* Trigger '[' case with array declarations */
extern int numbers[10];
float matrix[3][4];
char buffer[256];

/* Trigger '{' case with structure definitions */
struct Point {
    int x;
    int y;
    int z;
};

union Data {
    int i;
    float f;
    char str[20];
};

/* Default case triggers - identifiers, keywords, operators */
static const volatile unsigned long counter = 0;
typedef struct Point Point_t;
enum Color { RED, GREEN, BLUE };

#endif /* SIMPLE_H */
