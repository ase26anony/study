// Pseudo-code showing the dependency
cond_i = (i == 0) ? initial_cond : cond_{i-1}
if (cond_i > 0) {
    arr[i] = i;
    cond_i = i;  // This becomes cond_{i+1} for next iteration
}
