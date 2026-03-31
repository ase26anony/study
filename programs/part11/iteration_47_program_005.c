x₀ = 0
for i = 0 to n-1:
    // Phi node at loop header
    x₁ = φ(x₀, x₄)  // x₁ is the value of x at the start of iteration i
    if (x₁ == 0):
        x₂ = 1
    else:
        x₃ = 0
    // Phi node at merge point after if-else
    x₄ = φ(x₂, x₃)
