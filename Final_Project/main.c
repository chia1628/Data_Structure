#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "common.h"     // Include shared definitions
#include "hanoi.h"      // Include Student A's header file
#include "nine_ring.h"  // Include Student B's header file
#include <windows.h>


// The place where the global variable is actually defined
unsigned long long global_step_count = 0;

// Implementation of the function that increments the step counter
void increment_step() {
    global_step_count++;
}

// === Your core task: Mathematical analysis tools ===
// According to the project proposal, you need to verify the minimum number of moves
// for the N-disk/ring problems.

// Calculate theoretical steps for Tower of Hanoi: 2^N - 1  [cite: 52]
unsigned long long calc_hanoi_theoretical(int n) {
    return (unsigned long long)pow(2, n) - 1;
}


// Calculate theoretical steps for the Nine Linked Rings
// Note: The formula for the Nine Linked Rings is more complex and often depends
// on whether N is odd or even, or approximated as (2^(n+1)) / 3.
// Your goal is to *derive and verify* the correct formula.
// Here you can put a placeholder structure for your derived formula.
unsigned long long calc_nine_ring_theoretical(int n) {
    // This is where you will insert the formula you derived.
    // Hint: In many cases it is approximately (1 << (n+1)) / 3.
    if (n == 1) return 1;
    if (n == 2) return 2;
    // Temporarily return 0 or a formula you found
    return 0; 
}

int main() {
    int choice, n;
    clock_t start_time, end_time;
    double time_taken;

    while(1) {
        printf("\n===========================================\n");
        printf(" The Intersection of Recursive Mysteries: \n");
        printf("     Nine Linked Rings & Tower of Hanoi    \n"); // [cite: 2]
        printf("===========================================\n");
        printf("1. Tower of Hanoi\n");
        printf("2. Nine Linked Rings\n");
        printf("3. Exit Program\n");
        printf("Please choose (1-3): ");
        
        if (scanf("%d", &choice) != 1) {
            // Clear invalid input
            while(getchar() != '\n');
            continue;
        }

        if (choice == 3) {
            printf("Thank you for using the program. Goodbye!\n");
            break;
        }

        printf("Please enter the value of N (recommended 1–15, large values may take long): ");
        scanf("%d", &n);

        // Reset step counter
        global_step_count = 0;
        
        // Start timing
        start_time = clock();

        if (choice == 1) {
            printf("\n--- Starting Tower of Hanoi (N=%d) ---\n", n);
            
            // Call Student A's initialization + solver function
            // Assuming A provided this interface:
            solve_hanoi(n);
            
        } 
        // else if (choice == 2) {
        //     printf("\n--- Starting Nine Linked Rings (N=%d) ---\n", n);
            
        //     // Call Student B's initialization + solver function
        //     // Assuming B provided this interface:
        //     NineRingState game;
        //     init_nine_rings(&game, n); // written by Student B
        //     // The goal of Nine Linked Rings is to remove all rings
        //     remove_all_rings(&game, n); // written by Student B
        // }

        // Stop timing
        end_time = clock();
        time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

        // === Verification and Analysis Report (as required by the proposal) ===
        printf("\n----------------------------------------\n");
        printf(" Algorithm Analysis Report\n");
        printf("----------------------------------------\n");
        printf("Actual number of steps : %llu\n", global_step_count);
        printf("Execution time         : %.6f seconds\n", time_taken);
        
        // Show theoretical comparison
        unsigned long long theoretical = 0;
        if (choice == 1) {
            theoretical = calc_hanoi_theoretical(n);
            printf("Theoretical minimum    : %llu (Formula: 2^N - 1)\n", theoretical);
        } else {
            theoretical = calc_nine_ring_theoretical(n);
            printf("Theoretical minimum    : %llu (Formula to be verified)\n", theoretical);
        }

        if (theoretical > 0 && global_step_count == theoretical) {
            printf(">> Verification success! Actual steps match theory.\n");
        } else if (theoretical > 0) {
            printf(">> Verification mismatch! Please check whether the recursion logic includes extra moves.\n");
        }
        
        // Verify exponential growth
        printf("Complexity estimate    : O(2^%d)\n", n);
        printf("----------------------------------------\n");
    }

    return 0;
}
