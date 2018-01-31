#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "sunxi_mbr.h"
#include "script.h"

#define  MAX_PATH  320
int get_file_name(char *path, char *name)
{
	char buffer[MAX_PATH];
	int  i, length;
	char *pt;

	memset(buffer, 0, MAX_PATH);
	if(!IsFullName(path))
	{
	   if(getcwd(buffer, MAX_PATH ) == NULL)
	   {
			perror( "getcwd error" );
			return -1;
	   }
	   sprintf(buffer, "%s/%s", buffer, path);
	}
	else
	{
		strcpy(buffer, path);
	}

	length = strlen(buffer);
	pt = buffer + length - 1;

	while((*pt != '/') && (*pt != '\\'))
	{
		pt --;
	}
	i =0;
	pt ++;
	while(*pt)
	{
		if(*pt == '.')
		{
			name[i++] = '.';
			pt ++;
		}
		else if((*pt >= 'a') && (*pt <= 'z'))
		{
			name[i++] = *pt++;// - ('a' - 'A');
		}
		else
		{
			name[i++] = *pt ++;
		}
		if(i>=16)
		{
			break;
		}
	}

	return 0;
}
int IsFullName(const char *FilePath)
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

int get_partition_file(char *part_name, char *file_name)
{
	int part_handle;
	int  value[8], ret;
	char fullname[260];
	char filename[32];

	part_handle = script_parser_fetch_partition();
	if(part_handle <= 0)
	{
		return -1;
	}
	memset(value, 0, 8 * sizeof(int));
	if(!script_parser_fetch_mainkey_sub("name", part_handle, value))
	{
		if(!strcmp((char *)value, part_name))
		{
			memset(fullname, 0, 260);
			ret = script_parser_fetch_mainkey_sub("downloadfile", part_handle, (int *)fullname);
			if(!ret)
			{
				memset(filename, 0, 32);
				get_file_name(fullname, filename);
				memcpy(file_name, filename, sizeof(filename));
				printf("part_name=%s, downloadfile=%s\n",part_name, file_name);
				return 0;
			}
		}
	}

	return -1;
}

int fill_file_data(FILE *dest, unsigned long long fill_size)
{
	unsigned long long count = 0;
	int LEN_DATA = 8*1024;
	unsigned char data[LEN_DATA];

	if(dest == NULL)
	{
		printf("file dest is null\n");
		return -1;
	}

	if(fill_size <= 0)
	{
		printf("fill file size err\n");
		return -1;
	}

	memset(data, 0, sizeof(data));

	while(1)
	{
		unsigned long long leave = fill_size - count;
		int write_size = LEN_DATA;

		if(leave <= 0)
		{
			printf("copy end\n");
			break;
		}
		else if(leave < LEN_DATA)
		{
			write_size = (int)leave;
		}

		size_t w_res = fwrite(&data, 1, write_size, dest);
		count += w_res;
		if(write_size != w_res)
		{
			printf("fwrite fail\n");
			return -1;
		}
	}

	return 0;
}

int copy_file_data(FILE *src, FILE *dest, unsigned long long *fill_size)
{
	int res = 0;
	unsigned long long file_size = 0;
	const int LEN_DATA = 8*1024;
	unsigned char data[LEN_DATA];
	if((src == NULL) || (dest == NULL))
	{
		printf("file src or dest is null\n");
		return -1;
	}

	fseek(src, 0, SEEK_END);
	file_size = ftell(src);
	*fill_size = file_size;
	printf("file_size=%lld\n", file_size);
	rewind(src);

	while(1)
	{
		unsigned long long current = ftell(src);
		unsigned long long leave = file_size - current;
		int read_size = LEN_DATA;
		if(leave <= 0)
		{
			printf("copy end\n");
			break;
		}
		else if(leave < LEN_DATA){
			read_size = (int)leave;
		}

		size_t res_t = fread(&data, 1, read_size, src);
		if(res_t == read_size)
		{
			size_t w_res = fwrite(&data, 1, res_t, dest);
			if(res_t != w_res)
			{
				res = -1;
				printf("fwrite fail\n");
				break;
			}
		}
		else if(feof(src) != 0)
		{
			printf("feof res_t=%zu\n", res_t);
			break;
		}
		else
		{
			printf("read fail\n");
			res = -1;
			break;
		}
	}

	return res;
}

int write_boot_file(char *boot0_name, char *boot1_name, char *dest)
{
	int ret = 0;

	FILE  *img_file = NULL;
	FILE  *boot0_file = NULL;
	FILE  *boot1_file = NULL;
	unsigned long long file_size = 0;

	img_file = fopen(dest, "ab+");
	if(!img_file)
	{
		printf("unable to open dest file\n");
		ret = -1;
		goto _err_out;
	}
	fill_file_data(img_file, 20*1024*1024); //create 20M file

	boot0_file = fopen(boot0_name, "rb");
	if(!boot0_file)
	{
		printf("unable to open %s file\n", boot0_name);
		ret = -1;
		goto _err_out;
	}
	//boot0 
	if(copy_file_data(boot0_file, img_file, &file_size))
	{
		printf("unable to write %s file in 16 block\n", boot0_name);
		ret = -1;
		goto _err_out;
	}
	fill_file_data(img_file, ((256-16)*512 - file_size));
	//boot bak
	if(copy_file_data(boot0_file, img_file, &file_size))
	{
		printf("unable to write %s file in 256 block\n", boot0_name);
		ret = -1;
		goto _err_out;
	}
	fill_file_data(img_file, ((24576-256)*512 - file_size));

	boot1_file = fopen(boot1_name, "rb");
	if(!boot1_file)
	{
		printf("unable to open %s file\n", boot1_name);
		ret = -1;
		goto _err_out;
	}
	//boot1
	if(copy_file_data(boot1_file, img_file, &file_size))
	{
		printf("unable to write %s file in 24576 block\n", boot1_name);
		ret = -1;
		goto _err_out;
	}
	fill_file_data(img_file, ((32800-24576)*512 - file_size));
	//boot1 bak
	if(copy_file_data(boot1_file, img_file, &file_size))
	{
		printf("unable to write %s file in 32800 block\n", boot1_name);
		ret = -1;
		goto _err_out;
	}
	fill_file_data(img_file, ((40960-32800)*512 - file_size));

_err_out:
	if(img_file)
		fclose(img_file);
	if(boot0_file)
		fclose(boot0_file);
	if(boot1_file)
		fclose(boot1_file);
	return ret;
}

int write_boot_file(char *boot0_name, char *boot1_name, char *dest)
{
	int ret = 0;

	FILE  *img_file = NULL;
	FILE  *boot0_file = NULL;
	FILE  *boot1_file = NULL;
	unsigned long long file_size = 0;

	img_file = fopen(dest, "ab+");
	if(!img_file)
	{
		printf("unable to open dest file\n");
		ret = -1;
		goto _err_out;
	}
	fill_file_data(img_file, (16-0)*512);

	boot0_file = fopen(boot0_name, "rb");
	if(!boot0_file)
	{
		printf("unable to open %s file\n", boot0_name);
		ret = -1;
		goto _err_out;
	}
	//boot0 
	if(copy_file_data(boot0_file, img_file, &file_size))
	{
		printf("unable to write %s file in 16 block\n", boot0_name);
		ret = -1;
		goto _err_out;
	}
	fill_file_data(img_file, ((256-16)*512 - file_size));
	//boot bak
	if(copy_file_data(boot0_file, img_file, &file_size))
	{
		printf("unable to write %s file in 256 block\n", boot0_name);
		ret = -1;
		goto _err_out;
	}
	fill_file_data(img_file, ((24576-256)*512 - file_size));

	boot1_file = fopen(boot1_name, "rb");
	if(!boot1_file)
	{
		printf("unable to open %s file\n", boot1_name);
		ret = -1;
		goto _err_out;
	}
	//boot1
	if(copy_file_data(boot1_file, img_file, &file_size))
	{
		printf("unable to write %s file in 24576 block\n", boot1_name);
		ret = -1;
		goto _err_out;
	}
	fill_file_data(img_file, ((32800-24576)*512 - file_size));
	//boot1 bak
	if(copy_file_data(boot1_file, img_file, &file_size))
	{
		printf("unable to write %s file in 32800 block\n", boot1_name);
		ret = -1;
		goto _err_out;
	}
	fill_file_data(img_file, ((40960-32800)*512 - file_size));

_err_out:
	if(img_file)
		fclose(img_file);
	if(boot0_file)
		fclose(boot0_file);
	if(boot1_file)
		fclose(boot1_file);
	return ret;
}

int create_img_to_programmer(char *file_name, sunxi_mbr_t *mbr_info, char *img_name, int mbr_size)
{
	char filename[320];
	char filepath[320];
	int i, ret;
	int  part_handle;
	unsigned crc32_total;
	unsigned long long fill_size;
	unsigned long long file_size;
	FILE  *img_file = NULL;
	FILE  *src_file = NULL;

	crc32_total = calc_crc32((void *)&(mbr_info->version), (sizeof(sunxi_mbr_t) - 4));
	if(crc32_total != mbr_info->crc32)
	{
		printf("mbr crr err,src=%d,dst=%d\n", mbr_info->crc32, crc32_total);
		return -1;
	}

	img_file = fopen(file_name, "ab+");
	if(!img_file)
	{
		printf("unable to open script file\n");

		goto _err_out;
	}

	for(i = 0; i < mbr_info->PartCount; i++)
	{
		memset(filename, 0, sizeof(filename));
		printf("mbr_info->array[i].name=%s\n", mbr_info->array[i].name);
		ret = get_partition_file(mbr_info->array[i].name, filename);
		fill_size = (((unsigned long long)(mbr_info->array[i].lenhi) << 32) | mbr_info->array[i].lenlo) * 512;
		memset(filepath, 0, sizeof(filepath));
		if(!ret)
		{
			if(!strncmp("UDISK", mbr_info->array[i].name, sizeof("UDISK")))
			{
				continue;
			}
			else
			{
				if(!strncmp("system", mbr_info->array[i].name, sizeof("system")))
				{
					memset(filename, 0, sizeof(filename));
					memcpy(filename, "system_ext4.fex", sizeof("system_ext4.fex"));
				}
				else if(!strncmp("sysrecovery", mbr_info->array[i].name, sizeof("sysrecovery")))
				{
					memset(filename, 0, sizeof(filename));
					strncpy(filename, img_name, sizeof(filename));
				}

				GetFullPath(filepath, filename);
				printf("src=%s\n", filepath);
				src_file = fopen(filepath, "rb");
				if(!src_file)
				{
					printf("unable to open %s file\n", filepath);
					goto _err_out;
				}

				if(!copy_file_data(src_file, img_file, &file_size))
				{
					printf("line=%d, file_size, fill_size=%lld\n", __LINE__,file_size, fill_size);
					fill_size = fill_size - file_size;
					fill_file_data(img_file, fill_size);
				}
				fclose(src_file);
			}
		}
		else
		{
			printf("line=%d, fill_size=%lld\n", __LINE__, fill_size);
			fill_file_data(img_file, fill_size);
		}
	}

_err_out:
	if(img_file)
		fclose(img_file);
	return 0;
}

int main(int argc, char *argv[])
{
	char source[MAX_PATH];
	char mbr_name[MAX_PATH];
	char img_name[MAX_PATH];
	char img_file[MAX_PATH];

	FILE  *source_file = NULL;
	FILE  *mbr_file = NULL;
	char *pbuf = NULL;
	char *ppbuf = NULL;
	sunxi_mbr_t *mbr_info = NULL;
	int script_len = 0;
	int mbr_size = 0;
	int ret = 0;

	memset(source, 0, MAX_PATH);
	memset(mbr_name, 0, MAX_PATH);
	memset(img_name, 0, MAX_PATH);
	memset(img_file, 0, MAX_PATH);

	if(argc == 4)
	{
		GetFullPath(source, argv[1]);	//boot0
		GetFullPath(mbr_name, argv[2]);	//uboot
		GetFullPath(img_file, argv[3]);	//img_to_programmer
	}
	else if(argc == 5)
	{
		GetFullPath(source, argv[1]);	//sys_partition.bin
		GetFullPath(mbr_name, argv[2]);	//sunxi_mbr.fex
		memcpy(img_name, argv[3], MAX_PATH);	//dlinfo.fex
		GetFullPath(img_file, argv[4]);	//img_to_programmer
	}
	else
	{
		printf("invalid input\n");
		return 0;
	}

	printf("file path=%s\n", source);
	printf("file path=%s\n", mbr_name);
	printf("file path=%s\n", img_name);
	printf("file path=%s\n", img_file);

	if(argc == 4)
	{
		source_file = fopen(img_file, "wb");
		if(!source_file)
		{
			printf("clear file\n");
			goto _err_out;
		}
		fclose(source_file);
		source_file = NULL;
		if(write_boot_file(source, mbr_name, img_file))
		{
			printf("unable to write boot file\n");
			return -1;
		}
	}
	else
	{
		source_file = fopen(source, "rb");
		if(!source_file)
		{
			printf("unable to open script file\n");

			goto _err_out;
		}
		//读出脚本的数据
		//首先获取脚本的长度
		fseek(source_file, 0, SEEK_END);
		script_len = ftell(source_file);
		fseek(source_file, 0, SEEK_SET);
		//读出脚本所有的内容
		pbuf = (char *)malloc(script_len);
		if(!pbuf)
		{
			printf("unable to malloc memory for script\n");

			goto _err_out;
		}
		memset(pbuf, 0, script_len);
		fread(pbuf, 1, script_len, source_file);
		fclose(source_file);
		source_file = NULL;
		//script使用初始化
		if(SCRIPT_PARSER_OK != script_parser_init(pbuf))
		{
			goto _err_out;
		}

		mbr_file = fopen(mbr_name, "rb");
		if(!mbr_file)
		{
			printf("can't open sunxi_mbr.fex file\n");
			return -1;
		}

		ppbuf = malloc(sizeof(sunxi_mbr_t));
		if(!ppbuf)
		{
			printf("can't malloc memory for sunxi_mbr\n");
			goto _err_out;
		}

		memset(ppbuf, 0, sizeof(sunxi_mbr_t));

		fseek(mbr_file, 0, SEEK_SET);
		fread(ppbuf, 1, sizeof(sunxi_mbr_t), mbr_file);
		mbr_info = (sunxi_mbr_t *)ppbuf;
		fclose(mbr_file);
		mbr_file = NULL;

		printf("file=%s\n", mbr_name);
		mbr_info = (sunxi_mbr_t *)ppbuf;
		printf("crc32=0x%x\n", mbr_info->crc32);
		printf("version=0x%x\n", mbr_info->version);

		if(script_parser_fetch("mbr", "size", &mbr_size) || (!mbr_size))
		{
			mbr_size = 16384;
		}
		printf("mbr size = %d\n", mbr_size);
		mbr_size = mbr_size * 1024/512;
		ret = create_img_to_programmer(img_file, mbr_info, img_name, mbr_size);
		script_parser_exit();
	}

_err_out:
	if(pbuf)
	{
		free(pbuf);
	}
	if(ppbuf)
	{
		free(ppbuf);
	}
	if(source_file)
	{
		fclose(source_file);
	}
	if(mbr_file)
	{
		fclose(mbr_file);
	}

	return 0;
}
