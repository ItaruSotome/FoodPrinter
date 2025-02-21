// *****************************************************************************
// Gコードファイルによる3Dフードプリンター制御用プログラム
// *****************************************************************************
#include "global.h"

static int open_gcode_file(int, char* [], FILE**);
static int read_block(FILE*);

int main(int argc, char *argv[])
{
	FILE*	pFile;	//

	// 制御ボードをオープン
	if (open_Y2C_PC104(IO_UNIT_ID, (char*)"PMC-S4/16/32A-U")) {
		return 0;	// エラー
	}

	// Gコードファイルをオープン
	if (open_gcode_file(argc, argv, &pFile)) {
		perror("Failed to open the G-code file");	// エラー
		// 制御ボードをクローズ
		close_Y2C_PC104(IO_UNIT_ID, (char*)"PMC-S4/16/32A-U");
		return 0;
	}

	// 印刷開始
	puts("Press any key to start printing");
	_getch();
	// ブロックを読み込み実行
	read_block(pFile);

	// Gコードファイルをクローズ
	fclose(pFile);

	// 制御ボードをクローズ
	close_Y2C_PC104(IO_UNIT_ID, (char*)"PMC-S4/16/32A-U");

	return 0;
}

// -----------------------------------------------------------------------------
// コンソールからファイル名を取得してファイルをオープン
// -----------------------------------------------------------------------------
static int open_gcode_file(
	int		argc,
	char	*argv[],
	FILE**	pfp)
{
	char	fileName[MAX_STR_LENGTH];
	int ch;
	int result;
	
	if (argc == 2) {	// 引数1つ (ファイル名あり)
		strncpy_s(fileName, sizeof(fileName), argv[1], sizeof(fileName));
	}
	else {	// 引数にファイル名なし or 不明な引数
		puts("Input G-code file name");
		fgets(fileName, sizeof(fileName), stdin);
	}

	result = fopen_s(pfp, fileName, "r");

	return result;
}


// -----------------------------------------------------------------------------
// ファイルから行を読み込み印刷
// -----------------------------------------------------------------------------
static int read_block(FILE* file)
{
	DIMWORDSET	dimWordSet;
	POSITION	currPos;
	MODE		currMode;
	char		Buffer[MAX_STR_LENGTH];
	
	// 構造体を初期化
	initialize_mode(&currMode);
	initialize_position(&currPos);
	
	// 行の読み込み, EOFまで
	while (fgets(Buffer, sizeof(Buffer), file) != NULL) {
		printf("%s", Buffer);

		parse_block(&dimWordSet, &currPos, &currMode, Buffer);
		motion_control(&currMode, &dimWordSet, &currPos);
	}

	puts("");

	return 0;
}
