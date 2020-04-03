#include "stdafx.h"
#include "resource.h"
#include "NCVCaddin.h"
#include "ReadJW.h"

#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern AFX_EXTENSION_MODULE ReadJWDLL;

/////////////////////////////////////////////////////////////////////////////
//	JWC�p �\���̒�`�D!!!1byte�ײ���!!!

#pragma	pack(1)
// ͯ�ް���(200byte x 4)
typedef	struct	tagJWCHEAD {
	// ͯ�ް#1
	char	szJWID1[20];	// ���ʗp
	char	szJWID2[20];
	char	szDummy1[160];
	// ͯ�ް#2
	char	szDataCnt[200];	// ���C�~�C������C���_�C���_���ް�����CSV�`���Ŋi�[
	// ͯ�ް#3
	char	szDummy2[200];
	// ͯ�ް#4
	char	szTextAddress[200];	// �������ޯ̧�̱��ڽ
} JWCHEAD;

// �ް���(�ǂݔ�΂�)���
typedef	struct	tagJWCDATA {
	char	szDummy1[909];	// ���_���(404+404+101)
	char	szDummy2[88];	// �������(22+22+22+22)
} JWCDATA;	// ��`�̂�

// �ް���(ڲ�)���
typedef	struct	tagJWCLAYERINFO1 {
	float	dScale[JW_LAYERGRPOUP];		// ����(����4byte)
	char	szDummy[560];		// ڲԑ����(16+256+16+256+16)
} JWCLAYERINFO1;
typedef	struct	tagJWCLAYERINFO2 {
	short	nScale[JW_LAYERGRPOUP];		// ����(����2byte)
	char	szDummy[560];
} JWCLAYERINFO2;

// �ް���
typedef	struct	tagJWCLINE {
	CPointF	pts, pte;			// �n�_�I�_
	unsigned char	sType, sColor, sLayer;	// ������F�Cڲ�
	char	szDummy[3];
} JWCLINE;
typedef	struct	tagJWCCIRCLE {
	CPointF	ptc;				// ���SXY
	float	r;					// ���a
	short	flat;				// �G����x10000
	long	sq, eq, lq;			// �J�n�I���p�xx65536(�x), �X��x65536
	unsigned char	sType, sColor, sLayer;	// ������F�Cڲ�
	char	szDummy[3];
} JWCCIRCLE;
typedef	struct	tagJWCTEXT {
	CPointF	pts, pte;			// �n�_�I�_
	LPVOID	lpText;				// ������ւ̱��ڽ
	char	szDummy1;
	unsigned char	sLayer;		// ڲ�
	char	szDummy2[2];
} JWCTEXT;
typedef	struct	tagJWCPOINT {
	CPointF	pt;					// �_���W
	unsigned char	sLayer;		// ڲ�
	char	szDummy[3];
} JWCPOINT;
#pragma	pack()

/////////////////////////////////////////////////////////////////////////////
//	JWC�p ��͊֐���������

// ͯ�ް���
static	BOOL	CheckHeader(CFile&, BOOL&, int[], float&, int&);
// �ް���͂P
static	BOOL	CheckDataIntro(CFile&, BOOL, JWLAYER[]);
// ڲԖ��擾
static	BOOL	GetLayerName(CFile&, JWLAYER[]);
// �e���ް�����
static	BOOL	GetJwcLine(NCVCHANDLE, CFile&, int[], float, const JWLAYER[]);
static	BOOL	GetJwcCircle(NCVCHANDLE, CFile&, int[], float, const JWLAYER[]);
static	BOOL	GetJwcText(NCVCHANDLE, CFile&, int[], float, const JWLAYER[], const char*);
static	BOOL	GetJwcPoint(NCVCHANDLE, CFile&, int[], float, const JWLAYER[]);

/////////////////////////////////////////////////////////////////////////////
//	JWC �ǂݍ���
//	NCVC_CreateDXFDocument() �̑�Q�����Ŏw��D
//	NCVC�{�̂��玩���I�ɌĂ΂��

NCADDIN BOOL Read_JWC(NCVCHANDLE hDoc, LPCTSTR pszFile)
{
	// ؿ����DLL����Ăяo��
	CNCVCExtAddin_ManageState	managestate(ReadJWDLL.hModule);

	BOOL		bScale;		// ���ق�����(TRUE)����(FALSE)��
	int			nData[JW_MAXDATA],	// ���C�~�C������C���_�C���_�̐�
				nText;				// ������̈�̒���
	char*		pszText = NULL;		// �������
	float		dUnit;				// �P���Ư�
	JWLAYER		jwl[JW_LAYERGRPOUP*JW_LAYERGRPOUP];	// ڲԖ�
	CString		strMsg;

	/*	--- #pragma	pack(1) ���K�v�Ȃ��Ƃ��킩����
#ifdef _DEBUG
	printf("sizeof(JWCLINE)=%d\n", sizeof(JWCLINE));
	printf("sizeof(JWCCIRCLE)=%d\n", sizeof(JWCCIRCLE));
	printf("sizeof(JWCTEXT)=%d\n", sizeof(JWCTEXT));
	printf("sizeof(JWCPOINT)=%d\n", sizeof(JWCPOINT));
#endif
*/
	try {
		// ̧�ٵ����
		CFile	fp(pszFile, CFile::modeRead | CFile::shareDenyWrite);
		// ͯ�ް����
		if ( !CheckHeader(fp, bScale, nData, dUnit, nText) )
			return FALSE;
#ifdef _DEBUG
		printf("Read_JWC() bScale=%d nText=%d\n", bScale, nText);
		printf("           Cnt l=%d c=%d t=%d p=%d\n",
			nData[JW_LINE], nData[JW_CIRCLE], nData[JW_TEXT], nData[JW_POINT]);
#endif
		// �ް�������
		if ( !CheckDataIntro(fp, bScale, jwl) )
			return FALSE;
		// �{�ް�...�̑O��÷�ď���ڲԖ����擾(�ް��̓ǂݔ�΂�)
		fp.Seek(
			sizeof(JWCLINE)   * nData[JW_LINE] +
			sizeof(JWCCIRCLE) * nData[JW_CIRCLE] +
			sizeof(JWCTEXT)   * nData[JW_TEXT],
			CFile::current);
		// ��������
		if ( nText > 0 ) {
			pszText = new char[nText];
			if ( fp.Read(pszText, nText) != (UINT)nText ) {
				delete	pszText;
				strMsg.Format(IDS_ERR_JW, pszFile);
				AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
				return FALSE;
			}
		}
		fp.Seek(sizeof(JWCPOINT) * nData[JW_POINT], CFile::current);
		// ڲԏ��
		if ( !GetLayerName(fp, jwl) ) {
			if ( pszText ) delete pszText;
			return FALSE;
		}
		fp.Seek(	// �{�ް��̈ʒu�܂Ŗ߂�
			sizeof(JWCHEAD) + sizeof(JWCDATA) +
			(bScale ? sizeof(JWCLAYERINFO1) : sizeof(JWCLAYERINFO2)),
			CFile::begin);
		// Ҳ��ڰт���۸�ڽ�ް������
		NCVC_MainfrmProgressRange(0,
			nData[JW_LINE]+nData[JW_CIRCLE]+nData[JW_TEXT]+nData[JW_POINT]-1);
		// �{�ް��擾����
		if ( !GetJwcLine(hDoc, fp, nData, dUnit, jwl) ) {
			if ( pszText ) delete pszText;
			return FALSE;
		}
		if ( !GetJwcCircle(hDoc, fp, nData, dUnit, jwl) ) {
			if ( pszText ) delete pszText;
			return FALSE;
		}
		if ( !GetJwcText(hDoc, fp, nData, dUnit, jwl, pszText) ) {
			if ( pszText ) delete pszText;
			return FALSE;
		}
		if ( pszText ) delete pszText;
		fp.Seek(nText, CFile::current);	// �������ޯ̧���ǂݔ�΂�
		if ( !GetJwcPoint(hDoc, fp, nData, dUnit, jwl) )
			return FALSE;
	}
	catch ( CMemoryException* e ) {
		if ( pszText ) delete pszText;
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		e->Delete();
		return FALSE;
	}
	catch ( CFileException* e ) {
		if ( pszText ) delete pszText;
		strMsg.Format(IDS_ERR_FILE, pszFile);
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		e->Delete();
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	��͊֐�

BOOL CheckHeader(CFile& fp, BOOL& bScale, int nData[], float& dUnit, int& nTextLength)
{
	static	const	char	szJWID[] = "jw_cad(c)data.......";
	static	const	char	szDelimiter[] = ",";
	static	const	int		nPaperSize[] = {	// A0, A1, A2, A3, A4
		1189, 841, 594, 420, 297
	};
	int			i;
	char*		pszContext;
	char*		pszCnt;				// �ް���
	char*		pszTextAddress[2];	// �J�n�I�����ڽ
	int			nTextAddress[2],	// ���ڽ�l
				nPaper = 3,			// �p��(A3)
				nWindow = 518;		// ��}����޳(640�ޯĕ\��)
	JWCHEAD		jwh;
	CString		strMsg;

	// ͯ�ް���ǂݍ�������
	if ( fp.Read(&jwh, sizeof(JWCHEAD)) != sizeof(JWCHEAD) ||
			memcmp(szJWID, jwh.szJWID1, sizeof(jwh.szJWID1)) != 0 ) {
		strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	// ����(�������ۂ�)
	bScale = jwh.szJWID2[2] == 'f' ? TRUE : FALSE;
	// �ް����̎擾
	for ( i=0; i<JW_MAXDATA; nData[i++] = 0 );
	i = 0;
	pszCnt = strtok_s(jwh.szDataCnt, szDelimiter, &pszContext);
	while ( pszCnt && i<JW_MAXDATA ) {
		nData[i++] = atoi(pszCnt);
		pszCnt = strtok_s(NULL, szDelimiter, &pszContext);
	}
	// �p�����ށC��}����޳�̕����擾
	while ( pszCnt && i<JW_MAXDATA+7) {
		pszCnt = strtok_s(NULL, szDelimiter, &pszContext);
		i++;
	}
	if ( pszCnt )
		nPaper = atoi(pszCnt);
	while ( pszCnt && i<JW_MAXDATA+26) {
		pszCnt = strtok_s(NULL, szDelimiter, &pszContext);
		i++;
	}
	if ( pszCnt )
		nWindow = atoi(pszCnt);
	// �ƯČv�Z
	if ( nPaper<0 || nPaper>=SIZEOF(nPaperSize) ) {
		AfxMessageBox(IDS_ERR_JWPAPER, MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	dUnit = (float)nPaperSize[nPaper] / nWindow;
	// ������̈�̒����v�Z
	pszTextAddress[0] = strtok_s(jwh.szTextAddress, szDelimiter, &pszContext);
	pszTextAddress[1] = strtok_s(NULL, szDelimiter, &pszContext);
	for ( i=0; i<2; i++ ) {
		sscanf_s(strtok_s(pszTextAddress[i], ":", &pszContext), "%4x", &nTextAddress[0]);
		sscanf_s(strtok_s(NULL, ":", &pszContext), "%4x", &nTextAddress[1]);
		pszTextAddress[i] = (char *)(nTextAddress[0] * 16 + nTextAddress[1]);
	}
	nTextLength = pszTextAddress[1] - pszTextAddress[0];

	return TRUE;
}

BOOL CheckDataIntro(CFile& fp, BOOL bScale, JWLAYER jwl[])
{
	int			i, j;
	JWCLAYERINFO1	jwl1;
	JWCLAYERINFO2	jwl2;
	CString		strMsg;

	// �ް���(����)�̓ǂݔ�΂�
	fp.Seek(sizeof(JWCDATA), CFile::current);
	// �������ق��ۂ��œǂݍ��ލ\���̂�ύX
	if ( bScale ) {
		if ( fp.Read(&jwl1, sizeof(JWCLAYERINFO1)) != sizeof(JWCLAYERINFO1) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		for ( i=0; i<JW_LAYERGRPOUP; i++ ) {
			for ( j=0; j<JW_LAYERGRPOUP; j++ ) {
				jwl[i*JW_LAYERGRPOUP+j].dScale = jwl1.dScale[i];
			}
		}
	}
	else {
		if ( fp.Read(&jwl2, sizeof(JWCLAYERINFO2)) != sizeof(JWCLAYERINFO2) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		for ( i=0; i<JW_LAYERGRPOUP; i++ ) {
			for ( j=0; j<JW_LAYERGRPOUP; j++ ) {
				jwl[i*JW_LAYERGRPOUP+j].dScale = (double)jwl2.nScale[i];
			}
		}
	}

	return TRUE;
}

BOOL GetLayerName(CFile& fp, JWLAYER jwl[])
{
	char		szLayerArray[JW_LAYERGRPOUP*JW_LAYERGRPOUP][JW_LAYERLENGTH],
				szLayer[JW_LAYERLENGTH+1];
	CString		strMsg;

	if ( fp.Read(szLayerArray, sizeof(szLayerArray)) != sizeof(szLayerArray) ) {
		strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	// ڲԏ������ꂼ��ɕ���
	for ( int i=0; i<JW_LAYERGRPOUP*JW_LAYERGRPOUP; i++ ) {
		ZeroMemory(szLayer, sizeof(szLayer));
		memcpy(szLayer, szLayerArray[i], JW_LAYERLENGTH);
		jwl[i].strLayer = szLayer;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	�e���ް���͊֐�(���)

inline int CheckCircle(const JWCCIRCLE* jw)
{
//	return code 0:�^�~�C1:�~�ʁC2:�ȉ~�C3:�ȉ~��
	int	nResult;
	if ( jw->flat / 10000.0 == 1.0 )	// �G����
		nResult = ( jw->sq == jw->eq ) ? 0 : 1;
	else
		nResult = ( jw->sq == jw->eq ) ? 2 : 3;

	return nResult;
}

/////////////////////////////////////////////////////////////////////////////
//	�e���ް���͊֐�

BOOL GetJwcLine
	(NCVCHANDLE hDoc, CFile& fp, int nCnt[], float dUnit, const JWLAYER jwl[])
{
	BOOL		bResult;
	int			nLayer;
	float		dScale;
	JWCLINE		jw;
	DXFDATA		dxf;
	DPOINT		pts, pte;
	CString		strMsg;

	dxf.dwSize  = sizeof(DXFDATA);
	dxf.enType  = DXFLINEDATA;

	for ( int i=0; i<nCnt[JW_LINE]; i++ ) {
		if ( fp.Read(&jw, sizeof(JWCLINE)) != sizeof(JWCLINE) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		// ڲ�����
		nLayer = CheckDataLayer(jwl, jw.sLayer);
		if ( nLayer == DXFORGLAYER ) {
			dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
			pts = jw.pts * dScale;
			pte = jw.pte * dScale;
			NCVC_SetDXFLatheLine(hDoc, &pts, &pte);
		}
		if ( nLayer>=DXFCAMLAYER && nLayer<=DXFMOVLAYER ) {
			dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
			dxf.ptS    = jw.pts * dScale;
			dxf.de.ptE = jw.pte * dScale;
			dxf.nLayer = nLayer;
			lstrcpyn(dxf.szLayer, jwl[jw.sLayer].strLayer,
				min(sizeof(dxf.szLayer), jwl[jw.sLayer].strLayer.GetLength()+1));
			bResult = NCVC_AddDXFData(hDoc, &dxf);
			if ( !bResult ) {
				strMsg.Format(IDS_ERR_ADDNCVC, DXFLINEDATA, i);
				AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
				return FALSE;
			}
		}
		// ��۸�ڽ�ް�X�V(64�񂨂�)
		if ( (i & 0x003f) == 0 )
			NCVC_MainfrmProgressPos(i);
	}

	return TRUE;
}

BOOL GetJwcCircle
	(NCVCHANDLE hDoc, CFile& fp, int nCnt[], float dUnit, const JWLAYER jwl[])
{
	int			nPos;
	float		dScale;
	JWCCIRCLE	jw;
	DXFDATA		dxf;
	DPOINT		pt;
	double		lq;
	CString		strMsg;

	dxf.dwSize  = sizeof(DXFDATA);

	for ( int i=0; i<nCnt[JW_CIRCLE]; i++ ) {
		if ( fp.Read(&jw, sizeof(JWCCIRCLE)) != sizeof(JWCCIRCLE) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		// ڲ�����
		switch ( CheckDataLayer(jwl, jw.sLayer) ) {
		case DXFORGLAYER:
			// �^�~�̎��������_�o�^
			if ( CheckCircle(&jw) == 0 ) {
				dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
				pt = jw.ptc * dScale;
				NCVC_SetDXFCutterOrigin(hDoc, &pt, jw.r*dScale, FALSE);
			}
			break;
		case DXFCAMLAYER:
			dxf.nLayer = DXFCAMLAYER;
			lstrcpyn(dxf.szLayer, jwl[jw.sLayer].strLayer,
				min(sizeof(dxf.szLayer), jwl[jw.sLayer].strLayer.GetLength()+1));
			dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
			dxf.ptS = jw.ptc * dScale;
			switch ( CheckCircle(&jw) ) {
			case 0:		// �^�~
				dxf.enType = DXFCIRCLEDATA;
				dxf.de.dR = jw.r * dScale;
				break;
			case 1:		// �~��(�p�x�P�ʂ͓x)
				dxf.enType = DXFARCDATA;
				dxf.de.arc.r = jw.r * dScale;
				dxf.de.arc.sq = jw.sq / 65536.0;
				dxf.de.arc.eq = jw.eq / 65536.0;
				break;
			case 2:		// �ȉ~(�p�x�P�ʂ�׼ޱ�)
			case 3:		// �ȉ~��
				dxf.enType = DXFELLIPSEDATA;
				dxf.de.elli.sq = RAD(jw.sq / 65536.0);
				dxf.de.elli.eq = RAD(jw.eq / 65536.0);
				lq = RAD(jw.lq / 65536.0);
				dxf.de.elli.ptL.x = jw.r * cos(lq) * dScale;	// �����͌��_����̑��΍��W
				dxf.de.elli.ptL.y = jw.r * sin(lq) * dScale;
				dxf.de.elli.s = jw.flat / 10000.0;
				break;
			}
			if ( !NCVC_AddDXFData(hDoc, &dxf) ) {
				strMsg.Format(IDS_ERR_ADDNCVC, DXFCIRCLEDATA, i);
				AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
				return FALSE;
			}
			break;
		case DXFSTRLAYER:
			// �^�~�̎��������H�J�n�ʒu�o�^
			if ( CheckCircle(&jw) == 0 ) {
				dxf.nLayer = DXFSTRLAYER;
				lstrcpyn(dxf.szLayer, jwl[jw.sLayer].strLayer,
					min(sizeof(dxf.szLayer), jwl[jw.sLayer].strLayer.GetLength()+1));
				dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
				dxf.ptS   = jw.ptc * dScale;
				dxf.de.dR = jw.r * dScale;
				if ( !NCVC_AddDXFData(hDoc, &dxf) ) {
					strMsg.Format(IDS_ERR_ADDNCVC, DXFCIRCLEDATA, i);
					AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
					return FALSE;
				}
			}
			break;
		}
		// ��۸�ڽ�ް�X�V(64�񂨂�)
		nPos = i + nCnt[JW_LINE];
		if ( (nPos & 0x003f) == 0 )
			NCVC_MainfrmProgressPos(nPos);
	}

	return TRUE;
}

BOOL GetJwcText
	(NCVCHANDLE hDoc, CFile& fp, int nCnt[], float dUnit, const JWLAYER jwl[], const char* pszTextInfo)
{
	BOOL		bResult;
	int			nLayer, nPos, nBase = nCnt[JW_LINE] + nCnt[JW_CIRCLE];
	float		dScale;
	char*		pszText;
	JWCTEXT		jw;
	DXFDATA		dxf;
	CString		strMsg;

	dxf.dwSize  = sizeof(DXFDATA);
	dxf.enType  = DXFTEXTDATA;

	for ( int i=0; i<nCnt[JW_TEXT]; i++ ) {
		if ( fp.Read(&jw, sizeof(JWCTEXT)) != sizeof(JWCTEXT) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		// ڲ�����
		nLayer = CheckDataLayer(jwl, jw.sLayer);
		if ( nLayer>=DXFCAMLAYER && nLayer<=DXFCOMLAYER ) {
			dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
			dxf.ptS = jw.pts * dScale;
			dxf.nLayer = nLayer;
			pszText = (char *)jw.lpText - 0x40000000 + (unsigned int)pszTextInfo;
			lstrcpyn(dxf.szLayer, jwl[jw.sLayer].strLayer,
				min(sizeof(dxf.szLayer), jwl[jw.sLayer].strLayer.GetLength()+1));
			lstrcpyn(dxf.de.szText, pszText,
				min(sizeof(dxf.de.szText), lstrlen(pszText)+1));
			bResult = NCVC_AddDXFData(hDoc, &dxf);
			if ( !bResult ) {
				strMsg.Format(IDS_ERR_ADDNCVC, DXFTEXTDATA, i);
				AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
				return FALSE;
			}
		}
		// ��۸�ڽ�ް�X�V(64�񂨂�)
		nPos = i + nBase;
		if ( (nPos & 0x003f) == 0 )
			NCVC_MainfrmProgressPos(nPos);
	}

	return TRUE;
}

BOOL GetJwcPoint
	(NCVCHANDLE hDoc, CFile& fp, int nCnt[], float dUnit, const JWLAYER jwl[])
{
	int			nPos, nBase = nCnt[JW_LINE] + nCnt[JW_CIRCLE] + nCnt[JW_TEXT];
	float		dScale;
	JWCPOINT	jw;
	DXFDATA		dxf;
	CString		strMsg;

	dxf.dwSize  = sizeof(DXFDATA);
	dxf.enType  = DXFPOINTDATA;

	for ( int i=0; i<nCnt[JW_POINT]; i++ ) {
		if ( fp.Read(&jw, sizeof(JWCPOINT)) != sizeof(JWCPOINT) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		// ڲ�����
		if ( CheckDataLayer(jwl, jw.sLayer) == DXFCAMLAYER ) {
			lstrcpyn(dxf.szLayer, jwl[jw.sLayer].strLayer,
				min(sizeof(dxf.szLayer), jwl[jw.sLayer].strLayer.GetLength()+1));
			dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
			dxf.ptS = jw.pt * dScale;
			dxf.nLayer = DXFCAMLAYER;
			if ( !NCVC_AddDXFData(hDoc, &dxf) ) {
				strMsg.Format(IDS_ERR_ADDNCVC, DXFPOINTDATA, i);
				AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
				return FALSE;
			}
		}
		// ��۸�ڽ�ް�X�V(64�񂨂�)
		nPos = i + nBase;
		if ( (nPos & 0x003f) == 0 )
			NCVC_MainfrmProgressPos(nPos);
	}

	return TRUE;
}
