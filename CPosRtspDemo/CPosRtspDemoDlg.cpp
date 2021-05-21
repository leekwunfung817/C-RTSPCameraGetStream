
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



#define DIB_BUFFER_SIZE 10000000 //定义抓取图像的内在大小, 10MB





// CCPosRtspDemoDlg dialog



CCPosRtspDemoDlg::CCPosRtspDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCPosRtspDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);



	m_dwPlayerThreadId = 0;

	//为图像保存申请内在
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

	//初始化视频库
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

	//正在运行, 先停止
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

//获取程序运行目录
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


//根据DIB图像的数据生成BMP文件
//unsigned char *data 图像数据
//int width 图像宽
//int height 图像高度
//int bpp 颜色位数
//CString strName 保存文件名
void SaveBitmap(unsigned char *data, int width, int height, int bpp, CString strName)
{
	//BMP文件头信息
	BITMAPFILEHEADER bmpHeader = { 0 };
	bmpHeader.bfType = ('M' << 8) | 'B';
	bmpHeader.bfReserved1 = 0;
	bmpHeader.bfReserved2 = 0;
	bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpHeader.bfSize = bmpHeader.bfOffBits + width*height*bpp / 8; //BMP文件的大小

	BITMAPINFO bmpInfo = { 0 };
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = width;
	bmpInfo.bmiHeader.biHeight = -height;  // 反转图片
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = bpp;
	bmpInfo.bmiHeader.biCompression = 0;
	bmpInfo.bmiHeader.biSizeImage = 0;
	bmpInfo.bmiHeader.biXPelsPerMeter = 100;
	bmpInfo.bmiHeader.biYPelsPerMeter = 100;
	bmpInfo.bmiHeader.biClrUsed = 0;
	bmpInfo.bmiHeader.biClrImportant = 0;

	if (strName.IsEmpty()) return;

	//保存到文件中
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


//错误回调
static int ctx_open_timeout_interrupt_cb(LPVOID lpParam)
{
	CCPosRtspDemoDlg *pThis = (CCPosRtspDemoDlg*)lpParam;


	//return 1; 返回1 表示结束, 返回0继续等待

	return 0;//
}



//视频播放线程
DWORD WINAPI CCPosRtspDemoDlg::PlayerWorkThread(LPVOID lpParam)
{

	CCPosRtspDemoDlg *pThis = 0;
	pThis = (CCPosRtspDemoDlg *)lpParam;

	AVFormatContext	*pFormatCtx = NULL;	//视频格式分析器
	int i = 0;
	int videoindex = -1; //视频通话(因为一个摄像头中可能有多个RTSP视频通道,例如高清通道,标清通道等)
	AVCodecContext	*pCodecCtx = NULL;//解码器
	AVCodec			*pCodec = NULL;//
	AVFrame	*pFrame = NULL;//视频帧
	AVFrame *pFrameBGR = NULL;//RGB解码帧
	unsigned char *out_buffer = NULL;//解码缓冲,用于实时解码图像
	AVPacket *packet = NULL;//视频网络数据包
	int ret = 0;
	int got_picture = 0;//标记解码是否完成
	AVDictionary* opts = NULL;//解码相关参数设置

	int res = 0;

	struct SwsContext *img_convert_ctx = NULL;//图像转换器,用于把视频数据转换成RGB图像格式

	char chUrl[256] = { 0 };//视频url
	int size = 0;
	size = WideCharToMultiByte(CP_ACP, 0, pThis->m_strUrl, -1, chUrl, sizeof(chUrl) - 1, NULL, NULL);  //把CString转成char类型,返回长度,包括\0
	chUrl[size] = 0;


#ifdef USE_SDL
	//SDL图像显示库, 用此库可以大幅度提升视频显示的效率
	//------------SDL----------------
	int screen_w = 0;
	int screen_h = 0;
	SDL_Window *screen = NULL;
	SDL_Renderer* sdlRenderer = NULL;
	SDL_Texture* sdlTexture = NULL;
	SDL_Rect sdlRect;
	SDL_Thread *video_tid = NULL;
#endif


	//视频格式分析器初始化
	pFormatCtx = avformat_alloc_context();
	pFormatCtx->interrupt_callback.callback = ctx_open_timeout_interrupt_cb;
	pFormatCtx->interrupt_callback.opaque = pThis;
	//pFormatCtx->flags |= AVFMT_FLAG_NONBLOCK; //设置为非阻塞型



	av_dict_set(&opts, "stimeout", "3000000", 0); // 设置超时3秒
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);  //以tcp方式打开, 也可以使用udp
	//av_dict_set(&opts, "timeout", "3000000", 0);//设置超时3秒


	do
	{

		// 打开视频流，读取文件头信息到 pFormatCtx 结构体中
		res = avformat_open_input(&pFormatCtx, chUrl, NULL, &opts);
		if (res < 0)
		{
			//	printf("Couldn't open input stream.\n");
			break;
		}
		if (opts) av_dict_free(&opts);

		// 读取流信息到 pFormatCtx->streams 中
		// pFormatCtx->streams 是一个数组，数组大小是 pFormatCtx->nb_streams
		res = avformat_find_stream_info(pFormatCtx, NULL);
		if (res < 0)
		{
			//	printf("Couldn't find stream information.\n");
			break;
		}

		//从信息数组中查询视频通道
		//此处直接使用了第一个视频流通道, 可以自己根据需要选择其他视频通道
		videoindex = -1;
		for (i = 0; i < pFormatCtx->nb_streams; i++)//检查找到的所有视频流通道
		{
			if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)// 找到第一个视频通道, 直接使用此视频流通道
			{
				videoindex = i;
				break;
			}
		}
		if (videoindex == -1)//所有通道都不是video, 直接退出
		{
			//printf("Didn't find a video stream.\n");
			res = -1;
			break;
		}

		// 获取解码器上下文
		pCodecCtx = pFormatCtx->streams[videoindex]->codec;
		// 获取解码器
		pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
		if (pCodec == NULL)
		{
			//printf("Codec not found.\n");
			res = -1;
			break;
		}

		// 打开解码器
		res = avcodec_open2(pCodecCtx, pCodec, NULL);
		if (res < 0)
		{
			//printf("Could not open codec.\n");
			break;
		}

		packet = (AVPacket *)av_malloc(sizeof(AVPacket));//视频网络数据包
		//为视频帧解码申请内存空间
		pFrame = av_frame_alloc();


//////////////////////////////////////////////////////////////////////////////////////

		//把视频格式的数据转换为BMP图片使用的RGB格式所需要做的准备工作

		//为视频数据转换为RGB数据的转换器申请内存
		pFrameBGR = av_frame_alloc();
		int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1);//获取转换为RGB所需要的内存大小

		//申请要保存RGB数据的存储区
		out_buffer = (unsigned char *)av_malloc(numBytes * sizeof(unsigned char));
		if (out_buffer == NULL)
		{
			res = 0;
			break;
		}

		// 填充 pFrameBGR 中的字段(data、linsize等)(初始化RGB格式转换参数)
		av_image_fill_arrays(pFrameBGR->data, pFrameBGR->linesize, out_buffer,
			AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1);

		// 获取图像转换器
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
			pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
		if (img_convert_ctx == NULL)
		{
			res = -1;
			break;
		}

//////////////////////////////////////////////////////////////////////////////////////



#ifdef USE_SDL

		//设置显示图像时SDL库的参数

		//如果刚开始连接时退出, 因为窗体销毁, 导致出来窗体错误
		if (pThis->m_exit_rtsp)
		{
			break;
		}

		//设置了视频显示窗口, 显示视频到窗口中, 设置显示参数
		if (pThis->m_callbackPara && pCodecCtx->width > 0)	//需要显示
		{

			screen_w = pCodecCtx->width;
			screen_h = pCodecCtx->height;

			//如果需要独立的窗口显示视频
			//screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			//	screen_w, screen_h,SDL_WINDOW_OPENGL);

			//不需要独立窗口,使用windows界面中的窗口
			screen = SDL_CreateWindowFrom((void *)(pThis->m_callbackPara));
			if (!screen)
			{
				//printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
				res = -1;
				break;
			}
			sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
			//设置显示的图像格式为BRG24格式(因为前面解码时使用的是BRG24)
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
			::ShowWindow((HWND)pThis->m_callbackPara, SW_SHOWNORMAL);//显示windows中的窗口
		}

#endif

		CString strFileName;//保存的文件名
		CString strExePath;//保存的目录

		strExePath = GetAppPath();

		int capture_cnt = 0;//抓取图像计数
		int capture_amount = 0;//总共要抓几张
		int read_frame_result = 0;
		int frame_count = 0;//实时解码帧数统计

		memset(pThis->m_dib_buffer, 0, DIB_BUFFER_SIZE * sizeof(unsigned char));
		pThis->m_dib_picture_size = 0;


		//以上准备工作完成, 准备解码视频,转换RGB图像格式,保存到文件

		while (1)
		{
			//用户开始抓图 > 0表示开始抓取, ==0 表示正在抓取, < 0 表示完成
			if (pThis->m_iCapture > 0)
			{
				capture_amount = pThis->m_iCapture;
				pThis->m_iCapture = 0;
				capture_cnt = 0;

			}

			while (1)
			{	
				//读取一帧内容到网络数据包缓冲中
				read_frame_result = av_read_frame(pFormatCtx, packet);
				if (read_frame_result < 0)	//==0成功, <0表示有错误
				{
					break;
				}

				//此包中的数据通道 == 前面设置好的视频通道
				if (packet->stream_index == videoindex)	//是需要的流内容, 跳出
				{
					break;
				}
				else
				{
					av_free_packet(packet);	//其他通道的内容, 不需要, 释放, 继续读取
				}

				if (pThis->m_exit_rtsp)
				{
					break;
				}

			}

			//发生错误
			if (read_frame_result < 0 || pThis->m_exit_rtsp)//视频结束
			{
				break;
			}

			//解码视频数据
			// 返回 pCodecCtx 解码后的数据，注意只有在解码完整个 frame 时该函数才返回 0 
			res = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (res < 0)//解码错误
			{
				printf("Decode Error.\n");
				res = -1;
				break;
			}

			frame_count++; //帧统计

			
			if (got_picture)////解码成功
			{
				//把视频帧数据解码后填充到解码图像缓冲
				sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameBGR->data, pFrameBGR->linesize);
				
				pThis->DibBufferWrite((char *)pFrameBGR->data[0], pCodecCtx->width, pCodecCtx->height, 24);//把解码的BMP数据放到RAM中, 可以直接使用

				//循环保存所有抓拍的图像到BMP文件中
				if (capture_amount && capture_cnt < capture_amount)
				{
					COleDateTime timeNow;
					timeNow = COleDateTime::GetCurrentTime();

					strFileName.Format(_T("%s%s_%d.bmp"), strExePath, timeNow.Format(_T("%Y-%m-%d %H.%M.%S")), capture_cnt++);

					SaveBitmap(pFrameBGR->data[0], pCodecCtx->width, pCodecCtx->height, 24, strFileName);//把解码的BMP数据保存到BMP文件中
					
					if (capture_cnt >= capture_amount)pThis->m_iCapture = -1; //最后一张保存后结束
				}
				else
				{
					capture_amount = capture_cnt = 0;
					
				}


#ifdef USE_SDL

				//把解码后的BMP图像显示到windows的窗口中
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


				if (chUrl[1] == ':')//本地文件,因为解码速度很快,每帧后主动延时以保证显示1秒30帧
				{
					Sleep(33);
				}
			}

			av_free_packet(packet);//释放帧内容

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
			::ShowWindow((HWND)pThis->m_callbackPara, SW_SHOWNORMAL);//SDL销毁窗体后会把窗口隐藏, 此处再显示出来
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

	//清除缓冲
	memset(pThis->m_dib_buffer, 0, DIB_BUFFER_SIZE * sizeof(unsigned char));
	pThis->m_dib_picture_size = 0;

	pThis->m_dwPlayerThreadId = NULL;
	return res;
}



int CCPosRtspDemoDlg::RtspThreadStart(CString strUrl, DWORD_PTR callbackFun, DWORD_PTR callbackPara)
{
	if (strUrl.IsEmpty()) return 0;


	// 创建工作者线程  
	if (m_dwPlayerThreadId == 0)
	{

		m_strUrl = strUrl;//要播放视频的URL
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

	// 注册所有的格式和解码器
	av_register_all();
	avcodec_register_all(); //ffmpeg注册编解码器

	avformat_network_init();//初始化网络功能, ffmpeg已集成libRtsp, 可以直接播放rtsp等网络协议

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


//把解码的BMP图像数据复制到RAM缓冲区
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
	bmpInfo.bmiHeader.biHeight = -height;  // 反转图片
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = bpp;
	bmpInfo.bmiHeader.biCompression = 0;
	bmpInfo.bmiHeader.biSizeImage = 0;
	bmpInfo.bmiHeader.biXPelsPerMeter = 100;
	bmpInfo.bmiHeader.biYPelsPerMeter = 100;
	bmpInfo.bmiHeader.biClrUsed = 0;
	bmpInfo.bmiHeader.biClrImportant = 0;


	g_memCriticalSection.Lock();//线程锁定

	memcpy(m_dib_buffer + len, (&bmpHeader), sizeof(BITMAPFILEHEADER)); len += sizeof(BITMAPFILEHEADER);
	memcpy(m_dib_buffer + len, (&bmpInfo.bmiHeader), sizeof(BITMAPINFOHEADER)); len += sizeof(BITMAPINFOHEADER);

	if (len + dib_size <= buf_size)
	{
		memcpy(m_dib_buffer + len, data, dib_size); len += dib_size;
	}

	m_dib_picture_size = len;
	g_memCriticalSection.Unlock();//线程解锁

	return len;

}



//从图像缓冲区提取BMP数据,返回图像大小
int CCPosRtspDemoDlg::CaptureBmpToRam(char *pBuf, int len)
{

	if (m_dib_picture_size)
	{
		if (len > m_dib_picture_size)
		{
			g_memCriticalSection.Lock();//线程锁定
			memcpy(pBuf, m_dib_buffer, m_dib_picture_size);
			g_memCriticalSection.Unlock();//解锁
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

	m_exit_rtsp = 1;//退出线程

	int i = 0;

	//等线程退出
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


//抓取一帧图像, 保存到文件中
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
			//保存到文件中
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
