//PA2 - Converted to C (Starting state fixed to all '1's)
//20221018 started
//20221022 Xring_input, inputXring, print_ring, find_lead, solve_Xring, FINISH
//20221023 fix use R/S to be the first's judgement
//20221114 fix st/nd/rd of numbers > 10
//20221115 fix line 43 "linked" to "Linked", If S-rule judgement
#include <stdio.h>
#include <stdlib.h> // for malloc and free
#include <string.h> // for memset

// Function Prototypes (Declaration)
// NOTE: input_Xring is removed as per user request.
void print_ring(int cnt_print, int X, int Xring[]);
int find_lead(int cnt_lead, int Xring[]);
int on_ring(int X, int Xring[], int on_num);
void solve_Xring(char cmd, int X, int Xring[], int cnt_move);

int main()
{
	/*
	X for the number of the rings
	Xring for the status of the rings
	S_lead for the number of the rightmost ring on the sword
	*/
	int X, S_lead;
	int *Xring; // Pointer for dynamic array

	printf("Welcome to play X-Linked Ring!\n");
	printf("How many X-Linked Ring do you want to solve? (N)\n");

	if (scanf("%d", &X) != 1 || X < 0) {
		printf("Invalid input for number of rings.\n");
		return 1;
	}

	// Dynamic memory allocation for Xring array
	Xring = (int *)malloc(X * sizeof(int));
	if (Xring == NULL) {
		printf("Memory allocation failed.\n");
		return 1;
	}

    // --- MODIFICATION START ---
    // Force the initial state to all rings ON (all '1's)
    if (X > 0) {
        // Initialize all X elements to 1.
        // Note: memset is typically for bytes, but since we are setting int to 1
        // and 1 is represented as 0x0...01, this works correctly.
        // A safer way for integers is a loop, but memset is common for efficiency:
        // for (int i = 0; i < X; i++) Xring[i] = 1;

        // Simpler implementation for this specific case (setting to 1)
        // is to iterate, as memset is strictly for setting bytes.
        for (int i = 0; i < X; i++) {
            Xring[i] = 1;
        }
    }
    printf("\nInitial state set to all rings ON (111...): \n");
    // --- MODIFICATION END ---


	printf("The rings state of %d-Linked Ring is: ", X);
	print_ring(1, X, Xring);

	// The logic for R-rule/S-rule preview remains,
    // but the S-rule preview is often not applicable
    // when all rings are '1' and X>1, as the lead is the outermost ring (X).

	printf("\nIf run R-rule once, the rings state of %d-Linked Ring is: ", X);
	if(X != 0)
	{
		print_ring(1, X - 1, Xring);
		if(Xring[X - 1] == 1) printf("0");
		else printf("1");
	}

	printf("\nIf run S-rule once, the rings state of %d-Linked Ring is: ", X);
	if(X != 0)
	{
		S_lead = find_lead(X, Xring);
		// When all are '1's, S_lead will be X-1 (index of the outermost ring).
        // S-rule operates on the ring to the LEFT of S_lead, at index S_lead - 1.

		if(S_lead >= 0)
		{
            // The ring being operated on is at index lead - 1,
            // which is the ring numbered S_lead (if S_lead > 0).

            int target_ring_index = S_lead - 1;

            if (target_ring_index >= 0) {
                // Print rings before the target ring
                print_ring(1, target_ring_index, Xring);

                // Print the flipped state of the target ring
                if(Xring[target_ring_index] == 0) printf("1");
                else printf("0");

                // Print rings after the target ring
                if (target_ring_index + 2 <= X) {
                    print_ring(target_ring_index + 2, X, Xring);
                }
            } else {
                // This branch is for X=1. The logic here for S-rule is slightly ambiguous
                // in the original code's preview, but the solve logic handles X=1 correctly.
                printf("Can not apply S-rule on 1 ring!");
            }
		} else {
			printf("Can not apply S-rule!");
		}

		printf("\nLet's start to solve the %d-Linked Ring.\n", X);

		// When starting state is all '1's, the number of 'on' rings (X) is always odd.
        // Therefore, it should always start with R-rule (for N-Linked Rings starting from 11...1 to 00...0).

		if(find_lead(X, Xring) == -1) printf("The %d-Linked Ring is already solved!\n", X);
		else if(on_ring(X, Xring, 0) % 2 == 1)
		{
            // For the starting state 11...1, on_ring(X) = X.
            // If X is odd, start with R. If X is even, start with S.
            // Since the final move is R-rule (to turn 1st ring from 1 to 0),
            // The standard solution for 11..1 to 00..0 uses the parity of X.

            // Your original code's logic is based on the *current* state.
            // For 11...1, on_ring(X) = X.

			if(X % 2 == 1)
            {
                printf("Start with R-rule !! (since %d rings are ON and it's odd)\n", X);
			    solve_Xring('R', X, Xring, 1);
            } else {
                printf("Start with S-rule !! (since %d rings are ON and it's even)\n", X);
			    solve_Xring('S', X, Xring, 1);
            }
		}else{
			printf("Start with S-rule !! (since %d rings are ON and it's even)\n", X);
			solve_Xring('S', X, Xring, 1);
		}
	}

	if(X == 0) printf("\n\n");
	printf("Thanks for using!! Goodbye ~\n");

	// Free dynamically allocated memory
	free(Xring);

	return 0;
}


// Removed input_Xring as per user request

void print_ring(int cnt_print, int X, int Xring[])
{
	if(cnt_print <= X)
	{
		printf("%d", Xring[cnt_print - 1]);
		print_ring(cnt_print + 1, X, Xring);
	}
}

// find_lead returns the array index (0-based) of the rightmost '1' ring, or -1 if all are '0'.
int find_lead(int cnt_lead, int Xring[])
{
	if(cnt_lead < 1) return -1;
	else if(Xring[cnt_lead - 1] == 1) return (cnt_lead - 1);
	else return find_lead(cnt_lead - 1, Xring);
}

void solve_Xring(char cmd, int X, int Xring[], int cnt_move)
{
	int lead = find_lead(X, Xring);

	if (lead == -1) {
		// This should theoretically not be hit if the call is made correctly,
        // as the solved state (-1) is checked before the recursive call.
        // It serves as a safety exit.
		if(cnt_move > 1) { // If it's 1, it means the solved state was reached in the previous step (cnt_move - 1)
            printf("\nThe %d-Linked Ring is solved in %d step", X, cnt_move - 1);
            if((cnt_move - 1) > 1) printf("s.\n");
            else printf(".\n");
        }
		return;
	}

	switch(cmd)
	{
		case 'R':
			// R-rule applies to the outermost ring (index X-1)
			Xring[X - 1] = (Xring[X - 1] == 0) ? 1 : 0; // Flip the state

			if(X == 1) printf("!! Turn the 1st ring ");
			else if(X == 2) printf("!! Turn the 2nd ring ");
			else if(X == 3) printf("!! Turn the 3rd ring ");
			else printf("!! Turn the %dth ring ", X);

			if(Xring[X - 1] == 0) printf("down !!\n");
			else printf("on !!\n");
			break;

		case 'S':
			// S-rule applies to the ring LEFT of the rightmost '1' (index 'lead')
			// The ring to operate on is at index lead - 1

			if (lead <= 0) {
				printf("!! ERROR: S-rule cannot operate on a ring left of the 1st ring!!\n");
				return;
			}

			Xring[lead - 1] = (Xring[lead - 1] == 0) ? 1 : 0; // Flip the state
            // The ring number being turned is 'lead' (1-based index)

			if(lead == 1) printf("!! Turn the 1st ring ");
			else if(lead == 2) printf("!! Turn the 2nd ring ");
			else if(lead == 3) printf("!! Turn the 3rd ring ");
			else printf("!! Turn the %dth ring ", lead);

			if(Xring[lead - 1] == 0) printf("down !!\n");
			else printf("on !!\n");

			break;
	}

	printf("The rings state of %d-Linked Ring is: ", X);
	print_ring(1, X, Xring);
	printf("\n");

    // Check for the solved state *after* the move
	if(find_lead(X, Xring) != -1){
		if(cmd == 'R') solve_Xring('S', X, Xring, cnt_move + 1);
		else solve_Xring('R', X, Xring, cnt_move + 1);
	}else{
		// Solved state reached
		printf("\nThe %d-Linked Ring is solved in %d step", X, cnt_move);

		if(cnt_move > 1) printf("s.\n");
		else printf(".\n");
	}
}

// Counts the number of rings that are "on the sword" (state 1)
int on_ring(int X, int Xring[], int on_num)
{
	if(X == 0) return on_num;
	if(Xring[X - 1] == 1) on_num++;

	return on_ring(X - 1, Xring, on_num);
}
