#include "stdafx.h"
#include "guidedfilter.h"
#include "deHaze.h"

//#pragma comment( lib, "opencv_world310d.lib" ) 
using namespace std;
using namespace cv;


int _PriorSize = 10;			    //���ڴ�С,��ʴʱ�õĴ��ڴ�С
double _topbright = 0.001;		//������ߵ����ر���,�̶�0.1%
double _w = 0.99;				//w,Ϊ������ı��� Խ��ȥ��Խ��
double lightness = 0.1;			//���ȵ���
float t0 = 0.01;				//T(x)����Сֵ   ��Ϊ������txС��0 ����0 Ч������
int SizeH = 0;					//ͼƬ�߶�
int SizeW = 0;					//ͼƬ���
int SizeH_W = 0;				//ͼƬ�е������� �� H*W
Vec<float, 3> a;				//ȫ������Ĺ���ֵ
Mat trans_refine;
Mat dark_out1;
Mat img;

//����ͼƬ
Mat ReadImage(string fileName)
{

	Mat img = imread(fileName);

	SizeH = img.rows;
	SizeW = img.cols;
	SizeH_W = img.rows*img.cols;

	Mat real_img(img.rows, img.cols, CV_32FC3);
	img.convertTo(real_img, CV_32FC3);

	real_img = real_img / 255;

	return real_img;


	//����ͼƬ ����ת��Ϊ3ͨ���ľ���� 
	//����255 ����RBGȷ����0-1֮��
}



//���㰵ͨ��
//J^{dark}(x)=min( min( J^c(y) ) )
Mat DarkChannelPrior(Mat img)
{
	Mat dark = Mat::zeros(img.rows, img.cols, CV_32FC1);//�½�һ������Ԫ��Ϊ0�ĵ�ͨ���ľ���

	for (int i = 0; i<img.rows; i++)
	{
		for (int j = 0; j<img.cols; j++)
		{

			dark.at<float>(i, j) = min(
				min(img.at<Vec<float, 3>>(i, j)[0], img.at<Vec<float, 3>>(i, j)[1]),
				min(img.at<Vec<float, 3>>(i, j)[0], img.at<Vec<float, 3>>(i, j)[2])
				);//����������Сֵ�Ĺ���
		}
	}
	erode(dark, dark_out1, Mat::ones(_PriorSize, _PriorSize, CV_32FC1));//��������и�ʴ �����Ǵ��ڴ�С��ģ������ ,��Ӧ������Сֵ�˲�,�� ��ɫͼ���е�һ���Ķ���

	return dark_out1;//����dark_out1�õ���ȫ�ֱ�������Ϊ�������ط�ҲҪ�õ�
}
Mat DarkChannelPrior_(Mat img)//��������ڼ���tx�õ�����Ϊ����㰵ͨ��һ�����õ�������Сֵ�Ĺ��̣��仯���࣬���Ը��˸ľ���������
{
	double A = (a[0] + a[1] + a[2]) / 3.0;//ȫ���������ֵ �˴���3ͨ����ƽ��ֵ

	Mat dark = Mat::zeros(img.rows, img.cols, CV_32FC1);
	Mat dark_out = Mat::zeros(img.rows, img.cols, CV_32FC1);
	for (int i = 0; i<img.rows; i++)
	{
		for (int j = 0; j<img.cols; j++)
		{
			dark.at<float>(i, j) = min(
				min(img.at<Vec<float, 3>>(i, j)[0] / A, img.at<Vec<float, 3>>(i, j)[1] / A),
				min(img.at<Vec<float, 3>>(i, j)[0] / A, img.at<Vec<float, 3>>(i, j)[2] / A)
				);//ͬ��


		}
	}
	erode(dark, dark_out, Mat::ones(_PriorSize, _PriorSize, CV_32FC1));//ͬ��

	return dark_out;

}


//����A��ֵ
Vec<float, 3> Airlight(Mat img, Mat dark)//vec<float ,3>��ʾ��3����С��vector ����Ϊfloat
{
	int n_bright = _topbright*SizeH_W;

	Mat dark_1 = dark.reshape(1, SizeH_W);//����dark_1��һ����ͼƬ������ô���еľ��� ��������ѭ������

	vector<int> max_idx;

	float max_num = 0;

	Vec<float, 3> A(0, 0, 0);
	Mat RGBPixcels = Mat::ones(n_bright, 1, CV_32FC3);

	for (int i = 0; i<n_bright; i++)
	{
		max_num = 0;
		max_idx.push_back(max_num);
		for (float * p = (float *)dark_1.datastart; p != (float *)dark_1.dataend; p++)
		{
			if (*p>max_num)
			{
				max_num = *p;//��¼���յ����ֵ

				max_idx[i] = (p - (float *)dark_1.datastart);//λ��

				RGBPixcels.at<Vec<float, 3>>(i, 0) = ((Vec<float, 3> *)img.data)[max_idx[i]];//��Ӧ ������ͨ����ֵ��RGBPixcels

			}
		}
		((float *)dark_1.data)[max_idx[i]] = 0;//���ʹ��ı��Ϊ0�������Ͳ����ظ�����
	}


	for (int j = 0; j<n_bright; j++)
	{

		A[0] += RGBPixcels.at<Vec<float, 3>>(j, 0)[0];
		A[1] += RGBPixcels.at<Vec<float, 3>>(j, 0)[1];
		A[2] += RGBPixcels.at<Vec<float, 3>>(j, 0)[2];

	}//������ֵ�ۼ�

	A[0] /= n_bright;
	A[1] /= n_bright;
	A[2] /= n_bright;//��������   ��ȡ���з��ϵĵ��ƽ��ֵ��

	return A;
}


//Calculate Transmission Matrix
Mat TransmissionMat(Mat dark)
{
	double A = (a[0] + a[1] + a[2]) / 3.0;
	for (int i = 0; i < dark.rows; i++)
	{
		for (int j = 0; j < dark.cols; j++)
		{
			double temp = (dark_out1.at<float>(i, j));
			double B = fabs(A - temp);
			//	conut++;
			//cout << conut << endl;
			//if (B==)
			if (B - 0.3137254901960784 < 0.0000000000001)//K=80    80/255=0.31   ���︡����Ҫ����������������ȷ�ıȽ�
			{
				dark.at<float>(i, j) = (1 - _w*dark.at<float>(i, j))*
					(0.3137254901960784 / (B));//�˴�Ϊ�Ĺ���ʽ�Ӳ���
			}
			else
			{
				dark.at<float>(i, j) = 1 - _w*dark.at<float>(i, j);
			}
			if (dark.at<float>(i, j) <= 0.2)//��֤Tx��ʧ�棬��Ϊ�����ϳ����Ľ�����в���
			{
				dark.at<float>(i, j) = 0.5;
			}
			if (dark.at<float>(i, j) >= 1)//ͬ��
			{
				dark.at<float>(i, j) = 1.0;
			}

		}
	}

	return dark;
}
Mat TransmissionMat1(Mat dark)
{
	double A = (a[0] + a[1] + a[2]) / 3.0;
	for (int i = 0; i < dark.rows; i++)
	{
		for (int j = 0; j < dark.cols; j++)
		{

			dark.at<float>(i, j) = (1 - _w*dark.at<float>(i, j));

		}
	}

	return dark;
}
//Calculate Haze Free Image
Mat hazefree(Mat img, Mat t, Vec<float, 3> a, float exposure = 0)//�˴���exposure��ֵ��ʾȥ���Ӧ�ü�����ֵ��
{
	double AAA = a[0];
	if (a[1] > AAA)
		AAA = a[1];
	if (a[2] > AAA)
		AAA = a[2];
	//ȡa�е�����ֵ


	//�¿�һ������
	Mat freeimg = Mat::zeros(SizeH, SizeW, CV_32FC3);
	img.copyTo(freeimg);

	//������������������д�����Բ�������ѭ�����ȽϿ��
	Vec<float, 3> * p = (Vec<float, 3> *)freeimg.datastart;
	float * q = (float *)t.datastart;

	for (; p<(Vec<float, 3> *)freeimg.dataend && q<(float *)t.dataend; p++, q++)
	{
		(*p)[0] = ((*p)[0] - AAA) / std::max(*q, t0) + AAA + exposure;
		(*p)[1] = ((*p)[1] - AAA) / std::max(*q, t0) + AAA + exposure;
		(*p)[2] = ((*p)[2] - AAA) / std::max(*q, t0) + AAA + exposure;
	}

	return freeimg;
}


void printMatInfo(char * name, Mat m)
{
	cout << name << ":" << endl;
	cout << "\t" << "cols=" << m.cols << endl;
	cout << "\t" << "rows=" << m.rows << endl;
	cout << "\t" << "channels=" << m.channels() << endl;
}

int deHaze(String fileName, Mat& dst)
{
	clock_t start, finish;
	double time;
	Mat dark_channel;
	Mat trans;
	Mat free_img;
	start = clock();

	Mat raw = imread(fileName);
	if (raw.empty())
		return 0;

	SizeH = raw.rows;
	SizeW = raw.cols;
	SizeH_W = raw.rows*raw.cols;

	Mat real_img(raw.rows, raw.cols, CV_32FC3);
	raw.convertTo(real_img, CV_32FC3);

	real_img = real_img / 255;
	real_img.copyTo(img);

	//���㰵ͨ��
	//cout << "���㰵ͨ�� ..." << endl;
	dark_channel = DarkChannelPrior(img);


	//����ȫ�����ֵ
	//cout << "����Aֵ ..." << endl;
	a = Airlight(img, dark_channel);


	//����tx
	//cout << "Reading Refine Transmission..." << endl;
	trans_refine = TransmissionMat(DarkChannelPrior_(img));


	//�����˲� �õ���ϸ��͸����ͼ
	Mat tran = guidedFilter(img, trans_refine, 60, 0.0001);

	//ȥ��
	//cout << "Calculating Haze Free Image ..." << endl;
	/*
	�˴� �����tran�Ļ����ǵ����˲�����
	�����trans_refine ��û���õ����˲� Ч��������ô�ĺ�
	������ĸ������������������ȵģ�0.1�ȽϺ�
	*/
	free_img = hazefree(img, tran, a, 0.1);
	
	//����ʹ��ʱ��
	finish = clock();
	time = (double)(finish - start) / CLOCKS_PER_SEC;
	cout << "Total Time Cost: " << time << "s" << endl;

	//��ʾ
	imshow("ԭͼ", img);
	imshow("ȥ���", free_img);
	//����ͼƬ�Ĵ���

	//imwrite("output.jpg", free_img * 255);

	waitKey();
	cout << endl;
	return 1;
}

int deHaze(Mat src, Mat& dst,bool ifFilter)
{
	clock_t start, finish;
	double time;
	Mat dark_channel;
	Mat trans;
	Mat free_img;

	img.release();
	start = clock();

	if (src.empty())
		return 0;

	SizeH = src.rows;
	SizeW = src.cols;
	SizeH_W = src.rows*src.cols;

	Mat real_img(src.rows, src.cols, CV_32FC3);
	src.convertTo(real_img, CV_32FC3);

	real_img = real_img / 255;
	real_img.copyTo(img);

	//���㰵ͨ��
	//cout << "���㰵ͨ�� ..." << endl;
	dark_channel = DarkChannelPrior(img);


	//����ȫ�����ֵ
	//cout << "����Aֵ ..." << endl;
	a = Airlight(img, dark_channel);


	//����tx
	//cout << "Reading Refine Transmission..." << endl;
	trans_refine = TransmissionMat(DarkChannelPrior_(img));


	//�����˲� �õ���ϸ��͸����ͼ
	Mat tran = guidedFilter(img, trans_refine, 60, 0.0001);

	//ȥ��
	//cout << "Calculating Haze Free Image ..." << endl;
	/*
	�˴� �����tran�Ļ����ǵ����˲�����
	�����trans_refine ��û���õ����˲� Ч��������ô�ĺ�
	������ĸ������������������ȵģ�0.1�ȽϺ�
	*/
	if (ifFilter)
		free_img = hazefree(img, tran, a, lightness);
	else
		free_img = hazefree(img, trans_refine, a, lightness);
	//����ʹ��ʱ��
	finish = clock();
	time = (double)(finish - start) / CLOCKS_PER_SEC;

	free_img.copyTo(dst);
	//cout << "Total Time Cost: " << time << "s" << endl;

	//��ʾ
	//imshow("ԭͼ", img);
	//imshow("ȥ���", free_img);
	//����ͼƬ�Ĵ���

	//imwrite("output.jpg", free_img * 255);

	//waitKey(33);
	//cout << endl;
	return 1;
}

//����Ӧֱ��ͼȥ��
void adaptHistEqualize(Mat src, Mat& dst)
{
	cv::Mat clahe_img;
	cv::cvtColor(src, clahe_img, CV_BGR2Lab);
	std::vector<cv::Mat> channels(3);
	cv::split(clahe_img, channels);

	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
	clahe->setClipLimit(4);
	cv::Mat dst1;
	clahe->apply(channels[0], dst1);
	dst1.copyTo(channels[0]);
	cv::merge(channels, clahe_img);

	cv::cvtColor(clahe_img, dst, CV_Lab2BGR);
}
//��ֵȥ���㷨
void MedianBlurHaze(unsigned char * Scan0, int Width, int Height, int Stride, int DarkRadius, int MedianRadius, int P)
{
	int  X, Y, Diff, Min, F;
	unsigned char* Pointer, *DarkP, *FilterP, *FilterPC;
	unsigned char * DarkChannel = (unsigned char*)malloc(Width * Height);
	unsigned char * Filter = (unsigned char*)malloc(Width * Height);
	unsigned char * FilterClone = (unsigned char*)malloc(Width * Height);

	for (Y = 0; Y < Height; Y++)
	{
		Pointer = Scan0 + Y * Stride;
		DarkP = DarkChannel + Y * Width;             // ��ʵ��ͼ�����õ���ͼ��ͨ��     
		for (X = 0; X < Width; X++)
		{
			Min = *Pointer;
			if (Min > *(Pointer + 1)) Min = *(Pointer + 1);
			if (Min > *(Pointer + 2)) Min = *(Pointer + 2);
			*DarkP = (unsigned char)Min;
			DarkP++;
			Pointer += 3;
		}
	}
	memcpy(Filter, DarkChannel, Width * Height);                        // ��ȫ�ִ�����Aʱ���ƻ�DarkChannel�е�����

	//MinValue(DarkChannel, Width, Height, Width, DarkRadius);                // ��ȡ��ͨ��ֵ

	// ���ð�ͨ��������ȫ�ִ�����ֵA
	int Sum, Value, Threshold = 0;
	int SumR = 0, SumG = 0, SumB = 0, AtomR, AtomB, AtomG, Amount = 0;
	int* Histgram = (int*)calloc(256, sizeof(int));
	for (Y = 0; Y < Width * Height; Y++) Histgram[DarkChannel[Y]]++;
	for (Y = 255, Sum = 0; Y >= 0; Y--)
	{
		Sum += Histgram[Y];
		if (Sum > Height * Width * 0.01)
		{
			Threshold = Y;                                        // ѡȡ��ͨ��ֵ��ǰ1%��������������Ϊ��ѡ��
			break;
		}
	}
	AtomB = 0; AtomG = 0; AtomR = 0;
	for (Y = 0, DarkP = DarkChannel; Y < Height; Y++)
	{
		Pointer = Scan0 + Y * Stride;
		for (X = 0; X < Width; X++)
		{
			if (*DarkP >= Threshold)                            //    ��ԭͼ��ѡ�������ѡ���λ�õ�������Ϊ����ȫ�ִ�����A����Ϣ                        
			{
				SumB += *Pointer;
				SumG += *(Pointer + 1);
				SumR += *(Pointer + 2);
				Amount++;
			}
			Pointer += 3;
			DarkP++;
		}
	}
	AtomB = SumB / Amount;
	AtomG = SumG / Amount;
	AtomR = SumR / Amount;

	memcpy(DarkChannel, Filter, Width * Height);                        // �ָ�DarkChannel�е�����
	//MedianBlur(Filter, Width, Height, Width, MedianRadius, 50);          // ����1��ʹ����ֵ�˲�ƽ���������������Ҫ������ƽ����ͬʱ������ͼ���еı߽粿�֣�����ʵ����������ֵ�˲����ø�˹�˲�Ч���о����
	memcpy(FilterClone, Filter, Width * Height);

	DarkP = DarkChannel;
	FilterP = Filter;
	for (Y = 0; Y < Height * Width; Y++)              //����һ��ѭ������������ٶ�
	{
		Diff = *DarkP - *FilterP;                    //ͨ����|DarkP ��FilterP |ִ����ֵ�˲������Ƶľֲ���׼��������Ա�֤��׼����Ƶ�³����
		if (Diff < 0) Diff = -Diff;
		*FilterP = (unsigned char)Diff;
		DarkP++;
		FilterP++;
	}
	//MedianBlur(Filter, Width, Height, Width, MedianRadius, 50);

	FilterPC = FilterClone;
	FilterP = Filter;
	for (Y = 0; Y < Height * Width; Y++)
	{
		Diff = *FilterPC - *FilterP;                    // ����2��Ȼ���ǵ��нϺöԱȶȵ������������û���� �ⲿ������Ͳ���Ҫ��ȥ����
		if (Diff < 0) Diff = 0;                            // �����������������Ϊ������и�max(....,0)�Ĺ��̣�
		*FilterP = (unsigned char)Diff;
		FilterPC++;
		FilterP++;
	}

	DarkP = DarkChannel;
	FilterP = Filter;

	for (Y = 0; Y < Height * Width; Y++)
	{
		Min = *FilterP * P / 100;
		if (*DarkP > Min)
			*FilterP = Min;                                // �������Լ�������Ĵ�����Ļ
		else
			*FilterP = *DarkP;
		DarkP++;
		FilterP++;
	}

	FilterP = Filter;
	for (Y = 0; Y < Height; Y++)
	{
		Pointer = Scan0 + Y * Stride;
		for (X = 0; X < Width; X++)
		{
			F = *FilterP++;
			if (AtomB != F)
				Value = AtomB *(*Pointer - F) / (AtomB - F);
			else
				Value = *Pointer;
			//*Pointer++ = Clamp(Value);
			if (AtomG != F)
				Value = AtomG * (*Pointer - F) / (AtomG - F);
			else
				Value = *Pointer;
			//*Pointer++ = Clamp(Value);
			if (AtomR != F)
				Value = AtomR *(*Pointer - F) / (AtomR - F);
			else
				Value = *Pointer;
			//*Pointer++ = Clamp(Value);
		}
	}
	free(Histgram);
	free(Filter);
	free(DarkChannel);
	free(FilterClone);
}
/********************************************************************************
���߶�Retinexͼ����ǿ����
srcΪ������ͼ��
sigmaΪ��˹ģ����׼��
scaleΪ�Աȶ�ϵ��
*********************************************************************************/
void SSR(IplImage* src, int sigma, int scale)
{
	IplImage* src_fl = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);
	IplImage* src_fl1 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);
	IplImage* src_fl2 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);
	float a = 0.0, b = 0.0, c = 0.0;
	cvConvertScale(src, src_fl, 1.0, 1.0);//ת����Χ������ͼ��Ԫ������1.0��֤cvlog����
	cvLog(src_fl, src_fl1);

	cvSmooth(src_fl, src_fl2, CV_GAUSSIAN, 0, 0, sigma);        //SSR�㷨�ĺ���֮һ����˹ģ��

	cvLog(src_fl2, src_fl2);
	cvSub(src_fl1, src_fl2, src_fl);//Retinex��ʽ��Log(R(x,y))=Log(I(x,y))-Log(Gauss(I(x,y)))

	//����ͼ��ľ�ֵ�����SSR�㷨�ĺ���֮��
	//ʹ��GIMP��ת��������ʹ��ͼ��ľ�ֵ�������Ϣ���б任
	//û���������ж�
	CvScalar mean;
	CvScalar dev;
	cvAvgSdv(src_fl, &mean, &dev, NULL);//����ͼ��ľ�ֵ�ͱ�׼��
	double min[3];
	double max[3];
	double maxmin[3];
	for (int i = 0; i<3; i++)
	{
		min[i] = mean.val[i] - scale*dev.val[i];
		max[i] = mean.val[i] + scale*dev.val[i];
		maxmin[i] = max[i] - min[i];
	}
	float* data2 = (float*)src_fl->imageData;
	for (int i = 0; i<src_fl2->width; i++)
	{
		for (int j = 0; j<src_fl2->height; j++)
		{
			data2[j*src_fl->widthStep / 4 + 3 * i + 0] = 255 * (data2[j*src_fl->widthStep / 4 + 3 * i + 0] - min[0]) / maxmin[0];
			data2[j*src_fl->widthStep / 4 + 3 * i + 1] = 255 * (data2[j*src_fl->widthStep / 4 + 3 * i + 1] - min[1]) / maxmin[1];
			data2[j*src_fl->widthStep / 4 + 3 * i + 2] = 255 * (data2[j*src_fl->widthStep / 4 + 3 * i + 2] - min[2]) / maxmin[2];
		}
	}


	cvConvertScale(src_fl, src, 1, 0);
	cvReleaseImage(&src_fl);
	cvReleaseImage(&src_fl1);
	cvReleaseImage(&src_fl2);
}

/********************************************************************************
��߶�Retinexͼ����ǿ����  ��һ��ѡ��3�߶ȣ�
srcΪ������ͼ��
sigmaΪ��˹ģ����׼��
scaleΪ�Աȶ�ϵ��
*********************************************************************************/
void MSR(IplImage* src, int sigma_1, int sigma_2, int sigma_3, int scale)
{

	IplImage* src_fl = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);
	IplImage* src_fl1 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);
	IplImage* src_fl2 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);

	IplImage* src1_fl = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);
	IplImage* src1_fl1 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);
	IplImage* src1_fl2 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);

	IplImage* src2_fl = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);
	IplImage* src2_fl1 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);
	IplImage* src2_fl2 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, src->nChannels);




	cvConvertScale(src, src_fl, 1.0, 1.0);//ת����Χ������ͼ��Ԫ������1.0��֤cvlog����
	src1_fl = cvCloneImage(src_fl);
	src2_fl = cvCloneImage(src_fl);

	cvLog(src_fl, src_fl1);
	cvLog(src1_fl, src1_fl1);
	cvLog(src2_fl, src2_fl1);

	cvSmooth(src_fl, src_fl2, CV_GAUSSIAN, 0, 0, sigma_1);        //MSR�㷨�ĺ���֮һ����˹ģ��
	cvSmooth(src1_fl, src1_fl2, CV_GAUSSIAN, 0, 0, sigma_2);
	cvSmooth(src2_fl, src2_fl2, CV_GAUSSIAN, 0, 0, sigma_3);

	cvLog(src_fl2, src_fl2);
	cvLog(src1_fl2, src1_fl2);
	cvLog(src2_fl2, src2_fl2);

	cvSub(src_fl1, src_fl2, src_fl);//Retinex��ʽ��Log(R(x,y))=Log(I(x,y))-Log(Gauss(I(x,y)))
	cvSub(src1_fl1, src1_fl2, src1_fl1);
	cvSub(src2_fl1, src2_fl2, src2_fl1);

	cvConvertScale(src_fl, src_fl, 1.0 / 3.0, 0.0);//ÿ���߶ȶ�Ӧ��ȨֵΪ1/3
	cvConvertScale(src1_fl1, src1_fl1, 1.0 / 3.0, 0.0);
	cvConvertScale(src2_fl1, src2_fl1, 1.0 / 3.0, 0.0);

	cvAdd(src_fl, src1_fl1, src1_fl1);
	cvAdd(src1_fl1, src2_fl1, src2_fl1);

	//����ͼ��ľ�ֵ�����MSR�㷨�ĺ���֮��
	//ʹ��GIMP��ת��������ʹ��ͼ��ľ�ֵ�������Ϣ���б任
	//û���������ж�
	CvScalar mean;
	CvScalar dev;
	cvAvgSdv(src2_fl1, &mean, &dev, NULL);//����ͼ��ľ�ֵ�ͱ�׼��
	double min[3];
	double max[3];
	double maxmin[3];
	for (int i = 0; i<3; i++)
	{
		min[i] = mean.val[i] - scale*dev.val[i];
		max[i] = mean.val[i] + scale*dev.val[i];
		maxmin[i] = max[i] - min[i];
	}
	float* data2 = (float*)src2_fl1->imageData;
	for (int i = 0; i<src2_fl1->width; i++)
	{
		for (int j = 0; j<src2_fl1->height; j++)
		{
			data2[j*src2_fl1->widthStep / 4 + 3 * i + 0] = 255 * (data2[j*src2_fl1->widthStep / 4 + 3 * i + 0] - min[0]) / maxmin[0];
			data2[j*src2_fl1->widthStep / 4 + 3 * i + 1] = 255 * (data2[j*src2_fl1->widthStep / 4 + 3 * i + 1] - min[1]) / maxmin[1];
			data2[j*src2_fl1->widthStep / 4 + 3 * i + 2] = 255 * (data2[j*src2_fl1->widthStep / 4 + 3 * i + 2] - min[2]) / maxmin[2];
		}
	}


	cvConvertScale(src2_fl1, src, 1, 0);
	cvReleaseImage(&src_fl);
	cvReleaseImage(&src_fl1);
	cvReleaseImage(&src_fl2);
	cvReleaseImage(&src1_fl);
	cvReleaseImage(&src1_fl1);
	cvReleaseImage(&src1_fl2);
	cvReleaseImage(&src2_fl);
	cvReleaseImage(&src2_fl1);
	cvReleaseImage(&src2_fl2);

}