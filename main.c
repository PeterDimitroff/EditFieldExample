#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>

#define MAXLEN 256
#define FIELDWIDTH 24


int getch();
void addInput(char string[]);
void renewPrint(char string[]);

int currentChar = 0;
int strLen = 0;


int getch(void)
{
	int c;   
    static struct termios oldt, newt;

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

    /*This is your part:
    I choose 'e' to end input. Notice that EOF is also turned off
    in the non-canonical mode*/
    c = getchar();              

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);


    return c;
}

int addInput(char string[])
{
	char c = 0;
	int i = 0;

	c = getch();
	string[i] = c;
	currentChar++;
	if(strLen == MAXLEN-1)
	{
		string[MAXLEN-1] = 0;
		printf("\33[2K\r");
		printf("Maximum length reached.\n");
		return 1;;
	}
	return 0;
}

void renewPrint(char string[])
{
	printf("\33[2K\r");
	printf("%s", string);
}

int main()
{
	char string[MAXLEN];
	printf(string);
	memset(string, 0, MAXLEN);
	while(1)
	{
		if(addInput(string) == 1)
			break;
		renewPrint(string);
	}
	
	printf("\n");
	printf(string);
	
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




