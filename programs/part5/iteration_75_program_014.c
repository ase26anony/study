int t1 = arr[i * 2] * 3;
int t2 = arr[i * 2 + 1] * 5;
int t3 = t1 + t2;
int t4 = t3 << (i & 3);
sum += t4;
// t1 and t2 are still live here
int t5 = sum ^ t1;
int t6 = t5 * t2;
arr[i] = t6;
