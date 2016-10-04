#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>

#define MAXLEN 10
#define FIELDWIDTH 5

#define ESC 27
#define NEW_LINE 10
#define UP_ARROW 65
#define DOWN_ARROW 66
#define LEFT_ARROW 68
#define RIGHT_ARROW 67
#define HOME_KEY 72
#define END_KEY 70
#define BACKSPACE 127
#define DELETE 51
#define SPACE 32

typedef struct editField_t
{
	int stringSize;
	int fieldWidth;
	int cursorPosition;
	int fieldPosition;
	int strLen;
	int isLastCh;
	char *string;

}editField;

int initEditField(editField *edtFld, int strSize, int fWidth);
int destroyEditField(editField *edtFld);
int getch();
int addInput(editField *editF);
void specialCases(editField *editF);
void cursorMoveLeft(editField *editF);
void cursorMoveRight(editField *editF);
int backspace(editField *editF);
int del(editField *editF);
int makeSpace(editField *editF);
int catSpace(editField *editF);
int addChar(editField *editF, char c);

void maxLenReached(editField *editF);
void renewPrintStr(char string[]);
void renewPrintField(editField *editF);
void syncCursor(editField *editF);

int initEditField(editField *edtFld, int strSize, int fWidth)
{
	edtFld->fieldWidth = fWidth;
	edtFld->stringSize = strSize;
	edtFld->cursorPosition = 0;
	edtFld->fieldPosition = 0;
	edtFld->strLen = 0;
	edtFld->isLastCh = 1;
	if(NULL == (edtFld->string = malloc(strSize)) )
	{
		return -1;
	}

	return 0;
}

int destroyEditField(editField *edtFld)
{
	free(edtFld->string);
	return 0;
}

int getch(void)
{
	int c;
	static struct termios oldt;
	static struct termios newt;

	tcgetattr( STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);
	c = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

	return c;
}

int addInput(editField *editF)
{
	char c = 0;

	c = getch();
	if(isalnum(c))
	{
		if(editF->strLen == editF->stringSize-1)
			return 2;
		if( addChar(editF, c) != 0)
			return -1;
		if(editF->strLen == editF->stringSize-1)
			return 2;
	}
	else
		switch(c)
		{
			case SPACE:
				break;
			case ESC:
				specialCases(editF);
				break;
			case BACKSPACE:
				backspace(editF);
				break;
			case NEW_LINE:
				editF->string[editF->strLen] = 0;
				return 1;
			default:
				printf("Dude you done fucked up. I no understando your lengua, amigo.\n");
				return -1;
		}

	return 0;
}

void specialCases(editField *editF)
{
	char c;
	getch();
	c = getch();

	switch(c)
	{
		case LEFT_ARROW:
			cursorMoveLeft(editF);
			break;
		case RIGHT_ARROW:
			cursorMoveRight(editF);
			break;
		case DOWN_ARROW:
			break;
		case UP_ARROW:
			break;
		case DELETE:
			del(editF);
			c = getch();
			return;
		case HOME_KEY:
			editF->isLastCh = editF->cursorPosition = editF->fieldPosition = 0;
			break;
		case END_KEY:
			while(editF->cursorPosition < editF->strLen)
				cursorMoveRight(editF);
			break;
		default:
			renewPrintStr("That's a weird key dude. Press one I know pls.\n");
			printf("%c\n", c);
			getch();
			renewPrintStr(editF->string);
			break;
	}
}

void cursorMoveLeft(editField *editF)
{
	if(editF->cursorPosition == 0)
		return;
	editF->isLastCh = 0;
	editF->cursorPosition--;
	if(editF->cursorPosition < editF->fieldPosition)
		editF->fieldPosition = editF->cursorPosition;
}

void cursorMoveRight(editField *editF)
{
	if(editF->cursorPosition >= editF->strLen )
		return;

	editF->cursorPosition++;
	if(editF->cursorPosition == editF->strLen)
		editF->isLastCh = 1;
	if(editF->cursorPosition - editF->fieldPosition >= editF->fieldWidth)
	{
		if(editF->isLastCh)
			editF->fieldPosition = editF->cursorPosition - editF->fieldWidth;
		else
			editF->fieldPosition++;
	}
}

int backspace(editField *editF)
{
	if(editF->cursorPosition == 0)
		return 0;

	cursorMoveLeft(editF);
	editF->string[editF->cursorPosition] = 0;
	return catSpace(editF);
}

int del(editField *editF)
{
	if(editF->isLastCh)
		return 0;

	editF->string[editF->cursorPosition] = 0;
	return catSpace(editF);
}

int makeSpace(editField *editF)
{
	int i = 0;
	if(editF->strLen >= editF->stringSize)
		return 1;
	for(i = editF->strLen; i >= editF->cursorPosition; i--)
	{
		editF->string[i+1] = editF->string[i];
	}

	return 0;
}

int catSpace(editField *editF)
{
	int i;

	for(i = editF->cursorPosition; i <= editF->strLen; i++)
	{
		editF->string[i] = editF->string[i+1];
	}
	editF->string[i] = 0;
	editF->strLen--;
	if(editF->strLen - editF->fieldPosition < editF->fieldWidth && editF->fieldPosition)
	{
		editF->fieldPosition--;
	}
	if(editF->cursorPosition == editF->strLen)
		editF->isLastCh = 1;

	return 0;
}

int addChar(editField *editF, char c)
{
	int res = 0;
	if(editF->isLastCh)
	{
		editF->string[editF->cursorPosition] = c;
	}
	else
	{
		res = makeSpace(editF);
		if(res != 0)
			return res;
		editF->string[editF->cursorPosition] = c;
	}

	editF->strLen++;
	cursorMoveRight(editF);

	return 0;
	}

void maxLenReached(editField *editF)
{
	int i;
	editF->string[editF->stringSize-1] = 0;
	printf("\33[2K\r");
	renewPrintField(editF);
	printf(" - Maximum length reached!");
	for(i = 1; i < sizeof " - Maximum length reached!"; i++)
		printf("\033[1D");
	syncCursor(editF);
}

void renewPrintStr(char string[])
{
	printf("\33[2K\r");
	printf("%s", string);
}

void renewPrintField(editField *editF)
{
	int i;

	printf("\33[2K\r");
	if(editF->cursorPosition - editF->fieldPosition == editF->fieldWidth && !editF->isLastCh)
		for( i = editF->fieldPosition + 1; i <= editF->fieldWidth + editF->fieldPosition; i++)
			putchar(editF->string[i]);
	else
		for( i = editF->fieldPosition; i < editF->fieldWidth + editF->fieldPosition; i++)
			putchar(editF->string[i]);
}

void syncCursor(editField *editF)
{
	int i;

	if(editF->strLen >= editF->fieldWidth)
	{
		for(i = editF->fieldPosition + editF->fieldWidth; i > editF->cursorPosition; i--)
			printf("\033[1D");
	}
	else
	{
		for(i = editF->strLen; i > editF->cursorPosition; i--)
			printf("\033[1D");
	}
}

int main()
{
    editField editF;
    int res;
    int i;
    for(i = 0; i < 10; i++)
    {
		res = initEditField(&editF, MAXLEN, FIELDWIDTH);
		if(res == 0)
			break;
	}
	if(res != 0)
	{
		printf("Error - not enough memory.\n");
		exit(res);
	}

    memset(editF.string, 0, editF.stringSize);
    while(1)
    {
		res = addInput(&editF);
        if(res != 0 && res != 2)
            break;
		if(res == 2)
			maxLenReached(&editF);
		else
		{
			renewPrintField(&editF);
			syncCursor(&editF);
		}
	}
	switch(res)
	{
		case 1:
			printf("\nInput:\n%s\n", editF.string);
			break;
		case 2:
			break;
		default:
			perror("error");
			destroyEditField(&editF);
			exit(-1);
	}

	destroyEditField(&editF);

    return 0;
}
