#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>

#define MAXLEN 256
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
#define SPACE 32

#define defEditField(x)	/*must be put before all functions*/\
typedef struct editField_t\
{\
	int stringSize;\
	int fieldWidth;\
	int cursorPosition;\
	int fieldPosition;\
	int strLen;\
	int isLastCh;\
	char string[x];\
	\
} editField;

#define initEditField(edtFld, fWidth)\
edtFld.fieldWidth = fWidth;\
edtFld.stringSize = sizeof(edtFld.string);\
edtFld.cursorPosition = 0;\
edtFld.fieldPosition = 0;\
edtFld.strLen = 0;\
edtFld.isLastCh = 1;

defEditField(MAXLEN);

int getch();
int addInput(editField *editF);

void specialCases(editField *editF);
void cursorMoveLeft(editField *editF);
void cursorMoveRight(editField *editF);
int backspace(editField *editF);
int makeSpace(editField *editF);
int catSpace(editField *editF);
int addChar(editField *editF, char c);

void maxLenReached(editField *editF);
void renewPrintStr(char string[]);
void renewPrintField(editField *editF);
void syncCursor(editField *editF);

int getch(void)
{
	int c;   
    static struct termios oldt;
    static struct termios newt;

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr( STDIN_FILENO, &oldt);
    /*now the settings will be copied*/
    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON | ECHO);          

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    /*This is your part:*/
    c = getchar();

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

    return c;
}

int addInput(editField *editF)
{
	char c = 0;

	c = getch();
	if(isalnum(c))
	{
		if(editF->strLen == editF->stringSize-2)
			return 2;
		if( addChar(editF, c) != 0)
			return -1;
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
		case HOME_KEY:
			editF->isLastCh = editF->cursorPosition = editF->fieldPosition = 0;
			break;
		case END_KEY:
			while(editF->cursorPosition < editF->strLen)
				cursorMoveRight(editF);
			break;
		default:
			renewPrintStr("That's a weird key dude. Press one I know pls.\n");
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
	//printf("\033[1D");	not implemented this way because of renewPrintStr()
}

void cursorMoveRight(editField *editF)
{
	if(editF->cursorPosition >= editF->strLen )
		return;
	
	editF->cursorPosition++;
	if(editF->cursorPosition == editF->strLen)
		editF->isLastCh = 1;
	if(editF->cursorPosition - editF->fieldPosition > editF->fieldWidth)
		editF->fieldPosition = editF->cursorPosition - editF->fieldWidth;
	//printf("\033[1C");	not implemented this way because of renewPrintStr()
}

int backspace(editField *editF)
{
	if(editF->cursorPosition == 0)
		return 0;
		
	cursorMoveLeft(editF);
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
	int i = editF->cursorPosition;
	
	for(i = editF->cursorPosition; i <= editF->strLen; i++)
	{
		editF->string[i] = editF->string[i+1];
	}
	editF->string[i] = 0;
	editF->strLen--;
	
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

void maxLenReached(editField *editF) /******************TO BE REWORKED**************/
{
	editF->string[editF->stringSize-1] = 0;
	printf("\33[2K\r");
	printf("Maximum length reached.\n");
}

void renewPrintStr(char string[])
{
	printf("\33[2K\r");
	printf("%s", string);
}

void renewPrintField(editField *editF)
{
	int i = editF->fieldPosition;
	printf("\33[2K\r");
	for( i = editF->fieldPosition; i <= editF->fieldWidth + editF->fieldPosition; i++)
		putchar(editF->string[i]);
}

void syncCursor(editField *editF)
{
	int i = editF->fieldPosition + editF->fieldWidth;
	
	if(editF->strLen > editF->fieldWidth)
		for(i = editF->fieldPosition + editF->fieldWidth; i >= editF->cursorPosition; i--)
			printf("\033[1D");
	else
		for(i = editF->strLen; i > editF->cursorPosition; i--)
			printf("\033[1D");
}

int main()
{
	editField editF;
	initEditField(editF, FIELDWIDTH);
	int res = 0;
	
	memset(editF.string, 0, editF.stringSize);
	while(1)
	{
		if((res = addInput(&editF)) != 0)
			break;
		
		printf("\nc = %d; f = %d\n", editF.cursorPosition, editF.fieldPosition);
		renewPrintField(&editF);
		syncCursor(&editF);
	}
	switch(res)
	{
		case 1:
			printf("\nInput:\n%s\n", editF.string);
			break;
		case 2:
			maxLenReached(&editF);
			break;
		case -1:
			perror("error");
			exit(-1);
		
	}
	
	
	
	return 0;
}













/**
*
*
		* Adding characters in the middle of the string works properly (actually adds them instead of changing the current one)
		* Added more special cases
		* Combined all variables into a struct and added two macros for managing it. This makes portability a real possibility.
		*
*
*
*
**/




