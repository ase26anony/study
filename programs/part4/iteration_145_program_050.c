// Possible optimization without barrier:
int a = 1, b = 2, c = 3;
int temp1 = b + 1;  // b + 1 = 3
int temp2 = temp1 * 2;  // a * 2 = 6
b = temp2 - temp1;  // c - a = 3
a = temp1;
c = temp2;
