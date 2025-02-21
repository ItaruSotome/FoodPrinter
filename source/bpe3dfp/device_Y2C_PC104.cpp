// =============================================================================
// Y2C USB-PC104シリーズ用
// =============================================================================

#include "global.h"

static double	get_motion_time(PPOSITION, PPOSITION);
static void		precise_sleep(double);


// -----------------------------------------------------------------------------
// IOボードY2C USB-PC104シリーズ(AIO-84/16/32A-U)との通信
// ボードをオープン
// -----------------------------------------------------------------------------
int open_Y2C_PC104(
	int		UnitID,
	char	*pDeviceName)
{
	int		result;
	WORD	wAxis;

	result = YduOpen(UnitID, pDeviceName, YDU_OPEN_NORMAL);

	if(result)
	{
		printf("Failed to open %s.\n", pDeviceName);
		return result;	// エラー
	}
	printf("Successfully opened %s.\n", pDeviceName);

	// センサー設定
	wAxis = PMC_AXIS_X0 + PMC_AXIS_Y0 + PMC_AXIS_Z0 + PMC_AXIS_U0;
	result = YduPmcsSetSensorConfig(0, wAxis, PMC_LOGIC, 0x1F);		// EL, ORG, ONで検出

	return result;
}

// -----------------------------------------------------------------------------
// Y2C USB-PC104シリーズを閉じる
// -----------------------------------------------------------------------------
void close_Y2C_PC104(
	int		UnitID,
	char	*pDeviceName)
{
	YduClose(UnitID);
	printf("Successfully closed %s.\n", pDeviceName);
}


// -----------------------------------------------------------------------------
// Y2C_PC104用APIをコールしてIOボードからモータードライバーにパルスを送信
// -----------------------------------------------------------------------------
int drive_motor_Y2C_PC104(
	DWORD		axis,
	PPOSITION	pDisplace,
	PPOSITION	pSpeed)
{
	static MOTIONPMCS	Motion[4];
	int					nResult;

	// X軸
	Motion[0].wAccMode = PMC_ACC_NORMAL;
	Motion[0].dwLowSpeed = 1;
	Motion[0].lStep = pDisplace->px * XDIR;
	Motion[0].dwSpeed = (pSpeed->px == 0 ? 1 : pSpeed->px);	// dwSpeedに0をセットすると予期せぬ動作
	Motion[0].wAccTime = 300;
	// Y軸
	Motion[1].wAccMode = PMC_ACC_NORMAL;
	Motion[1].dwLowSpeed = 1;
	Motion[1].lStep = pDisplace->py * YDIR;
	Motion[1].dwSpeed = (pSpeed->py == 0 ? 1 : pSpeed->py);
	Motion[1].wAccTime = 300;
	// Z軸
	Motion[2].wAccMode = PMC_ACC_NORMAL;
	Motion[2].dwLowSpeed = 1;
	Motion[2].lStep = pDisplace->pz * ZDIR;
	Motion[2].dwSpeed = (pSpeed->pz == 0 ? 1 : pSpeed->pz);
	Motion[2].wAccTime = 500;
	// エクストルーダ
	Motion[3].wAccMode = PMC_ACC_NORMAL;
	Motion[3].dwLowSpeed = 1;
	Motion[3].lStep = pDisplace->pe * EDIR;
	Motion[3].dwSpeed = (pSpeed->pe == 0 ? 1 : pSpeed->pe);
	Motion[3].wAccTime = 1;

	// モータードライバーに指示送信
	nResult = YduPmcsSetMotion(0, axis, PMC_PTP, Motion);
	nResult = YduPmcsStartMotion(0, axis, PMC_CONST, PMC_PTP);

	// 移動時間を計算してスリープ
	precise_sleep(1000 * get_motion_time(pDisplace, pSpeed));

	return nResult;
}

// ---------------------------------------------------------------------------
// 動作時間を計算
// X,Y,Zの三軸で最も長い移動時間を返す
// ---------------------------------------------------------------------------
static double get_motion_time(
	PPOSITION	pDisplace,
	PPOSITION	pSpeed)
{
	double	time = 0.0;
	double	ty, tz, te;

	if (pSpeed->px > 0) {
		time = (double)(abs(pDisplace->px)) / pSpeed->px;
	}
	if (pSpeed->py > 0) {
		ty = (double)(abs(pDisplace->py)) / pSpeed->py;
		if (ty > time)
			time = ty;
	}
	if (pSpeed->pz > 0) {
		tz = (double)(abs(pDisplace->pz)) / pSpeed->pz;
		if (tz > time)
			time = tz;
	}

	// エクストルーダ
	if (pSpeed->pe > 0) {
		te = (double)(abs(pDisplace->pe)) / pSpeed->pe;
		if (te > time)
			time = te;
	}

	return time;
}

// ---------------------------------------------------------------------------
// 高精度Sleep
// Win32 APIを使用（移植時に注意）
// ---------------------------------------------------------------------------
static void precise_sleep(double milliseconds) 
{
	LARGE_INTEGER frequency, start, current;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);

	double target = milliseconds / 1000.0; // ミリ秒を秒に変換

	while (true) {
		QueryPerformanceCounter(&current);
		double elapsed = (double)(current.QuadPart - start.QuadPart) / frequency.QuadPart;
		if (elapsed >= target) {
			break;
		}
	}

	return;
}
