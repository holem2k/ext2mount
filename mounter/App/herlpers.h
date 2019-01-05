#if !defined(AFX_HELPERS_H__48ACD89E_4FE5_46B1_849A_F86F7DD4C130__INCLUDED_)
#define AFX_HELPERS_H__48ACD89E_4FE5_46B1_849A_F86F7DD4C130__INCLUDED_

class CUnderAdmin
{
public:
	BOOL Check();
};


class CRunOnce
{
public:
	CRunOnce(LPCTSTR lpAppSign);
	~CRunOnce();
	BOOL IsRunning();
	void Show(LPCTSTR lpWindowCaption);
protected:
	HANDLE m_hMutex;
	BOOL m_bRunning;
};


#endif // AFX_HELPERS_H__48ACD89E_4FE5_46B1_849A_F86F7DD4C130__INCLUDED_
