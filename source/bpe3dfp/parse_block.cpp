// ===========================================================================
// Gコードブロック文字列からコード、ディメンションワードを抽出し、
// MODE構造体、DIMWORDSET構造体に格納
// ===========================================================================
#include "global.h"

// gctod関数における電卓型, 最小単位入力型の切り替えパラメータ
#define INPUT_AUTO			0
#define INPUT_CALCULATOR	1
#define INPUT_MINIMUM		2

// ローカル関数宣言
static int		initialize_dim_word_set(PDIMWORDSET);
static int		interprete_G_address(PPOSITION, PMODE, char**);
static int		scan_dimension_word(PDIMWORDSET, char**);
static double	gctod(char**, int);
inline static int set_dim_word(PDIMWORD, double);
static int		set_work_coordinate(PPOSITION, PDIMWORDSET);

static int		display_dim_word_set(PDIMWORDSET);	// デバッグ用

// -----------------------------------------------------------------------------
// モード構造体を初期化
// -----------------------------------------------------------------------------
int initialize_mode(PMODE pCurrMode)
{
	pCurrMode->groupMotion = 0;			// G00 位置決め
	pCurrMode->groupCoordinate = 54;	// G54 XY平面
	pCurrMode->groupPositioning = 90;	// G90 アブソリュート指定

	return 0;
}

// -----------------------------------------------------------------------------
// 位置・速度構造体を初期化
// -----------------------------------------------------------------------------
int initialize_position(PPOSITION pCurrPos)
{
	pCurrPos->rx = 0.0;
	pCurrPos->ry = 0.0;
	pCurrPos->rz = 0.0;
	pCurrPos->re = 0.0;

	pCurrPos->rf = DEFAULT_F;
	
	pCurrPos->px = 0;
	pCurrPos->py = 0;
	pCurrPos->pz = 0;
	pCurrPos->pe = 0;

	return 0;
}

// -----------------------------------------------------------------------------
// 2024/11/24
// Gコードのブロック(行)に含まれるGコマンド, X～Eの座標パラメーターを解釈
// -----------------------------------------------------------------------------
int parse_block(
	PDIMWORDSET	pDimWordSet,
	PPOSITION	pCurrPos,
	PMODE		pCurrMode,
	char		*pStrBlock)
{
	char		*pStr;

	// ブロック文字列へのポインタ
	pStr = pStrBlock;
	// フラグ初期化
	initialize_dim_word_set(pDimWordSet);

	while (true) {
		switch (*pStr) {
		// ブロック終点
		case '\0':
		case '\r':
		case ';':
			return 0;

		case '%':		// プログラムスタート
			return 0;

		case '(':		// コメント
			return 0;
		
		// Gコマンドアドレスを解釈
		case 'G':
			interprete_G_address(pCurrPos, pCurrMode, &pStr);
			break;
		
		// Mコマンドアドレスを解釈(スキップ)
		case 'M':
			return 0;

		// ディメンションワードを解釈
		case 'X':
		case 'Y':
		case 'Z':
		case 'E':
		case 'R':
		case 'I':
		case 'J':
		case 'K':
		case 'F':
			scan_dimension_word(pDimWordSet, &pStr);
			break;
		default:
			pStr++;
			break;
		}

	}

	return 0;
}


// -----------------------------------------------------------------------------
// ディメンションワード構造体を初期化
// -----------------------------------------------------------------------------
static int initialize_dim_word_set(PDIMWORDSET	pDimWordSet)
{
	pDimWordSet->X.isSet = false;
	pDimWordSet->Y.isSet = false;
	pDimWordSet->Z.isSet = false;
	pDimWordSet->I.isSet = false;
	pDimWordSet->J.isSet = false;
	pDimWordSet->K.isSet = false;
	pDimWordSet->R.isSet = false;
	pDimWordSet->E.isSet = false;
	pDimWordSet->F.isSet = false;

	pDimWordSet->X.value = 0.0;
	pDimWordSet->Y.value = 0.0;
	pDimWordSet->Z.value = 0.0;
	pDimWordSet->I.value = 0.0;
	pDimWordSet->J.value = 0.0;
	pDimWordSet->K.value = 0.0;
	pDimWordSet->R.value = 0.0;
	pDimWordSet->E.value = 0.0;
	pDimWordSet->F.value = 0.0;

	return 0;
}

// -----------------------------------------------------------------------------
// Gアドレスを解釈 G__
// モーダル -> モード更新
// ワンショット -> コード実行
// -----------------------------------------------------------------------------
static int interprete_G_address(
	PPOSITION	pCurrPos,
	PMODE		pCurrMode,
	char		**ppStrBlock)
{
	DIMWORDSET	dimWordSet;	// G92用ディメンションワード

	int		g_address;
	char	*endpoint;

	// アドレスを読み取り, ブロック文字列ポインタを送り
	g_address = strtol(++(*ppStrBlock), &endpoint, 10);
	*ppStrBlock = endpoint;

	switch (g_address) {
	// 補間動作選択(モーダル)
	case 0:
	case 1:
		pCurrMode->groupMotion = g_address;
		break;

	// ワーク座標選択(モーダル)
	case 54:
		pCurrMode->groupCoordinate = g_address;
		break;

	// アブソリュート・インクリメント指定(モーダル)
	case 90:
		pCurrMode->groupPositioning = g_address;
		break;
	case 91:	// Under construction
		break;

	// 作業座標系設定(ワンショット)
	case 92:
		initialize_dim_word_set(&dimWordSet);
		scan_dimension_word(&dimWordSet, ppStrBlock);
		set_work_coordinate(pCurrPos, &dimWordSet);
		display_dim_word_set(&dimWordSet);
		break;
	}

	return 0;
}

// -----------------------------------------------------------------------------
// 文字列からディメンションワードを抽出してディメンション構造体へ格納
// ディメンションワードを検出
// ディメンションワードの数値を変換
// -----------------------------------------------------------------------------
static int scan_dimension_word(
	PDIMWORDSET		pDimWordSet,
	char			**ppStrBlock)
{
	// ディメンションワード構造体を初期化
	initialize_dim_word_set(pDimWordSet);

	// Gワード以降の文字列からディメンションワードを抽出
	while (true) {
		switch (**ppStrBlock) {
		// ブロック終端
		case '\0':
		case '\r':
		case ';':
			return 0;

		// ディメンションワードを検出し、それに続く数値部分をスキャン
		case 'X':
			(*ppStrBlock)++;
			set_dim_word(&(pDimWordSet->X), gctod(ppStrBlock, INPUT_CALCULATOR));
			break;
		case 'Y':
			(*ppStrBlock)++;
			set_dim_word(&(pDimWordSet->Y), gctod(ppStrBlock, INPUT_CALCULATOR));
			break;
		case 'Z':
			(*ppStrBlock)++;
			set_dim_word(&(pDimWordSet->Z), gctod(ppStrBlock, INPUT_CALCULATOR));
			break;
		case 'E':
			(*ppStrBlock)++;
			set_dim_word(&(pDimWordSet->E), gctod(ppStrBlock, INPUT_CALCULATOR));
			break;
		case 'R':
			(*ppStrBlock)++;
			set_dim_word(&(pDimWordSet->R), gctod(ppStrBlock, INPUT_AUTO));
			break;
		case 'I':
			(*ppStrBlock)++;
			set_dim_word(&(pDimWordSet->I), gctod(ppStrBlock, INPUT_AUTO));
			break;
		case 'J':
			(*ppStrBlock)++;
			set_dim_word(&(pDimWordSet->J), gctod(ppStrBlock, INPUT_AUTO));
			break;
		case 'K':
			(*ppStrBlock)++;
			set_dim_word(&(pDimWordSet->K), gctod(ppStrBlock, INPUT_AUTO));
			break;
		case 'F':
			(*ppStrBlock)++;
			set_dim_word(&(pDimWordSet->F), gctod(ppStrBlock, INPUT_CALCULATOR));
			break;
		default:
			(*ppStrBlock)++;
		}

	}
}

// -----------------------------------------------------------------------------
// ディメンジョンワードの数値部分を数値変換して返す
// INPUT_AUTO : '.'がある場合は単位は[mm], ない場合[um]
// INPUT_CALCULATOR : [mm]
// INPUT_MINIMUM : [um]
// -----------------------------------------------------------------------------
static double gctod(
	char**	ppStrBlock,
	int		inputType)	// 電卓型, 最小単位入力型
{
	char* pBuff;
	char Buff[64];			// Buffに, 0~9, -, .のみをコピーしてatofでdoubleに変換
	double	unit = 0.001;	// '.'がある場合[mm], ない場合[um]

	pBuff = Buff;

	while (true) {
		if (**ppStrBlock == '\0')
			break;
		if (**ppStrBlock == '\n')
			break;
		if (**ppStrBlock == ' ')
			break;

		if (**ppStrBlock == '.') {
			unit = 1.0;
			*pBuff = **ppStrBlock;
			pBuff++;
			(*ppStrBlock)++;
			continue;
		}
		if (**ppStrBlock == '-') {
			*pBuff = **ppStrBlock;
			pBuff++;
			(*ppStrBlock)++;
			continue;
		}
		if (isdigit(**ppStrBlock)) {
			*pBuff = **ppStrBlock;
			pBuff++;
			(*ppStrBlock)++;
			continue;
		}

		break;
	}

	*pBuff = '\0';

	switch (inputType) {
	case INPUT_AUTO:
		return atof(Buff) * unit;
	case INPUT_CALCULATOR:
		return atof(Buff);
	case INPUT_MINIMUM:
		return atof(Buff) * 0.001;
	}

	return 0;
}

// -----------------------------------------------------------------------------
// ディメンションワードをスキャンして値をセット
// -----------------------------------------------------------------------------
inline static int set_dim_word(
	PDIMWORD	pDimWord,
	double		value)
{
	pDimWord->value = value;
	pDimWord->isSet = true;

	return 0;
}

// -----------------------------------------------------------------------------
// G92 ワーク座標系を設定
// -----------------------------------------------------------------------------
static int set_work_coordinate(
	PPOSITION	pCurrPos,
	PDIMWORDSET	pDimWordSet)
{
	// ディメンションワードで指定があった軸のみ更新
	if (pDimWordSet->X.isSet) {
		pCurrPos->rx = pDimWordSet->X.value;
		pCurrPos->px = pCurrPos->rx / XUNIT;
	}
	if (pDimWordSet->Y.isSet) {
		pCurrPos->ry = pDimWordSet->Y.value;
		pCurrPos->py = pCurrPos->ry / YUNIT;
	}
	if (pDimWordSet->Z.isSet) {
		pCurrPos->rz = pDimWordSet->Z.value;
		pCurrPos->pz = pCurrPos->rz / ZUNIT;
	}
	if (pDimWordSet->E.isSet) {
		pCurrPos->re = pDimWordSet->E.value;
		pCurrPos->pe = pCurrPos->re / EUNIT;
	}

	return 0;
}

// -----------------------------------------------------------------------------
// ディメンションワードを表示
// デバッグ用
// -----------------------------------------------------------------------------
static int display_dim_word_set(
	PDIMWORDSET		pDimWordSet
)
{
	if (pDimWordSet->X.isSet) printf("X:%lf ", pDimWordSet->X.value);
	if (pDimWordSet->Y.isSet) printf("Y:%lf ", pDimWordSet->Y.value);
	if (pDimWordSet->Z.isSet) printf("Z:%lf ", pDimWordSet->Z.value);
	if (pDimWordSet->E.isSet) printf("E:%lf ", pDimWordSet->E.value);
	if (pDimWordSet->R.isSet) printf("R:%lf ", pDimWordSet->R.value);
	if (pDimWordSet->I.isSet) printf("I:%lf ", pDimWordSet->I.value);
	if (pDimWordSet->J.isSet) printf("J:%lf ", pDimWordSet->J.value);
	if (pDimWordSet->K.isSet) printf("K:%lf ", pDimWordSet->K.value);
	if (pDimWordSet->F.isSet) printf("F:%lf ", pDimWordSet->F.value);

	puts("");
	
	return 0;
}