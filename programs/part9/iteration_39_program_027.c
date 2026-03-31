i1 = 0
while (true) {
  i2 = φ(i1, i3)  // Phi node: i2 = i1 on first iteration, i3 on others
  if (i2 >= n) break
  if (i2 == 0) {
    // body - executes only on first iteration
  }
  i3 = i2 + 1
}
