#include <stdio.h>

/*
 * First set of functions: These functions do identical operations regardless of optimization level,
 * so they should be pruned.
 */
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

// Resolver function for scale_samples
void scale_samples_resolver(int *data, int count) {
    scale_samples_O2(data, count);
}

/*
 * Second set of functions: These functions use vectorizable operations on arrays,
 * so different optimization levels will produce different results. These should NOT be pruned.
 */
__attribute__((optimize("O2")))
void process_array_O2(float *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * arr[i] + arr[i] * 2.5f;
    }
}

__attribute__((optimize("O3")))
void process_array_O3(float *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * arr[i] + arr[i] * 2.5f;
    }
}

// Resolver function for process_array
void process_array_resolver(float *arr, int size) {
    process_array_O3(arr, size);
}

/*
 * Main function to test the cloned functions
 */
int main() {
    // Test first set of functions (should be pruned)
    int int_data[5] = {1, 2, 3, 4, 5};
    scale_samples_resolver(int_data, 5);
    printf("Scaled integers: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", int_data[i]);
    }
    printf("\n");

    // Test second set of functions (should not be pruned)
    float float_data[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    process_array_resolver(float_data, 5);
    printf("Processed floats: ");
    for (int i = 0; i < 5; i++) {
        printf("%.2f ", float_data[i]);
    }
    printf("\n");

    return 0;
}
