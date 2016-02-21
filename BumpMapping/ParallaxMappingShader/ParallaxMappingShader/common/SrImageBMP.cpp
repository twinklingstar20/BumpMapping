/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/
#include "SrImageBmp.h"
#include "SrColorQuant.h"
#include <malloc.h>
#include <stdlib.h>
#include <algorithm>
#include <assert.h>


SrImageBmp::SrImageBmp(int isReadOnly):SrImage()
{
	m_ptrInfoHeader	=	NULL;
	m_ptrFileHeader	=	NULL;
	m_ptrColorMap	=	NULL;
	m_ptrImageData	=	NULL;
	m_inIsReadOnly	=	isReadOnly;
}

SrImageBmp::~SrImageBmp()
{
	empty();
}

void SrImageBmp::empty()
{
	if( m_ptrInfoHeader )
	{
		free(m_ptrInfoHeader);
		m_ptrInfoHeader = NULL;
	}
	if( m_ptrFileHeader )
	{
		free(m_ptrFileHeader);
		m_ptrFileHeader = NULL;
	}
	if( m_ptrColorMap )
	{
		free(m_ptrColorMap);
		m_ptrColorMap = NULL;
	}
	if( m_ptrImageData )
	{
		free(m_ptrImageData);
		m_ptrImageData = NULL;
	}
}

bool SrImageBmp::readHeader(FILE* flPtrImage)
{
	BITMAPFILEHEADER* ptrFileHeader = (BITMAPFILEHEADER*)malloc(sizeof(BITMAPFILEHEADER));
	if( !ptrFileHeader )
	{
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}
	BITMAPINFOHEADER* ptrInfoHeader = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER));
	if( !ptrInfoHeader )
	{
		free(ptrFileHeader);
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}
	if( fread(ptrFileHeader,sizeof(BITMAPFILEHEADER),1,flPtrImage)!=1 )
	{
		free(ptrFileHeader);
		free(ptrInfoHeader);
		m_inError = IMAGE_READ_FAIL;
		return false;
	}
	if( fread(ptrInfoHeader,sizeof(BITMAPINFOHEADER),1,flPtrImage)!=1 )
	{
		free(ptrFileHeader);
		free(ptrInfoHeader);
		m_inError = IMAGE_READ_FAIL;
		return false;
	}
	m_ptrFileHeader = ptrFileHeader;
	m_ptrInfoHeader = ptrInfoHeader;
	return true;
}

bool SrImageBmp::writeHeader(FILE* flPtrImage)
{
	if( fwrite(m_ptrFileHeader,sizeof(BITMAPFILEHEADER),1,flPtrImage)!=1 )
	{
		m_inError = IMAGE_READ_FAIL;
		return false;
	}
	if( fwrite(m_ptrInfoHeader,sizeof(BITMAPINFOHEADER),1,flPtrImage)!=1 )
	{
		m_inError = IMAGE_READ_FAIL;
		return false;
	}
	return true;
}

bool SrImageBmp::readColorMap(FILE* flPtrImage)
{
	int rgbQuadNumber = 0;
	RGBQUAD* ptrQuads = NULL;
	//���biClrUsed��Ϊ0����ʹ�ø���ֵ��Ϊ��ɫ��ĳߴ硣
	if( m_ptrInfoHeader->biClrUsed )
	{
		rgbQuadNumber = m_ptrInfoHeader->biClrUsed;
	}
	else if( m_ptrInfoHeader->biBitCount==1 )
	{
		rgbQuadNumber = 2;
	}
	else if( m_ptrInfoHeader->biBitCount==4 )
	{
		rgbQuadNumber = 16;
	}
	else if( m_ptrInfoHeader->biBitCount==8 )
	{
		rgbQuadNumber = 256;
	}
	if( rgbQuadNumber )
	{
		ptrQuads = (RGBQUAD *)malloc(sizeof(RGBQUAD)*rgbQuadNumber);
		if( !ptrQuads )
		{
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		if( fread(ptrQuads,sizeof(RGBQUAD),rgbQuadNumber,flPtrImage)!= rgbQuadNumber )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrQuads);
			return false;
		}
	}
	m_ptrColorMap = ptrQuads;
	return true;
}


long SrImageBmp::getLineBytes(int imgWidth,int bitCount)const
{
	return ((imgWidth*bitCount+31)>>5)<<2;
}

int	 SrImageBmp::getWidth()const
{
	assert(m_ptrInfoHeader);
	return m_ptrInfoHeader->biWidth;
}
int	 SrImageBmp::getHeight()const
{
	assert(m_ptrInfoHeader);
	return m_ptrInfoHeader->biHeight;
}


unsigned char* SrImageBmp::getImageData()const
{
	return m_ptrImageData;
}

unsigned long	 SrImageBmp::getFileSize()const
{
	assert(m_ptrInfoHeader);
	return m_ptrFileHeader->bfSize;
}

unsigned long		SrImageBmp::getCompression()const
{
	assert(m_ptrInfoHeader);
	return m_ptrInfoHeader->biCompression;
}
unsigned short		SrImageBmp::getPixelDepth()const
{
	return m_ptrInfoHeader->biBitCount;
}

bool				SrImageBmp::getIsRGB()const
{
	assert(m_ptrInfoHeader);
	return m_ptrInfoHeader->biBitCount!=32;
}

bool SrImageBmp::isValid()const
{
	return m_ptrImageData!=NULL;
}

bool SrImageBmp::decodeFile(BYTE* ptrImageData,unsigned char*& rgbPtrOutData)
{
	int pixelCount = m_ptrInfoHeader->biWidth * m_ptrInfoHeader->biHeight;
	int col , row;
	int outDataIndex = 0;
	int dwLineBytes=getLineBytes(m_ptrInfoHeader->biWidth,m_ptrInfoHeader->biBitCount);
	//���ڴ洢BMP�ļ�������ɵ�RGB��ɫ����
	if( m_ptrInfoHeader->biBitCount==32 )
	{
		rgbPtrOutData = (unsigned char*)malloc(pixelCount*4);
	}
	else
	{
		rgbPtrOutData = (unsigned char*)malloc(pixelCount*3);
	}
	if(!rgbPtrOutData)
	{
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}	
	if( m_ptrInfoHeader->biBitCount==1 || m_ptrInfoHeader->biBitCount==4 || m_ptrInfoHeader->biBitCount==8 )
	{//����ļ���Ϣͷ��biBitCount��ֵ��1��4������8������LUT����������ʹ�õ�����ɫ������
		int bitCount = m_ptrInfoHeader->biBitCount;
		int maskNum;
		BYTE mask1[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
		BYTE shift1[]={0,1,2,3,4,5,6,7};
		BYTE mask4[]={0x0f,0xf0};
		BYTE shift4[]={0,4};
		BYTE mask8[]={0xff};
		BYTE shift8[]={0};
		BYTE* ptrMask,*ptrShift;
		switch(bitCount)
		{
		case 1:
			{
				maskNum = 8;
				ptrMask = mask1;
				ptrShift= shift1;
			}
			break;
		case 4:
			{
				maskNum = 2;
				ptrMask = mask4;
				ptrShift= shift4;
			}
			break;
		case 8:
			{
				maskNum = 1;
				ptrMask = mask8;
				ptrShift= shift8;
			}
			break;
		}
		int bit = 0 , byteIndx = 0; 
		BYTE tmpdata , indxData;

		for( row=0 ; row<m_ptrInfoHeader->biHeight ; row++ )
		{
			for( col=0 , byteIndx=0 ; col<m_ptrInfoHeader->biWidth ; byteIndx++)
			{
				tmpdata = *(ptrImageData+byteIndx);
				for( bit=0 ; bit<maskNum && col<m_ptrInfoHeader->biWidth; bit++,col++)
				{
					indxData = (tmpdata&ptrMask[bit])>>ptrShift[bit];
					*(rgbPtrOutData+outDataIndex++)		= (m_ptrColorMap+indxData)->rgbRed;
					*(rgbPtrOutData+outDataIndex++)		= (m_ptrColorMap+indxData)->rgbGreen;
					*(rgbPtrOutData+outDataIndex++)		= (m_ptrColorMap+indxData)->rgbBlue;
				}
			}

			ptrImageData +=dwLineBytes;
		}
	}
	else if( m_ptrInfoHeader->biBitCount==16 )
	{//����ļ���Ϣͷ��biBitCountֵ��16
		for( row=0 ; row<m_ptrInfoHeader->biHeight ; row++ )
		{
			BYTE* tmpPtrData = ptrImageData;
			for( col=0 ; col<m_ptrInfoHeader->biWidth ; col++,tmpPtrData+=2 )
			{//һ����16λ�����1λ����������5λ��R���м�5λ��G�����5λ��B��
				//��ý����������3λ
				WORD wPixel = *(WORD*)tmpPtrData;
				*(rgbPtrOutData+outDataIndex)		= ((wPixel&0x7C00) >> 10);
				*(rgbPtrOutData+outDataIndex + 1)	= ((wPixel&0x03E0) >> 5);
				*(rgbPtrOutData+outDataIndex + 2)	= ((wPixel&0x001F) >> 0);

				//�ο�http://www.ryanjuckett.com/programming/graphics/26-parsing-colors-in-a-tga-file�еķ���
				*(rgbPtrOutData+outDataIndex)		= (*(rgbPtrOutData+outDataIndex)<<3) | (*(rgbPtrOutData+outDataIndex)>>2);
				*(rgbPtrOutData+outDataIndex + 1)	= (*(rgbPtrOutData+outDataIndex+1)<<3) | (*(rgbPtrOutData+outDataIndex+1)>>2);
				*(rgbPtrOutData+outDataIndex + 2)	= (*(rgbPtrOutData+outDataIndex+2)<<3) | (*(rgbPtrOutData+outDataIndex+2)>>2);
				outDataIndex += 3;
			}
			ptrImageData +=dwLineBytes;
		}
	}
	else if( m_ptrInfoHeader->biBitCount==24 )
	{//�ļ���Ϣͷ��biBitCount��24
		for( row=0 ; row<m_ptrInfoHeader->biHeight ; row++ )
		{
			BYTE* tmpPtrData = ptrImageData;
			for( col=0 ; col<m_ptrInfoHeader->biWidth ; col++ )
			{//3���ֽڣ����һ��������R,�м�һ����G�����һ����B
				*(rgbPtrOutData+outDataIndex+2)	= *(tmpPtrData++);
				*(rgbPtrOutData+outDataIndex+1)	= *(tmpPtrData++);
				*(rgbPtrOutData+outDataIndex)	= *(tmpPtrData++);
				outDataIndex +=3;
			}
			ptrImageData +=dwLineBytes;
		}
	}
	else if( m_ptrInfoHeader->biBitCount==32 )
	{//�ļ���Ϣͷ��biBitCount��32
		for( row=0 ; row<m_ptrInfoHeader->biHeight ; row++ )
		{
			BYTE* tmpPtrData = ptrImageData;
			for( col=0 ; col<m_ptrInfoHeader->biWidth ; col++)
			{//4���ֽڣ��������ֽڷֱ���R,G,B,Apha
				*(rgbPtrOutData+outDataIndex+2)	= *(tmpPtrData++);
				*(rgbPtrOutData+outDataIndex+1)	= *(tmpPtrData++);
				*(rgbPtrOutData+outDataIndex)	= *(tmpPtrData++);
				*(rgbPtrOutData+outDataIndex+3)	= *(tmpPtrData++);
				outDataIndex +=4;
			}
			ptrImageData +=dwLineBytes;
		}
	}
	return true;
}

bool SrImageBmp::checkReadFileFormat(FILE* flFileHandle)
{
	//biPlanes����Ϊ1��biWidth��biHeight����ΪС�ڵ���0��ֵ��
	//biSize������BITMAPINFOHEADER�ṹ��Ĵ�С��bfType������"BM"
	if( m_ptrInfoHeader->biPlanes!=1 ||
		m_ptrInfoHeader->biWidth<=0 ||
		m_ptrInfoHeader->biHeight<=0 ||
		m_ptrInfoHeader->biSize!=sizeof(BITMAPINFOHEADER) ||
		m_ptrFileHeader->bfType!=(('M'<<8)+'B'))
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	//�ж�biBitCount�Ƿ����ڼ���{1,4,8,16,24,32}�е�ֵ
	if( !(m_ptrInfoHeader->biBitCount==1) &&
		!(m_ptrInfoHeader->biBitCount==4) &&
		!(m_ptrInfoHeader->biBitCount==8) &&
		!(m_ptrInfoHeader->biBitCount==16) &&
		!(m_ptrInfoHeader->biBitCount==24) &&
		!(m_ptrInfoHeader->biBitCount==32))
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	//�ж��Ƿ���֧�ֵ��ļ�����
	if( !(m_ptrInfoHeader->biCompression==BI_RGB || 
		m_ptrInfoHeader->biCompression==BI_RLE4&&m_ptrInfoHeader->biBitCount==4 ||
		m_ptrInfoHeader->biCompression==BI_RLE8&&m_ptrInfoHeader->biBitCount==8))
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
#if (SR_IMAGE_BMP_CHECK_OFFBITS)
	//�ж�BMP�ļ�ͷ�е�bfOffBitsֵ�Ƿ���ȷ
	//��windows�£��������������ȷ������Ӱ��ͼ����ʾ������ʾBMPͼ��
	int rgbNum = 0;
	if( m_ptrInfoHeader->biClrUsed )
		rgbNum = m_ptrInfoHeader->biClrUsed;
	else if( m_ptrInfoHeader->biBitCount==1 )
		rgbNum = 2;
	else if( m_ptrInfoHeader->biBitCount==4 )
		rgbNum = 16;
	if( m_ptrInfoHeader->biBitCount==8 )
		rgbNum = 256;
	if( m_ptrFileHeader->bfOffBits!= (rgbNum<<2)+54 )
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
#endif

#if (SR_IMAGE_BMP_CHECK_FILESIZE)
	//�ж��ļ�ͷ��bfSize�Ƿ���ȷ
	int current = ftell(flFileHandle);
	if( fseek(flFileHandle,0,SEEK_END)==-1 )
	{
		m_inError = IMAGE_SEEK_FAIL;
		return false;
	}
	int fileSize = ftell(flFileHandle);
	if( fileSize!=m_ptrFileHeader->bfSize )
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	if( fseek(flFileHandle,current,SEEK_SET)==-1 )
	{
		m_inError = IMAGE_SEEK_FAIL;
		return false;
	}
#endif
	return true;
}

bool SrImageBmp::readFile(const char* chPtrImageFile,unsigned char*& ptrOutData ,int& inRGBType)
{
	if( m_inIsReadOnly!=IMAGE_READ_ONLY )
	{
		m_inError = IMAGE_OBJECT_WRITE_ONLY;
		return false;
	}

	FILE *fp;
	fp = fopen(chPtrImageFile,"rb");
	if( !fp )
	{
		m_inError = IMAGE_OPEN_FAIL;
		return false;
	}
	//���ļ�ͷ���ļ���Ϣͷ
	if( !readHeader(fp) )
	{
		fclose(fp);	
		return false;
	}
#ifdef _CONSOLE
	printFileHeader(m_ptrFileHeader);

	printFileHeader(m_ptrInfoHeader);
#endif
	if( !checkReadFileFormat(fp) )
	{
		empty();
		fclose(fp);
		return false;
	}
	//���ļ��ж�ȡ��ɫ������
	if( !readColorMap(fp) )
	{
		empty();
		fclose(fp);
		return false;
	}
	if( m_ptrInfoHeader->biCompression == BI_RLE8 ||
		m_ptrInfoHeader->biCompression == BI_RLE4)
	{
		//�����г̱�����н���
		if( !decodeRLE(fp,ptrOutData) )
		{
			empty();			
			fclose(fp);
			return false;
		}
	}
	else if( !readUncompression(fp,ptrOutData) )	//biCompressionΪBI_RGB
	{
		
		fclose(fp);
		empty();
		return false;
	}
	fclose(fp);

	if( m_ptrInfoHeader->biBitCount==32 )
		inRGBType = IMAGE_RGBA;
	else
		inRGBType = IMAGE_RGB;
	m_ptrImageData = ptrOutData;

	return true;
}

bool SrImageBmp::decodeRLE(FILE* flPtrImage,BYTE*& btPtrImageData)
{
	BYTE firstByte,secondByte;
	BYTE buffer[2];
	int xIndx = 0 , yIndx = 0 , curY = 0;
	int pixelCount = m_ptrInfoHeader->biWidth * m_ptrInfoHeader->biHeight;
	BYTE* ptrImageData = (BYTE*)malloc(pixelCount*3);
	if( !ptrImageData )
	{
		m_inError = IMAGE_READ_FAIL;
		return false;
	}

	while(true)
	{
		if( fread(&firstByte,1,1,flPtrImage) != 1 )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrImageData);
			return false;
		}
		if( firstByte!=0 )
		{
			if( fread(&secondByte,1,1,flPtrImage) != 1 )
			{
				m_inError = IMAGE_READ_FAIL;
				free(ptrImageData);
				return false;
			}
			if( firstByte + xIndx + m_ptrInfoHeader->biWidth * yIndx  > pixelCount )
			{
				m_inError = IMAGE_UNKNOWN_FORMAT;
				free(ptrImageData);
				return false;
			}
			curY = yIndx * 3 * m_ptrInfoHeader->biWidth;
			for( ; firstByte>0 ;  )
			{
				if( m_ptrInfoHeader->biCompression == BI_RLE8 )
				{
					*(ptrImageData + curY + 3*xIndx + 2 )	= m_ptrColorMap[secondByte].rgbBlue;
					*(ptrImageData + curY + 3*xIndx + 1 )	= m_ptrColorMap[secondByte].rgbGreen;
					*(ptrImageData + curY + 3*xIndx )		= m_ptrColorMap[secondByte].rgbRed;
					xIndx ++;firstByte--;
				}
				else if( m_ptrInfoHeader->biCompression == BI_RLE4 )
				{
					*(ptrImageData + curY + 3*xIndx + 2 )		= m_ptrColorMap[(secondByte>>4)&0x0f].rgbBlue;
					*(ptrImageData + curY + 3*xIndx + 1 )	= m_ptrColorMap[(secondByte>>4)&0x0f].rgbGreen;
					*(ptrImageData + curY + 3*xIndx )	= m_ptrColorMap[(secondByte>>4)&0x0f].rgbRed;
					xIndx ++;firstByte--;
					if( firstByte>0 )
					{
						*(ptrImageData + curY + 3*xIndx + 2)		= m_ptrColorMap[secondByte&0x0f].rgbBlue;
						*(ptrImageData + curY + 3*xIndx + 1 )	= m_ptrColorMap[secondByte&0x0f].rgbGreen;
						*(ptrImageData + curY + 3*xIndx )	= m_ptrColorMap[secondByte&0x0f].rgbRed;
						xIndx ++;firstByte--;
					}
				}
			}
		}
		else
		{
			if( fread(&secondByte,1,1,flPtrImage) != 1 )
			{
				m_inError = IMAGE_READ_FAIL;
				free(ptrImageData);
				return false;
			}
			if( secondByte == 0x01 )
			{
				btPtrImageData = ptrImageData;
				return true;
			}
			else if( secondByte==0x00 )
			{
				yIndx ++;
				xIndx = 0;
			}
			else if( secondByte==0x02 )
			{
				if( fread(buffer,1,2,flPtrImage) != 2 )
				{
					free(ptrImageData);
					m_inError = IMAGE_READ_FAIL;
					return false;
				}
				xIndx += buffer[0];
				yIndx += buffer[1];
				if( xIndx + m_ptrInfoHeader->biWidth * yIndx  > pixelCount )
				{
					m_inError = IMAGE_UNKNOWN_FORMAT;
					free(ptrImageData);
					return false;
				}
			}
			else
			{
				if( xIndx + m_ptrInfoHeader->biWidth * yIndx +  secondByte> pixelCount )
				{
					m_inError = IMAGE_UNKNOWN_FORMAT;
					free(ptrImageData);
					return false;
				}
				curY = yIndx * 3 * m_ptrInfoHeader->biWidth;
				int tmpNum = secondByte;
				for( ; tmpNum>0 ; )
				{
					if( fread(buffer,1,1,flPtrImage) != 1 )
					{
						m_inError = IMAGE_READ_FAIL;
						free(ptrImageData);
						return false;
					}
					if( m_ptrInfoHeader->biCompression == BI_RLE8 )
					{
						*(ptrImageData + curY + 3*xIndx + 2)		= m_ptrColorMap[buffer[0]].rgbBlue;
						*(ptrImageData + curY + 3*xIndx + 1 )	= m_ptrColorMap[buffer[0]].rgbGreen;
						*(ptrImageData + curY + 3*xIndx )	= m_ptrColorMap[buffer[0]].rgbRed;
						xIndx ++;tmpNum--;
					}
					else if( m_ptrInfoHeader->biCompression == BI_RLE4 )
					{
						*(ptrImageData + curY + 3*xIndx + 2)		= m_ptrColorMap[(buffer[0]&0xf0)>>4].rgbBlue;
						*(ptrImageData + curY + 3*xIndx + 1 )	= m_ptrColorMap[(buffer[0]&0xf0)>>4].rgbGreen;
						*(ptrImageData + curY + 3*xIndx )	= m_ptrColorMap[(buffer[0]&0xf0)>>4].rgbRed;
						xIndx ++;tmpNum--;
						if( tmpNum>0 )
						{
							*(ptrImageData + curY + 3*xIndx + 2)		= m_ptrColorMap[buffer[0]&0x0f].rgbBlue;
							*(ptrImageData + curY + 3*xIndx + 1 )	= m_ptrColorMap[buffer[0]&0x0f].rgbGreen;
							*(ptrImageData + curY + 3*xIndx )	= m_ptrColorMap[buffer[0]&0x0f].rgbRed;
							xIndx ++;tmpNum--;
						}
					}
				}
				if(  (m_ptrInfoHeader->biCompression==BI_RLE8 && secondByte%2==1) ||
					(m_ptrInfoHeader->biCompression==BI_RLE4 && secondByte%4==1))
				{
					if( fread(buffer,1,1,flPtrImage) != 1 )
					{
						m_inError = IMAGE_READ_FAIL;
						free(ptrImageData);
						return false;
					}
				}
			}
		}
	}

	btPtrImageData = ptrImageData;
	return true;
}

bool SrImageBmp::readUncompression(FILE* filePtrImage,unsigned char*& ptrOutData)
{
	int dwLineBytes=getLineBytes(m_ptrInfoHeader->biWidth,m_ptrInfoHeader->biBitCount);
	int	imageSize = dwLineBytes*m_ptrInfoHeader->biHeight;
	BYTE* ptrImageData=(BYTE*)malloc(imageSize);
	if(!ptrImageData)
	{
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}

	//��BMP�ж�ȡͼ������
	if( imageSize != fread(ptrImageData,1,imageSize,filePtrImage) )
	{
		m_inError = IMAGE_READ_FAIL;
		free(ptrImageData);
		return false;
	}

	//�Զ�ȡ��ͼ�����ݽ��н���
	if( !decodeFile(ptrImageData,ptrOutData) )
	{
		free(ptrImageData);
		return false;
	}
	free(ptrImageData);

	return true;
}


bool SrImageBmp::writeColorMapImage(FILE* filePtrImage,const SrColorQuant& colorQuant)
{
	int row , col;
	DWORD dwLineBytes=getLineBytes(m_ptrInfoHeader->biWidth,m_ptrInfoHeader->biBitCount);
	BYTE* wrFileData = (BYTE*)malloc(dwLineBytes);
	BYTE* byPtrImageData = m_ptrImageData;
	if( !wrFileData )
	{
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}
	int wrFileDataNum = 0;
	int numColorPerByte = 8/m_ptrInfoHeader->biBitCount;
	int numIndex = 0 , bmpByte = 0;
	int shift;
	if( m_ptrInfoHeader->biBitCount==4 ) shift = 4;
	else if( m_ptrInfoHeader->biBitCount==8 ) shift = 0;
	for( row=0 ; row<m_ptrInfoHeader->biHeight ; row++ )
	{
		for( col=0 ; col<m_ptrInfoHeader->biWidth ; col++ )
		{
			BYTE red = *byPtrImageData;
			BYTE green = *(byPtrImageData+1);
			BYTE blue = *(byPtrImageData+2);
			bmpByte |= colorQuant.indexOctree(red,green,blue);
			numIndex ++;
			if( numIndex==numColorPerByte )
			{
				*( wrFileData + wrFileDataNum++ ) = bmpByte;
				bmpByte = numIndex = 0;
			}
			else
			{
				bmpByte = bmpByte<<shift;
			}
			byPtrImageData += 3;
		}
		if( m_ptrInfoHeader->biCompression==BI_RGB)
		{
			if( dwLineBytes!=fwrite(wrFileData,1,dwLineBytes,filePtrImage) )
			{
				free(wrFileData);
				m_inError = IMAGE_WRITE_FAIL;
				return false;
			}
		}
		wrFileDataNum = 0;
	}
	free(wrFileData);
	return true;
}


bool SrImageBmp::writeNoColorMapImage(FILE* filePtrImage)
{
	int row ,col;
	int inWidth = m_ptrInfoHeader->biWidth , inHeight = m_ptrInfoHeader->biHeight;
	int inBitCount = m_ptrInfoHeader->biBitCount;
	DWORD dwLineBytes = getLineBytes(inWidth,inBitCount);
	BYTE* wrFileData = (BYTE*)malloc(dwLineBytes);
	if( !wrFileData )	
	{
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}
	WORD tmpRgbData=0;
	BYTE* tmpPtrData = m_ptrImageData;
	int inFileDataNum = 0;
	switch(inBitCount)
	{
	case 16:
		{
			for( row=0 ; row<inHeight ; row++ )
			{
				inFileDataNum = 0;
				for( col=0 ; col<inWidth ; col++ )
				{
					tmpRgbData = ((((WORD)*(tmpPtrData))>>3)<<10) + ((((WORD)*(tmpPtrData+1))>>3)<<5) + (((WORD)*(tmpPtrData+2))>>3);
					*((WORD*)(wrFileData + inFileDataNum)) = tmpRgbData;
					inFileDataNum += 2;
					tmpPtrData += 3;
				}
				if( dwLineBytes!=fwrite(wrFileData,1,dwLineBytes,filePtrImage) )
				{
					free(wrFileData);
					m_inError = IMAGE_WRITE_FAIL;
					return false;
				}
			}
		}
		break;
	case 24:
		{
			for( row=0 ; row<inHeight ; row++ )
			{
				inFileDataNum = 0;
				for( col=0 ; col<inWidth ; col++ )
				{
					*(wrFileData + inFileDataNum + 2)		= *(tmpPtrData++);
					*(wrFileData + inFileDataNum + 1)	= *(tmpPtrData++);
					*(wrFileData + inFileDataNum)	= *(tmpPtrData++);
					inFileDataNum += 3;
				}
				if( dwLineBytes != fwrite(wrFileData,1,dwLineBytes,filePtrImage) )
				{
					free(wrFileData);
					m_inError = IMAGE_WRITE_FAIL;
					return false;
				}
			}
		}
		break;
	default:
		{
			m_inError = IMAGE_UNKNOWN_FORMAT;
			free(wrFileData);
		}
		return false;
	}
	free(wrFileData);
	return true;
}

bool SrImageBmp::writeColorMap(FILE* filePtrImage,SrColorQuant& colorQuant)
{
	int rgbQuadNumber = 0;
	RGBQUAD* rgbPtrRGBQuad = NULL;

	if( m_ptrInfoHeader->biBitCount<=8 )
	{
		if( m_ptrInfoHeader->biBitCount==1 )
			rgbQuadNumber = 2;
		else if( m_ptrInfoHeader->biBitCount==4 )
			rgbQuadNumber = 16;
		else if( m_ptrInfoHeader->biBitCount==8 )
			rgbQuadNumber = 256;

		if( m_ptrInfoHeader->biBitCount==1 )
		{
			rgbPtrRGBQuad = (RGBQUAD *)malloc(sizeof(RGBQUAD)*rgbQuadNumber);
			if( !rgbPtrRGBQuad )	
			{
				m_inError = IMAGE_NO_MEMORY;
				return false;
			}
			rgbPtrRGBQuad[0].rgbRed = 0;
			rgbPtrRGBQuad[0].rgbGreen = 0;
			rgbPtrRGBQuad[0].rgbBlue = 0;
			rgbPtrRGBQuad[0].rgbReserved = 255;

			rgbPtrRGBQuad[1].rgbRed = 255;
			rgbPtrRGBQuad[1].rgbGreen = 255;
			rgbPtrRGBQuad[1].rgbBlue = 255;
			rgbPtrRGBQuad[1].rgbReserved = 255;

			if( rgbQuadNumber!=fwrite(rgbPtrRGBQuad,sizeof(RGBQUAD),rgbQuadNumber,filePtrImage) )
			{
				free(rgbPtrRGBQuad);
				m_inError = IMAGE_WRITE_FAIL;
				return false;
			}
			m_ptrColorMap = rgbPtrRGBQuad;
			return true;

		}
		else
		{
			//����˲����ṹ��������ɫ����
			if( !colorQuant.buildOctree(m_ptrImageData,m_ptrInfoHeader->biWidth*m_ptrInfoHeader->biHeight,rgbQuadNumber) )
				return false;
			BYTE *ptrPal , *tmpPtrPal;
			int leafCount = colorQuant.getLeafNodeCount(), i;
			ptrPal = (BYTE *)malloc(leafCount*3);
			if( !ptrPal )
			{
				m_inError = IMAGE_NO_MEMORY;
				return false;
			}
			rgbPtrRGBQuad = (RGBQUAD *)malloc(sizeof(RGBQUAD)*leafCount);
			if( !rgbPtrRGBQuad )	
			{
				m_inError = IMAGE_NO_MEMORY;
				free(ptrPal);
				return false;
			}
			memset(rgbPtrRGBQuad,0,sizeof(RGBQUAD)*leafCount);
			colorQuant.getColorPallette(ptrPal);
			tmpPtrPal = ptrPal;
			for( i=0 ; i<leafCount ; i++ )
			{
				(rgbPtrRGBQuad + i)->rgbBlue	= *(tmpPtrPal + 2);
				(rgbPtrRGBQuad + i)->rgbGreen	= *(tmpPtrPal + 1);
				(rgbPtrRGBQuad + i)->rgbRed		= *tmpPtrPal;
				(rgbPtrRGBQuad + i)->rgbReserved= 255;
				tmpPtrPal += 3;
			}

			if( leafCount != fwrite(rgbPtrRGBQuad,sizeof(RGBQUAD),leafCount,filePtrImage) )
			{
				free(rgbPtrRGBQuad);
				free(ptrPal);
				return false;
			}
			free(ptrPal);
			m_ptrColorMap = rgbPtrRGBQuad;
			m_ptrInfoHeader->biClrUsed = leafCount;
		}
	}
	return true;
}

bool SrImageBmp::writeImageData(FILE* filePtrImage,SrColorQuant& colorQuant)
{

	if( m_ptrInfoHeader->biBitCount<=8 )
	{
		if( m_ptrInfoHeader->biBitCount==1 )
		{
			//д��ֵͼ������
			if( !writeBinary(filePtrImage) )
				return false;
			return true;
		}
		else
		{
			//���ʹ�õ���ɫ���ҷǶ�ֵͼ���ͼ�����ݵ�д�����
			if( !writeColorMapImage(filePtrImage,colorQuant) )
			{
				return false;
			}
		}

	}
	else
	{
		//д�벻ʹ����ɫ�������
		if(!writeNoColorMapImage(filePtrImage) )
			return false;
	}

	return true;
}



bool SrImageBmp::initHeader(long inWidth,long inHeight,unsigned short usBitCount )
{
	BITMAPFILEHEADER* ptrFileHeader = (BITMAPFILEHEADER*)malloc(sizeof(BITMAPFILEHEADER));
	if( !ptrFileHeader )
	{
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}
	BITMAPINFOHEADER* ptrInfoHeader = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER));
	if( !ptrInfoHeader )
	{
		free(ptrFileHeader);
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}

	memset(ptrFileHeader,0,sizeof(BITMAPFILEHEADER));
	memset(ptrInfoHeader,0,sizeof(BITMAPINFOHEADER));

	LONG imageSize = getLineBytes(inWidth,usBitCount) * inHeight;
	ptrFileHeader->bfType =('M'<<8) + 'B';

	ptrInfoHeader->biBitCount = usBitCount;
	ptrInfoHeader->biHeight = inHeight;
	ptrInfoHeader->biWidth = inWidth;
	ptrInfoHeader->biPlanes = 1;
	ptrInfoHeader->biSize = 40;
	ptrInfoHeader->biCompression = BI_RGB;


	m_ptrFileHeader = ptrFileHeader;
	m_ptrInfoHeader = ptrInfoHeader;
	return true;

}

bool SrImageBmp::writeFile(const char* chPtrImageFile)
{
	if( m_inIsReadOnly!=IMAGE_WRITE_ONLY )
	{
		m_inError = IMAGE_OBJECT_READ_ONLY;
		return false;
	}
	//���û�п�д�����ݣ�����false
	if( !m_ptrImageData )	
	{
		m_inError = IMAGE_UNKNOWN;
		return false;
	}
	FILE *fp;
	fp = fopen(chPtrImageFile,"wb");
	if( !fp ) 
	{
		m_inError = IMAGE_OPEN_FAIL;
		return false;
	}
	//�ճ��ļ�ͷ���ļ���Ϣͷ��λ�ã������ݷ���������
	int fileHeader = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	if( fseek(fp,fileHeader,SEEK_SET)==-1 )
	{
		m_inError = IMAGE_SEEK_FAIL;
		return false;
	}
	//����ɫ�������е�����д���ļ��У����biBitCount��4����8ʱ�����ʼ����ɫ������SrColorQuant
	SrColorQuant colorQuant;
	if( !writeColorMap(fp,colorQuant) )
	{
		fclose(fp);
		empty();
		return false;
	}
	m_ptrFileHeader->bfOffBits = ftell(fp);

#ifdef _CONSOLE
	printFileHeader(this->m_ptrFileHeader);

	printFileHeader(m_ptrInfoHeader);
#endif
	//дͼ������
	if( !writeImageData(fp,colorQuant) )
	{
		fclose(fp);
		empty();
		return false;
	}
	m_ptrFileHeader->bfSize = ftell(fp);
	m_ptrInfoHeader->biSizeImage = m_ptrFileHeader->bfSize - m_ptrFileHeader->bfOffBits;
	//�ƶ��ļ�ָ������ͷ��д�ļ�ͷ���ļ���Ϣͷ
	if( fseek(fp,0,SEEK_SET)==-1 )
	{
		m_inError = IMAGE_SEEK_FAIL;
		empty();
		return false;
	}
	if( !writeHeader(fp) )
	{
		fclose(fp);
		empty();
		return false;
	}
	fclose(fp);
	return true;
}

bool SrImageBmp::writeBinary(FILE* filePtrImage)
{
	//Gray= 0.072169B+ 0.715160G+ 0.212671R
	//�ҶȻ�
	BYTE* ptrGrayImage = (BYTE*)malloc(m_ptrInfoHeader->biWidth * m_ptrInfoHeader->biHeight);
	if( !ptrGrayImage ) 
	{
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}
	int grayIndex = 0 , i=0 , j=0;
	BYTE* btRgbData = m_ptrImageData;

	for( i=0 ; i<m_ptrInfoHeader->biHeight ; i++ )
		for( j=0 ; j<m_ptrInfoHeader->biWidth ; j++ )
		{
			*(ptrGrayImage + grayIndex) = (BYTE)(0.212671 * (float)*(btRgbData+2) + 
				0.715160 * (float)*(btRgbData+1) + 
				0.072169 * (float)* btRgbData);
			grayIndex ++;
			btRgbData +=3;

		}
		//Kittler�㷨
		BYTE* ptrLineAdd,*ptrNextLine,*ptrPreLine;
		int grads = 0, sumGrads = 0 ,xGrads , yGrads;
		int threshold , sumGrayGrads = 0;
		for ( i=1 ; i < m_ptrInfoHeader->biHeight-1 ; i++ )              
		{
			ptrLineAdd = ptrGrayImage+i*m_ptrInfoHeader->biWidth;
			ptrNextLine = ptrGrayImage+(i+1)*m_ptrInfoHeader->biWidth;
			ptrPreLine = ptrGrayImage+(i-1)*m_ptrInfoHeader->biWidth;
			for( j=1 ; j<m_ptrInfoHeader->biWidth-1 ; j++ )
			{
				//��ˮƽ��ֱ���������ݶ�
				xGrads = abs(ptrLineAdd[j-1]-ptrLineAdd[j+1]);
				yGrads = abs(ptrPreLine[j]-ptrNextLine[j]);
				grads= xGrads>yGrads? xGrads:yGrads;
				sumGrads += grads;

				//�ݶ��뵱ǰ��ҶȵĻ�
				sumGrayGrads += grads*ptrLineAdd[j];   
			}
		}
		threshold=sumGrayGrads/sumGrads;

		int dwLineBytes = getLineBytes(m_ptrInfoHeader->biWidth,1);
		BYTE *ptrLineImage = (BYTE*)malloc(dwLineBytes);
		if( !ptrLineImage )
		{
			free(ptrGrayImage);
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		BYTE tmpLineData = 0;
		BYTE* tmpImageData = ptrGrayImage;
		int lineIndex = 0;

		for( i=0 ; i < m_ptrInfoHeader->biHeight ; i++ )
		{ 
			lineIndex = tmpLineData = 0;
			for( j=0 ; j<m_ptrInfoHeader->biWidth ; j++ )
			{
				tmpLineData |= *tmpImageData>threshold?1:0;
				if( j%8==0 )
				{
					*(ptrLineImage + lineIndex) = tmpLineData;
					tmpLineData = 0;
					lineIndex ++;
				}
				tmpLineData = tmpLineData<<1;
				tmpImageData ++;
			}
			if( dwLineBytes!=fwrite(ptrLineImage,1,dwLineBytes,filePtrImage))
			{
				free(ptrGrayImage);
				free(ptrLineImage);
				m_inError = IMAGE_WRITE_FAIL;
				return false;
			}
		}
		free(ptrGrayImage);
		free(ptrLineImage);
		return true;
}

bool	SrImageBmp::loadImageData(unsigned char* ucPtrRgbData ,long inWidth,long inHeight,unsigned short usBitCount)
{
	if( m_inIsReadOnly!=IMAGE_WRITE_ONLY )
	{
		m_inError = IMAGE_OBJECT_READ_ONLY;
		return false;
	}
	if( !ucPtrRgbData )	
	{
		m_inError = IMAGE_UNKNOWN;
		return false;
	}
	//��մ洢�ڶ����е�ԭBMPͼ������
	empty();
	//��ʼ���ļ�ͷ���ļ���Ϣͷ���������л��в��ֱ���û����ȫ��ʼ����
	if( !initHeader(inWidth,inHeight,usBitCount) )
		return false;

	//�ж�Ҫд��BMP�ļ��������Ƿ��Ǳ�ʵ����֧�ֵĸ�ʽ
	if( !checkWriteFileFormat() )
	{
		empty();
		return false;
	}

#ifdef _CONSOLE
	printFileHeader(this->m_ptrFileHeader);

	printFileHeader(m_ptrInfoHeader);
#endif
	//��Ҫд��BMP�ļ������ݸ��ƽ�BMP������
	int byteCount = inWidth*inHeight*3 , i;
	BYTE* ptrImageData = (BYTE*)malloc(byteCount);
	if( !ptrImageData )
	{
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}
	for( i=0 ; i<byteCount ; i++ )
	{
		ptrImageData[i] = ucPtrRgbData[i];
		ptrImageData[i] = ucPtrRgbData[i];
		ptrImageData[i] = ucPtrRgbData[i];
	}
	m_ptrImageData = ptrImageData;

	return true;
}

bool	SrImageBmp::checkWriteFileFormat()
{
	//biPlanes����Ϊ1��biWidth��biHeight����ΪС�ڵ���0��ֵ��
	//biSize������BITMAPINFOHEADER�ṹ��Ĵ�С��bfType������"BM"
	if( m_ptrInfoHeader->biPlanes!=1 ||
		m_ptrInfoHeader->biWidth<=0 ||
		m_ptrInfoHeader->biHeight<=0 ||
		m_ptrInfoHeader->biSize!=sizeof(BITMAPINFOHEADER) ||
		m_ptrFileHeader->bfType!=(('M'<<8)+'B'))
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	//�ж�biBitCount�Ƿ����ڼ���{1,4,8,16,24}�е�ֵ
	if( !(m_ptrInfoHeader->biBitCount==1) &&
		!(m_ptrInfoHeader->biBitCount==4) &&
		!(m_ptrInfoHeader->biBitCount==8) &&
		!(m_ptrInfoHeader->biBitCount==16) &&
		!(m_ptrInfoHeader->biBitCount==24))
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	//�ж��Ƿ���֧�ֵ��ļ�����
	if( !(m_ptrInfoHeader->biCompression==BI_RGB))
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}

	return true;
}




#ifdef _DEBUG
#ifdef _CONSOLE
void SrImageBmp::printFileHeader(BITMAPFILEHEADER *bmfh)
{
	printf("The contents in the file header of the BMP file:\n");
	printf("bfOffBits: %ld\n",bmfh->bfOffBits);
	printf("bfReserved1: %ld\n",bmfh->bfReserved1);
	printf("bfReserved2: %ld\n",bmfh->bfReserved2);
	printf("bfSize: %ld\n",bmfh->bfSize);
	printf("bfType: %ld\n",bmfh->bfType);
}

void SrImageBmp::printFileHeader(BITMAPINFOHEADER *bmih)
{
	printf("The content in the info header of the BMP file:\n");
	printf("biBitCount: %ld\n",bmih->biBitCount);
	printf("biClrImportant: %ld\n",bmih->biClrImportant);
	printf("biClrUsed: %ld\n",bmih->biClrUsed);
	printf("biCompression: %ld\n",bmih->biCompression);
	printf("biHeight: %ld\n",bmih->biHeight);
	printf("biPlanes: %ld\n",bmih->biPlanes);
	printf("biSize: %ld\n",bmih->biSize);
	printf("biSizeImage: %ld\n",bmih->biSizeImage);
	printf("biWidth: %ld\n",bmih->biWidth);
	printf("biXPelsPerMeter: %ld\n",bmih->biXPelsPerMeter);
	printf("biYPelsPerMeter: %ld\n",bmih->biYPelsPerMeter);
}
#endif
#endif


