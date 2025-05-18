#include <cstdio>
#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <Windows.h> 
#include <conio.h>
#define N 9 
#define UNDO_MAX 100 


typedef struct {
	int r;
	int c;
	int before;
	int after;
} Move;

Move undo_stack[UNDO_MAX]; 
int undo_top = -1; 


#define BOARD_SIZE 9 
#define BLANK 0 

enum Key { KEY_UP = 72, KEY_DOWN = 80, KEY_LEFT = 75, KEY_RIGHT = 77 };


int solution[BOARD_SIZE][BOARD_SIZE]; 
int puzzle[BOARD_SIZE][BOARD_SIZE]; 

int base[N][N];
int cnt = 0; 
HANDLE hOutput[2]; // 두 개의 화면 버퍼
int activeBuffer = 0; // 현재 활성화된 버퍼

void initConsoleBuffers() {
	CONSOLE_CURSOR_INFO cursorInfo;
	cursorInfo.bVisible = FALSE; // 커서 숨기기
	cursorInfo.dwSize = 1;

	for (int i = 0; i < 2; i++) {
		hOutput[i] = CreateConsoleScreenBuffer(
			GENERIC_READ | GENERIC_WRITE,
			0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleCursorInfo(hOutput[i], &cursorInfo); // 커서 숨기기
	}

	SetConsoleActiveScreenBuffer(hOutput[activeBuffer]);
}

void flipBuffers() {
	activeBuffer = 1 - activeBuffer;
	SetConsoleActiveScreenBuffer(hOutput[activeBuffer]);
}

void printToBuffer() {
	HANDLE hBuffer = hOutput[1 - activeBuffer];
	COORD pos = { 0, 0 };
	SetConsoleCursorPosition(hBuffer, pos);

	wchar_t output[4096] = L"";  // 유니코드 버퍼
	int idx = 0;

	idx += swprintf(output + idx, 4096 - idx, L"+-------+-------+-------+\n");

	for (int k = 0; k < N; k++) {
		idx += swprintf(output + idx, 4096 - idx, L"|");
		for (int l = 0; l < N; l++) {
			if (puzzle[k][l] == 0)
				idx += swprintf(output + idx, 4096 - idx, L" _");
			else if (puzzle[k][l] == -1)
				idx += swprintf(output + idx, 4096 - idx, L" M");
			else
				idx += swprintf(output + idx, 4096 - idx, L" %d", puzzle[k][l]);

			if (l % 3 == 2)
				idx += swprintf(output + idx, 4096 - idx, L" |");
		}
		idx += swprintf(output + idx, 4096 - idx, L"\n");
		if ((k + 1) % 3 == 0)
			idx += swprintf(output + idx, 4096 - idx, L"+-------+-------+-------+\n");
	}

	DWORD written;
	WriteConsoleW(hBuffer, output, wcslen(output), &written, NULL);
}


int calculateScore() {
	int correct = 0, incorrect = 0;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (puzzle[i][j] == solution[i][j]) {
				correct++;
			}
			else {
				incorrect++;
			}
		}
	}
	cnt += correct * 2;     // 정답당 +2점
	cnt -= incorrect * 3;  // 오답당 -3점
	return incorrect;
}


void printToBufferEND(int duration, int incorrectCount) {
	HANDLE hBuffer = hOutput[1 - activeBuffer];
	COORD pos = { 0, 0 };
	SetConsoleCursorPosition(hBuffer, pos);

	DWORD written;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hBuffer, &csbi);
	DWORD consoleSize = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacterW(hBuffer, L' ', consoleSize, pos, &written);
	FillConsoleOutputAttribute(hBuffer, csbi.wAttributes, consoleSize, pos, &written);
	SetConsoleCursorPosition(hBuffer, pos);

	wchar_t output[512] = L"";
	int idx = 0;

	idx += swprintf(output + idx, 512 - idx, L"\n 게임 완료! 축하합니다! \n");

	if (incorrectCount == 0)
		idx += swprintf(output + idx, 512 - idx, L" 모든 칸을 정확히 맞췄습니다! 🎉\n");
	else
		idx += swprintf(output + idx, 512 - idx, L" 틀린 칸 수: %d개\n", incorrectCount);

	idx += swprintf(output + idx, 512 - idx, L" 시간: %d초\n", duration);
	idx += swprintf(output + idx, 512 - idx, L" 최종 점수: %d점\n", cnt);

	WriteConsoleW(hBuffer, output, wcslen(output), &written, NULL);
}

void shuffle(int *arr, int size) {
	for (int i = size - 1; i > 0; i--) {
		int j = rand() % (i + 1); 
		int tmp = arr[i]; 
		arr[i] = arr[j]; 
		arr[j] = tmp;
	}
}

void shuffle_numbers() {
	int map[N + 1]; 
	for (int i = 1; i <= 9; i++) map[i] = i; 
	shuffle(map + 1, 9);

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			solution[i][j] = map[base[i][j]];
}

void create_base_pattern() {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			base[i][j] = (i * 3 + i / 3 + j) % 9 + 1;
		}
		
	}
}

void rand_void() {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			puzzle[i][j] = solution[i][j];
		}
	}
	for (int i = 0; i < 30; i++) {
		int voidI = rand() % 9, voidJ = rand() % 9;
		puzzle[voidI][voidJ] = 0;
	}
}

bool is_game_complete() {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (puzzle[i][j] != solution[i][j]) {
				return false;
			}
		}
	}
	return true;
}



int main() {
	clock_t start, finish;
	double duration;
	srand((unsigned int)time(NULL));

	create_base_pattern();
	shuffle_numbers();
	rand_void();

	char c = 0;
	char ci = 0, cj = 0;

	initConsoleBuffers(); // 콘솔 버퍼 초기화

	start = clock();

	while (1) {
		char buffer = puzzle[ci][cj];
		puzzle[ci][cj] = -1;

		printToBuffer();     // 백 버퍼에 그리기
		flipBuffers();       // 버퍼 전환

		puzzle[ci][cj] = buffer;

		c = _getch();
		if (c == -32) {
			c = _getch();
			switch (c) {
			case KEY_LEFT:  if (cj > 0) cj--; break;
			case KEY_RIGHT: if (cj < 8) cj++; break;
			case KEY_UP:    if (ci > 0) ci--; break;
			case KEY_DOWN:  if (ci < 8) ci++; break;
			}
		}
		else if ('0' <= c && c <= '9') {
			if (puzzle[ci][cj] == 0) {
				puzzle[ci][cj] = c - '0';
				undo_top++;
				undo_stack[undo_top].c = ci;
				undo_stack[undo_top].r = cj;
				undo_stack[undo_top].before = 0;
				undo_stack[undo_top].after = c - '0';
			}
		}
		else if (c == 'u' || c == 'U') {
			puzzle[undo_stack[undo_top].c][undo_stack[undo_top].r] = undo_stack[undo_top].before;
			undo_top--;
			cnt -= 10;
		}

		if (is_game_complete() || c == 'p') {
			finish = clock();
			duration = (int)(finish - start) / CLOCKS_PER_SEC;

			int incorrectCount = calculateScore();  // 정답/오답 점수 반영
			printToBufferEND(duration, incorrectCount); // 메시지 출력
			flipBuffers();
			_getch();
			break;
		}
	}

	return 0;
}


