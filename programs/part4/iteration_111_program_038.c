// NOT safe to transform to:
if (cond > 0) {  // Can't hoist this!
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
        cond = i;
    }
}
