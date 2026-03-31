void copy(struct S10 *d, const struct S10 *s) {
    for(int i = 0; i < 10; i++) {
        d->a[i] = s->a[i];
    }
}
