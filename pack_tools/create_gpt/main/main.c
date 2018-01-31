#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include "include/types.h"
#include "include/gpt.h"

#define MAX_PATH		(250)
#define MALLOC_LENGTH	(4 * 1024 * 1024)
#define MAX_GPT_LENGTH	(8 * 1024)

extern void GetFullPath(char *dName, const char *sName);
extern int sunxi_mbr_convert_to_gpt(void *sunxi_mbr_buf, char *gpt_buf, int storage_type, u32 total_sectors);

void usage(void)
{
	printf("usage:\n");
	printf("1. create_gpt sunxi_mbr.fex sunxi_gpt.fex flash_sectors\n");
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int mbr_len = 0;
	int gpt_len = 0;
	unsigned int flash_sectors = 0;

	FILE *mbr_file = NULL;
	FILE *gpt_file = NULL;

	char mbr_name[MAX_PATH];
	char gpt_name[MAX_PATH];

	char *gpt_buf = NULL;
	char *mbr_buf = NULL;

	memset(mbr_name, 0, MAX_PATH);
	memset(gpt_name, 0, MAX_PATH);

	if (argc == 4)
	{
		if (argv[1] == NULL)
		{
			printf("create_gpt err: one of the input filename is empty\n");
			return __LINE__;
		}

		GetFullPath(mbr_name, argv[1]);
		GetFullPath(gpt_name, argv[2]);

		flash_sectors = atoi(argv[3]);
		printf("flash sectors=%d\n", flash_sectors);
	}
	else
	{
		printf("parameters is invalid\n");
		usage();
		return __LINE__;
	}

	printf("mbr name file path=%s\n", mbr_name);
	printf("gpt name file path=%s\n", gpt_name);

	mbr_file = fopen(mbr_name, "rb");
	if (!mbr_file)
	{
		printf("create_gpt err: uable to create file %s\n", mbr_name);
		ret = -1;
		goto _err_out;
	}

	gpt_file = fopen(gpt_name, "wb");
	if (!gpt_file)
	{
		ret = -1;
		printf("create_gpt err: uable to create file %s\n", gpt_name);
		goto _err_out;
	}

	mbr_buf = (char *)malloc(MALLOC_LENGTH);
	if (!mbr_buf)
	{
		printf("unable to malloc for mbr buf\n");
		ret = -1;
		goto _err_out;
	}
	memset(mbr_buf, 0, MALLOC_LENGTH);
	fseek(mbr_file, 0, SEEK_END);
	mbr_len = ftell(mbr_file);
	fseek(mbr_file, 0, SEEK_SET);

	fread(mbr_buf, 1, mbr_len, mbr_file);
	fclose(mbr_file);
	mbr_file = NULL;
	
	gpt_buf = (char *)malloc(MALLOC_LENGTH);
	if (!gpt_buf)
	{
		printf("unable to malloc for gpt buf\n");
		ret = -1;
		goto _err_out;
	}

	gpt_len = sunxi_mbr_convert_to_gpt(mbr_buf, gpt_buf, STORAGE_EMMC, flash_sectors);

	printf("gpt_len=%d\n", gpt_len);

	if (gpt_len > MAX_GPT_LENGTH)
	{
		printf("!!!!!! err, partition is too more, it's limited to 56\n");
		printf("!!!!!! err, partition is too more, it's limited to 56\n");
		printf("!!!!!! err, partition is too more, it's limited to 56\n");
		printf("!!!!!! err, partition is too more, it's limited to 56\n");
		ret = -1;
		goto _err_out;
	}
	else
	{
		fwrite(gpt_buf, 1, gpt_len, gpt_file);
	}

_err_out:
	if (gpt_file)
	{
		fclose(gpt_file);
	}

	if (mbr_file)
	{
		fclose(mbr_file);
	}

	if (gpt_buf)
	{
		free(gpt_buf);
		gpt_buf = NULL;
	}

	if (mbr_buf)
	{
		free(mbr_buf);
		gpt_buf = NULL;
	}

	return ret;
}
