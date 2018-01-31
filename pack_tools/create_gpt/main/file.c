// update.cpp : Defines the entry point for the console application.
//
#include <malloc.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define  MAX_PATH  260

static int IsFullName(const char *FilePath);

static int IsFullName(const char *FilePath)
{
    if (isalpha(FilePath[0]) && ':' == FilePath[1])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void GetFullPath(char *dName, const char *sName)
{
    char Buffer[MAX_PATH];

	if(IsFullName(sName))
	{
	    strcpy(dName, sName);
		return ;
	}

   /* Get the current working directory: */
   if(getcwd(Buffer, MAX_PATH ) == NULL)
   {
        perror( "getcwd error" );
        return ;
   }
   sprintf(dName, "%s/%s", Buffer, sName);
}
