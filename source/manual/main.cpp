// *****************************************************************************
// �R�}���h���C�������3D�t�[�h�v�����^�[����p�v���O����
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

	// ����{�[�h���I�[�v��
	if (open_Y2C_PC104(IO_UNIT_ID, (char*)"PMC-S4/16/32A-U")) {
		return 0;	// �G���[
	}

	// �\���̂�������
	initialize_mode(&currMode);
	initialize_position(&currPos);

	// �J�n���b�Z�[�W
	puts("Input G-Code from the kyeboard. Press 'Q'or'q' to quit the program");

	// �f�o�b�O
	while (true) {
		fgets(Buffer, MAX_STR_LENGTH, stdin);

		if (Buffer[0] == 'Q' || Buffer[0] == 'q')
			break;

		parse_block(&dimWordSet, &currPos, &currMode, Buffer);
		motion_control(&currMode, &dimWordSet, &currPos);
	}

	// ����{�[�h���N���[�Y
	close_Y2C_PC104(IO_UNIT_ID, (char*)"PMC-S4/16/32A-U");

	return 0;
}
