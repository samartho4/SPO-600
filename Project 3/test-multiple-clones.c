#include <stdio.h>

/**
 * First function set: Basic scalar operation
 * Both variants are identical and should be pruned
 */
__attribute__((optimize("O2")))
int add_numbers_O2(int a, int b) {
    return a + b;
}

__attribute__((optimize("O3")))
int add_numbers_O3(int a, int b) {
    return a + b;
}

void add_numbers_resolver(int a, int b) {
    add_numbers_O2(a, b);
}

/**
 * Second function set: Array operation that can be vectorized
 * The O3 version should be vectorized differently than O2
 * So these should NOT be pruned
 */
__attribute__((optimize("O2")))
void process_array_O2(int *data, int count) {
    for (int i = 0; i < count; i++) {
        data[i] = data[i] * 2 + 5;
    }
}

__attribute__((optimize("O3")))
void process_array_O3(int *data, int count) {
    for (int i = 0; i < count; i++) {
        data[i] = data[i] * 2 + 5;
    }
}

void process_array_resolver(int *data, int count) {
    process_array_O2(data, count);
}

/**
 * Third function: Mixed scenario with both scalar and 
 * vectorizable operations
 */
__attribute__((optimize("O2")))
void mixed_ops_O2(int *data, int count, int scalar) {
    int local = scalar * 2;
    
    printf("Scalar value: %d\n", local);
    
    for (int i = 0; i < count; i++) {
        data[i] = data[i] * local;
    }
}

__attribute__((optimize("O3")))
void mixed_ops_O3(int *data, int count, int scalar) {
    int local = scalar * 2;
    
    printf("Scalar value: %d\n", local);
    
    for (int i = 0; i < count; i++) {
        data[i] = data[i] * local;
    }
}

void mixed_ops_resolver(int *data, int count, int scalar) {
    mixed_ops_O2(data, count, scalar);
}

int main() {
    int result = add_numbers_resolver(5, 7);
    printf("Add result: %d\n", result);
    
    int data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    process_array_resolver(data, 10);
    
    printf("Processed array: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");
    
    int more_data[5] = {10, 20, 30, 40, 50};
    mixed_ops_resolver(more_data, 5, 3);
    
    printf("Mixed ops result: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", more_data[i]);
    }
    printf("\n");
    
    return 0;
}
