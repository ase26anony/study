/* simple.h - Basic delimiter coverage */
#ifndef SIMPLE_H
#define SIMPLE_H

/* Trigger '(' case - function prototype */
void simple_function(int param);

/* Trigger '[' case - array declaration */
extern int numbers[10];

/* Trigger '{' case - structure definition */
struct SimpleStruct {
    int member1;
    char member2;
};

/* Trigger default case - keywords and identifiers */
static const volatile unsigned long counter = 0;

#endif /* SIMPLE_H */
