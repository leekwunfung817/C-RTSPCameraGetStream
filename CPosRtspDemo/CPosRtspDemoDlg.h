
// CPosRtspDemoDlg.h : header file
//

#pragma once


#include"afxmt.h"//ͬ������ʱ��Ҫ������ͷ�ļ�


// CCPosRtspDemoDlg dialog
class CCPosRtspDemoDlg : public CDialogEx
{
// Construction
public:
	CCPosRtspDemoDlg(CWnd* pParent = NULL);	// standard constructor

	CString m_strUrl;//��ƵURL
	int m_exit_rtsp;//�����Ƿ��˳��߳�
	DWORD m_dwPlayerThreadId; //�����߳�ID

	DWORD_PTR m_callbackFun;//�ص�����
	DWORD_PTR m_callbackPara;//�ص������Ĳ���

	int RtspThreadStart(CString strUrl, DWORD_PTR callbackFun, DWORD_PTR callbackPara);//������Ƶ�߳̿�ʼ
	static DWORD WINAPI PlayerWorkThread(LPVOID lpParam);//��Ƶ�����߳�

	static int Rtsp_Init();//RTSP��ʼ��, �ڳ�������ʱ�����ȳ�ʼ��
	static void Rtsp_Release();//�ͷ�RTSP����,�������ʱ�ͷ�

	char *m_dib_buffer;//�����洢ץȡ��ͼ��
	unsigned int m_dib_picture_size;//ͼ���С

	CCriticalSection g_memCriticalSection; //����,�߳�����

	int DibBufferWrite(char *data, int width, int height, int bpp);//��ÿһ֡ͼ��ʵʱ���浽RAM��

	int CaptureBmpToRam(char *pBuf, int len);

	int m_iCapture;//ץȡͼ�������

// Dialog Data
	enum { IDD = IDD_CPOSRTSPDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_static_video;
	CEdit m_edit_num;
	CEdit m_edit_url;
	afx_msg void OnBnClickedBLoad();
	afx_msg void OnBnClickedBCapture();
	afx_msg void OnBnClickedBPreview();
	afx_msg void OnClose();
	afx_msg void OnBnClickedBFrame();
};
