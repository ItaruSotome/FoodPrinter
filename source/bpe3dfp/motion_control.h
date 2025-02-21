#pragma once

#define DIMWORD_ERROR		0
#define DIMWORD_ORTHOGONAL	1
#define DIMWORD_ARC_RADIUS	2
#define DIMWORD_ARC_CENTER	3

int motion_control(PMODE, PDIMWORDSET, PPOSITION);
