#include <stdio.h>

// First function with two variants
__attribute__((optimize("O2")))
void scale_samples_O2(int *data, int count) {
    for (int i = 0; i < count; i++) {
        data[i] = data[i] * 2;
    }
}

__attribute__((optimize("O3")))
void scale_samples_O3(int *data, int count) {
    for (int i = 0; i < count; i++) {
        data[i] = data[i] * 2;
    }
}

// Resolver function
void scale_samples_resolver(int *data, int count) {
    scale_samples_O2(data, count);
}

int main() {
    int data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    scale_samples_resolver(data, 10);
    for (int i = 0; i < 10; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");
    return 0;
}
