#include "stdafx.h"
#include "resource.h"
#include "NCVCaddin.h"
#include "ReadJW.h"
#include "JwwClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern AFX_EXTENSION_MODULE ReadJWDLL;

/////////////////////////////////////////////////////////////////////////////
//	JWC�p ��͊֐���������

// ͯ�ް���
static	void	ReadJWWinfo(CArchive&, JWLAYER[]);
// �ް��o�^
static	BOOL	JWWtoNCVCdata(NCVCHANDLE, const CJwwData&, const CJwwBlock&, const JWLAYER[]);

/////////////////////////////////////////////////////////////////////////////
//	JWW �ǂݍ���
//	NCVC_CreateDXFDocument() �̑�Q�����Ŏw��D
//	NCVC�{�̂��玩���I�ɌĂ΂��

NCADDIN BOOL Read_JWW(NCVCHANDLE hDoc, LPCTSTR pszFile)
{
	// ؿ����DLL����Ăяo��
	CNCVCExtAddin_ManageState	managestate(ReadJWDLL.hModule);

	POSITION	pos;
	BOOL		bResult = TRUE;
	JWLAYER		jwl[JW_LAYERGRPOUP*JW_LAYERGRPOUP];	// ڲԖ�
	CString		strMsg;
	CJwwData	lstJWW;
	CJwwBlock	lstBlk;
	CFile		fp;

	// ̧�ٵ����
	if ( !fp.Open(pszFile, CFile::modeRead | CFile::shareDenyWrite) ) {
		strMsg.Format(IDS_ERR_FILE, pszFile);
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	try {
		// ��۰���ނ̐ݒ�
//		lstJWW.SetSize(0, 1024);
//		lstBlk.SetSize(0, 1024);
		// �����ލ\�z
		CArchive	ar(&fp, CArchive::load);
		// ͯ�ް�ǂݍ���
		ReadJWWinfo(ar, jwl);
		// �ް��ǂݍ���
		lstJWW.Serialize(ar);	// ����P�s�œǂݍ��݂Ƹ׽��޼ު�Ă̐���
		// ��ۯ���`�̓ǂݍ���
		lstBlk.Serialize(ar);	// CDataList���

		// -- �摜�ް����͖���

		// NCVC���ް��o�^
		bResult = JWWtoNCVCdata(hDoc, lstJWW, lstBlk, jwl);
	}
	catch ( CUserException* e ) {	// from CJwwHead::Serialize()
		e->Delete();
		return FALSE;
	}
	catch ( CMemoryException* e ) {
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		e->Delete();
		bResult = FALSE;
	}
	catch ( CArchiveException* e ) {
		strMsg.Format(IDS_ERR_FILE, pszFile);
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		e->Delete();
		bResult = FALSE;
	}
	catch ( CFileException* e ) {
		strMsg.Format(IDS_ERR_FILE, pszFile);
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		e->Delete();
		bResult = FALSE;
	}

	// CObArray���ނ̍폜
	for ( pos=lstJWW.GetHeadPosition(); pos; )
		delete	lstJWW.GetNext(pos);
	for ( pos=lstBlk.GetHeadPosition(); pos; )
		delete	lstBlk.GetNext(pos);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	��͊֐�

void ReadJWWinfo(CArchive& ar, JWLAYER jwl[])
{
	extern	DWORD	g_dwVersion;
	CJwwHead		jwh;
	CJwwLayerInfo	jwi[JW_LAYERGRPOUP];
	int				i, j;
	DWORD			dw;		// ��а�ǂݍ���
	double			dd;
	CString			ss;

	// ͯ�ް���ǂݍ���
	jwh.Serialize(ar);
	// ���فCڲԖ����̓ǂݍ���
	ar >> dw;
	for ( i=0; i<JW_LAYERGRPOUP; i++ )
		jwi[i].Serialize(ar);
	// DWORD x14 -- ��а
	// DWORD x5  -- ���@�֌W�̐ݒ�
	// DWORD x1  -- ��а
	// DWORD x1  -- ���`��̍ő啝
	for ( i=0; i<21; i++ )
		ar >> dw;
	// �v�����^�o�͔͈͂̌��_(X,Y)
	// �v�����^�o�͔{��
	ar >> dd >> dd >> dd;
	// �v�����^90����]�o�́A�v�����^�o�͊�_�ʒu
	// �ڐ��ݒ胂�[�h
	// �ڐ��\���ŏ��Ԋu�h�b�g
	// �ڐ��\���Ԋu(X,Y)
	// �ڐ���_(X,Y)
	ar >> dw >> dw
		>> dd >> dd >> dd >> dd >> dd;
	// ڲԖ�(ڲԸ�ٰ�߂̽��قྯ�)
	for ( i=0; i<JW_LAYERGRPOUP; i++ ) {
		for ( j=0; j<JW_LAYERGRPOUP; j++ ) {
			ar >> jwl[i*JW_LAYERGRPOUP+j].strLayer;
			jwl[i*JW_LAYERGRPOUP+j].dScale = jwi[i].GetScale();
		}
	}
	// ڲԸ�ٰ�ߖ�
	for ( i=0; i<JW_LAYERGRPOUP; i++ )
		ar >> ss;

	// -- �ȍ~�̐��l�ް���NCVC�ł͓ǂݔ�΂� --

	// ���e�v�Z�̏���
	// ����ʍ���
	// �ܓx
	// 9�`15�̑���̎w��(DWORD)
	// �ǖʓ��e����ʍ���
	ar >> dd >> dd >> dw >> dd;
	// �V��}�̏����iVer.3.00�ȍ~)
	// ����ʍ���
	// �V��}�̔��a�~2
	if ( g_dwVersion >= JWWVER_300 )
		ar >> dd >> dd;
	// 2.5D�̌v�Z�P��(0�ȊO��mm�P�ʂŌv�Z)
	ar >> dw;
	// �ۑ����̉�ʔ{��(�Ǎ��ނƑO��ʔ{���ɂȂ�)
	// �͈͋L���{���Ɗ�_(X,Y)
	ar >> dd >> dd >> dd >> dd >> dd >> dd;
	// �}�[�N�W�����v�{���A��_(X,Y)����у��C���O���[�v
	if ( g_dwVersion >= JWWVER_300 ) {
		for ( i=1; i<=8; i++ )
			ar >> dd >> dd >> dd >> dw;
		// �����̕`����(Ver.4.05�ȍ~�j
		ar >> dd >> dd >> dd >> dw >> dd >> dd >> dd >> dw;
	}
	else {
		for ( i=1; i<=4; i++ )
			ar >> dd >> dd >> dd;
	}
	// �����Ԋu
	for ( i=0; i<=9; i++ ) 
		ar >> dd;
	// ���������̗����o�̐��@
	ar >> dd;
	// �F�ԍ����Ƃ̉�ʕ\���F�A����
	for ( i=0; i<=9; i++ )
	    ar >> dw >> dw;
	// �F�ԍ����Ƃ̃v�����^�o�͐F�A�����A���_���a
	for ( i=0; i<=9; i++ )
		ar >> dw >> dw >> dd;
	// ����ԍ�2����9�܂ł̃p�^�[���A1���j�b�g�̃h�b�g���A�s�b�`�A�v�����^�o�̓s�b�`
	for ( i=2; i<=9; i++ )
		ar >> dw >> dw >> dw >> dw;
	// �����_����1����5�܂ł̃p�^�[���A��ʕ\���U���E�s�b�`�A�v�����^�o�͐U���E�s�b�`
	for ( i=11; i<=15; i++ )
		ar >> dw >> dw >> dw >> dw >> dw;
	// �{������ԍ�6����9�܂ł̃p�^�[���A1���j�b�g�̃h�b�g���A�s�b�`�A�v�����^�o�̓s�b�`
	for ( i=16; i<=19; i++ )
	    ar >> dw >> dw >> dw >> dw;
	// ���_����ʕ`�掞�̎w�蔼�a�ŕ`��
	// ���_���v�����^�o�͎��A�w�蔼�a�ŏ���
	// BitMap�E�\���b�h���ŏ��ɕ`�悷��
	// �t�`��, �t�T�[�`, �J���[���
	// ���C�����̈��, �F�ԍ����̈��
	// ���C���O���[�v�܂��̓��C�����Ƃ̃v�����^�A���o�͎w��
	// �v�����^���ʃ��C��(�\���̂݃��C��)�̃O���[�o�͎w��
	// �v�����^�o�͎��ɕ\���̂݃��C���͏o�͂��Ȃ�
	// ��}���ԁiVer.2.23�ȍ~�j
	// 2.5D�̎n�_�ʒu���ݒ肳��Ă��鎞�̃t���O�iVer.2.23�ȍ~�j
	// 2.5D�̓����}�E���Ր}�E�A�C�\���}�̎��_�����p�iVer.2.23�ȍ~�j
	ar >> dw >> dw >> dw >> dw >> dw
		>> dw >> dw >> dw >> dw >> dw
		>> dw >> dw >> dw >> dw >> dw >> dw;
	// 2.5D�̓����}�̎��_�����E���_����iVer.2.23�ȍ~�j
	// 2.5D�̒��Ր}�̎��_�����E���_����iVer.2.23�ȍ~�j
	// 2.5D�̃A�C�\���}�̎��_�����p�iVer.2.23�ȍ~�j
	// ���̒����w��̍ŏI�l�iVer.2.25�ȍ~�j
	// ��`���@�����@�E�c���@�w��̍ŏI�l�iVer.2.25�ȍ~�j
	// �~�̔��a�w��̍ŏI�l�iVer.2.25�ȍ~�j
	// �\���b�h��C�ӐF�ŏ����t���O�A�\���b�h�̔C�ӐF�̊���l�iVer.2.30�ȍ~�j
	ar >> dd >> dd >> dd >> dd >> dd
		>> dd >> dd >> dd >> dd >> dw >> dw;

	// SXF�Ή��g�����F��`�iVer.4.20�ȍ~�j
	if ( g_dwVersion >= JWWVER_420 ) {
		for ( i=0; i<=256; i++ )
			ar >> dw >> dw;
		for ( i=0; i<=256; i++ )
			ar >> ss >> dw >> dw >> dd;
		for ( i=0; i<=32; i++ )
			ar >> dw >> dw >> dw >> dw;
		for ( i=0; i<=32; i++ ) {
			ar >> ss >> dw;
			for ( j=1; j<=10; j++ )
				ar >> dd;
		}
	}

	// ������1����10�܂ł̕������A�����A�Ԋu�A�F�ԍ�
	for ( i=1; i<=10; i++ )
		ar >> dd >> dd >> dd >> dw;
	// �����ݕ����̕������A�����A�Ԋu�A�F�ԍ��A�����ԍ�
	// �����ʒu�����̍s�ԁA������
	// ������_�̂���ʒu�g�p�̃t���O
	// ������_�̉������̂���ʒu���A���A�E
	// ������_�̏c�����̂���ʒu���A���A��
	ar >> dd >> dd >> dd >> dw >> dw
		>> dd >> dd >> dw >> dd >> dd >> dd
		>> dd >> dd >> dd;
}

/////////////////////////////////////////////////////////////////////////////
//	�e���ް���͊֐�

BOOL JWWtoNCVCdata
	(NCVCHANDLE hDoc, const CJwwData& lstJWW, const CJwwBlock& lstBlk, const JWLAYER jwl[])
{
	POSITION	pos;
	int			i;
	CData*	pData;
	CString	strMsg;

	// Ҳ��ڰт���۸�ڽ�ް������
	NCVC_MainfrmProgressRange(0, lstJWW.GetCount());

	for ( i=0, pos=lstJWW.GetHeadPosition(); pos; i++ ) {
		pData = lstJWW.GetNext(pos);
		if ( !pData->JWWtoNCVCdata(hDoc, lstBlk, jwl) ) {
			strMsg.Format(IDS_ERR_ADDNCVC, -1, i+1);	// ��Q�����͏ȗ�
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		// ��۸�ڽ�ް�X�V(64�񂨂�)
		if ( (i & 0x003f) == 0 )
			NCVC_MainfrmProgressPos(i);
	}

	return TRUE;
}
