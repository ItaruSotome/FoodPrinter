#pragma once

#include "g_code.h"
#include "configuration.h"

#define MAX_STR_LENGTH	1024

// -----------------------------------------------------------
// Gコード, ディメンションワード要素
// 要素が存在しない場合があるため、isSetで有無を判別
// std::optional でもよいが、C++17以降に対応する必要がある
// -----------------------------------------------------------
typedef struct
{
	double	value;
	bool	isSet;

}DIMWORD, *PDIMWORD;

// -----------------------------------------------------------
// Gコード, ディメンションワードのセット
// G00 X_Y_Z_E_F_
// G01 X_Y_Z_E_F_
// -----------------------------------------------------------
typedef struct
{
	// 直線・円弧補間
	DIMWORD	X;	// 移動先(X)
	DIMWORD	Y;	// 移動先(Y)
	DIMWORD	Z;

	// 円弧補間
	DIMWORD	R;	// 半径指定
	DIMWORD	I;	// 中心指定(X)
	DIMWORD	J;	// 中心指定(Y)
	DIMWORD	K;	// 中心指定(Z)

	// 動作速度
	DIMWORD	F;

	// エクストルーダー
	DIMWORD	E;

}DIMWORDSET, * PDIMWORDSET;

// -----------------------------------------------------------
// 位置情報, 移動速度情報
// ワーク座標・機械座標現在位置
// 物理座標 -> ステップ座標
// -----------------------------------------------------------
typedef struct
{
	// 実数座標, mm
	double	rx;
	double	ry;
	double	rz;
	double	re;	// エクストルーダー

	double	rf;	// 動作速度 mm/min

	// ステップ座標 step もしくは軸ごとの移動速度 step/s
	int		px;
	int		py;
	int		pz;
	int		pe;

}POSITION, *PPOSITION;

// -----------------------------------------------------------
// モード
// G00 移動速度
// -----------------------------------------------------------
typedef struct
{
	int		groupMotion;		// Group 1		G00, G01, G02, G03
	int		groupCoordinate;	// Group 9		G54, G55, G56, G57
	int		groupPositioning;	// Group 12		G90, G91

}MODE, *PMODE;
