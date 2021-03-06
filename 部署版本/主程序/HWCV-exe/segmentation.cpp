/*
主函数文件：segmentation.cpp 主函数的实现文件
*/
#include "segmentation.h"
#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )//没有界面运行

int ComputeImage(vector<string> files, double bzckesa[50][50], double *wcd, int conti)
{
	int i, ii, jj, k, size;
	double	bzcu[50][50] = { 0 };			//标准差中的u
	double	featurep[50][50][30] = { 0 };	//所有图像的轮廓方向特征初始化//干什么	//30
	int		feature[50][50][30] = { 0 };	//所有图像的特征值初始化	//30找出30的位置或者50的位置限制。。。。带入num_dir==49的情况进行类比
	int		featx[50][50] = { 0 };			//循环赋值的feature
	int		featureall;					//图像特征值和				//做什么用

	size = files.size();
	for (i = 0; i < size; i++)
	{
		memset(featx, 0, sizeof(featx));
		singlefeature((char*)files[i].c_str(), featx);				//featx[][50]
		featureall = 0;                                    //图像特征值和的初始化
		for (ii = 0; ii < 48; ii++)                             //将featx存起来,回头看能不能用函数换掉
			for (jj = ii + 1; jj < 47; jj++)
			{
			feature[ii][jj][i] = featx[ii][jj];
			featureall = featureall + featx[ii][jj];
			}
		//求轮廓方向特征featurep，式(5)  与标准差中的u的和
		for (ii = 0; ii < 48; ii++)
			for (jj = ii + 1; jj < 47; jj++)
			{
			featurep[ii][jj][i] = (double)featx[ii][jj] / featureall;
			bzcu[ii][jj] += (double)featx[ii][jj] / featureall * 1000;     //标准差的值过小,进行放大（1）
			}
	}
	//处理完一个人的每一张图片后
	for (ii = 0; ii < 48; ii++)//求标准差中的u
		for (jj = ii + 1; jj < 47; jj++)
			bzcu[ii][jj] = bzcu[ii][jj] / size;
	//求相似性就是带权卡方wcd
	for (i = 0; i < size; i++)
		for (ii = 0; ii < 48; ii++)
			for (jj = ii + 1; jj < 47; jj++)
				if (featurep[ii][jj][i] * featurep[ii][jj][conti] != 0 && bzckesa[ii][jj] != -1)
					wcd[i] += pow((featurep[ii][jj][i] - featurep[ii][jj][conti]), 2) / ((featurep[ii][jj][i] + featurep[ii][jj][conti])*bzckesa[ii][jj]);

	memset(feature, 0, sizeof(feature));
	memset(featurep, 0, sizeof(featurep));

	return 1;
}


/*主函数*/
int main(int argc,char* argv[])
{
	if(argc<2) return -1;
	//for(int iii=0;iii<argc;iii++)
	//	cout<<"argv"<<iii<<" "<<argv[iii]<<endl;
	char *fpname1 = argv[1];

//	cout<<"开始鉴定"<<fpname1<<endl;
	//char *fpname1 = "010209400748";
	/*变量定义*/
	int		i, ii, jj;
	double  bzckesa[50][50] = { 0 };
	double  wcd[30] = { 0 };

	int		featdif[30] = { 0 };
	float	maxx = 0;			//最大特征值的标号与值
	int		xyimgnum = 0;				//嫌疑图片的数目
	char	str[80];					//存储地址

	vector<string> suspict;		//记录嫌疑图片地址
	vector<float> suspict_wcd;	//嫌疑图片的wcd值
	vector<string> files;		//存储该生所有考试文件路径
	vector<string> dateVec, subjectVec, stuNum2;
	vector<string> flagVec;//记录查到的学生的所有考试信息

	/*读取配置文件，并配置各项参数*/
	ReadConfig("D:/HWCV/config/configure.cfg");
//	cout<<"ReadConfig success"<<endl;
	/*初始化log文件*/
	memset(g_log_rec, 0, sizeof(g_log_rec));
	strcat(g_log_rec, GetTime());
//	strcat(g_log_rec, "  配置文件读取完毕 ");
	strcat(g_log_rec, "\t考生考号：");
	strcat(g_log_rec, fpname1);
	strcat(g_log_rec, "\t图片总数为：");

	/*
	string temp=g_db_hostName;
	temp+=g_db_dBName;temp+=g_db_userName;temp+=g_db_password;
	SaveLog((char*)temp.c_str(), g_log_adr, "a");
	*/

	/*步骤：读取标准差文件*/
	int bzccolumns = 47;//txt文件中有47列
	vector<double *> output_bzc;
	if (!ReadScanf("D:\\HWCV\\config\\stdfile.db", bzccolumns, output_bzc)) return 0;
	for (ii = 0; ii < 48; ii++)//output_vector可视为二维数组;输出数组元素：
		for (jj = ii + 1; jj < 47; jj++)
			bzckesa[ii][jj] = output_bzc[ii][jj];

	//查询数据库
	string stuNum = fpname1;
	cout << "开始查询数据库" << endl;
	DbImg(stuNum, dateVec, subjectVec, stuNum2);

	if (dateVec.size() == 0)
	{
		strcat(g_log_rec, "0\n\n");
		SaveLog(g_log_rec, g_log_adr, "a");
		return 0;
	}
	
	//生成路径
	int nn = 0;
	vector<int> img_exist_vector;
	for (int cp_i = 0; cp_i < subjectVec.size(); cp_i++)
	{
		//去除4 9 开头的学号的学生
		/*if (!strcmp(subjectVec[cp_i].substr(0, 1).c_str(), "4") || !strcmp(subjectVec[cp_i].substr(0, 1).c_str(), "9"))
		{
			continue;
		}
		else
		*/
		files.push_back(CrPath(dateVec[cp_i], subjectVec[cp_i], stuNum2[cp_i]));
	}
	cout << "生成路径结束" << endl;

	int size = files.size();/*找到的路径的数量*/

	//开始对每一张图片进行处理
	for (int r = 0; r < size; r++)
	{
		memset(wcd, 0, sizeof(wcd));
		ComputeImage(files, bzckesa, wcd, r);
		xyimgnum = 0;

		//求卡方距离的最大值 
		for (i = 0; i < size; i++)
		{
			if (wcd[i]>0.12)
			{
				flagVec.push_back("1");//嫌疑标记1
				xyimgnum++;
				suspict.push_back(files[i].c_str());
				suspict_wcd.push_back(wcd[i]);
			}
			flagVec.push_back("0");//嫌疑标记1
		}
		if (xyimgnum < 3)
		{
			flagVec.clear();
			break;
		}
	}

	/*结果更新数据库*/
	DbUpdate(stuNum, dateVec, subjectVec, stuNum2, flagVec);
	cout << "数据库更新完毕" << endl;

	/*将结果存入log文件*/
//	memset(g_log_rec, 0, sizeof(g_log_rec));
	/*strcat(g_log_rec, "\t考生考号：");
	strcat(g_log_rec, fpname1);
	strcat(g_log_rec, "\t图片总数为：");*/
	char pic_num[20];
	_itoa(size, pic_num, 10);
	strcat(g_log_rec, pic_num);
	if (xyimgnum > 0)
	{
		strcat(g_log_rec, "\n");
		for (i = 0; i < xyimgnum; i++)
		{
			strcat(g_log_rec, "\t\t\t\t嫌疑图像：");
			strcat(g_log_rec, suspict[i].c_str());
			strcat(g_log_rec, "\t相似度：");
			float sim=(1.0-suspict_wcd[i])*100;
			char a[20];
			sprintf(a, "%g", sim);
			strcat(g_log_rec, a);
			strcat(g_log_rec, "%%\n");
		}
	}
	else strcat(g_log_rec, "\t无嫌疑图像！\n"); 
	strcat(g_log_rec, "\n");

	SaveLog(g_log_rec, g_log_adr, "a");
	cout << "结果存入log文件：" << g_log_adr <<" 完毕"<< endl;


	/*善后*/
	suspict.clear();
	suspict_wcd.clear();
	output_bzc.clear();
	memset(g_log_rec, 0, sizeof(g_log_rec));
	memset(bzckesa, 0, sizeof(bzckesa));
	memset(wcd, 0, sizeof(wcd));
	memset(featdif, 0, sizeof(featdif));
	files.clear();
	dateVec.clear();
	subjectVec.clear();
	stuNum2.clear();
	flagVec.clear();

	/*返回值*/
	return 0;
}


