#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include "include/sunxi_mbr.h"
#include "include/types.h"
#include "include/gpt.h"
#include "include/crc.h"


int sunxi_mbr_convert_to_gpt(void *sunxi_mbr_buf, char *gpt_buf, int storage_type, u32 total_sectors)
{
	legacy_mbr   *remain_mbr;
	sunxi_mbr_t  *sunxi_mbr = (sunxi_mbr_t *)sunxi_mbr_buf;

	char         *pbuf = gpt_buf;
	gpt_header   *gpt_head;
	gpt_entry    *pgpt_entry = NULL;
	char*         gpt_entry_start=NULL;
	u32           data_len = 0;
	u32           logic_offset = 0;
	int           i,j = 0;

	unsigned char guid[16] = {0x88,0x38,0x6f,0xab,0x9a,0x56,0x26,0x49,0x96,0x68,0x80,0x94,0x1d,0xcb,0x40,0xbc};
	unsigned char part_guid[16] = {0x46,0x55,0x08,0xa0,0x66,0x41,0x4a,0x74,0xa3,0x53,0xfc,0xa9,0x27,0x2b,0x8e,0x45};

	if (strncmp((const char*)sunxi_mbr->magic, SUNXI_MBR_MAGIC, 8))
	{
		printf("%s:not sunxi mbr, can't convert to GPT partition, magic=%s\n", __func__, sunxi_mbr->magic);
		return 0;
	}

	if (crc32(0, (unsigned char *)(sunxi_mbr_buf + 4), SUNXI_MBR_SIZE - 4) != sunxi_mbr->crc32)
	{
		printf("%s:sunxi mbr crc error, can't convert to GPT partition\n",__func__);
		return 0;
	}

	if (storage_type == STORAGE_EMMC || storage_type == STORAGE_EMMC3
		|| storage_type == STORAGE_SD)
	{
		logic_offset = CONFIG_MMC_LOGICAL_OFFSET;
	}
	else
	{
		logic_offset = 0;
	}

	/* 1. LBA0: write legacy mbr,part type must be 0xee */
	remain_mbr = (legacy_mbr *)pbuf;
	memset(remain_mbr, 0x0, 512);
	remain_mbr->partition_record[0].sector = 0x2;
	remain_mbr->partition_record[0].cyl = 0x0;
	remain_mbr->partition_record[0].sys_ind = EFI_PMBR_OSTYPE_EFI_GPT;
	remain_mbr->partition_record[0].end_head = 0xFF;
	remain_mbr->partition_record[0].end_sector = 0xFF;
	remain_mbr->partition_record[0].end_cyl = 0xFF;
	remain_mbr->partition_record[0].start_sect = 1UL;
	remain_mbr->partition_record[0].nr_sects = 0xffffffff;
	remain_mbr->signature = MSDOS_MBR_SIGNATURE;
	data_len += 512;

	/* 2. LBA1: fill primary gpt header */
	gpt_head = (gpt_header *)(pbuf + data_len);
	gpt_head->signature= GPT_HEADER_SIGNATURE;
	gpt_head->revision = GPT_HEADER_REVISION_V1;
	gpt_head->header_size = GPT_HEADER_SIZE;
	gpt_head->header_crc32 = 0x00;
	gpt_head->reserved1 = 0x0;
	gpt_head->my_lba = 0x01;
	gpt_head->alternate_lba = total_sectors - 1;
	gpt_head->first_usable_lba = sunxi_mbr->array[0].addrlo + logic_offset;
	/*1 GPT head + 32 GPT entry*/
	gpt_head->last_usable_lba = total_sectors - (1 + 32) - 1;
	memcpy(gpt_head->disk_guid.b,guid,16);
	gpt_head->partition_entry_lba = (storage_type == STORAGE_NAND) ? 2 : PRIMARY_GPT_ENTRY_OFFSET;
	gpt_head->num_partition_entries = 0x80;
	gpt_head->sizeof_partition_entry = GPT_ENTRY_SIZE;
	gpt_head->partition_entry_array_crc32 = 0;
	data_len += 512;

	/* 3. LBA2~LBAn: fill gpt entry */
	gpt_entry_start = (pbuf + data_len);
	for (i = 0; i < sunxi_mbr->PartCount; i++)
	{
		/*udisk is the first part*/
		int pos = (i == sunxi_mbr->PartCount-1) ? 0: i+1;
		pgpt_entry = (gpt_entry *)(gpt_entry_start + (pos)*GPT_ENTRY_SIZE);

		pgpt_entry->partition_type_guid = PARTITION_BASIC_DATA_GUID;

		memcpy(pgpt_entry->unique_partition_guid.b,part_guid, 16);
		pgpt_entry->unique_partition_guid.b[15] = part_guid[15] + i;

		pgpt_entry->starting_lba = ((u64)sunxi_mbr->array[i].addrhi<<32) + sunxi_mbr->array[i].addrlo + logic_offset;
		pgpt_entry->ending_lba = pgpt_entry->starting_lba \
			+((u64)sunxi_mbr->array[i].lenhi<<32)  \
			+ sunxi_mbr->array[i].lenlo-1;

		//UDISK partition
		if(i == sunxi_mbr->PartCount-1)
		{
			pgpt_entry->ending_lba = gpt_head->last_usable_lba - 1;
		}

		printf("GPT:%-12s: %-12llx  %-12llx\n", sunxi_mbr->array[i].name, pgpt_entry->starting_lba, pgpt_entry->ending_lba);
		if(sunxi_mbr->array[i].ro == 1)
		{
			pgpt_entry->attributes.fields.type_guid_specific = 0x6000;
		}
		else
		{
			pgpt_entry->attributes.fields.type_guid_specific = 0x8000;
		}

		//ASCII to unicode
		memset(pgpt_entry->partition_name, 0, PARTNAME_SZ*sizeof(efi_char16_t));
		for(j = 0; j < strlen((char *)sunxi_mbr->array[i].name); j++ )
		{
			pgpt_entry->partition_name[j] = (efi_char16_t)sunxi_mbr->array[i].name[j];
		}
		data_len += GPT_ENTRY_SIZE;

	}

	//entry crc
	gpt_head->partition_entry_array_crc32 = crc32(0, (unsigned char *)gpt_entry_start, (gpt_head->num_partition_entries)*(gpt_head->sizeof_partition_entry));


	//gpt crc
	gpt_head->header_crc32 = crc32(0, (unsigned char *)gpt_head, sizeof(gpt_header));
	printf("gpt_head->header_crc32 = 0x%x\n",gpt_head->header_crc32);

	/* 4. LBA-1: the last sector fill backup gpt header */

	return data_len;
}

