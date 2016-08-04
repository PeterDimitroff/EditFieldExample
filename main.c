#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>

#define MAXLEN 256
#define FIELDWIDTH 24
#define ESC 27
#define NEW_LINE 10
#define UP_ARROW 65
#define DOWN_ARROW 66
#define LEFT_ARROW 68
#define RIGHT_ARROW 67
#define HOME_KEY 72
#define END_KEY 70
#define BACKSPACE 127


int getch();
int addInput(char string[]);

void specialCases(char string[]);
void moveLeft();
void moveRight();
void backspace(char string[]);

void maxLenReached(char string[]);
void renewPrint(char string[]);

int cursorPosition = 0;
int strLen = 0;
int isLastCh = 1;


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

int addInput(char string[])
{
	char c = 0;

	c = getch();
	switch(c)
	{
		case ESC:
			specialCases(string);
			break;
		case BACKSPACE:
			backspace(string);
			break;
		case NEW_LINE:
			string[strLen] = 0;
			return 1;
		default:
			string[cursorPosition] = c;
			cursorPosition++;
			if(cursorPosition == strLen)
				isLastCh = 1;
			if(isLastCh)
				strLen++;
			if(strLen == MAXLEN-2)
				return 2;
			break;
	}
	
	return 0;
}

void specialCases(char string[])
{
	char c;
	getch();
	c = getch();
	//printf("SPECIAL KEY: %c = '%d'\n", c, c);
	switch(c)
	{
		case LEFT_ARROW:
			moveLeft();
			break;	
		case RIGHT_ARROW:
			moveRight();
			break;
		case HOME_KEY:
			while(cursorPosition)
				moveLeft();
			break;
		case END_KEY:
			while(cursorPosition < strLen)
				moveRight();
			break;
		default:
			renewPrint("That's a weird key dude. Press one I know pls.\n");
			getch();
			renewPrint(string);
			break;
		
		
		
	}
	
	
}

void moveLeft()
{
	
	if(cursorPosition == 0)
		return;
	isLastCh = 0;
	cursorPosition--;
	printf("\0331D");
	//printf("\033[?1l");
	
}

void moveRight()
{
	
	if(cursorPosition >= strLen )
		return;
	
	cursorPosition++;
	if(cursorPosition == strLen)
		isLastCh = 1;
	printf("\0331C");
	
}

void backspace(char string[])
{
	moveLeft();
	string[cursorPosition] = 0;
}

void maxLenReached(char string[]) /******************TO BE REWORKED**************/
{
	string[MAXLEN-1] = 0;
	printf("\33[2K\r");
	printf("Maximum length reached.\n");
}

void renewPrint(char string[])
{
	printf("\33[2K\r");
	printf("%s", string);
}

int main()
{
	char string[MAXLEN];
	int res = 0;
	printf(string);
	memset(string, 0, MAXLEN);
	while(1)
	{
		if((res = addInput(string)) != 0)
			break;
		renewPrint(string);
	}
	switch(res)
	{
		case 1:
			printf("\nInput:\n%s\n", string);
			break;
		case 2:
			maxLenReached(string);
			break;
		
	}
	
	
	
	return 0;
}













/**
*
*
*	Backspace = '\b'
*	Return = '\r'
*
*
*
*
*
*
*
**/




