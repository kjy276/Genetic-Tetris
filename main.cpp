#include "Tetris.h"

void RemoveCursor(void);

int main() {
	RemoveCursor();
	block move_one;
	block ghost_one;
	int history_num = 0;
	for (int i = 0; i < 5; i++) {
		move_one.dots[i] = dot();
		ghost_one.dots[i] = dot();
	}
	move_one.blocktype = 0;
	move_one.rotate = -1;
	move_one.is_ghost = 0;

	int seq_num = gamestart();
	for (int m = 0; m < seq_num; m++) {
		history_num = get_gene(); // 0~1 gene. parent. (previous genertion, 1st, 2nd gene)
		crossover_mutate_gene(); // 2~12 gene. child. 
		// all_random_gene(); // test

		ofstream outFile("gene_history.txt", ios::app);
		outFile << endl << "generation " << history_num + 1 << endl;


		system("cls");
		for (int n = 2; n < 12; n++) {
			gameover = 0;
			gotoxy(0, 23);
			cout << "GEN (" << m + 1 << "/" << seq_num << ") PHASE (" << n + 1 << "/12)";

			tetris_genome[n][24] = 0;

			clearmap();
			printmap(n);

			srand(13);

			create_block(&move_one, &ghost_one, rand() % 7 + 1, n);

			while (gameover == 0) {
				for (int i = 0; i < 3; i++) {
					Sleep(tempo);
					if (move_one.dots[0].x == target_x_value && move_one.rotate == target_r_value) {
						move_drop(move_one, ghost_one, n);
					}
					else if (move_one.dots[0].x < target_x_value) {
						move_right(move_one);
					}
					else if (move_one.dots[0].x != target_x_value) {
						move_left(move_one);
					}
					if (move_one.rotate != target_r_value) {
						rotate_block(move_one);
					}
				}
				Sleep(tempo);
				move_down(move_one, ghost_one, n);
			}//do tetris game

			print_deadmap(n);

			gotoxy(0, 23);
			cout << "GEN (" << m + 1 << "/" << seq_num << ") PHASE (" << n + 1 << "/12)";

			for (int i = 0; i < 24; i++) {
				outFile << tetris_genome[n][i] << " ";
			}
			outFile << endl << tetris_genome[n][24] << endl;
			Sleep(100);

		}
		outFile.close();

		ofstream outFile2("gene_now.txt");
		outFile2 << "generation " << history_num + 1 << endl;

		int max_score, max_i, first_score;

		max_score = tetris_genome[0][24];
		max_i = 0;
		for (int i = 1; i < 12; i++) {
			if (max_score <= tetris_genome[i][24]) {
				max_score = tetris_genome[i][24];
				max_i = i;
			}
		}
		for (int i = 0; i < 24; i++) {
			outFile2 << tetris_genome[max_i][i] << " ";
		}
		outFile2 << endl << tetris_genome[max_i][24] << endl;
		first_score = tetris_genome[max_i][24];
		tetris_genome[max_i][24] = 0;
		// fisrt parent

		for (int j = 0; j < 1; j++) {
			max_score = tetris_genome[0][24];
			max_i = 0;
			for (int i = 1; i < 12; i++) {
				if (max_score <= tetris_genome[i][24]) {
					max_score = tetris_genome[i][24];
					max_i = i;
				}
			}
			if (tetris_genome[max_i][24] == first_score) {
				tetris_genome[max_i][24] = 0;
				j--;
			}
			else {
				for (int i = 0; i < 24; i++) {
					outFile2 << tetris_genome[max_i][i] << " ";
				}
				outFile2 << endl << tetris_genome[max_i][24] << endl;
				tetris_genome[max_i][24] = 0;
			}
		}// second parent
		outFile2.close();
		Sleep(2000);
	}
	return 0;
}
void RemoveCursor(void)
{
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0; // bVisible 멤버 변경
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo); // 변경값 적용
}