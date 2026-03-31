a[i] = b[i] * c[i] + a[i-1];  // When i = 0, a[i-1] = a[-1] → OUT OF BOUNDS!
