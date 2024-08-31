/**
 * @file quicksort_openmp.cpp
 * @brief A program to generate random integers, save them to a file, read them back, sort them, and save the sorted integers.
 * 
 * This program generates a specified number of random integers, saves them in a CSV file, 
 * reads them back from the file, sorts them using Quick Sort with parallelism, 
 * and then saves the sorted integers to another file. It demonstrates basic file operations, 
 * dynamic memory management, sorting algorithms, and parallel computing using OpenMP in C++.
 * 
 * @authors Flores T. Adrian, Ramirez R. Andrea
 * @date Aug 31
 */

#include <iostream>     // For standard input/output stream operations
#include <cmath>        // For mathematical functions (e.g., sqrt, pow)
#include <fstream>      // For file input/output operations
#include <string>       // For string manipulations
#include <cstdlib>      // For random number generation (rand, srand)
#include <ctime>        // For seeding the random number generator (time)
#include <chrono>       // For high-resolution clock and timing
#include <omp.h>        // For OpenMP parallelism

#define INFILE "input_numbers.csv"  // Name of the file where generated numbers will be saved
#define OUTFILE "sorted_numbers.csv"  // Name of the file where sorted numbers will be saved
#define N 100                       // Default number of random integers to generate

using namespace std;
using namespace std::chrono;

/**
 * @brief Generates an array of random integers.
 * 
 * This function generates 'n' random integers within the range [0, 999] and stores them 
 * in the provided array. The generation process is parallelized to improve performance for 
 * large arrays.
 * 
 * @param numbers A pointer to the array where the generated integers will be stored.
 * @param n The number of random integers to generate.
 */
void generate_random_numbers(int* numbers, int n) {
    for (int i = 0; i < n; i++) {
        numbers[i] = rand() % 1000;  // Generate a random integer between 0 and 999
    }
}

/**
 * @brief Writes an array of integers to a CSV file.
 * 
 * This function takes an array of integers and writes them to the specified file in CSV format. 
 * Each integer is separated by a comma. If the file cannot be opened, an error message is displayed.
 * 
 * @param numbers A pointer to the array of integers to be written to the file.
 * @param n The number of integers in the array.
 * @param filename The name of the file where the integers will be written.
 */
void write_numbers_to_file(const int* numbers, int n, const string& filename) {
    ofstream outfile(filename);
    if (outfile.is_open()) {
        for (int i = 0; i < n; i++) {
            outfile << numbers[i];
            if (i < n - 1) {
                outfile << ',';  // Separate numbers with commas
            }
        }
        outfile.close();  // Close the file after writing is complete
        cout << "Numbers written to " << filename << endl;
    } else {
        cerr << "Error opening file " << filename << endl;  // Display an error if the file cannot be opened
    }
}

/**
 * @brief Reads integers from a CSV file and stores them in an array.
 * 
 * This function reads integers from the specified CSV file and stores them in the provided array.
 * It returns the number of integers successfully read from the file.
 * 
 * @param numbers A pointer to the array where the read integers will be stored.
 * @param filename The name of the file to read the integers from.
 * @return int The number of integers read from the file. Returns -1 if the file could not be opened.
 */
int read_numbers_from_file(int* numbers, const string& filename) {
    ifstream infile(filename);
    if (infile.is_open()) {
        string line;
        int count = 0;
        while (getline(infile, line, ',')) {  // Read numbers separated by commas
            numbers[count++] = stoi(line);   // Convert string to integer and store in the array
        }
        infile.close();  // Close the file after reading is complete
        return count;
    } else {
        cerr << "Error opening file " << filename << endl;  // Display an error if the file cannot be opened
        return -1;
    }
}

/**
 * @brief Sorts an array of integers using the Quick Sort algorithm.
 * 
 * This function implements the Quick Sort algorithm, which is an efficient, 
 * in-place sorting algorithm. It selects a pivot element and partitions the 
 * array such that elements less than the pivot come before it and elements 
 * greater than the pivot come after it. The function recursively applies 
 * the same process to the sub-arrays. The sorting process is parallelized to 
 * improve performance for large arrays.
 * 
 * @param numbers A pointer to the array of integers to be sorted.
 * @param low The starting index of the sub-array to be sorted.
 * @param high The ending index of the sub-array to be sorted.
 */
void quickSort(int* numbers, int low, int high) {
    if (low >= high) {
        return; // Base case: If the sub-array has one or no elements, it is already sorted
    }
    int pivot = numbers[(high + low) / 2]; // Select the pivot element
    int new_low = low;
    int new_high = high;
    while (new_low <= new_high) {
        while (numbers[new_low] < pivot) new_low++;
        while (numbers[new_high] > pivot) new_high--;
        if (new_low <= new_high) {
            int tmp = numbers[new_low];
            numbers[new_low] = numbers[new_high];
            numbers[new_high] = tmp;
            new_low++;
            new_high--;
        }
    }

    #pragma omp task
    quickSort(numbers, low, new_high); // Sort the left sub-array

    #pragma omp task
    quickSort(numbers, new_low, high); // Sort the right sub-array
}

/**
 * @brief The main function that drives the program.
 * 
 * This function generates random integers, writes them to a file, reads them back from the file,
 * sorts them in parallel, and then writes the sorted integers to another file. The number of integers 
 * to generate can be specified via command-line arguments. If no argument is provided, a default value 
 * of 25 is used.
 * 
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments.
 * @return int Returns 0 upon successful execution.
 */
int main(int argc, char* argv[]) {
    srand(time(0));  // Seed the random number generator with the current time

    // Determine the number of integers to generate
    int n = N;  // Default value of N

    if (argc > 1) {
        string arg = argv[1];
        try {
            n = stoi(arg);  // Convert the argument to an integer
        } catch (exception &err) {
            cerr << "Error: The number of integers to generate must be an integer." << endl;
            return 1;  // Exit the program with an error code
        }
    }

    cout << "Generating " << n << " random integers" << endl;

    // Dynamically allocate memory for the array
    int* numbers = new int[n];  

    // Record the start time
    auto start = high_resolution_clock::now();

    // Generate random numbers
    generate_random_numbers(numbers, n);
    
    // Write the generated integers to a file
    write_numbers_to_file(numbers, n, INFILE);

    // Read the generated integers from file
    int count = read_numbers_from_file(numbers, INFILE);
    if (count > 0) {
        // Sort numbers from array in parallel
        #pragma omp parallel
        {
            #pragma omp single
            quickSort(numbers, 0, count - 1);
        }
        // Write the sorted integers to a file
        write_numbers_to_file(numbers, count, OUTFILE);
    }

    // Record the end time
    auto end = high_resolution_clock::now();

    // Calculate the elapsed time in seconds as a double
    duration<double> execution_time = (end - start);
    cout << "Execution time: " << execution_time.count() << " seconds" << endl;

    // Clean up dynamically allocated memory
    delete[] numbers;

    return 0;  // Indicate that the program ended successfully
}