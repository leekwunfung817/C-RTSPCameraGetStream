
// PythonSocketDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CPythonSocketDlg dialog
class CPythonSocketDlg : public CDialogEx
{
// Construction
public:
	CPythonSocketDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PYTHONSOCKET_DIALOG };

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
	afx_msg void OnBnClickedBOpen();
	afx_msg void OnBnClickedBSend();
	CEdit m_edit_path_name;
	CEdit m_edit_receive;
	CEdit m_edit_server_port;
	CEdit m_edit_server_url;
	afx_msg void OnBnClickedOk();
	CEdit m_edit_timeout;
	CButton m_check_checksum;
};
