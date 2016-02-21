/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/
#ifndef SR_IMAGES_COLOR_QUANT_H_
#define SR_IMAGES_COLOR_QUANT_H_
/** \addtogroup images
  @{
*/

#include <malloc.h>
#include <algorithm>

/*	
/brief ��ɫ������

ͨ���˲����㷨��ʵ����ɫ����������һ���������ɫ������MaxColors�����������ɫ��һ��ҪС�����ֵ
���Դ����������ɫ�壬ʹ��ԭʼ��ɫ���Կ��ٵļ�������Ӧ��ɫ�������ֵ
*/

class SrColorQuant
{
protected:

#pragma pack(push)
#pragma pack(1)
	typedef struct  _OctreeNode
	{
		bool			blIsLeaf;							// TRUE if node has no children
		unsigned char	inColorIndex;						// Index of the pallette
		unsigned int	uiPixelCount;						// Number of pixels represented by this leaf
		unsigned int	uiRedSum;							// Sum of red components
		unsigned int	uiGreenSum;							// Sum of green components
		unsigned int	uiBlueSum;							// Sum of blue components
		_OctreeNode*	ptrChild[8];						// Pointers to child nodes
		_OctreeNode*	ptrNext;							// Pointer to next reducible node
	}OctreeNode;
#pragma  pack(pop)

public:
	SrColorQuant( );
	~SrColorQuant();
	/*
	\brief �����˲���
	\param[in] btPtrRgbData	��Ҫ��������ɫ����,ÿ����ɫռ�����ֽڣ�����Ϊ�졢�̡�����ÿ����ɫ������1���ֽڱ��档
	\param[in] inPixelCount ��Ҫ��������ɫ����
	\param[in] nMaxColors �涨����������������ɫ��������ֵ����[1,256]��Χ�ڡ�
	\return ���ð˲�����������ɫ���������������0����˲�������ʧ��
	*/
	int		buildOctree( unsigned char* btPtrRgbData,int inPixelCount,int nMaxColors);
	/*
	\brief ������ɫRGB,���Ѵ������İ˲����л�������ɫ������
	\param[in] (byRed,byGreen,byBlue) RGB��ɫֵ
	*/
	unsigned char  indexOctree(unsigned char byRed,unsigned char byGreen,unsigned char byBlue)const;
	/*
	\brief Ҷ�ڵ��������ֵ
	*/
	int		getMaxPixelCount() const;
	/*
	\brief  Ҷ�ڵ�������������������ɫ����
	*/
	int		getLeafNodeCount()const;
	/*
	\brief ����˲���Ϊ�գ�����true
	*/
	bool	isEmpty()const;
	/*
	\brief ���������ĵ�ɫ�壬ÿ����ɫռ�����ֽڣ�����Ϊ�졢�̡�����ÿ����ɫ������1���ֽڱ��档
	\param[in] ptrColorPal ����Ϊ�գ��ڴ���Ҫ��ǰ���䣬������getLeafNodeCount()*3��С���ڴ�ռ䡣
	*/
	void	getColorPallette(unsigned char* ptrColorPal )const;

protected:
	void		getColorPallette(OctreeNode*ptrTree, unsigned char* ptrColorPal)const;
	OctreeNode* createNode (int inLevel);
	/*
	\brief	�ϲ��˲����ڵ㣬����Ҷ�ڵ���
	*/
	void		reduceTree ();
	/*
	\brief �ݹ�����˲����в�����ɫֵ
	\param[in] inLevel��ʾ��ǰ����Ĳ�������ʼֵ��0��ÿ�ݹ�һ�㣬ֵ��1 
	*/
	bool		addColor(OctreeNode*& ptrTreeNode,unsigned char byRed,unsigned char byGreen,unsigned char byBlue,int inLevel);
	/*
	\brief ����ÿ��Ҷ�ڵ����ɫֵͬʱ����ɫ����ֵ����Ϊ1.
	*/
	void		setColorIndex(OctreeNode* ptrTree,int& inIndex);
	/*
	\brief	�����˲����ڴ�
	*/
	void		freeOctree(OctreeNode*& ocPtrTree);
	/*
	\brief ��հ˲���
	*/
	void		empty();


	unsigned int	m_inMaxPixelCount;
	int				m_inLeafCount;	
	OctreeNode*		m_ptrReducibleNodes[9];
	OctreeNode*		m_ptrTree;
};


/** @} */
#endif