/* simple.h - Basic delimiter cases for coverage */

#ifndef SIMPLE_H
#define SIMPLE_H

/* Trigger '(' case - function prototypes */
void simple_func(int arg);
int calculate_sum(int a, int b, int c);
char* get_message(void);

/* Trigger '[' case - array declarations */
extern int numbers[10];
float matrix[3][4];
char buffer[256];

/* Trigger '{' case - structure definitions */
struct Point {
    int x;
    int y;
};

union Data {
    int i;
    float f;
    char str[20];
};

/* Default case triggers - various non-delimiter characters */
static const volatile unsigned long counter = 0;
typedef int* IntPtr;
enum Color { RED, GREEN, BLUE };

#endif /* SIMPLE_H */
