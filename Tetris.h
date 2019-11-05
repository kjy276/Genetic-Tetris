#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <fstream>
#include <string>

using namespace std;
#define MAP_X 10
#define MAP_Y 20

int level = 1000;
int map1[MAP_Y][MAP_X];
int target_max_value;
int target_x_value;
int target_r_value;
// int target[10][4]; // test
int tetris_genome[12][25] = { 0 }; // (left, below, right) touch bonus 3 x 7 = 21 / line_break bonus = 22 / make_hole_panulty = 23 / low_y_bonus = 24 / score_record = 25
int tempo = 10;

int gameover = 0;

struct dot {
	int x;
	int y;
	dot() {
		x = 0;
		y = 0;
	}
};

struct block {
	dot dots[5]; // dots[0] makes it easy to rotate and value check.
	int is_ghost; // 0 = real block, 1 = ghost block.
	int blocktype; // 1 ~ 7 => I J L O S T Z shape
	int rotate; // 0 ~ 3 have direction. -1 means before initialized
	block() {
	}
};
void create_block(block *one, block *ghost, int type, int num);
void create_ghost_block(block *ghost, int type, int num);
void clearmap(void);
void printmap(int num);
void move_down(block &input, block &ghost, int num);
void move_drop(block &input, block &ghost, int num);
void move_left(block &input);
void move_right(block &input);
void color_set(block input);
void color_set(int input);
void color(unsigned short color);
void gotoxy(int x, int y);
void rotate_block(block &input);
void break_line_check(int num);
void print_deadmap(int num);
void get_target_info(block *input, int num);
int get_gene(void);
void block_over(block *input, block *ghost, int num);

int gamestart(void) {
	int temp;
	do
	{
		cout << "Please input Generation(more than 0) : ";
		cin >> temp;
	} while (temp < 1);

	do
	{
		cout << "Please input tempo(more than 4) : ";
		cin >> tempo;
	} while (tempo < 5);
	return temp;
}

/* get gene from file */
int get_gene(void) {
	string buf;
	//int gen_input[24];
	int history_num;
	ifstream inFile("gene_now.txt");
	inFile >> buf;
	inFile >> history_num;
	if (inFile.is_open()) {
		for (int i = 0; i < 25; i++) {
			inFile >> tetris_genome[0][i];
		}
		for (int i = 0; i < 25; i++) {
			inFile >> tetris_genome[1][i];
		}
		inFile.close();
		return history_num;
	}
	else {
		for (int i = 0; i < 25; i++) {
			tetris_genome[0][i] = 0;
		}
		for (int i = 0; i < 25; i++) {
			tetris_genome[1][i] = 0;
		}
		return 0;
	}
}

/* make child from parent */
void crossover_mutate_gene(void) {
	srand(time(NULL));
	int temp;
	for (int i = 2; i < 12; i++) {
		for (int j = 0; j < 24; j++) {
			temp = rand();
			if (temp % 11 < 5) {
				tetris_genome[i][j] = tetris_genome[0][j];
			}
			else if (temp % 11 < 10) {
				tetris_genome[i][j] = tetris_genome[1][j];
			}
			else {
				tetris_genome[i][j] = rand() % 100;
			}
		}
	}
}

/* make all random gene for test */
void all_random_gene(void) {
	srand(time(NULL));
	for (int j = 0; j < 12; j++) {
		for (int i = 0; i < 24; i++) {
			tetris_genome[j][i] = rand() % 100;
		}
	}
}

/* Clear Map */
void clearmap(void) {
	for (int i = 0; i < MAP_Y; i++) {
		for (int j = 0; j < MAP_X; j++) {
			map1[i][j] = 0;
		}
	}
}

/* Print score */
void printscore(int num) {
	color(15);
	gotoxy(MAP_X, num);
	cout << " phase " << num + 1 << "/ ";
	for (int i = 0; i < 24; i++) {
		cout << tetris_genome[num][i] << " ";
	}
	cout << "/score: " << tetris_genome[num][24];
}

/* Print Map */
void printmap(int num) {

	// system("cls");
	for (int i = 0; i < MAP_Y; i++) {
		for (int j = 0; j < MAP_X; j++) {
			if (map1[i][j] == 0) {
				color_set(map1[i][j]);
				gotoxy(j, i);
				printf("%c%c", 0xa1, 0xe0);	// empty box
			}
			else {
				color_set(map1[i][j]);
				gotoxy(j, i);
				printf("%c%c", 0xa1, 0xe1); // full box
			}
		}/*
		cout << "          ";
		for (int j = 0; j < MAP_X; j++) {
			color_set(map1[i][j]);
			cout << map1[i][j];
		}
		cout << endl;
		*/
	}
	/*
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 10; j++) {
			cout << target[j][i] << " ";
		}
		cout << endl;
	}*/
	for (int i = 0; i <= num; i++) {
		printscore(i);
	}
	// cout << "T_M_V : " << target_max_value << " T_M_X : " << target_x_value << " T_M_R : " << target_r_value << " score: " << score << endl;
}
/* Print white map */
void print_deadmap(int num) {
	system("cls");
	color(15);
	for (int i = 0; i < MAP_Y; i++) {
		for (int j = 0; j < MAP_X; j++) {
			if (map1[i][j] == 0) {
				gotoxy(j, i);
				printf("%c%c", 0xa1, 0xe0); // empty box
			}
			else {
				gotoxy(j, i);
				printf("%c%c", 0xa1, 0xe1); // full box
			}
		}
		//cout << endl;
	}
	for (int i = 0; i <= num; i++) {
		printscore(i);
	}
}

/* Make block and compute their target location */
void create_block(block *one, block *ghost, int type, int num) {
	target_max_value = -2e9;
	/*for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 4; j++) {
			target[i][j] = 0; //
		}
	}*/
	for (int i = 1; i <= 11; i++) {
		for (int j = 0; j < 4; j++) {
			ghost->is_ghost = i;
			ghost->rotate = j - 1;
			create_ghost_block(ghost, type, num);
		}
	}

	one->rotate = -1;
	one->blocktype = type;
	one->dots[0].x = 4;
	one->dots[0].y = 0;
	one->is_ghost = 0;
	rotate_block(*one);

	if (one->rotate == -1 && !one->is_ghost) {
		gameover = 1;
	}
	tetris_genome[num][24] += 10;
}

/* create ghost block for compute block's target location */
void create_ghost_block(block *ghost, int type, int num) {
	ghost->blocktype = type;
	ghost->dots[0].x = 0;
	ghost->dots[0].y = 0;
	rotate_block(*ghost);

	if (ghost->is_ghost <= 2) {
		for (int i = 0; i < ghost->is_ghost; i++) {
			move_left(*ghost);
		}
	} // x = 0~2
	else {
		for (int i = 0; i < ghost->is_ghost - 3; i++) {
			move_right(*ghost);
		}
	} // x = 3~

	move_drop(*ghost, *ghost, num);
}

/* if block touch bottom case */
void block_over(block *input, block *ghost, int num) {
	if (!input->is_ghost) {
		for (int i = 1; i <= 4; i++) {
			map1[input->dots[i].y][input->dots[i].x] = input->blocktype;
		}
		break_line_check(num);
		input->rotate = -1;
		create_block(input, ghost, rand() % 7 + 1, num);
		printmap(num);

	}
	else {
		get_target_info(input, num);
	}
}

/* compute target location */
void get_target_info(block *input, int num) {
	int sum = 0;
	for (int i = 1; i <= 4; i++) {
		if (input->dots[i].x == 0) {
			sum += tetris_genome[num][0 + (input->blocktype - 1) * 3];
		}
		else if (map1[input->dots[i].y][input->dots[i].x - 1] != 0) {
			sum += tetris_genome[num][0 + (input->blocktype - 1) * 3];
			// check left.
		}
		if (input->dots[i].y == MAP_Y - 1) {
			sum += tetris_genome[num][1 + (input->blocktype - 1) * 3];
		}
		else if (map1[input->dots[i].y + 1][input->dots[i].x] != 0) {
			sum += tetris_genome[num][1 + (input->blocktype - 1) * 3];
			// check bottom.
		}
		if (input->dots[i].x == MAP_X - 1) {
			sum += tetris_genome[num][2 + (input->blocktype - 1) * 3];
		}
		else if (map1[input->dots[i].y][input->dots[i].x + 1] != 0) {
			sum += tetris_genome[num][2 + (input->blocktype - 1) * 3];
			// check right.
		}
	}

	//
	int check = 0;
	for (int i = MAP_Y - 1; i >= 0; i--) {
		for (int j = 0; j < MAP_X; j++) {
			if (map1[i][j] == 0) {
				check++;
				break;
			}
		} // delete line => check 0
		if (check == 0) {
			sum += tetris_genome[num][21];
			// line break bonus
		}
	}
	for (int i = 1; i <= 4; i++) {
		map1[input->dots[i].y][input->dots[i].x] = 8;
	}
	for (int i = 1; i <= 4; i++) {
		if (input->dots[i].y < MAP_Y - 1) {
			if (map1[input->dots[i].y + 1][input->dots[i].x] == 0) {
				sum -= tetris_genome[num][22];
				// make hole minus
			}
		}
	}
	for (int i = 1; i <= 4; i++) {
		map1[input->dots[i].y][input->dots[i].x] = 0;
	}
	//
	sum += (input->dots[0].y)*tetris_genome[num][23]; // y value bonus
	// target[input->dots[0].x][input->rotate % 4 - 1] = sum;
	if (sum >= target_max_value) {
		target_max_value = sum;
		target_x_value = input->dots[0].x;
		target_r_value = input->rotate;
	}
}

/* check line destroy */
void break_line_check(int num) {
	int jump = 0, check = 0;
	for (int i = MAP_Y - 1; i >= 0; i--) {
		check = 0;

		for (int j = 0; j < MAP_X; j++) {
			if (map1[i][j] == 0) {
				check++;
				break;
			}
		} // line break check

		if (jump != 0 && i + 1 < MAP_Y && check != 0) {
			for (int j = 0; j < MAP_X; j++) {
				map1[i + jump][j] = map1[i][j];
				map1[i][j] = 0;
			}
		}

		if (check == 0) {
			for (int j = 0; j < MAP_X; j++) {
				map1[i][j] = 0;
			}
			jump++;
			tetris_genome[num][24] += 100;
		}
	}
}

/* block move down */
void move_down(block &input, block &ghost, int num) {
	if (!input.is_ghost) {
		for (int i = 1; i <= 4; i++) {
			color(15);
			gotoxy(input.dots[i].x, input.dots[i].y);
			printf("%c%c", 0xa1, 0xe0);
		}
	}
	block temp = input;
	for (int i = 0; i < 5; i++) {
		input.dots[i].y++;
	}
	for (int i = 1; i <= 4; i++) {
		if (input.dots[i].y >= MAP_Y) {
			input = temp;
			block_over(&input, &ghost, num);
			break;
		}
		else if (map1[input.dots[i].y][input.dots[i].x] != 0) {
			input = temp;
			block_over(&input, &ghost, num);
			break;
		}
	}
	if (!input.is_ghost) {
		color_set(input);
		for (int i = 1; i <= 4; i++) {
			gotoxy(input.dots[i].x, input.dots[i].y);
			printf("%c%c", 0xa1, 0xe1);
		}
	}
}

/* block move hard drop */
void move_drop(block &input, block &ghost, int num) {
	for (int i = 1; i <= 4; i++) {
		color(15);
		gotoxy(input.dots[i].x, input.dots[i].y);
		printf("%c%c", 0xa1, 0xe0);
	}
	block temp = input;
	for (int k = 0; k < MAP_Y; k++) {
		temp = input;
		for (int i = 0; i < 5; i++) {
			input.dots[i].y++;
		}
		for (int i = 1; i <= 4; i++) {
			if (input.dots[i].y >= MAP_Y) {
				input = temp;
				k += MAP_Y;
				break;
			}
			else if (map1[input.dots[i].y][input.dots[i].x] != 0) {
				input = temp;
				k += MAP_Y;
				break;
			}
		}
	}/*
	color_set(input);
	for (int i = 1; i <= 4; i++) {
		gotoxy(input.dots[i].x, input.dots[i].y);
		printf("%c%c", 0xa1, 0xe1);
	}*/
	move_down(input, ghost, num);
}

/* block move left */
void move_left(block &input) {
	if (!input.is_ghost) {
		for (int i = 1; i <= 4; i++) {
			color(15);
			gotoxy(input.dots[i].x, input.dots[i].y);
			printf("%c%c", 0xa1, 0xe0);
		}
	}
	block temp = input;
	for (int i = 0; i < 5; i++) {
		input.dots[i].x--;
	}
	for (int i = 1; i <= 4; i++) {
		if (input.dots[i].x < 0) {
			input = temp;
			break;
		}
		else if (map1[input.dots[i].y][input.dots[i].x] != 0) {
			input = temp;
			break;
		}
	}
	if (!input.is_ghost) {
		color_set(input);
		for (int i = 1; i <= 4; i++) {
			gotoxy(input.dots[i].x, input.dots[i].y);
			printf("%c%c", 0xa1, 0xe1);
		}
	}
}

/* block move right */
void move_right(block &input) {
	if (!input.is_ghost) {
		for (int i = 1; i <= 4; i++) {
			color(15);
			gotoxy(input.dots[i].x, input.dots[i].y);
			printf("%c%c", 0xa1, 0xe0);
		}
	}
	block temp = input;
	for (int i = 0; i < 5; i++) {
		input.dots[i].x++;
	}
	for (int i = 1; i <= 4; i++) {
		if (input.dots[i].x >= MAP_X) {
			input = temp;
			break;
		}
		else if (map1[input.dots[i].y][input.dots[i].x] != 0) {
			input = temp;
			break;
		}
	}
	if (!input.is_ghost) {
		color_set(input);
		for (int i = 1; i <= 4; i++) {
			gotoxy(input.dots[i].x, input.dots[i].y);
			printf("%c%c", 0xa1, 0xe1);
		}
	}
}

/* make colorful map */
void color_set(block input) {
	switch (input.blocktype) { // I J L O S T Z 
	case 1:
		color(12);
		break;  // I block
	case 2:
		color(14);
		break; // J block
	case 3:
		color(13);
		break; // L block	
	case 4:
		color(11);
		break; // O block
	case 5:
		color(10);
		break; // S block	
	case 6:
		color(9);
		break; // T block
		/*
	case 7:
		color(8);
		break; // Z block*/
	default:
		color(15);
		break;
	}
}

/* make colorful block */
void color_set(int input) {
	switch (input) { // I J L O S T Z 
	case 1:
		color(12);
		break;  // I block
	case 2:
		color(14);
		break; // J block
	case 3:
		color(13);
		break; // L block	
	case 4:
		color(11);
		break; // O block
	case 5:
		color(10);
		break; // S block	
	case 6:
		color(9);
		break; // T block
		/*
	case 7:
		color(8);
		break; // Z block
		*/
	default:
		color(15);
		break;
	}
}

void color(unsigned short color)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, color);
}
void gotoxy(int x, int y) {
	COORD pos = { 2 * (x + 1),y + 1 };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

/*
enum BLOCKTYPE {
I_type = 0,
J_type,
L_type,
O_type,
S_type,
T_type,
Z_type
};
*/

/* block rotation */
void rotate_block(block &input) {
	block temp = input;
	input.rotate = (input.rotate + 1) % 4;
	if (!input.is_ghost) {
		for (int i = 1; i <= 4; i++) {
			color(15);
			gotoxy(temp.dots[i].x, temp.dots[i].y);
			printf("%c%c", 0xa1, 0xe0);
		}
		color_set(input);
	}
	switch (input.blocktype) { // I J L O S T Z 
	case 1:
		if (input.rotate % 4 == 0) {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 2;
			input.dots[4].x = input.dots[0].x + 1;
			input.dots[4].y = input.dots[0].y + 3;
		}
		else if (input.rotate % 4 == 1) {
			input.dots[1].x = input.dots[0].x;
			input.dots[1].y = input.dots[0].y + 1;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 2;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x + 3;
			input.dots[4].y = input.dots[0].y + 1;
		}
		else if (input.rotate % 4 == 2) {
			input.dots[1].x = input.dots[0].x + 2;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 2;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 2;
			input.dots[3].y = input.dots[0].y + 2;
			input.dots[4].x = input.dots[0].x + 2;
			input.dots[4].y = input.dots[0].y + 3;
		}
		else {
			input.dots[1].x = input.dots[0].x;
			input.dots[1].y = input.dots[0].y + 2;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 2;
			input.dots[3].x = input.dots[0].x + 2;
			input.dots[3].y = input.dots[0].y + 2;
			input.dots[4].x = input.dots[0].x + 3;
			input.dots[4].y = input.dots[0].y + 2;
		}
		break;  // I block
	case 2:
		if (input.rotate % 4 == 0) {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 2;
			input.dots[4].x = input.dots[0].x;
			input.dots[4].y = input.dots[0].y + 2;
		}
		else if (input.rotate % 4 == 1) {
			input.dots[1].x = input.dots[0].x;
			input.dots[1].y = input.dots[0].y + 1;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 2;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x;
			input.dots[4].y = input.dots[0].y;
		}
		else if (input.rotate % 4 == 2) {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 2;
			input.dots[4].x = input.dots[0].x + 2;
			input.dots[4].y = input.dots[0].y;
		}
		else {
			input.dots[1].x = input.dots[0].x;
			input.dots[1].y = input.dots[0].y + 1;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 2;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x + 2;
			input.dots[4].y = input.dots[0].y + 2;
		}
		break; // J block
	case 3:
		if (input.rotate % 4 == 0) {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 2;
			input.dots[4].x = input.dots[0].x + 2;
			input.dots[4].y = input.dots[0].y + 2;
		}
		else if (input.rotate % 4 == 1) {
			input.dots[1].x = input.dots[0].x;
			input.dots[1].y = input.dots[0].y + 1;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 2;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x;
			input.dots[4].y = input.dots[0].y + 2;
		}
		else if (input.rotate % 4 == 2) {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 2;
			input.dots[4].x = input.dots[0].x;
			input.dots[4].y = input.dots[0].y;
		}
		else {
			input.dots[1].x = input.dots[0].x;
			input.dots[1].y = input.dots[0].y + 1;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 2;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x + 2;
			input.dots[4].y = input.dots[0].y;
		}
		break; // L block	
	case 4:
		input.dots[1].x = input.dots[0].x;
		input.dots[1].y = input.dots[0].y;
		input.dots[2].x = input.dots[0].x;
		input.dots[2].y = input.dots[0].y + 1;
		input.dots[3].x = input.dots[0].x + 1;
		input.dots[3].y = input.dots[0].y;
		input.dots[4].x = input.dots[0].x + 1;
		input.dots[4].y = input.dots[0].y + 1;
		break; // O block
	case 5:
		if (input.rotate % 4 == 0) {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y + 1;
			input.dots[2].x = input.dots[0].x + 2;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x;
			input.dots[3].y = input.dots[0].y + 2;
			input.dots[4].x = input.dots[0].x + 1;
			input.dots[4].y = input.dots[0].y + 2;
		}
		else if (input.rotate % 4 == 1) {
			input.dots[1].x = input.dots[0].x;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x + 1;
			input.dots[4].y = input.dots[0].y + 2;
		}
		else if (input.rotate % 4 == 2) {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 2;
			input.dots[2].y = input.dots[0].y;
			input.dots[3].x = input.dots[0].x;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x + 1;
			input.dots[4].y = input.dots[0].y + 1;
		}
		else {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 2;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x + 2;
			input.dots[4].y = input.dots[0].y + 2;
		}
		break; // S block	
	case 6:
		if (input.rotate % 4 == 0) {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x + 2;
			input.dots[4].y = input.dots[0].y + 1;
		}
		else if (input.rotate % 4 == 1) {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 2;
			input.dots[4].x = input.dots[0].x + 2;
			input.dots[4].y = input.dots[0].y + 1;
		}
		else if (input.rotate % 4 == 2) {
			input.dots[1].x = input.dots[0].x;
			input.dots[1].y = input.dots[0].y + 1;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 2;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x + 1;
			input.dots[4].y = input.dots[0].y + 2;
		}
		else {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 2;
			input.dots[4].x = input.dots[0].x;
			input.dots[4].y = input.dots[0].y + 1;
		}
		break; // T block
	case 7:
		if (input.rotate % 4 == 0) {
			input.dots[1].x = input.dots[0].x;
			input.dots[1].y = input.dots[0].y + 1;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 2;
			input.dots[4].x = input.dots[0].x + 2;
			input.dots[4].y = input.dots[0].y + 2;
		}
		else if (input.rotate % 4 == 1) {
			input.dots[1].x = input.dots[0].x + 1;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x;
			input.dots[4].y = input.dots[0].y + 2;
		}
		else if (input.rotate % 4 == 2) {
			input.dots[1].x = input.dots[0].x;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 1;
			input.dots[2].y = input.dots[0].y;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x + 2;
			input.dots[4].y = input.dots[0].y + 1;
		}
		else {
			input.dots[1].x = input.dots[0].x + 2;
			input.dots[1].y = input.dots[0].y;
			input.dots[2].x = input.dots[0].x + 2;
			input.dots[2].y = input.dots[0].y + 1;
			input.dots[3].x = input.dots[0].x + 1;
			input.dots[3].y = input.dots[0].y + 1;
			input.dots[4].x = input.dots[0].x + 1;
			input.dots[4].y = input.dots[0].y + 2;
		}
		break; // Z block
	}
	for (int i = 1; i <= 4; i++) {
		if (input.dots[i].x < 0 || input.dots[i].x >= MAP_X || input.dots[i].y < 0 || input.dots[i].y >= MAP_Y) {
			input = temp;
			break;
		}
		else if (map1[input.dots[i].y][input.dots[i].x] != 0) {
			input = temp;
			break;
		}
	}
	if (!input.is_ghost) {
		for (int i = 1; i <= 4; i++) {
			gotoxy(input.dots[i].x, input.dots[i].y);
			printf("%c%c", 0xa1, 0xe1);
		}
	}
}
