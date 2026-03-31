int get_and_increment(int* p) {
    (*p)++;
    return *p;
}

int x = 5;
if (get_and_increment(&x) > 5) {
    x = 10;
}
