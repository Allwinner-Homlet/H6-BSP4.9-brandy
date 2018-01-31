/*
**********************************************************************************************************************
*											        eGon
*						           the Embedded GO-ON Bootloader System
*									       eGON arm boot sub-system
*
*						  Copyright(C), 2006-2014, Allwinner Technology Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      : Jerry
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/

#ifndef __CREATE_CERT__H__
#define __CREATE_CERT__H__

int create_cert_for_toc0(char *lpCfg, toc_descriptor_t *toc0, char *keypath);
int create_cert_for_toc1(char *lpCfg, toc_descriptor_t *toc1, char *keypath, char *cnfpath_base);

int createcnf_for_package(char *lpCfg, toc_descriptor_t *package, int type);

//int createcnf_for_package(char *lpCfg, toc_descriptor_t *package);
int u8_to_str(u8 *p_buff_u8, u32 u8_len, u8 *p_str, u32 str_buff_len);
int __sunxi_bytes_merge(u8 *dst, u32 dst_len, u8 *src, uint src_len);
int	__merge_certif_for_toc0(char *hash_value, u8 *key_n, u8 *key_e, u8 *key_d, char *cert_name);

#endif  /*__CREATE_CERT__H__*/
