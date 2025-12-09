#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>  // For boolean data type (bool, true, false)
#include "common.h"     // Include shared definitions
#include "hanoi_ver2.h"      // Include Student A's header file
#include "nine.h"  // Include Student B's header file
#include <windows.h>

// The place where the global variable is actually defined
unsigned long long global_step_count = 0;

// Implementation of the function that increments the step counter
void increment_step() {
    global_step_count++;
}

void wait_for_a_while(int milliseconds) {
    if (milliseconds <= 0) return;
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif
}

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void gotoxy(int x, int y) {
    #ifdef _WIN32
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    #else
    printf("\033[%d;%dH", y + 1, x + 1);
    #endif
}

void hide_cursor(void) {
    #ifdef _WIN32
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
    #else
    printf("\033[?25l");
    #endif
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
    double hanoi_time_taken;
    double ring_time_taken;
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
            break;
        }
        else if(choice != 1 && choice != 2) {
            invalid = true;
            // printf("Invalid choice. Please select 1, 2, or 3.\n");
            continue;
        }
        else if(choice == 3){ // 自選九連環
            printf("Please enter the value of N (recommended 1–15, large values may take long): ");
            scanf("%d", &n);
            int nine_custom_step_count = nine_custom(n);
        }
        // else if(choice == 1 || choice == 2){
        //     printf("Please enter the value of N (recommended 1–15, large values may take long): ");
        //     scanf("%d", &n);
            
        //     // Reset step counter
        //     global_step_count = 0;

        //     // Start timing
        //     start_time = clock();
            
        //     clear_screen();
        //     printf("--- Starting Tower of Hanoi (N=%d) ---\n", n);
        //     printf("Initial state set to all disks ON stack A\n");
        //     solve_hanoi(n);
        //     printf("Press Enter to see steps of Nine Linked Rings...");
        //     pause_until_enter();
        //     int hanoi_step = global_step_count;
        //     global_step_count = 0;
        //     end_time = clock();
        //     hanoi_time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

        //     start_time = clock();
        //     clear_screen();
        //     printf("\n--- Starting Nine Linked Rings (N=%d) ---\n", n);
        //     int *Xring; // Pointer for dynamic array
        //     Xring = (int *)malloc(n * sizeof(int));
        //     if (Xring == NULL) {
        //         printf("Memory allocation failed.\n");
        //         return 1;
        //     }

        //     // --- MODIFICATION START ---
        //     // Force the initial state to all rings ON (all '1's)
        //     if (n > 0) {
        //         for (int i = 0; i < n; i++) {
        //             Xring[i] = 1;
        //         }
        //     }
        //     printf("\nInitial state set to all rings ON (111...): \n");
        //     // --- MODIFICATION END ---

        //     gotoxy(0, 12); // 回到左上角，而不是清除螢幕
        //     printf("The rings state of %d-Linked Ring is: ", n);
        //     print_ring(1, n, Xring);
        //     wait_for_a_while(1000);
        //     printf("\n");
        //     if(n != 0){
        //         printf("\nLet's start to solve the %d-Linked Ring.\n", n);
        //         if(find_lead(n, Xring) == -1) printf("The %d-Linked Ring is already solved!\n", n);
        //         else if(on_ring(n, Xring, 0) % 2 == 1){
        //             if(n % 2 == 1){
        //                 printf("Let's start to solve the %d-Linked Ring with R-rule(since %d rings are ON and it's odd)\n", n);
        //                 solve_Xring('R', n, Xring, 1);
        //             }
        //             else {
        //                 printf("Let's start to solve the %d-Linked Ring with S-rule(since %d rings are ON and it's even)\n", n);
        //                 solve_Xring('S', n, Xring, 1);
        //             }
        //         }
        //         else{
        //             printf("Start with S-rule !! (since %d rings are ON and it's even)\n", n);
        //             solve_Xring('S', n, Xring, 1);
        //         }
        //     }
        //     end_time = clock();
        //     ring_time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        // }
        

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
        
        printf("Exited loop!\n");
    }

    return 0;
}
