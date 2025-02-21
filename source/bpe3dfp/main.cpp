// *****************************************************************************
// G�R�[�h�t�@�C���ɂ��3D�t�[�h�v�����^�[����p�v���O����
// *****************************************************************************
#include "global.h"

static int open_gcode_file(int, char* [], FILE**);
static int read_block(FILE*);

int main(int argc, char *argv[])
{
	FILE*	pFile;	//

	// ����{�[�h���I�[�v��
	if (open_Y2C_PC104(IO_UNIT_ID, (char*)"PMC-S4/16/32A-U")) {
		return 0;	// �G���[
	}

	// G�R�[�h�t�@�C�����I�[�v��
	if (open_gcode_file(argc, argv, &pFile)) {
		perror("Failed to open the G-code file");	// �G���[
		// ����{�[�h���N���[�Y
		close_Y2C_PC104(IO_UNIT_ID, (char*)"PMC-S4/16/32A-U");
		return 0;
	}

	// ����J�n
	puts("Press any key to start printing");
	_getch();
	// �u���b�N��ǂݍ��ݎ��s
	read_block(pFile);

	// G�R�[�h�t�@�C�����N���[�Y
	fclose(pFile);

	// ����{�[�h���N���[�Y
	close_Y2C_PC104(IO_UNIT_ID, (char*)"PMC-S4/16/32A-U");

	return 0;
}

// -----------------------------------------------------------------------------
// �R���\�[������t�@�C�������擾���ăt�@�C�����I�[�v��
// -----------------------------------------------------------------------------
static int open_gcode_file(
	int		argc,
	char	*argv[],
	FILE**	pfp)
{
	char	fileName[MAX_STR_LENGTH];
	int ch;
	int result;
	
	if (argc == 2) {	// ����1�� (�t�@�C��������)
		strncpy_s(fileName, sizeof(fileName), argv[1], sizeof(fileName));
	}
	else {	// �����Ƀt�@�C�����Ȃ� or �s���Ȉ���
		puts("Input G-code file name");
		fgets(fileName, sizeof(fileName), stdin);
	}

	result = fopen_s(pfp, fileName, "r");

	return result;
}


// -----------------------------------------------------------------------------
// �t�@�C������s��ǂݍ��݈��
// -----------------------------------------------------------------------------
static int read_block(FILE* file)
{
	DIMWORDSET	dimWordSet;
	POSITION	currPos;
	MODE		currMode;
	char		Buffer[MAX_STR_LENGTH];
	
	// �\���̂�������
	initialize_mode(&currMode);
	initialize_position(&currPos);
	
	// �s�̓ǂݍ���, EOF�܂�
	while (fgets(Buffer, sizeof(Buffer), file) != NULL) {
		printf("%s", Buffer);

		parse_block(&dimWordSet, &currPos, &currMode, Buffer);
		motion_control(&currMode, &dimWordSet, &currPos);
	}

	puts("");

	return 0;
}
