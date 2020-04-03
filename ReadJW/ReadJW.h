/*
	�g��DLL�łͺ��߲ٴװ�ɂȂ� 
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
	�̑���
	����߂ɂ���Ƃ������ݽ�׸�������ؿ������ق�L���ɂ���
	�ʏ� AFX_EXTENSION_MODULE �� hModule ��n��
*/
#pragma once

/////////////////////////////////////////////////////////////////////////////
//	CNCVCExtAddin_ManageState

class	CNCVCExtAddin_ManageState
{
	HINSTANCE	m_hInstance;

public:
	CNCVCExtAddin_ManageState(HMODULE hModule) {
		m_hInstance = ::AfxGetResourceHandle();
		::AfxSetResourceHandle(hModule);
	}
	~CNCVCExtAddin_ManageState() {
		::AfxSetResourceHandle(m_hInstance);
	}
};

/////////////////////////////////////////////////////////////////////////////
//	define

#define	JW_LINE				0
#define	JW_CIRCLE			1
#define	JW_TEXT				2
#define	JW_POINT			3
#define	JW_MAXDATA			4
#define	JW_LAYERGRPOUP		16
#define	JW_LAYERLENGTH		8

typedef	struct	tagJWLAYER {
	CString	strLayer;		// ڲԖ�(MAX�W����)
	double	dScale;			// ڲ�(��ٰ��)���Ƃ̽���
} JWLAYER;

#include "PointTemplate.h"

/////////////////////////////////////////////////////////////////////////////
//	common function

// NCVC�̓ǂݍ���ڲ�����
int		CheckDataLayer(const JWLAYER jwl[], UINT nLayer);
