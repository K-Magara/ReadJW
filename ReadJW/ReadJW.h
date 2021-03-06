/*
	拡張DLLではｺﾝﾊﾟｲﾙｴﾗｰになる 
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
	の代わり
	ｽｺｰﾌﾟにあるときだけｺﾝｽﾄﾗｸﾀ引数のﾘｿｰｽﾊﾝﾄﾞﾙを有効にする
	通常 AFX_EXTENSION_MODULE の hModule を渡す
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
	CString	strLayer;		// ﾚｲﾔ名(MAX８文字)
	double	dScale;			// ﾚｲﾔ(ｸﾞﾙｰﾌﾟ)ごとのｽｹｰﾙ
} JWLAYER;

#include "PointTemplate.h"

/////////////////////////////////////////////////////////////////////////////
//	common function

// NCVCの読み込みﾚｲﾔﾁｪｯｸ
int		CheckDataLayer(const JWLAYER jwl[], UINT nLayer);
