// *****************************************************************************
// コマンドラインからの3Dフードプリンター制御用プログラム
// *****************************************************************************
#include "global.h"

static int open_gcode_file(int, char* [], FILE**);
static int read_block(FILE*);

int main(int argc, char *argv[])
{
	char	Buffer[MAX_STR_LENGTH];
	DIMWORDSET	dimWordSet;
	POSITION	currPos;
	MODE		currMode;

	// 制御ボードをオープン
	if (open_Y2C_PC104(IO_UNIT_ID, (char*)"PMC-S4/16/32A-U")) {
		return 0;	// エラー
	}

	// 構造体を初期化
	initialize_mode(&currMode);
	initialize_position(&currPos);

	// 開始メッセージ
	puts("Input G-Code from the kyeboard. Press 'Q'or'q' to quit the program");

	// デバッグ
	while (true) {
		fgets(Buffer, MAX_STR_LENGTH, stdin);

		if (Buffer[0] == 'Q' || Buffer[0] == 'q')
			break;

		parse_block(&dimWordSet, &currPos, &currMode, Buffer);
		motion_control(&currMode, &dimWordSet, &currPos);
	}

	// 制御ボードをクローズ
	close_Y2C_PC104(IO_UNIT_ID, (char*)"PMC-S4/16/32A-U");

	return 0;
}
