#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>  // For boolean data type (bool, true, false)
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
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // 丟棄所有字元
    }
}

void pause_until_enter() {
    // 1. 清空任何殘留的輸入
    clear_input_buffer();

    bool keep_going = false;
    printf("Press Enter to return to main menu...");
    fflush(stdout); // 確保提示文字立即顯示

    while(!keep_going){
        if (getchar() == '\n') {
            keep_going = true;
        }
    }
}
int main() {
    int choice, n;
    clock_t start_time, end_time;
    double time_taken;
    bool invalid = false;

    while(1) {
        clear_screen();
        if(invalid){
            printf("Invalid choice. Please select 1, 2, or 3.\n");
            invalid = false;
        }
        printf("\n===========================================\n");
        printf(" The Intersection of Recursive Mysteries: \n");
        printf("     Nine Linked Rings & Tower of Hanoi    \n");
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
            wait_for_a_while(3000);
            break;
        }
        else if(choice != 1 && choice != 2) {
            invalid = true;
            // printf("Invalid choice. Please select 1, 2, or 3.\n");
            continue;
        }
        printf("Please enter the value of N (recommended 1~5, larger values may take longer): ");
        scanf("%d", &n);

        // Reset step counter
        global_step_count = 0;

        // Start timing
        start_time = clock();
        
        clear_screen();
        if (choice == 1) {
            printf("--- Starting Tower of Hanoi (N=%d) ---\n", n);
            printf("Initial state set to all disks ON stack A\n");
            // Call Student A's initialization + solver function
            // Assuming A provided this interface:
            solve_hanoi(n);

        }
        else if (choice == 2) {
            printf("\n--- Starting Nine Linked Rings (N=%d) ---\n", n);
            // Dynamic memory allocation for Xring array
	        int *Xring; // Pointer for dynamic array
            Xring = (int *)malloc(n * sizeof(int));
            if (Xring == NULL) {
                printf("Memory allocation failed.\n");
                return 1;
            }

            // --- MODIFICATION START ---
            // Force the initial state to all rings ON (all '1's)
            if (n > 0) {
                for (int i = 0; i < n; i++) {
                    Xring[i] = 1;
                }
            }
            printf("\nInitial state set to all rings ON (111...): \n");
            // --- MODIFICATION END ---

            // gotoxy(0, 12); // 回到左上角，而不是清除螢幕
            printf("The rings state of %d-Linked Ring is: ", n);
            print_ring(1, n, Xring);
            wait_for_a_while(1000);
            printf("\n");
            if(n != 0){
            // }{
                printf("\nLet's start to solve the %d-Linked Ring.\n", n);

                // When starting state is all '1's, the number of 'on' rings (X) is always odd.
                // Therefore, it should always start with R-rule (for N-Linked Rings starting from 11...1 to 00...0).

                if(find_lead(n, Xring) == -1) printf("The %d-Linked Ring is already solved!\n", n);
                else if(on_ring(n, Xring, 0) % 2 == 1){
                    // For the starting state 11...1, on_ring(X) = X.
                    // If X is odd, start with R. If X is even, start with S.
                    // Since the final move is R-rule (to turn 1st ring from 1 to 0),
                    // The standard solution for 11..1 to 00..0 uses the parity of X.

                    // Your original code's logic is based on the *current* state.
                    // For 11...1, on_ring(X) = X.

                    if(n % 2 == 1){
                        printf("Let's start to solve the %d-Linked Ring with R-rule(since %d rings are ON and it's odd)\n", n);
                        solve_Xring('R', n, Xring, 1);
                    }
                    else {
                        printf("Let's start to solve the %d-Linked Ring with S-rule(since %d rings are ON and it's even)\n", n);
                        solve_Xring('S', n, Xring, 1);
                    }
                }
                else{
                    printf("Start with S-rule !! (since %d rings are ON and it's even)\n", n);
                    solve_Xring('S', n, Xring, 1);
                }
	        }
        }

        // Stop timing
        end_time = clock();
        time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

        // === Verification and Analysis Report (as required by the proposal) ===
        printf("\n----------------------------------------\n");
        printf(" Algorithm Analysis Report\n");
        printf("----------------------------------------\n");
        printf("Actual number of steps : %llu\n", global_step_count);
        //printf("Execution time         : %.6f seconds\n", time_taken);

        // // Show theoretical comparison
        // unsigned long long theoretical = 0;
        // if (choice == 1) {
        //     theoretical = calc_hanoi_theoretical(n);
        //     printf("Theoretical minimum    : %llu (Formula: 2^N - 1)\n", theoretical);
        // } else {
        //     theoretical = calc_nine_ring_theoretical(n);
        //     printf("Theoretical minimum    : %llu (Formula to be verified)\n", theoretical);
        // }

        // if (theoretical > 0 && global_step_count == theoretical) {
        //     printf(">> Verification success! Actual steps match theory.\n");
        // } else if (theoretical > 0) {
        //     printf(">> Verification mismatch! Please check whether the recursion logic includes extra moves.\n");
        // }

        // // Verify exponential growth
        // printf("Complexity estimate    : O(2^%d)\n", n);
        printf("----------------------------------------\n");
        bool keep_going = false;
        pause_until_enter();
        printf("Exited loop!\n");
    }

    return 0;
}
