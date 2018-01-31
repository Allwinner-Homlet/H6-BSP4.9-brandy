// update.cpp : Defines the entry point for the console application.
//
#include <malloc.h>
#include "types.h"
#include <string.h>
#include "script.h"
#include "crc.h"
#include "sunxi_mbr.h"
#include <ctype.h>
#include <unistd.h>

struct env_image_redundant {
        uint32_t        crc;    /* CRC32 over data bytes    */
//        unsigned char   flags;  /* active or obsolete */
        char            data[];
};

int add_part_to_env(char *file, char *value)
{
	unsigned int env_size = 0;
	FILE *env_file = NULL;
	char *buff = NULL;
	struct env_image_redundant *env;
	int i;

	env_file = fopen(file,"rb+");
	if(env_file == NULL)
	{
		printf("open %s error. \n",file);
		goto error_;
	}
	fseek(env_file, 0, SEEK_END);
	env_size = ftell(env_file);
	fseek(env_file, 0, SEEK_SET);
	buff = malloc(env_size * sizeof(char));
	if(buff == NULL)
	{
		printf("malloc error.\n");
		goto error_;
	}
	if(fread(buff, env_size, 1, env_file) == env_size)
	{
		printf("fread error. \n");
		goto error_;
	}
	for(i=env_size-1; i>=0; i--)
	{
		if(*(buff+i) != '\0')
			break;
	}
	memcpy((buff+i+2), value, strlen(value));
	env = (struct env_image_redundant*) buff;
	env->crc =  calc_crc32(env->data, env_size - sizeof(int));
	printf("the crc :%x\n",env->crc);
	
	fseek(env_file, 0, SEEK_SET);
	fwrite(env, env_size, 1, env_file);

	fclose(env_file);	
	free(buff);	
	return 0;

error_:
	if(env_file != NULL)
		fclose(env_file);
	if(buff != NULL)
		free(buff);
	return -1;
}


int main (int argc, char*argv[])
{
	sunxi_mbr_t    *mbr;
	char * buff = NULL;
	int mbr_offset = 0;
	int i = 0;
	int b;
	FILE *mbr_file = NULL;
	char mtdparts[128] = "mtdparts=spi0.0:";
	char tmp[20];

	printf("arg1:%s,arg2:%s\n",argv[1],argv[2]);
	mbr_file = fopen(argv[1],"r");
	if(mbr_file ==NULL)
	{
		printf("open %s fail\n",argv[1]);
		goto error_main;
	}
	buff = malloc(SUNXI_MBR_SIZE * sizeof(char));
	if(buff == NULL)
	{
		printf("malloc error.\n");
		goto error_main;
	}
	if(fread(buff,SUNXI_MBR_SIZE,1,mbr_file) == SUNXI_MBR_SIZE)
	{
		printf("read file %s error\n",argv[2]);
		goto error_main;
	}
	for (i = 0; i < SUNXI_MBR_COPY_NUM; i++)
	{
		mbr = (sunxi_mbr_t*)buff;
		if (!strncmp((const char*)mbr->magic, SUNXI_MBR_MAGIC, 8))
		{
			unsigned crc = 0;

			crc = calc_crc32((void *)&mbr->version, SUNXI_MBR_SIZE-4);
			if (crc == mbr->crc32)
			{
				printf("used mbr [%d], count = %d\n", i, mbr->PartCount);
				for(b=0; b<mbr->PartCount; b++)
				{
					printf("name:%s\n",mbr->array[b].name);
					printf("size:%d\n",mbr->array[b].lenlo);
					printf("ro:%d\n",mbr->array[b].ro);
					printf("-----------------\n");
					if(b == 0)
					{
						if(mbr->array[b].ro == 1)
							sprintf(tmp, "%dK(%s)ro",(mbr->array[b].lenlo *512/1024),mbr->array[b].name);
						else
							sprintf(tmp, "%dK(%s)",(mbr->array[b].lenlo *512/1024),mbr->array[b].name);
	
					}
					else
					{
						if(mbr->array[b].ro == 1)
							sprintf(tmp, ",%dK(%s)ro",(mbr->array[b].lenlo *512/1024),mbr->array[b].name);
						else
							sprintf(tmp, ",%dK(%s)",(mbr->array[b].lenlo *512/1024),mbr->array[b].name);
						
					}
					strcat(mtdparts, tmp);
				}
				printf("mtd:%s\n",mtdparts);
				add_part_to_env(argv[2],mtdparts);
				fclose(mbr_file);
				free(buff);
				return 0;
			}
		}

		printf("crc mbr copy[%d] failed\n", i);
		mbr_offset += (SUNXI_MBR_SIZE >> 9);
	}
	printf("mbr crc error\n");

error_main:
	if(mbr_file != NULL)
		fclose(mbr_file);
	if(buff != NULL)
		free(buff);
	return -1;
}
