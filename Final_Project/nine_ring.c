#include <stdio.h>
#include <stdlib.h> // for malloc and free
#include <string.h> // for memset
#include <windows.h> // 為了使用 Sleep
#include "common.h"
#include "nine_ring.h"
#include "hanoi.h"

void print_ring(int cnt_print, int X, int Xring[]){

	if(cnt_print <= X){
		//printf("%d", Xring[cnt_print - 1]);
        if(Xring[cnt_print - 1]==1){
            printf("  (=)  ");
        }
        else {
            printf("  ---  ");
        }
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

void solve_Xring(char cmd, int X, int Xring[], int cnt_move){
    increment_step();
	int lead = find_lead(X, Xring);
	gotoxy(0, 10); // 回到左上角，而不是清除螢幕
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

	switch(cmd){
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
    // clear_screen();

	printf("The rings state of %d-Linked Ring is: ", X);
	print_ring(1, X, Xring);
    wait_for_a_while();
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
