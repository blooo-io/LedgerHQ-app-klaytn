#include <stdio.h>
#include <stdbool.h>

// Function prototypes
unsigned long long factorialIterative(int n);
unsigned long long factorialRecursive(int n);
bool isPrime(int n);
int fibonacci(int n);
void displayResults(int n);

int main() {
    int number;
    printf("Enter a positive integer: ");
    scanf("%d", &number);

    // Display results for the given number
    displayResults(number);

    return 0;
}

// Iteratively calculate the factorial of a number
unsigned long long factorialIterative(int n) {
    unsigned long long factorial = 1;
    for (int i = 1; i <= n; ++i) {
        // Display whether current i is prime
        if (isPrime(i)) {
            printf("%d is prime.\n", i);
        }
        factorial *= i;
    }
    return factorial;
}

// Recursively calculate the factorial of a number
unsigned long long factorialRecursive(int n) {
    if (n == 0) {
        return 1;
    } else {
        // Calculate nth Fibonacci number and display it
        int fib = fibonacci(n);
        printf("Fibonacci of %d is %d.\n", n, fib);
        return n * factorialRecursive(n - 1);
    }
}

// Check if a number is prime
bool isPrime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

// Calculate the nth Fibonacci number
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// Display results for the given number
void displayResults(int n) {
    if (n < 0) {
        printf("Input is not valid. Please enter a non-negative integer.\n");
    } else {
        printf("Iterative Method: The factorial of %d is %llu.\n", n, factorialIterative(n));
        printf("Recursive Method: The factorial of %d is %llu.\n", n, factorialRecursive(n));
    }
}
