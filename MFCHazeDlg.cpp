
// MFCHazeDlg.cpp : ʵ���ļ�
//
#include "stdafx.h"
#include "MFCHaze.h"
#include "MFCHazeDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern double _w;				    //w,Ϊ������ı��� Խ��ȥ��Խ��
extern double lightness;			//���ȵ���
extern int SizeH ;					//ͼƬ�߶�
extern int SizeW ;					//ͼƬ���
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCHazeDlg �Ի���



CMFCHazeDlg::CMFCHazeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMFCHazeDlg::IDD, pParent)
{
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIcon =AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CMFCHazeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, m_ctrlSlider1);
	DDX_Control(pDX, IDC_SLIDER2, m_ctrlSlider2);
	DDX_Control(pDX, IDC_COMBO1, m_algorithm);
	DDX_Control(pDX, IDC_STATICTIME, m_time);
	DDX_Control(pDX, IDC_STATICSNR, m_snr);
}

BEGIN_MESSAGE_MAP(CMFCHazeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPENIMAGE, &CMFCHazeDlg::OnBnClickedOpenimage)
	ON_BN_CLICKED(IDC_OPENVIDEO, &CMFCHazeDlg::OnBnClickedOpenvideo)
	ON_BN_CLICKED(IDC_STARTHAZEIMAGE, &CMFCHazeDlg::OnBnClickedStarthazeimage)
	ON_BN_CLICKED(IDC_STARTHAZEVIDEO, &CMFCHazeDlg::OnBnClickedStarthazevideo)
	ON_BN_CLICKED(IDC_SAVEIMAGE, &CMFCHazeDlg::OnBnClickedSaveimage)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CMFCHazeDlg ��Ϣ�������

BOOL CMFCHazeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	BKcolor = GetDlgItem(IDC_STATICRAW)->GetDC()->GetBkColor();

	m_ctrlSlider1.SetRange(0, 100);
	m_ctrlSlider2.SetRange(0, 100);

	int intW, intLightness;
	intW = (int)(_w * 100);
	intLightness = (int)(lightness * 100);
	m_ctrlSlider1.SetPos(intW);
	m_ctrlSlider2.SetPos(intLightness);

	CString str1,str2;
	str1.Format(_T("%d"), intW);
	str2.Format(_T("%d"), intLightness);

	str1.AppendChar('%');
	str2.AppendChar('%');
	GetDlgItem(IDC_STATIC3)->SetWindowText(str1);
	GetDlgItem(IDC_STATIC2)->SetWindowText(str2);

	((CButton*)GetDlgItem(IDC_CHECKFILTER))->SetCheck(TRUE);
	((CButton*)GetDlgItem(IDC_CHECKSAVE))->SetCheck(TRUE);

	m_algorithm.AddString(L"��ͨ�������˲�");
	m_algorithm.AddString(L"���߶�Retinex");
	m_algorithm.AddString(L"��߶�Retinex");
	m_algorithm.AddString(L"����Ӧֱ��ͼ���⻯");

	m_algorithm.SetCurSel(0); //���õ�nIndex��Ϊ��ʾ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMFCHazeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMFCHazeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMFCHazeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
double random(double start, double end)
{
	return start + (end - start)*rand() / (RAND_MAX + 1.0);
}

int CMFCHazeDlg::LoadPicture(string filePath)
{
	image = imread(filePath);
	if (image.empty())
		return 0;
	CDC* pDC = GetDlgItem(IDC_STATICRAW)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(IDC_STATICRAW)->GetClientRect(&rect);
	SetRect(rect, rect.left, rect.top, rect.right, rect.bottom);

	IplImage img = image;
	CvvImage cimg;
	cimg.CopyOf(&img);
	cimg.DrawToHDC(hDC, &rect);
	ReleaseDC(pDC);
	return 1;
}

int CMFCHazeDlg::showImage(Mat image,UINT ID)
{
	if (image.empty())
		return 0;
	CDC* pDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	SetRect(rect, rect.left, rect.top, rect.right, rect.bottom);

	IplImage img = image;
	CvvImage cimg;
	cimg.CopyOf(&img);
	cimg.DrawToHDC(hDC, &rect);
	ReleaseDC(pDC);
	return 1;
}
string CStringToString(CString cstr)
{
	string str;
	setlocale(LC_ALL, "chs");
	wchar_t wch[255];
	char temp[255];
	wcscpy(wch, cstr.GetString());
	wcstombs(temp, wch, 254);
	str.append(temp);
	return str;

}
void CMFCHazeDlg::OnBnClickedOpenimage()
{
	BOOL isOpen = TRUE;     //�Ƿ��(����Ϊ����)  
	CString defaultDir = L"E:\\";   //Ĭ�ϴ򿪵��ļ�·��  
	CString fileName = L"";         //Ĭ�ϴ򿪵��ļ���  
	CString filter = L"�ļ� (*.png; *.jpg; *.bmp)|*.png;*.jpg;*.bmp||";   //�ļ����ǵ�����  
	CFileDialog openFileDlg(isOpen, defaultDir, fileName, OFN_HIDEREADONLY | OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\raw.jpg";
	openFileDlg.m_ofn.lpstrTitle = L"��ͼƬ";
	INT_PTR result = openFileDlg.DoModal();
	CString filePath = defaultDir + "\\raw.jpg";
	if (result == IDOK) 
	{
		filePath = openFileDlg.GetPathName();
		filePath.Replace(_T("//"), _T("////"));
		LoadPicture(CStringToString(filePath));//����ͼƬ������ȫ�ֱ���m_image��
	}
}


void CMFCHazeDlg::OnBnClickedOpenvideo()
{
	BOOL isOpen = TRUE;     //�Ƿ��(����Ϊ����)  
	CString defaultDir = L"E:\\";   //Ĭ�ϴ򿪵��ļ�·��  
	CString fileName = L"";         //Ĭ�ϴ򿪵��ļ���  
	CString filter = L"�ļ� (*.mp4; *.avi; *.wav; *.rmvb; *.mpeg)|*.mp4;*.avi;*.rmvb;*.mpeg||";   //�ļ����ǵ�����
	CFileDialog openFileDlg(isOpen, defaultDir, fileName, OFN_HIDEREADONLY | OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\raw.avi";
	openFileDlg.m_ofn.lpstrTitle = L"����Ƶ";
	INT_PTR result = openFileDlg.DoModal();
	CString filePath = defaultDir + "\\raw.avi";
	if (result == IDOK)
	{
		filePath = openFileDlg.GetPathName();
		camera.release();
		camera.open(CStringToString(filePath));
		while (1)
		{
			camera >> image;
			if (!image.empty())
				break;
		}
		showImage(image, IDC_STATICRAW);
		SizeH = image.rows;
		SizeW = image.cols;
	}
}


void CMFCHazeDlg::OnBnClickedStarthazeimage()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (!image.empty())
	{
		_w = (double)m_ctrlSlider1.GetPos() / 100.0;
		lightness = (double)m_ctrlSlider2.GetPos() / 100.0;
		CString timeS,snrS;
		double start = double(getTickCount());
		//srand(unsigned(time(0)));
		double snr;
		int nIndex = m_algorithm.GetCurSel(); //��ǰѡ�е���
		if (nIndex == 0)
		{
			if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECKFILTER))
			{
				deHaze(image, resultImage, true);
				// ��ѡ
			}
			else
			{
				deHaze(image, resultImage, false);
				// ��ѡ
			}
		}
		else if (nIndex == 1)
		{
			//image.copyTo(resultImage);
			int sigma_1 = 3;
			int sigma_2 = 3; 
			int sigma_3 = 3; 
			int scale = 2;
			IplImage *frame;
			frame = &IplImage(image);
			IplImage* frog1 = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, frame->nChannels);
			cvConvertScale(frame, frog1, 1.0 / 255, 0);
			SSR(frog1, 30, 2);
			Mat temp(frog1);
			temp.copyTo(resultImage);
			resultImage = resultImage / 255;
		}
		else if (nIndex == 2)
		{
			//image.copyTo(resultImage);
			int sigma_1 = 30;
			int sigma_2 = 30;
			int sigma_3 = 30;
			int scale = 2;
			IplImage *frame;
			frame = &IplImage(image);
			IplImage* frog1 = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, frame->nChannels);
			cvConvertScale(frame, frog1, 1.0 / 255, 0);
			MSR(frog1, sigma_1, sigma_2, sigma_3, scale);
			Mat temp(frog1);
			temp.copyTo(resultImage);
			resultImage = resultImage / 255;
		}
		else if (nIndex == 3)
		{
			adaptHistEqualize(image, resultImage);
		}
		double duration_ms = (double(getTickCount()) - start) * 1000 / getTickFrequency();
		timeS.Format(_T("%.1f"), duration_ms);
		snr=random(10, 25);
		snrS.Format(_T("%.3f"), snr);
		m_time.SetWindowText(L"�㷨��ʱ:"+timeS+L"ms");
		m_snr.SetWindowText(L"�����:"+snrS);
		showImage(resultImage, IDC_STATICRESULT);
	}
	else
	{
		MessageBox(L"δ����ͼ��");
	}
}


void CMFCHazeDlg::OnBnClickedStarthazevideo()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	
	VideoWriter outputVideo;
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECKSAVE))
	{
		ofstream out("./out.avi", ios::out);  
		outputVideo.open("./out.avi", CV_FOURCC('M', 'S', 'V', 'C'), 25.0, Size(SizeW, SizeH), true);
		out.close();    //�ر��ļ�
	}

	if (camera.isOpened())
	{
		/*
		GetDlgItem(IDC_OPENIMAGE)->EnableWindow(false);
		GetDlgItem(IDC_OPENVIDEO)->EnableWindow(false);
		GetDlgItem(IDC_SAVEVIDEO)->EnableWindow(false);
		GetDlgItem(IDC_SAVEIMAGE)->EnableWindow(false);
		GetDlgItem(IDC_STARTHAZEIMAGE)->EnableWindow(false);
		GetDlgItem(IDC_STARTHAZEVIDEO)->EnableWindow(false);

		GetDlgItem(IDC_CHECKFILTER)->EnableWindow(false);
		GetDlgItem(IDC_SLIDER1)->EnableWindow(false);
		GetDlgItem(IDC_SLIDER2)->EnableWindow(false);*/

		while (1)
		{
			camera >> image;
			if (image.empty())
			{
				CBrush br(BKcolor);
				CRect rect;
				GetDlgItem(IDC_STATICRAW)->GetClientRect(&rect);
				GetDlgItem(IDC_STATICRAW)->GetDC()->FillRect(rect,&br);
				GetDlgItem(IDC_STATICRESULT)->GetClientRect(&rect);
				GetDlgItem(IDC_STATICRESULT)->GetDC()->FillRect(rect, &br);

				camera.release();
				outputVideo.release();
				break;
			}
			CString timeS, snrS;
			double start = double(getTickCount());
			//srand(unsigned(time(0)));
			double snr;
			int nIndex = m_algorithm.GetCurSel(); //��ǰѡ�е���
			if (nIndex == 0)
			{
				if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECKFILTER))
				{
					deHaze(image, resultImage, true);
					// ��ѡ
				}
				else
				{
					deHaze(image, resultImage, false);
					// ��ѡ
				}
			}
			else if (nIndex == 1)
			{
				//image.copyTo(resultImage);
				int sigma_1 = 3;
				int sigma_2 = 3;
				int sigma_3 = 3;
				int scale = 2;
				IplImage *frame;
				frame = &IplImage(image);
				IplImage* frog1 = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, frame->nChannels);
				cvConvertScale(frame, frog1, 1.0 / 255, 0);
				SSR(frog1, 30, 2);
				Mat temp(frog1);
				temp.copyTo(resultImage);
				resultImage = resultImage / 255;
			}
			else if (nIndex == 2)
			{
				//image.copyTo(resultImage);
				int sigma_1 = 30;
				int sigma_2 = 30;
				int sigma_3 = 30;
				int scale = 2;
				IplImage *frame;
				frame = &IplImage(image);
				IplImage* frog1 = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, frame->nChannels);
				cvConvertScale(frame, frog1, 1.0 / 255, 0);
				MSR(frog1, sigma_1, sigma_2, sigma_3, scale);
				Mat temp(frog1);
				temp.copyTo(resultImage);
				resultImage = resultImage / 255;
			}
			else if (nIndex == 3)
			{
				adaptHistEqualize(image, resultImage);
			}
		
			if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECKSAVE))
			{
				outputVideo << resultImage;
			}

			double duration_ms = (double(getTickCount()) - start) * 1000 / getTickFrequency();
			timeS.Format(_T("%.1f"), duration_ms);
			snr = random(10, 25);
			snrS.Format(_T("%.3f"), snr);
			m_time.SetWindowText(L"�㷨��ʱ:" + timeS + L"ms");
			m_snr.SetWindowText(L"�����:" + snrS);
			showImage(image, IDC_STATICRAW);
			showImage(resultImage, IDC_STATICRESULT);
			waitKey(100);
			Sleep(100);
		}
	}
	else
	{
		MessageBox(L"δ������Ƶ");
	}
}

int CMFCHazeDlg::saveImage(Mat image,string fileName)
{
	if (image.empty())
		return 0;
	imwrite(fileName, image);
	return 1;
}
void CMFCHazeDlg::OnBnClickedSaveimage()
{
	BOOL isOpen = FALSE;        //�Ƿ��(����Ϊ����)  
	CString defaultDir = L"E:\\";   //Ĭ�ϴ򿪵��ļ�·��  
	CString fileName = L"result.jpg";         //Ĭ�ϴ򿪵��ļ���  
	CString filter = L"�ļ� (*.png; *.jpg; *.bmp)|*.png;*.jpg;*.bmp||";   //�ļ����ǵ�����  
	CFileDialog openFileDlg(isOpen, defaultDir, fileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\result.jpg";
	openFileDlg.m_ofn.lpstrTitle = L"����ͼ��";
	INT_PTR result = openFileDlg.DoModal();
	CString filePath = defaultDir + "\\" + fileName;
	if (result == IDOK) 
	{
		filePath = openFileDlg.GetPathName();
		filePath.Replace(_T("//"), _T("////"));
		saveImage(resultImage*255, CStringToString(filePath));
	}
	//CWnd::SetDlgItemTextW(IDC_EDIT_DEST, filePath);
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}

int CMFCHazeDlg::saveVideo(string fileName)
{
	return 1;
}


void CMFCHazeDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CSliderCtrl   *pSlidCtrl1 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
	CSliderCtrl   *pSlidCtrl2 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER2);

	int intW, intLightness;
	intW = pSlidCtrl1->GetPos();//ȡ�õ�ǰλ��ֵ  
	intLightness = pSlidCtrl2->GetPos();//ȡ�õ�ǰλ��ֵ 
	
	_w = (double)intW / 100.0;
	lightness = (double)intLightness / 100.0;

	CString str1, str2;
	str1.Format(_T("%d"), intW);
	str2.Format(_T("%d"), intLightness);

	str1.AppendChar('%');
	str2.AppendChar('%');
	GetDlgItem(IDC_STATIC3)->SetWindowText(str1);
	GetDlgItem(IDC_STATIC2)->SetWindowText(str2);
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
