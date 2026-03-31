// Possible reordering without barrier:
int temp1 = b + 1;  // b = 2, temp1 = 3
int temp2 = temp1 * 2;  // temp2 = 6
c = temp2;  // c = 6
a = temp1;  // a = 3
b = temp2 - temp1;  // b = 3
