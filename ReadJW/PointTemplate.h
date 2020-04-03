// PointTemplate.h
//	(NCVC�{�̂�����p)
//////////////////////////////////////////////////////////////////////

#pragma once

#include <math.h>
#include <float.h>
#include "boost/operators.hpp"		// ���Z�q�̎蔲����`
#include "boost/math/constants/constants.hpp"	// PI�̒�`

const double EPS = 0.0005;			// NC�̌v�Z�덷
const double NCMIN = 0.001;			// NC�̌������덷
const double PI = boost::math::constants::pi<double>();

// Radian�ϊ�
inline	double	RAD(double dVal)
{
	return dVal * PI / 180.0;
}
// Degree�ϊ�
inline	double	DEG(double dVal)
{
	return dVal * 180.0 / PI;
}

// 1/1000 �l�̌ܓ�
inline	double	RoundUp(double dVal)
{
	return _copysign( floor(fabs(dVal) * 1000.0 + 0.5) / 1000.0, dVal );
}
// 1/1000 �؂�̂�
inline	double	RoundCt(double dVal)
{
	return _copysign( floor(fabs(dVal) * 1000.0) / 1000.0, dVal );
}

//////////////////////////////////////////////////////////////////////
// �����^ CPoint �̐��`

template<class T>
class CPointT :
	// +=, -=, *=, /= �� +, -, * / ��������`
	boost::arithmetic			< CPointT<T>,
	boost::arithmetic			< CPointT<T>, T,
	boost::equality_comparable	< CPointT<T>,
	boost::equality_comparable	< CPointT<T>, T
	> > > >
{
public:
	union {
		struct {
			T	x, y;
		};
		T	xy[2];
	};

	// �R���X�g���N�^
	CPointT() {
		x = y = 0;
	}
	CPointT(T xy) {
		x = y = xy;
	}
	CPointT(T xx, T yy) {
		SetPoint(xx, yy);
	}
	CPointT(const CPoint& pt) {
		SetPoint(pt.x, pt.y);
	}
	// ���Z�q��`
	CPointT<T>&	operator = (T xy) {
		x = y = xy;
		return *this;
	}
	CPointT<T>&	operator = (const CPoint& pt) {
		SetPoint(pt.x, pt.y);
		return *this;
	}
	CPointT<T>&	operator += (T d) {
		x += d;		y += d;
		return *this;
	}
	CPointT<T>&	operator += (const CPointT<T>& pt) {
		x += pt.x;		y += pt.y;
		return *this;
	}
	CPointT<T>&	operator -= (T d) {
		x -= d;		y -= d;
		return *this;
	}
	CPointT<T>&	operator -= (const CPointT<T>& pt) {
		x -= pt.x;		y -= pt.y;
		return *this;
	}
	CPointT<T>&	operator *= (T d) {
		x *= d;		y *= d;
		return *this;
	}
	CPointT<T>&	operator *= (const CPointT<T>& pt) {
		x *= pt.x;		y *= pt.y;
		return *this;
	}
	CPointT<T>&	operator /= (T d) {
		x /= d;		y /= d;
		return *this;
	}
	CPointT<T>&	operator /= (const CPointT<T>& pt) {
		x /= pt.x;		y /= pt.y;
		return *this;
	}
	bool	IsMatchPoint(const CPointT* pt) const {
		CPointT<T>	ptw(x - pt->x, y - pt->y);
		return ptw.hypot() < NCMIN;
	}
	bool		operator == (const CPointT& pt) const {
		return x==pt.x && y==pt.y;
	}
	bool		operator == (T pt) const {
		return x==pt && y==pt;
	}
	T&		operator[] (size_t a) {
		ASSERT(a>=0 && a<SIZEOF(xy));
		return xy[a];
	}
	T		operator[] (size_t a) const {
		ASSERT(a>=0 && a<SIZEOF(xy));
		return xy[a];
	}
	T		hypot(void) const {
		return ::_hypot(x, y);
	}
	// �ϊ��֐�
	operator CPoint() const {
		return CPoint((int)x, (int)y);
	}
	operator DPOINT() const {		// NCVCdefine.h
		DPOINT	pt;	// �ݽ�׸���`�ł��Ȃ�...
		pt.x = x;	pt.y = y;	pt.z = 0;
		return pt;
	}
	CPointT<T>	RoundUp(void) const {
		return CPointT<T>(::RoundUp(x), ::RoundUp(y));
	}
	// ����֐�
	void	SetPoint(T xx, T yy) {
		x = xx;		y = yy;
	}
	// ���W��]
	void	RoundPoint(T q) {
		T	cos_q = cos(q), sin_q = sin(q);
		CPointT<T>	pt(x, y);
		x = pt.x * cos_q - pt.y * sin_q;
		y = pt.x * sin_q + pt.y * cos_q;
	}
};
typedef	CPointT<double>		CPointD;
typedef	CPointT<float>		CPointF;
