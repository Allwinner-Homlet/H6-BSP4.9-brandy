/*
 * (C) Copyright 2014
 * jaon_Yin(yinxinliang),  Software Engineering, yinxinliang@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
 
#ifndef __SUNXI_BMP_HEAD_H__
#define __SUNXI_BMP_HEAD_H__
typedef  struct  tagBITMAPFILEHEADER
{ 
unsigned short int  bfType;       //λͼ�ļ������ͣ�����ΪBM 
unsigned long       bfSize;       //�ļ���С�����ֽ�Ϊ��λ
unsigned short int  bfReserverd1; //λͼ�ļ������֣�����Ϊ0 
unsigned short int  bfReserverd2; //λͼ�ļ������֣�����Ϊ0 
unsigned long       bfbfOffBits;  //λͼ�ļ�ͷ�����ݵ�ƫ���������ֽ�Ϊ��λ
}__attribute__ ((packed))BITMAPFILEHEADER;

typedef  struct  tagBITMAPINFOHEADER 
{ 
long biSize;                        //�ýṹ��С���ֽ�Ϊ��λ
long  biWidth;                     //ͼ�ο��������Ϊ��λ
long  biHeight;                     //ͼ�θ߶�������Ϊ��λ
short int  biPlanes;               //Ŀ���豸�ļ��𣬱���Ϊ1 
short int  biBitcount;             //��ɫ��ȣ�ÿ����������Ҫ��λ��
long  biCompression;        //λͼ��ѹ������
long  biSizeImage;              //λͼ�Ĵ�С�����ֽ�Ϊ��λ
long  biXPelsPermeter;       //λͼˮƽ�ֱ��ʣ�ÿ��������
long  biYPelsPermeter;       //λͼ��ֱ�ֱ��ʣ�ÿ��������
long  biClrUsed;            //λͼʵ��ʹ�õ���ɫ���е���ɫ��
long  biClrImportant;       //λͼ��ʾ��������Ҫ����ɫ��
}__attribute__ ((packed))BITMAPINFOHEADER;

typedef struct 
{
	BITMAPFILEHEADER file;
	BITMAPINFOHEADER info;
}bmp_haedr;

#endif