/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/

#ifndef SR_IMAGES_IMAGE_H_
#define SR_IMAGES_IMAGE_H_
/** \addtogroup images
  @{
*/

#include <stdio.h>
#include <malloc.h>
#include <algorithm>

/* error codes */
#define		EIMAGE_OK				0x0000 		/* success */
#define		IMAGE_NO_MEMORY			0x0001		/* out of memory */
#define		IMAGE_OPEN_FAIL			0x0002		/* file open fail */
#define		IMAGE_SEEK_FAIL			0x0004		/* file seek fail */
#define		IMAGE_READ_FAIL			0x0008		/* file read fail */
#define		IMAGE_WRITE_FAIL		0x0010		/* file write fail */
#define		IMAGE_UNKNOWN_FORMAT	0x0020		/* unknown fiel format */
#define		IMAGE_OVERFLOW			0x0040		/* data overflow */
#define		IMAGE_OBJECT_READ_ONLY	0x0080		/* object read only */
#define		IMAGE_OBJECT_WRITE_ONLY	0x0100		/* object write only */
#define		IMAGE_UNKNOWN			0x0200		/* unknown error */


#define		IMAGE_RGB				0x01		/* the data is stored in the RGB format */
#define		IMAGE_RGBA				0x02		/* the data is stored in the RGBA format */
#define		IMAGE_GRAYSCALE			0x03


#define		LSN(value)	((value) & 0x0f)			/* Least-significant nibble */
#define		MSN(value)	(((value) & 0xf0) >> 4)		/* Most-significant nibble  */

#define		IMAGE_READ_ONLY			0
#define		IMAGE_WRITE_ONLY		1

/*
\brief	ͼ������ļ��Ļ���

�ṩ��ͼ������ļ���Ļ���������һЩ�������͵Ķ��壬�������Ķ���
*/

class SrImage
{
protected:
	typedef unsigned short WORD;
	typedef unsigned long DWORD;
	typedef long LONG;
	typedef unsigned char BYTE;

public:
	SrImage()
	{
	}
	virtual ~SrImage()
	{

	}
	virtual bool	isValid()const
	{
		return false;
	}
	virtual int		getWidth()const
	{
		return 0;
	}
	virtual int		getHeight()const
	{
		return 0;
	}
	virtual unsigned char* getImageData()const
	{
		return NULL;
	}
	int		getErrorId()
	{
		return m_inError;
	}
	void	resetErrorId()
	{
		m_inError = EIMAGE_OK;
	}
protected:
	static int		m_inError;

};


/** @} */
#endif