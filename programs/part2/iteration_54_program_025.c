int x = 5;
int y = x + x++;  // UB: read and modification unsequenced
