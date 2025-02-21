// 2024/12/10
// 
// ===========================================================================
// ���߂��ꂽG�R�[�h�A�f�B�����V�������[�h���ړ��ʁA���x���v�Z
// �ړ��ʁA�ړ����x���X�e�b�v���ɕϊ�
// ���݈ʒu�̍X�V
// ===========================================================================
#include "global.h"

static int		validate_dimention_word(int, PDIMWORDSET);
static int		move_to(PMODE, PDIMWORDSET, PPOSITION);

static int		get_displace_linear_absolute(PPOSITION, PDIMWORDSET, PPOSITION);
static int		get_speed_G00(PPOSITION);
static int		get_speed_interpolation(PPOSITION, PPOSITION, PPOSITION);

static int		update_current_position(PPOSITION, PPOSITION);

// -----------------------------------------------------------------------------
// �f�B�����V�������[�h���Z�b�g����Ă���ꍇ�A����X�e�b�v���̌v�Z
// 
// -----------------------------------------------------------------------------
int motion_control(
	PMODE		pCurrMode,
	PDIMWORDSET	pDimWordSet,
	PPOSITION	pCurrPos)
{
	// F���Z�b�g����Ă���ꍇ�AF�̌��ݒl���X�V
	if (pDimWordSet->F.isSet)
		pCurrPos->rf = pDimWordSet->F.value;

	// G00 ~ G01
	switch (pCurrMode->groupMotion)
	{
	// G00 �ʒu����
	case 0:
		if (validate_dimention_word(0, pDimWordSet) != DIMWORD_ORTHOGONAL)
			return 0;
		move_to(pCurrMode, pDimWordSet, pCurrPos);
		break;
	// G01 �������
	case 1:
		if (validate_dimention_word(0, pDimWordSet) != DIMWORD_ORTHOGONAL)
			return 0;
		move_to(pCurrMode, pDimWordSet, pCurrPos);
		break;
	// G02, G03 �~�ʕ��
	case 2:
	case 3:	// Under construction
		break;
	}

	return 0;
}

// -----------------------------------------------------------------------------
// G���[�h�ɑΉ������f�B�����V�������[�h���Z�b�g����Ă��邩�m�F
// -----------------------------------------------------------------------------
static int validate_dimention_word(
	int			Gword,
	PDIMWORDSET	pDimWordSet)
{
	switch (Gword)
	{
	case 0:
	case 1:
		if (pDimWordSet->X.isSet || pDimWordSet->Y.isSet || pDimWordSet->Z.isSet || pDimWordSet->E.isSet) {
			if (pDimWordSet->R.isSet || pDimWordSet->I.isSet || pDimWordSet->J.isSet || pDimWordSet->K.isSet)
				return DIMWORD_ERROR;
			else
				return DIMWORD_ORTHOGONAL;
		}
		return DIMWORD_ERROR;
	case 2:
		// under construction
		return 0;
	case 3:
		// under construction
		return 0;
	}

	return 0;
}

// -----------------------------------------------------------------------------
// G00�����s, �ʒu����
// G01
// -----------------------------------------------------------------------------
static int move_to(
	PMODE		pCurrMode,
	PDIMWORDSET pDimWordSet,
	PPOSITION	pCurrPos)
{
	// �ړ���, �ړ����x, step/s
	POSITION	Displace, Speed;
	WORD		axis;	// �����������w��

	// �ʒu���E�ړ����x�\���̂�������
	initialize_position(&Displace);
	initialize_position(&Speed);

	// �ړ��ʂ��v�Z
	switch (pCurrMode->groupPositioning)
	{
	case 90:	// �A�u�\�����[�g
		axis = get_displace_linear_absolute(pCurrPos, pDimWordSet, &Displace);
		break;
	case 91:	// �C���N�������^��
		// under construction
		break;
	}

	// �ړ����x���擾
	switch (pCurrMode->groupMotion)
	{
	case 0:
		get_speed_G00(&Speed);
		break;
	case 1:
		get_speed_interpolation(&Speed, &Displace, pCurrPos);
		break;
	}

	// PC104�փp���X�����w��
	drive_motor_Y2C_PC104(axis, &Displace, &Speed);

	// ���݈ʒu���A�b�v�f�[�g
	update_current_position(pCurrPos, &Displace);

	return 0;
}

// -----------------------------------------------------------------------------
// �����ړ�(G00, G01)�ɂ����Ĉړ��ʂ�step���Ōv�Z
// �A�u�\�����[�g�w��(G90)
// -----------------------------------------------------------------------------
static int get_displace_linear_absolute(
	PPOSITION	pCurrPos,
	PDIMWORDSET	pDimWordSet,
	PPOSITION	pDisplace)
{
	WORD	axis = 0;	// ���삳���鎲

	// X��
	if (pDimWordSet->X.isSet) {
		axis += PMC_AXIS_X0;
		pDisplace->rx = pDimWordSet->X.value - pCurrPos->rx;
		pDisplace->px = pDimWordSet->X.value / XUNIT - pCurrPos->px;
	}
	// Y��
	if (pDimWordSet->Y.isSet) {
		axis += PMC_AXIS_Y0;
		pDisplace->ry = pDimWordSet->Y.value - pCurrPos->ry;
		pDisplace->py = pDimWordSet->Y.value / YUNIT - pCurrPos->py;
	}
	// Z��
	if (pDimWordSet->Z.isSet) {
		axis += PMC_AXIS_Z0;
		pDisplace->rz = pDimWordSet->Z.value - pCurrPos->rz;
		pDisplace->pz = pDimWordSet->Z.value / ZUNIT - pCurrPos->pz;
	}
	// U�� �G�N�X�g���[�_
	if (pDimWordSet->E.isSet) {
		axis += PMC_AXIS_U0;
		pDisplace->re = pDimWordSet->E.value - pCurrPos->re;
		pDisplace->pe = pDimWordSet->E.value / EUNIT - pCurrPos->pe;
	}

	return axis;
}

// -----------------------------------------------------------------------------
// G00�ł̈ړ����x��step/s�Ōv�Z
// -----------------------------------------------------------------------------
static int get_speed_G00(PPOSITION	pSpeed)
{
	pSpeed->rx = MAX_VX / 60.0;
	pSpeed->ry = MAX_VY / 60.0;
	pSpeed->rz = MAX_VZ / 60.0;
	pSpeed->re = MAX_VE / 60.0;

	pSpeed->px = pSpeed->rx / XUNIT;
	pSpeed->py = pSpeed->ry / YUNIT;
	pSpeed->pz = pSpeed->rz / ZUNIT;
	pSpeed->pe = pSpeed->re / EUNIT;

	return 0;
}

// -----------------------------------------------------------------------------
// G01 ��ԓ���ł̈ړ����x��step/s�Ōv�Z
// -----------------------------------------------------------------------------
static int get_speed_interpolation(
	PPOSITION	pSpeed,
	PPOSITION	pDisplace,
	PPOSITION	pCurrPos)
{
	double	distance;	// 3�����ړ�����

	distance = sqrt(pDisplace->rx * pDisplace->rx + pDisplace->ry * pDisplace->ry + pDisplace->rz * pDisplace->rz);

	if (distance > 0) {
		// X��
		pSpeed->rx = fabs(pDisplace->rx / distance * pCurrPos->rf / 60.0);
		pSpeed->px = pSpeed->rx / XUNIT;
		// Y��
		pSpeed->ry = fabs(pDisplace->ry / distance * pCurrPos->rf / 60.0);
		pSpeed->py = pSpeed->ry / YUNIT;
		// Z��
		pSpeed->rz = fabs(pDisplace->rz / distance * pCurrPos->rf / 60.0);
		pSpeed->pz = pSpeed->rz / ZUNIT;
		// �G�N�X�g���[�_
		pSpeed->re = fabs(pDisplace->re / distance * pCurrPos->rf / 60.0);
		pSpeed->pe = pSpeed->re / EUNIT;
	}
	else {	// �G�N�X�g���[�_�̂ݓ���w��̏ꍇ
		pSpeed->rx = 0.0;
		pSpeed->ry = 0.0;
		pSpeed->rz = 0.0;
		pSpeed->re = pCurrPos->rf / 60.0;

		pSpeed->px = 0;
		pSpeed->py = 0;
		pSpeed->pz = 0;
		pSpeed->pe = pSpeed->re / EUNIT;
	}

	// Z���̍ő�ړ����x���m�F, XY-Z��Ԃ�Z���x���ő�l�𒴂���ꍇ�͒���
	if (pSpeed->rz > MAX_VZ / 60) {
		pSpeed->rz = MAX_VZ / 60;
		pSpeed->pz = pSpeed->rz / ZUNIT;
	}

	// U���̍ő�ړ����x���m�F
	if (pSpeed->re > MAX_VE / 60) {
		pSpeed->re = MAX_VE / 60;
		pSpeed->pe = pSpeed->re / EUNIT;
	}

	return 0;
}


// -----------------------------------------------------------------------------
// ���݈ʒu���A�b�v�f�[�g
// -----------------------------------------------------------------------------
static int update_current_position(
	PPOSITION	pCurrPos,
	PPOSITION	pDisplace
	)
{
	pCurrPos->rx += pDisplace->rx;
	pCurrPos->ry += pDisplace->ry;
	pCurrPos->rz += pDisplace->rz;
	pCurrPos->re += pDisplace->re;

	pCurrPos->px += pDisplace->px;
	pCurrPos->py += pDisplace->py;
	pCurrPos->pz += pDisplace->pz;
	pCurrPos->pe += pDisplace->pe;

	return 0;
}