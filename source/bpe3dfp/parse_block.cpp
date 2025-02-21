// ===========================================================================
// G�R�[�h�u���b�N�����񂩂�R�[�h�A�f�B�����V�������[�h�𒊏o���A
// MODE�\���́ADIMWORDSET�\���̂Ɋi�[
// ===========================================================================
#include "global.h"

// gctod�֐��ɂ�����d��^, �ŏ��P�ʓ��͌^�̐؂�ւ��p�����[�^
#define INPUT_AUTO			0
#define INPUT_CALCULATOR	1
#define INPUT_MINIMUM		2

// ���[�J���֐��錾
static int		initialize_dim_word_set(PDIMWORDSET);
static int		interprete_G_address(PPOSITION, PMODE, char**);
static int		scan_dimension_word(PDIMWORDSET, char**);
static double	gctod(char**, int);
inline static int set_dim_word(PDIMWORD, double);
static int		set_work_coordinate(PPOSITION, PDIMWORDSET);

static int		display_dim_word_set(PDIMWORDSET);	// �f�o�b�O�p

// -----------------------------------------------------------------------------
// ���[�h�\���̂�������
// -----------------------------------------------------------------------------
int initialize_mode(PMODE pCurrMode)
{
	pCurrMode->groupMotion = 0;			// G00 �ʒu����
	pCurrMode->groupCoordinate = 54;	// G54 XY����
	pCurrMode->groupPositioning = 90;	// G90 �A�u�\�����[�g�w��

	return 0;
}

// -----------------------------------------------------------------------------
// �ʒu�E���x�\���̂�������
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
// G�R�[�h�̃u���b�N(�s)�Ɋ܂܂��G�R�}���h, X�`E�̍��W�p�����[�^�[������
// -----------------------------------------------------------------------------
int parse_block(
	PDIMWORDSET	pDimWordSet,
	PPOSITION	pCurrPos,
	PMODE		pCurrMode,
	char		*pStrBlock)
{
	char		*pStr;

	// �u���b�N������ւ̃|�C���^
	pStr = pStrBlock;
	// �t���O������
	initialize_dim_word_set(pDimWordSet);

	while (true) {
		switch (*pStr) {
		// �u���b�N�I�_
		case '\0':
		case '\r':
		case ';':
			return 0;

		case '%':		// �v���O�����X�^�[�g
			return 0;

		case '(':		// �R�����g
			return 0;
		
		// G�R�}���h�A�h���X������
		case 'G':
			interprete_G_address(pCurrPos, pCurrMode, &pStr);
			break;
		
		// M�R�}���h�A�h���X������(�X�L�b�v)
		case 'M':
			return 0;

		// �f�B�����V�������[�h������
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
// �f�B�����V�������[�h�\���̂�������
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
// G�A�h���X������ G__
// ���[�_�� -> ���[�h�X�V
// �����V���b�g -> �R�[�h���s
// -----------------------------------------------------------------------------
static int interprete_G_address(
	PPOSITION	pCurrPos,
	PMODE		pCurrMode,
	char		**ppStrBlock)
{
	DIMWORDSET	dimWordSet;	// G92�p�f�B�����V�������[�h

	int		g_address;
	char	*endpoint;

	// �A�h���X��ǂݎ��, �u���b�N������|�C���^�𑗂�
	g_address = strtol(++(*ppStrBlock), &endpoint, 10);
	*ppStrBlock = endpoint;

	switch (g_address) {
	// ��ԓ���I��(���[�_��)
	case 0:
	case 1:
		pCurrMode->groupMotion = g_address;
		break;

	// ���[�N���W�I��(���[�_��)
	case 54:
		pCurrMode->groupCoordinate = g_address;
		break;

	// �A�u�\�����[�g�E�C���N�������g�w��(���[�_��)
	case 90:
		pCurrMode->groupPositioning = g_address;
		break;
	case 91:	// Under construction
		break;

	// ��ƍ��W�n�ݒ�(�����V���b�g)
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
// �����񂩂�f�B�����V�������[�h�𒊏o���ăf�B�����V�����\���̂֊i�[
// �f�B�����V�������[�h�����o
// �f�B�����V�������[�h�̐��l��ϊ�
// -----------------------------------------------------------------------------
static int scan_dimension_word(
	PDIMWORDSET		pDimWordSet,
	char			**ppStrBlock)
{
	// �f�B�����V�������[�h�\���̂�������
	initialize_dim_word_set(pDimWordSet);

	// G���[�h�ȍ~�̕����񂩂�f�B�����V�������[�h�𒊏o
	while (true) {
		switch (**ppStrBlock) {
		// �u���b�N�I�[
		case '\0':
		case '\r':
		case ';':
			return 0;

		// �f�B�����V�������[�h�����o���A����ɑ������l�������X�L����
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
// �f�B�����W�������[�h�̐��l�����𐔒l�ϊ����ĕԂ�
// INPUT_AUTO : '.'������ꍇ�͒P�ʂ�[mm], �Ȃ��ꍇ[um]
// INPUT_CALCULATOR : [mm]
// INPUT_MINIMUM : [um]
// -----------------------------------------------------------------------------
static double gctod(
	char**	ppStrBlock,
	int		inputType)	// �d��^, �ŏ��P�ʓ��͌^
{
	char* pBuff;
	char Buff[64];			// Buff��, 0~9, -, .�݂̂��R�s�[����atof��double�ɕϊ�
	double	unit = 0.001;	// '.'������ꍇ[mm], �Ȃ��ꍇ[um]

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
// �f�B�����V�������[�h���X�L�������Ēl���Z�b�g
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
// G92 ���[�N���W�n��ݒ�
// -----------------------------------------------------------------------------
static int set_work_coordinate(
	PPOSITION	pCurrPos,
	PDIMWORDSET	pDimWordSet)
{
	// �f�B�����V�������[�h�Ŏw�肪���������̂ݍX�V
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
// �f�B�����V�������[�h��\��
// �f�o�b�O�p
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