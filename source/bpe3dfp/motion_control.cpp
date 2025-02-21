// 2024/12/10
// 
// ===========================================================================
// 解釈されたGコード、ディメンションワードより移動量、速度を計算
// 移動量、移動速度をステップ数に変換
// 現在位置の更新
// ===========================================================================
#include "global.h"

static int		validate_dimention_word(int, PDIMWORDSET);
static int		move_to(PMODE, PDIMWORDSET, PPOSITION);

static int		get_displace_linear_absolute(PPOSITION, PDIMWORDSET, PPOSITION);
static int		get_speed_G00(PPOSITION);
static int		get_speed_interpolation(PPOSITION, PPOSITION, PPOSITION);

static int		update_current_position(PPOSITION, PPOSITION);

// -----------------------------------------------------------------------------
// ディメンションワードがセットされている場合、動作ステップ数の計算
// 
// -----------------------------------------------------------------------------
int motion_control(
	PMODE		pCurrMode,
	PDIMWORDSET	pDimWordSet,
	PPOSITION	pCurrPos)
{
	// Fがセットされている場合、Fの現在値を更新
	if (pDimWordSet->F.isSet)
		pCurrPos->rf = pDimWordSet->F.value;

	// G00 ~ G01
	switch (pCurrMode->groupMotion)
	{
	// G00 位置決め
	case 0:
		if (validate_dimention_word(0, pDimWordSet) != DIMWORD_ORTHOGONAL)
			return 0;
		move_to(pCurrMode, pDimWordSet, pCurrPos);
		break;
	// G01 直線補間
	case 1:
		if (validate_dimention_word(0, pDimWordSet) != DIMWORD_ORTHOGONAL)
			return 0;
		move_to(pCurrMode, pDimWordSet, pCurrPos);
		break;
	// G02, G03 円弧補間
	case 2:
	case 3:	// Under construction
		break;
	}

	return 0;
}

// -----------------------------------------------------------------------------
// Gワードに対応したディメンションワードがセットされているか確認
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
// G00を実行, 位置決め
// G01
// -----------------------------------------------------------------------------
static int move_to(
	PMODE		pCurrMode,
	PDIMWORDSET pDimWordSet,
	PPOSITION	pCurrPos)
{
	// 移動量, 移動速度, step/s
	POSITION	Displace, Speed;
	WORD		axis;	// 動かす軸を指定

	// 位置情報・移動速度構造体を初期化
	initialize_position(&Displace);
	initialize_position(&Speed);

	// 移動量を計算
	switch (pCurrMode->groupPositioning)
	{
	case 90:	// アブソリュート
		axis = get_displace_linear_absolute(pCurrPos, pDimWordSet, &Displace);
		break;
	case 91:	// インクリメンタル
		// under construction
		break;
	}

	// 移動速度を取得
	switch (pCurrMode->groupMotion)
	{
	case 0:
		get_speed_G00(&Speed);
		break;
	case 1:
		get_speed_interpolation(&Speed, &Displace, pCurrPos);
		break;
	}

	// PC104へパルス発生指示
	drive_motor_Y2C_PC104(axis, &Displace, &Speed);

	// 現在位置をアップデート
	update_current_position(pCurrPos, &Displace);

	return 0;
}

// -----------------------------------------------------------------------------
// 直線移動(G00, G01)において移動量をstep数で計算
// アブソリュート指定(G90)
// -----------------------------------------------------------------------------
static int get_displace_linear_absolute(
	PPOSITION	pCurrPos,
	PDIMWORDSET	pDimWordSet,
	PPOSITION	pDisplace)
{
	WORD	axis = 0;	// 動作させる軸

	// X軸
	if (pDimWordSet->X.isSet) {
		axis += PMC_AXIS_X0;
		pDisplace->rx = pDimWordSet->X.value - pCurrPos->rx;
		pDisplace->px = pDimWordSet->X.value / XUNIT - pCurrPos->px;
	}
	// Y軸
	if (pDimWordSet->Y.isSet) {
		axis += PMC_AXIS_Y0;
		pDisplace->ry = pDimWordSet->Y.value - pCurrPos->ry;
		pDisplace->py = pDimWordSet->Y.value / YUNIT - pCurrPos->py;
	}
	// Z軸
	if (pDimWordSet->Z.isSet) {
		axis += PMC_AXIS_Z0;
		pDisplace->rz = pDimWordSet->Z.value - pCurrPos->rz;
		pDisplace->pz = pDimWordSet->Z.value / ZUNIT - pCurrPos->pz;
	}
	// U軸 エクストルーダ
	if (pDimWordSet->E.isSet) {
		axis += PMC_AXIS_U0;
		pDisplace->re = pDimWordSet->E.value - pCurrPos->re;
		pDisplace->pe = pDimWordSet->E.value / EUNIT - pCurrPos->pe;
	}

	return axis;
}

// -----------------------------------------------------------------------------
// G00での移動速度をstep/sで計算
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
// G01 補間動作での移動速度をstep/sで計算
// -----------------------------------------------------------------------------
static int get_speed_interpolation(
	PPOSITION	pSpeed,
	PPOSITION	pDisplace,
	PPOSITION	pCurrPos)
{
	double	distance;	// 3次元移動距離

	distance = sqrt(pDisplace->rx * pDisplace->rx + pDisplace->ry * pDisplace->ry + pDisplace->rz * pDisplace->rz);

	if (distance > 0) {
		// X軸
		pSpeed->rx = fabs(pDisplace->rx / distance * pCurrPos->rf / 60.0);
		pSpeed->px = pSpeed->rx / XUNIT;
		// Y軸
		pSpeed->ry = fabs(pDisplace->ry / distance * pCurrPos->rf / 60.0);
		pSpeed->py = pSpeed->ry / YUNIT;
		// Z軸
		pSpeed->rz = fabs(pDisplace->rz / distance * pCurrPos->rf / 60.0);
		pSpeed->pz = pSpeed->rz / ZUNIT;
		// エクストルーダ
		pSpeed->re = fabs(pDisplace->re / distance * pCurrPos->rf / 60.0);
		pSpeed->pe = pSpeed->re / EUNIT;
	}
	else {	// エクストルーダのみ動作指定の場合
		pSpeed->rx = 0.0;
		pSpeed->ry = 0.0;
		pSpeed->rz = 0.0;
		pSpeed->re = pCurrPos->rf / 60.0;

		pSpeed->px = 0;
		pSpeed->py = 0;
		pSpeed->pz = 0;
		pSpeed->pe = pSpeed->re / EUNIT;
	}

	// Z軸の最大移動速度を確認, XY-Z補間でZ速度が最大値を超える場合は注意
	if (pSpeed->rz > MAX_VZ / 60) {
		pSpeed->rz = MAX_VZ / 60;
		pSpeed->pz = pSpeed->rz / ZUNIT;
	}

	// U軸の最大移動速度を確認
	if (pSpeed->re > MAX_VE / 60) {
		pSpeed->re = MAX_VE / 60;
		pSpeed->pe = pSpeed->re / EUNIT;
	}

	return 0;
}


// -----------------------------------------------------------------------------
// 現在位置をアップデート
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