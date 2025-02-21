#pragma once

#include "g_code.h"
#include "configuration.h"

#define MAX_STR_LENGTH	1024

// -----------------------------------------------------------
// G�R�[�h, �f�B�����V�������[�h�v�f
// �v�f�����݂��Ȃ��ꍇ�����邽�߁AisSet�ŗL���𔻕�
// std::optional �ł��悢���AC++17�ȍ~�ɑΉ�����K�v������
// -----------------------------------------------------------
typedef struct
{
	double	value;
	bool	isSet;

}DIMWORD, *PDIMWORD;

// -----------------------------------------------------------
// G�R�[�h, �f�B�����V�������[�h�̃Z�b�g
// G00 X_Y_Z_E_F_
// G01 X_Y_Z_E_F_
// -----------------------------------------------------------
typedef struct
{
	// �����E�~�ʕ��
	DIMWORD	X;	// �ړ���(X)
	DIMWORD	Y;	// �ړ���(Y)
	DIMWORD	Z;

	// �~�ʕ��
	DIMWORD	R;	// ���a�w��
	DIMWORD	I;	// ���S�w��(X)
	DIMWORD	J;	// ���S�w��(Y)
	DIMWORD	K;	// ���S�w��(Z)

	// ���쑬�x
	DIMWORD	F;

	// �G�N�X�g���[�_�[
	DIMWORD	E;

}DIMWORDSET, * PDIMWORDSET;

// -----------------------------------------------------------
// �ʒu���, �ړ����x���
// ���[�N���W�E�@�B���W���݈ʒu
// �������W -> �X�e�b�v���W
// -----------------------------------------------------------
typedef struct
{
	// �������W, mm
	double	rx;
	double	ry;
	double	rz;
	double	re;	// �G�N�X�g���[�_�[

	double	rf;	// ���쑬�x mm/min

	// �X�e�b�v���W step �������͎����Ƃ̈ړ����x step/s
	int		px;
	int		py;
	int		pz;
	int		pe;

}POSITION, *PPOSITION;

// -----------------------------------------------------------
// ���[�h
// G00 �ړ����x
// -----------------------------------------------------------
typedef struct
{
	int		groupMotion;		// Group 1		G00, G01, G02, G03
	int		groupCoordinate;	// Group 9		G54, G55, G56, G57
	int		groupPositioning;	// Group 12		G90, G91

}MODE, *PMODE;
