/* simple.h - Basic delimiter cases for gengtype parser coverage */

#ifndef SIMPLE_H
#define SIMPLE_H

/* Trigger '(' case with function prototypes */
void simple_function(int arg);
int calculate_sum(int a, int b, int c);
char* get_message(void);

/* Trigger '[' case with array declarations */
extern int numbers[10];
float matrix[3][4];
const char* strings[5];

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

/* Mixed default case characters */
typedef unsigned long int counter_t;
static volatile const int MAX_VALUE = 100;
enum Color { RED, GREEN, BLUE };

#endif /* SIMPLE_H */
