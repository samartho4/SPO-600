int compute(int mode, int a, int b) {
    if (mode == 1) return a + b;
    else return a * b;
}

int main() {
    return compute(1, 2, 3);
}
