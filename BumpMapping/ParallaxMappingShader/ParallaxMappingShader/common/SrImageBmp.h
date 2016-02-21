/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/
#ifndef SR_IMAGES_IMAGE_BMP_H_
#define SR_IMAGES_IMAGE_BMP_H_
/** \addtogroup images
  @{
*/

#include "SrImage.h"
#include "SrColorQuant.h"

#ifndef BI_RGB
#define BI_RGB		0x00
#endif

#ifndef	BI_RLE8
#define BI_RLE8	0x01
#endif

#ifndef BI_RLE4
#define BI_RLE4	0x02
#endif

/*
\brief BMP�ļ��Ķ�д������

һ��BMP�����ֻ�ܽ���BMP�ļ��Ķ�ȡ����д�롣

��BMP�ļ�������BMP�ļ�������������������
		   ��1��֧�����ر�������1��4��8��16��24��32,ͼ��ѹ������ΪBI_RGB
		   ��2��ѹ������ΪBI_RLE4��BI_RLE8����Ӧ�����ر������ֱ���4��8
		   ��3��BMP�ļ��Ŀ�͸߱������0��biPlanes������1��������"BM"����
дBMP�ļ���֧��д�����ر�������1��4��8��16��24,ͼ��ѹ������ΪBI_RGB��BMP�ļ�
*/

class SrImageBmp: public SrImage
{

protected:

	//һЩBMP�ļ����ļ�ͷ���ļ���Ϣͷ��ʹ�д���Ҳ��Ӱ����ȷʹ��BMP�е�ͼ����Ϣ
	//�Ƿ����ļ�ͷ��λͼ���ݵ���ʼλ��
#define  SR_IMAGE_BMP_CHECK_OFFBITS 0
	//�Ƿ����ļ�ͷ���ļ���С
#define  SR_IMAGE_BMP_CHECK_FILESIZE 0

#pragma pack(push) 
#pragma pack(1)
	/*
		BMP�ļ����Ĳ�����ɣ�BMP�ļ�ͷ(14�ֽ�)��λͼ��Ϣͷ(40�ֽ�)����ɫ��λͼ���ݡ�
	*/

	/*
		BMP�ļ�ͷ(14�ֽ�)
		BMP�ļ�ͷ���ݽṹ����BMP�ļ������͡��ļ���С��λͼ��ʼλ�õ���Ϣ�� 
	*/
	typedef struct tagBITMAPFILEHEADER
	{
		WORD bfType;				// λͼ�ļ������ͣ�����ΪBMP(0-1�ֽ�)
		DWORD bfSize;				// λͼ�ļ��Ĵ�С�����ֽ�Ϊ��λ(2-5�ֽ�)
		WORD bfReserved1;			// λͼ�ļ������֣�����Ϊ0(6-7�ֽ�)
		WORD bfReserved2;			// λͼ�ļ������֣�����Ϊ0(8-9�ֽ�)
		DWORD bfOffBits;			// λͼ���ݵ���ʼλ�ã��������λͼ�ļ�ͷ��ƫ������ʾ�����ֽ�Ϊ��λ(10-13�ֽ�)
	} BITMAPFILEHEADER;

	/*
		λͼ��Ϣͷ(40�ֽ�)
		BMPλͼ��Ϣͷ��������˵��λͼ�ĳߴ����Ϣ��
	*/
	typedef struct tagBITMAPINFOHEADER
	{
		DWORD biSize;				// ���ṹ��ռ���ֽ���,ͨ��40���ֽ�(14-17�ֽ�)
		LONG biWidth;				// λͼ�Ŀ�ȣ�������Ϊ��λ(18-21�ֽ�)
		LONG biHeight;				// λͼ�ĸ߶ȣ�������Ϊ��λ(22-25�ֽ�)
		WORD biPlanes;				// Ŀ���豸�ļ��𣬱���Ϊ1(26-27�ֽ�)
		WORD biBitCount;			// ÿ�����������λ����������1(˫ɫ)��(28-29�ֽ�)��4(16ɫ)��8(256ɫ)��24(���ɫ)֮һ
		DWORD biCompression;		// λͼѹ�����ͣ������� 0(��ѹ��),(30-33�ֽ�)
									// 1(BI_RLE8ѹ������)��2(BI_RLE4ѹ������)֮һ
		DWORD biSizeImage;			// λͼ�Ĵ�С�����ֽ�Ϊ��λ(34-37�ֽ�)
		LONG biXPelsPerMeter;		// λͼˮƽ�ֱ��ʣ�ÿ��������(38-41�ֽ�)��������δʹ�ø���Ϣ
		LONG biYPelsPerMeter;		// λͼ��ֱ�ֱ��ʣ�ÿ��������(42-45�ֽ�)��������δʹ�ø���Ϣ
		DWORD biClrUsed;			// λͼʵ��ʹ�õ���ɫ�������е���ɫ��(46-49�ֽ�)
		DWORD biClrImportant;		// λͼ��ʾ��������Ҫ����ɫ��(50-53�ֽ�)��������δʹ�ø���Ϣ
	}BITMAPINFOHEADER;

	/*
		��ɫ�� 
		��ɫ������˵��λͼ�е���ɫ���������ɸ����ÿһ��������һ��RGBQUAD���͵Ľṹ������һ����ɫ��
	*/
	typedef struct tagRGBQUAD 
	{
		BYTE rgbBlue;			// ��ɫ������(ֵ��ΧΪ0-255)
		BYTE rgbGreen;			// ��ɫ������(ֵ��ΧΪ0-255)
		BYTE rgbRed;			// ��ɫ������(ֵ��ΧΪ0-255)
		BYTE rgbReserved;		// ����������Ϊ0
	}RGBQUAD;
	/*
		λͼ��Ϣͷ����ɫ�����λͼ��Ϣ
	*/
	typedef struct tagBITMAPINFO
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[1];
	}BITMAPINFO;
#pragma pack(pop)

public:
	SrImageBmp(int isReadOnly);
	virtual ~SrImageBmp();
	/*
	\brief	��BMP�ļ��ж�ȡ���ݣ����ҷ�������
	\param[in] chPtrImageFile ��ȡ��BMP�ļ���
	\param[out] ptrOutData ���inRGBType����IMAGE_RGB,�򷵻ص�RGB��ɫ���ݣ�ÿ����ɫռ�����ֽڣ�����Ϊ�졢�̡�����ÿ����ɫ������1���ֽڱ��棻
						   ���inRGBType����IMAGE_RGBA,�򷵻ص�RGBA��ɫ����,ÿ����ɫռ�ĸ��ֽڣ�����Ϊ�졢�̡�����Alpha��ÿ����ɫ������1���ֽڱ���;
						   ��������������ptrOutDatak�е��ڴ棬����BMP������й���
	\param[out] inRGBType ֻ����IMAGE_RGBA��IMAGE_RGB�е�һ��ֵ����ʾ���ص����ݸ�ʽ��RGBA����RGB
	\return true�����ļ��ɹ���false�����ļ�ʧ�ܣ�ͨ��getErrorId()��������ô�����š�
	*/
	bool	readFile(const char* chPtrImageFile,unsigned char*& ptrOutData ,int& inRGBType);
	/*
	\brief	��RGB��ɫ���ݱ����BMP�����У��Ա����д�ļ�������
	\param[in] ucPtrRgbData RGB��ɫ���ݣ�Ҫ�����BMP�����е����ݣ�����Ϊ�졢�̡�����ÿ����ɫ������1���ֽڱ���
	\param[in] inWidth ָ��д��BMP����Ŀ��������0
	\param[in] inHeight ָ��д��BMP����ĸߣ��������0
	\param[in] usBitCount ָ��BMP������ÿ����ɫռ��λ����BMP�ļ�ֻ֧��1,4,8,16,24,32��6����ֵ
	\param[in] ulCompression ����ֻ֧��BI_RGB���͵�д���ļ�����
	\return true��װ���ļ��ɹ���false��װ���ļ�ʧ�ܣ�ͨ��getErrorId()��������ô�����š�
	*/
	bool	loadImageData(unsigned char* ucPtrRgbData ,long inWidth,long inHeight,unsigned short usBitCount=8 );
	/*
	\brief	��������BMP�����е�����д��BMP�ļ���
	\param[in] chPtrImageFile д���BMP�ļ���
	\return true��д�ļ��ɹ���false��д�ļ�ʧ�ܣ�ͨ��getErrorId()��������ô�����š�
	*/
	bool	writeFile(const char* chPtrImageFile);

	virtual bool	isValid()const;
	virtual int		getWidth()const;
	virtual int		getHeight()const;
	/*
	\brief	����BMP�����е�RGB���ݻ���RGBA����
	*/
	virtual unsigned char* getImageData()const;

	unsigned long		getFileSize()const;
	/*
	\brief	�ж�BMP������BMP��ѹ�����ͣ�ֻ����BI_RGB����BI_RGBA
	*/
	unsigned long		getCompression()const;
	/*
	\brief	ÿ�����صı�����
	*/
	unsigned short		getPixelDepth()const;
	/*
	\brief	�ж�BMP�����д洢����RGB���ݻ���RGBA����
	*/
	bool				getIsRGB()const;


private:
	/*
	\brief	����BMP�����е���������
	*/
	void	empty();
	/*
	\brief	����BMP�ļ�һ�е��ֽ���
	*/
	long	getLineBytes(int imgWidth,int bitCount)const;
	/*
	\brief	�ж϶�ȡ��BMP�ļ���ʽ�Ƿ���ȷ
	*/
	bool	checkReadFileFormat(FILE* flFileHandle);
	bool	readUncompression(FILE* filePtrImage,unsigned char*& ptrOutData);
	bool	decodeRLE(FILE* flPtrImage,BYTE*& btPtrImageData);
	/*
	\brief	���ļ��ж�ȡBMP�ļ�ͷ���ļ���Ϣͷ
	*/
	bool	readHeader(FILE* flPtrImage);
	bool	readColorMap(FILE* flPtrImage);

	bool	decodeFile(BYTE* ptrImageData,unsigned char*& rgbPtrOutData);

	bool	initHeader(long inWidth,long inHeight,unsigned short usBitCount);
	bool	writeHeader(FILE* flPtrImage);
	bool	writeColorMap(FILE* filePtrImage,SrColorQuant& colorQuant);
	bool	writeImageData(FILE* flPtrFile,SrColorQuant& colorQuant);
	bool	writeBinary(FILE* filePtrImage);
	bool	writeColorMapImage(FILE* filePtrImage,const SrColorQuant& colorQuant);
	bool	writeNoColorMapImage(FILE* filePtrImage);
	bool	checkWriteFileFormat();

public:
#ifdef _DEBUG
#ifdef _CONSOLE
	void printFileHeader(BITMAPFILEHEADER *bmfh);

	void printFileHeader(BITMAPINFOHEADER *bmih);
#endif
#endif

private:

	BITMAPINFOHEADER*	m_ptrInfoHeader;
	BITMAPFILEHEADER*	m_ptrFileHeader;
	RGBQUAD*			m_ptrColorMap;		
	BYTE*				m_ptrImageData;

	int					m_inIsReadOnly;

};

/** @} */
#endif