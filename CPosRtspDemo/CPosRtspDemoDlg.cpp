
// CPosRtspDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CPosRtspDemo.h"
#include "CPosRtspDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




#define USE_SDL

#ifdef USE_SDL
#include "SDL2/SDL.h"
#endif // USE_SDL



extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};


#pragma comment(lib, "video/lib/avcodec.lib")
#pragma comment(lib, "video/lib/avformat.lib")
#pragma comment(lib, "video/lib/avutil.lib")
#pragma comment(lib, "video/lib/avdevice.lib")
#pragma comment(lib, "video/lib/avfilter.lib")
#pragma comment(lib, "video/lib/postproc.lib")
#pragma comment(lib, "video/lib/swresample.lib")
#pragma comment(lib, "video/lib/swscale.lib")

#ifdef USE_SDL
#pragma comment(lib, "video/lib/SDL2.lib")
#pragma comment(lib, "video/lib/SDL2main.lib")
#endif



#define DIB_BUFFER_SIZE 10000000 //����ץȡͼ������ڴ�С, 10MB





// CCPosRtspDemoDlg dialog



CCPosRtspDemoDlg::CCPosRtspDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCPosRtspDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);



	m_dwPlayerThreadId = 0;

	//Ϊͼ�񱣴���������
	m_dib_buffer = new char[DIB_BUFFER_SIZE];
	memset(m_dib_buffer, 0, DIB_BUFFER_SIZE * sizeof(unsigned char));
	m_dib_picture_size = 0;

	m_iCapture = 0;


}

void CCPosRtspDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIDEO, m_static_video);
	DDX_Control(pDX, IDC_EDIT_NUM, m_edit_num);
	DDX_Control(pDX, IDC_EDIT_URL, m_edit_url);
}

BEGIN_MESSAGE_MAP(CCPosRtspDemoDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_B_LOAD, &CCPosRtspDemoDlg::OnBnClickedBLoad)
	ON_BN_CLICKED(IDC_B_CAPTURE, &CCPosRtspDemoDlg::OnBnClickedBCapture)
	ON_BN_CLICKED(IDC_B_PREVIEW, &CCPosRtspDemoDlg::OnBnClickedBPreview)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_B_FRAME, &CCPosRtspDemoDlg::OnBnClickedBFrame)
END_MESSAGE_MAP()


// CCPosRtspDemoDlg message handlers

BOOL CCPosRtspDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_edit_url.SetWindowTextW(_T("rtsp://192.168.1.10:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream"));

	m_edit_num.SetWindowTextW(_T("30"));

	//��ʼ����Ƶ��
	Rtsp_Init();



	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCPosRtspDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCPosRtspDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCPosRtspDemoDlg::OnBnClickedBLoad()
{

	CFileDialog dlgOpen(TRUE, _T("*.*"), _T(""), NULL, _T(""));//"*.dat|*.dat|All Files|*.*|"

	if (dlgOpen.DoModal() == IDOK)
	{
		m_edit_url.SetWindowTextW(dlgOpen.GetPathName());
	}

}


void CCPosRtspDemoDlg::OnBnClickedBCapture()
{

	CString strNum;
	m_edit_num.GetWindowTextW(strNum);
	m_iCapture = _wtoi(strNum);
	int i = 0;

	for (i = m_iCapture; i >0; i--)
	{
		if (m_iCapture < 0)
		{
			MessageBox(_T("Capture Complete"));
			break;
		}
		Sleep(200);
	}
			
}


void CCPosRtspDemoDlg::OnBnClickedBPreview()
{
	CString strUrl;
	m_edit_url.GetWindowTextW(strUrl);

	//��������, ��ֹͣ
	if (m_dwPlayerThreadId)
	{
		m_exit_rtsp = 1;

		int i = 0;

		for (i = 0; i < 300; i++)
		{
			if (m_dwPlayerThreadId == 0)
			{
				break;
			}
			Sleep(10);
		}
	}

	RtspThreadStart(strUrl, 0, (DWORD_PTR)m_static_video.m_hWnd);


}




//////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

//��ȡ��������Ŀ¼
CString GetAppPath()
{
	CString strPath;
	GetModuleFileName(NULL, strPath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	strPath.ReleaseBuffer();
	int iPos;
	iPos = strPath.ReverseFind(_T('\\'));
	strPath = strPath.Left(iPos + 1);
	return strPath;
}


//����DIBͼ�����������BMP�ļ�
//unsigned char *data ͼ������
//int width ͼ���
//int height ͼ��߶�
//int bpp ��ɫλ��
//CString strName �����ļ���
void SaveBitmap(unsigned char *data, int width, int height, int bpp, CString strName)
{
	//BMP�ļ�ͷ��Ϣ
	BITMAPFILEHEADER bmpHeader = { 0 };
	bmpHeader.bfType = ('M' << 8) | 'B';
	bmpHeader.bfReserved1 = 0;
	bmpHeader.bfReserved2 = 0;
	bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpHeader.bfSize = bmpHeader.bfOffBits + width*height*bpp / 8; //BMP�ļ��Ĵ�С

	BITMAPINFO bmpInfo = { 0 };
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = width;
	bmpInfo.bmiHeader.biHeight = -height;  // ��תͼƬ
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = bpp;
	bmpInfo.bmiHeader.biCompression = 0;
	bmpInfo.bmiHeader.biSizeImage = 0;
	bmpInfo.bmiHeader.biXPelsPerMeter = 100;
	bmpInfo.bmiHeader.biYPelsPerMeter = 100;
	bmpInfo.bmiHeader.biClrUsed = 0;
	bmpInfo.bmiHeader.biClrImportant = 0;

	if (strName.IsEmpty()) return;

	//���浽�ļ���
	CFile f;
	BOOL r = FALSE;
	if (f.Open(strName, CFile::modeCreate | CFile::modeWrite))
	{
		f.Write(&bmpHeader, sizeof(BITMAPFILEHEADER));
		f.Write(&bmpInfo.bmiHeader, sizeof(BITMAPINFOHEADER));
		f.Write(data, width * height * bpp / 8);
		f.Close();
	}


}


//����ص�
static int ctx_open_timeout_interrupt_cb(LPVOID lpParam)
{
	CCPosRtspDemoDlg *pThis = (CCPosRtspDemoDlg*)lpParam;


	//return 1; ����1 ��ʾ����, ����0�����ȴ�

	return 0;//
}



//��Ƶ�����߳�
DWORD WINAPI CCPosRtspDemoDlg::PlayerWorkThread(LPVOID lpParam)
{

	CCPosRtspDemoDlg *pThis = 0;
	pThis = (CCPosRtspDemoDlg *)lpParam;

	AVFormatContext	*pFormatCtx = NULL;	//��Ƶ��ʽ������
	int i = 0;
	int videoindex = -1; //��Ƶͨ��(��Ϊһ������ͷ�п����ж��RTSP��Ƶͨ��,�������ͨ��,����ͨ����)
	AVCodecContext	*pCodecCtx = NULL;//������
	AVCodec			*pCodec = NULL;//
	AVFrame	*pFrame = NULL;//��Ƶ֡
	AVFrame *pFrameBGR = NULL;//RGB����֡
	unsigned char *out_buffer = NULL;//���뻺��,����ʵʱ����ͼ��
	AVPacket *packet = NULL;//��Ƶ�������ݰ�
	int ret = 0;
	int got_picture = 0;//��ǽ����Ƿ����
	AVDictionary* opts = NULL;//������ز�������

	int res = 0;

	struct SwsContext *img_convert_ctx = NULL;//ͼ��ת����,���ڰ���Ƶ����ת����RGBͼ���ʽ

	char chUrl[256] = { 0 };//��Ƶurl
	int size = 0;
	size = WideCharToMultiByte(CP_ACP, 0, pThis->m_strUrl, -1, chUrl, sizeof(chUrl) - 1, NULL, NULL);  //��CStringת��char����,���س���,����\0
	chUrl[size] = 0;


#ifdef USE_SDL
	//SDLͼ����ʾ��, �ô˿���Դ����������Ƶ��ʾ��Ч��
	//------------SDL----------------
	int screen_w = 0;
	int screen_h = 0;
	SDL_Window *screen = NULL;
	SDL_Renderer* sdlRenderer = NULL;
	SDL_Texture* sdlTexture = NULL;
	SDL_Rect sdlRect;
	SDL_Thread *video_tid = NULL;
#endif


	//��Ƶ��ʽ��������ʼ��
	pFormatCtx = avformat_alloc_context();
	pFormatCtx->interrupt_callback.callback = ctx_open_timeout_interrupt_cb;
	pFormatCtx->interrupt_callback.opaque = pThis;
	//pFormatCtx->flags |= AVFMT_FLAG_NONBLOCK; //����Ϊ��������



	av_dict_set(&opts, "stimeout", "3000000", 0); // ���ó�ʱ3��
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);  //��tcp��ʽ��, Ҳ����ʹ��udp
	//av_dict_set(&opts, "timeout", "3000000", 0);//���ó�ʱ3��


	do
	{

		// ����Ƶ������ȡ�ļ�ͷ��Ϣ�� pFormatCtx �ṹ����
		res = avformat_open_input(&pFormatCtx, chUrl, NULL, &opts);
		if (res < 0)
		{
			//	printf("Couldn't open input stream.\n");
			break;
		}
		if (opts) av_dict_free(&opts);

		// ��ȡ����Ϣ�� pFormatCtx->streams ��
		// pFormatCtx->streams ��һ�����飬�����С�� pFormatCtx->nb_streams
		res = avformat_find_stream_info(pFormatCtx, NULL);
		if (res < 0)
		{
			//	printf("Couldn't find stream information.\n");
			break;
		}

		//����Ϣ�����в�ѯ��Ƶͨ��
		//�˴�ֱ��ʹ���˵�һ����Ƶ��ͨ��, �����Լ�������Ҫѡ��������Ƶͨ��
		videoindex = -1;
		for (i = 0; i < pFormatCtx->nb_streams; i++)//����ҵ���������Ƶ��ͨ��
		{
			if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)// �ҵ���һ����Ƶͨ��, ֱ��ʹ�ô���Ƶ��ͨ��
			{
				videoindex = i;
				break;
			}
		}
		if (videoindex == -1)//����ͨ��������video, ֱ���˳�
		{
			//printf("Didn't find a video stream.\n");
			res = -1;
			break;
		}

		// ��ȡ������������
		pCodecCtx = pFormatCtx->streams[videoindex]->codec;
		// ��ȡ������
		pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
		if (pCodec == NULL)
		{
			//printf("Codec not found.\n");
			res = -1;
			break;
		}

		// �򿪽�����
		res = avcodec_open2(pCodecCtx, pCodec, NULL);
		if (res < 0)
		{
			//printf("Could not open codec.\n");
			break;
		}

		packet = (AVPacket *)av_malloc(sizeof(AVPacket));//��Ƶ�������ݰ�
		//Ϊ��Ƶ֡���������ڴ�ռ�
		pFrame = av_frame_alloc();


//////////////////////////////////////////////////////////////////////////////////////

		//����Ƶ��ʽ������ת��ΪBMPͼƬʹ�õ�RGB��ʽ����Ҫ����׼������

		//Ϊ��Ƶ����ת��ΪRGB���ݵ�ת���������ڴ�
		pFrameBGR = av_frame_alloc();
		int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1);//��ȡת��ΪRGB����Ҫ���ڴ��С

		//����Ҫ����RGB���ݵĴ洢��
		out_buffer = (unsigned char *)av_malloc(numBytes * sizeof(unsigned char));
		if (out_buffer == NULL)
		{
			res = 0;
			break;
		}

		// ��� pFrameBGR �е��ֶ�(data��linsize��)(��ʼ��RGB��ʽת������)
		av_image_fill_arrays(pFrameBGR->data, pFrameBGR->linesize, out_buffer,
			AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1);

		// ��ȡͼ��ת����
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
			pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
		if (img_convert_ctx == NULL)
		{
			res = -1;
			break;
		}

//////////////////////////////////////////////////////////////////////////////////////



#ifdef USE_SDL

		//������ʾͼ��ʱSDL��Ĳ���

		//����տ�ʼ����ʱ�˳�, ��Ϊ��������, ���³����������
		if (pThis->m_exit_rtsp)
		{
			break;
		}

		//��������Ƶ��ʾ����, ��ʾ��Ƶ��������, ������ʾ����
		if (pThis->m_callbackPara && pCodecCtx->width > 0)	//��Ҫ��ʾ
		{

			screen_w = pCodecCtx->width;
			screen_h = pCodecCtx->height;

			//�����Ҫ�����Ĵ�����ʾ��Ƶ
			//screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			//	screen_w, screen_h,SDL_WINDOW_OPENGL);

			//����Ҫ��������,ʹ��windows�����еĴ���
			screen = SDL_CreateWindowFrom((void *)(pThis->m_callbackPara));
			if (!screen)
			{
				//printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
				res = -1;
				break;
			}
			sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
			//������ʾ��ͼ���ʽΪBRG24��ʽ(��Ϊǰ�����ʱʹ�õ���BRG24)
			sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);//SDL_PIXELFORMAT_IYUV
			if (!sdlTexture)
			{
				res = -1;
				break;
			}

			sdlRect.x = 0;
			sdlRect.y = 0;
			sdlRect.w = screen_w;
			sdlRect.h = screen_h;
			//------------SDL End------------
			::ShowWindow((HWND)pThis->m_callbackPara, SW_SHOWNORMAL);//��ʾwindows�еĴ���
		}

#endif

		CString strFileName;//������ļ���
		CString strExePath;//�����Ŀ¼

		strExePath = GetAppPath();

		int capture_cnt = 0;//ץȡͼ�����
		int capture_amount = 0;//�ܹ�Ҫץ����
		int read_frame_result = 0;
		int frame_count = 0;//ʵʱ����֡��ͳ��

		memset(pThis->m_dib_buffer, 0, DIB_BUFFER_SIZE * sizeof(unsigned char));
		pThis->m_dib_picture_size = 0;


		//����׼���������, ׼��������Ƶ,ת��RGBͼ���ʽ,���浽�ļ�

		while (1)
		{
			//�û���ʼץͼ > 0��ʾ��ʼץȡ, ==0 ��ʾ����ץȡ, < 0 ��ʾ���
			if (pThis->m_iCapture > 0)
			{
				capture_amount = pThis->m_iCapture;
				pThis->m_iCapture = 0;
				capture_cnt = 0;

			}

			while (1)
			{	
				//��ȡһ֡���ݵ��������ݰ�������
				read_frame_result = av_read_frame(pFormatCtx, packet);
				if (read_frame_result < 0)	//==0�ɹ�, <0��ʾ�д���
				{
					break;
				}

				//�˰��е�����ͨ�� == ǰ�����úõ���Ƶͨ��
				if (packet->stream_index == videoindex)	//����Ҫ��������, ����
				{
					break;
				}
				else
				{
					av_free_packet(packet);	//����ͨ��������, ����Ҫ, �ͷ�, ������ȡ
				}

				if (pThis->m_exit_rtsp)
				{
					break;
				}

			}

			//��������
			if (read_frame_result < 0 || pThis->m_exit_rtsp)//��Ƶ����
			{
				break;
			}

			//������Ƶ����
			// ���� pCodecCtx ���������ݣ�ע��ֻ���ڽ��������� frame ʱ�ú����ŷ��� 0 
			res = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (res < 0)//�������
			{
				printf("Decode Error.\n");
				res = -1;
				break;
			}

			frame_count++; //֡ͳ��

			
			if (got_picture)////����ɹ�
			{
				//����Ƶ֡���ݽ������䵽����ͼ�񻺳�
				sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameBGR->data, pFrameBGR->linesize);
				
				pThis->DibBufferWrite((char *)pFrameBGR->data[0], pCodecCtx->width, pCodecCtx->height, 24);//�ѽ����BMP���ݷŵ�RAM��, ����ֱ��ʹ��

				//ѭ����������ץ�ĵ�ͼ��BMP�ļ���
				if (capture_amount && capture_cnt < capture_amount)
				{
					COleDateTime timeNow;
					timeNow = COleDateTime::GetCurrentTime();

					strFileName.Format(_T("%s%s_%d.bmp"), strExePath, timeNow.Format(_T("%Y-%m-%d %H.%M.%S")), capture_cnt++);

					SaveBitmap(pFrameBGR->data[0], pCodecCtx->width, pCodecCtx->height, 24, strFileName);//�ѽ����BMP���ݱ��浽BMP�ļ���
					
					if (capture_cnt >= capture_amount)pThis->m_iCapture = -1; //���һ�ű�������
				}
				else
				{
					capture_amount = capture_cnt = 0;
					
				}


#ifdef USE_SDL

				//�ѽ�����BMPͼ����ʾ��windows�Ĵ�����
				if (pThis->m_callbackPara && pCodecCtx->width > 0)
				{
						//SDL---------------------------
						SDL_UpdateTexture(sdlTexture, NULL, pFrameBGR->data[0], pFrameBGR->linesize[0]);
						SDL_RenderClear(sdlRenderer);
						//SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );  
						SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
						SDL_RenderPresent(sdlRenderer);
				}

				//SDL End-----------------------
#endif


				if (chUrl[1] == ':')//�����ļ�,��Ϊ�����ٶȺܿ�,ÿ֡��������ʱ�Ա�֤��ʾ1��30֡
				{
					Sleep(33);
				}
			}

			av_free_packet(packet);//�ͷ�֡����

			if (pThis->m_exit_rtsp)
			{
				break;
			}

		}

	} while (0);





#ifdef USE_SDL
	if (screen_w)
	{
		if (sdlTexture) SDL_DestroyTexture(sdlTexture);
		if (sdlRenderer) SDL_DestroyRenderer(sdlRenderer);
		if (screen)
		{
			SDL_DestroyWindow(screen);
			::ShowWindow((HWND)pThis->m_callbackPara, SW_SHOWNORMAL);//SDL���ٴ�����Ѵ�������, �˴�����ʾ����
		}
	}
	//--------------
#endif


	if (img_convert_ctx) sws_freeContext(img_convert_ctx);

	if (out_buffer) av_free(out_buffer);

	if (packet) av_free(packet);

	if (pFrameBGR) av_frame_free(&pFrameBGR);
	if (pFrame) av_frame_free(&pFrame);
	if (pCodecCtx) avcodec_close(pCodecCtx);
	if (pFormatCtx) avformat_close_input(&pFormatCtx);

	//�������
	memset(pThis->m_dib_buffer, 0, DIB_BUFFER_SIZE * sizeof(unsigned char));
	pThis->m_dib_picture_size = 0;

	pThis->m_dwPlayerThreadId = NULL;
	return res;
}



int CCPosRtspDemoDlg::RtspThreadStart(CString strUrl, DWORD_PTR callbackFun, DWORD_PTR callbackPara)
{
	if (strUrl.IsEmpty()) return 0;


	// �����������߳�  
	if (m_dwPlayerThreadId == 0)
	{

		m_strUrl = strUrl;//Ҫ������Ƶ��URL
		m_callbackFun = callbackFun;
		m_callbackPara = callbackPara;
		m_exit_rtsp = FALSE;
		CreateThread(NULL, 0, PlayerWorkThread, this, 0, &m_dwPlayerThreadId);
	}

	return 1;

}

int CCPosRtspDemoDlg::Rtsp_Init()
{
	int res = 0;

	// ע�����еĸ�ʽ�ͽ�����
	av_register_all();
	avcodec_register_all(); //ffmpegע��������

	avformat_network_init();//��ʼ�����繦��, ffmpeg�Ѽ���libRtsp, ����ֱ�Ӳ���rtsp������Э��

#ifdef USE_SDL


	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		//printf("Could not initialize SDL - %s\n", SDL_GetError());
		res = -1;

	}
#endif



	return res;
}

void CCPosRtspDemoDlg::Rtsp_Release()
{
#ifdef USE_SDL

	SDL_Quit();
#endif



}


//�ѽ����BMPͼ�����ݸ��Ƶ�RAM������
int CCPosRtspDemoDlg::DibBufferWrite(char *data, int width, int height, int bpp)
{
	if (data == NULL)
	{
		m_dib_picture_size = 0;
		return 0;
	}

	int buf_size = DIB_BUFFER_SIZE;
	int len = 0;
	int dib_size = width * height * bpp / 8;
	BITMAPFILEHEADER bmpHeader = { 0 };
	bmpHeader.bfType = ('M' << 8) | 'B';
	bmpHeader.bfReserved1 = 0;
	bmpHeader.bfReserved2 = 0;
	bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpHeader.bfSize = bmpHeader.bfOffBits + width*height*bpp / 8;

	BITMAPINFO bmpInfo = { 0 };
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = width;
	bmpInfo.bmiHeader.biHeight = -height;  // ��תͼƬ
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = bpp;
	bmpInfo.bmiHeader.biCompression = 0;
	bmpInfo.bmiHeader.biSizeImage = 0;
	bmpInfo.bmiHeader.biXPelsPerMeter = 100;
	bmpInfo.bmiHeader.biYPelsPerMeter = 100;
	bmpInfo.bmiHeader.biClrUsed = 0;
	bmpInfo.bmiHeader.biClrImportant = 0;


	g_memCriticalSection.Lock();//�߳�����

	memcpy(m_dib_buffer + len, (&bmpHeader), sizeof(BITMAPFILEHEADER)); len += sizeof(BITMAPFILEHEADER);
	memcpy(m_dib_buffer + len, (&bmpInfo.bmiHeader), sizeof(BITMAPINFOHEADER)); len += sizeof(BITMAPINFOHEADER);

	if (len + dib_size <= buf_size)
	{
		memcpy(m_dib_buffer + len, data, dib_size); len += dib_size;
	}

	m_dib_picture_size = len;
	g_memCriticalSection.Unlock();//�߳̽���

	return len;

}



//��ͼ�񻺳�����ȡBMP����,����ͼ���С
int CCPosRtspDemoDlg::CaptureBmpToRam(char *pBuf, int len)
{

	if (m_dib_picture_size)
	{
		if (len > m_dib_picture_size)
		{
			g_memCriticalSection.Lock();//�߳�����
			memcpy(pBuf, m_dib_buffer, m_dib_picture_size);
			g_memCriticalSection.Unlock();//����
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	return m_dib_picture_size;
}






void CCPosRtspDemoDlg::OnClose()
{

	m_exit_rtsp = 1;//�˳��߳�

	int i = 0;

	//���߳��˳�
	for (i = 0; i <300; i++)
	{
		if (m_dwPlayerThreadId == 0)
		{
			break;
		}
		Sleep(10);
	}



	delete[]m_dib_buffer;


	CDialogEx::OnClose();
}


//ץȡһ֡ͼ��, ���浽�ļ���
void CCPosRtspDemoDlg::OnBnClickedBFrame()
{
	char *buf = 0;
	buf = new char[DIB_BUFFER_SIZE];
	int size = 0;
	size = CaptureBmpToRam(buf, DIB_BUFFER_SIZE);

	if (size)
	{
		CString str = _T("bmp(*.bmp)|*.bmp|All Files (*.*)|*.*||");
		CFileDialog dlgOpen(FALSE, _T(".bmp"), _T(""), NULL, str);

		if (dlgOpen.DoModal() == IDOK)
		{
			CString strFile = dlgOpen.GetPathName();
			//���浽�ļ���
			CFile f;
			BOOL r = FALSE;
			if (f.Open(strFile, CFile::modeCreate | CFile::modeWrite))
			{

				f.Write(buf, size);
				f.Close();
			}

		}
	}


	delete []buf;

}
