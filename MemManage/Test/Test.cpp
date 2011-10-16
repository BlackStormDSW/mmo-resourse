// Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "AsynchronousExecute.h"



//! 是否使用系统方式分配内存
//#define USE_C_LIB_ALLOC

inline unsigned long TestTilePool(void);
inline unsigned long TestBlockPool(void);
inline unsigned long GeneralTest(void);
inline unsigned long TestObjPool(void);
LPVOID testMemPool(LPVOID pInParam);

const long g_ThreadNum = 10;
void Print(const char *pFileName, const char *pTxt);
bool bExitInfoThd = false;
LPVOID OutRunningInfo(LPVOID pInParam);

struct tagParam
{
	unsigned long uRandBroud;
	unsigned long uBaseBroud;

	tagParam(void){}
	tagParam(unsigned long uRand, unsigned long uBase):uRandBroud(uRand),uBaseBroud(uBase){}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! main()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long lastSize = 0;

int _tmain(int argc, _TCHAR* argv[])
{

	tagParam *pParam1 = new tagParam(100, 200);
	tagParam *pParam2 = new tagParam[16];

	SAFE_DELETE(pParam1);
	SAFE_DELETE(pParam2);

	srand((unsigned long)time(NULL));

	BBB *pBBBBBB = new BBB;
	void *pVoid = (void*)pBBBBBB;
	delete pBBBBBB;

	SET_OUT_INFO_SETUP(Print, "./");
	
	BBB *pBBB = OBJ_CREATE(BBB);
	pBBB->str = "12345";
	
	OBJ_RELEASE(AAA, pBBB);

	OUT_RUNNING_INFO(FALSE);

	//! 进行第一次调用，自动初始化对象
	//void *pTmp = M_ALLOC(100);
	//M_FREE(pTmp, 100);


#ifdef USE_IN_SYNC_SYSTEM
	
	//! 同步测试
	{
		TestObjPool();

		unsigned long uTileTime = TestTilePool();
		printf("小块内存分配测试平均耗时（ms）：%d\r\n", uTileTime);
		unsigned long uBlockTime = TestBlockPool();
		printf("大块内存分配测试平均耗时（ms）：%d\r\n", uBlockTime);
		unsigned long uGeneralTime = GeneralTest();

		printf("小块内存分配测试平均耗时（ms）：%d\r\n", uTileTime);
		printf("大块内存分配测试平均耗时（ms）：%d\r\n", uBlockTime);
		printf("综合内存分配测试平均耗时（ms）：%d\r\n", uGeneralTime);
		printf("---------------完成---------------\r\n");
	}
#else
	//! 异步测试
	{
		tagParam *pParam0 = (tagParam*)M_ALLOC(sizeof(tagParam));
		pParam0->uRandBroud = 1024 * 16;
		pParam0->uBaseBroud = 2048;
		printf("同步执行时间1：");
		testMemPool(pParam0);
		printf("同步执行时间2：");
		testMemPool(pParam0);
		M_FREE(pParam0, sizeof(tagParam));

		AsynchronousExecute MyAsynchronousExpire(g_ThreadNum, 300, 3000);
		MyAsynchronousExpire.BeginWork(g_ThreadNum);

		MyAsynchronousExpire.Execute(OutRunningInfo, NULL);

		printf("异步执行时间[size: 1~%d byte]：\r\n", 1024 * 1024 * 512);

		tagParam arrParam[g_ThreadNum];
		for (long i = 0; i < g_ThreadNum; ++i)
		{
			arrParam[i].uRandBroud = 1024 * 1024 * 512;
			arrParam[i].uBaseBroud = 2048;
			MyAsynchronousExpire.Execute(testMemPool, &arrParam[i]);
		}

		Sleep(100);
		OUT_RUNNING_INFO(FALSE);
		getchar();
		bExitInfoThd = true;
		MyAsynchronousExpire.Exit();
	}
#endif
	
	getchar();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! other
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long TestTilePool(void)
{
	printf("---------------TilePool Test----------------\r\n");

	static unsigned long uMinGranularity = 4;//MIN_GRANULARITY
	unsigned long arrTileSize[] = 
	{
		uMinGranularity, 
		uMinGranularity * 2,
		uMinGranularity * 3,
		uMinGranularity * 4,
		uMinGranularity * 5,
		uMinGranularity * 5,
		uMinGranularity * 7,
		uMinGranularity * 8,
		uMinGranularity * 9,
		uMinGranularity * 10,
		uMinGranularity * 11,
		uMinGranularity * 11,
		uMinGranularity * 12,
		uMinGranularity * 13,
		uMinGranularity * 14,
		uMinGranularity * 15,
		uMinGranularity * 16,
		uMinGranularity * 17,
		uMinGranularity * 18,
		uMinGranularity * 19,
		uMinGranularity * 20,
		uMinGranularity * 21,
		uMinGranularity * 22,
		uMinGranularity * 23,
		uMinGranularity * 24,
		uMinGranularity * 25,
		uMinGranularity * 26,
		uMinGranularity * 27,
		uMinGranularity * 28,
		uMinGranularity * 29,
		uMinGranularity * 30,
	};

	unsigned long uUseTime = 0;
	for (LONG i = 0; i < sizeof(arrTileSize) / sizeof(long); ++i)
	{
		printf("分配范围（byte）：%u ~ %u 耗时（ms）：", arrTileSize[i], arrTileSize[i] + uMinGranularity - 1);

		tagParam *pParam = (tagParam*)M_ALLOC(sizeof(tagParam));
		pParam->uRandBroud = uMinGranularity;
		pParam->uBaseBroud = arrTileSize[i];
		uUseTime += (unsigned long)testMemPool(pParam);
		M_FREE(pParam, sizeof(tagParam));
	}
	return uUseTime / (sizeof(arrTileSize) / sizeof(long));
}
unsigned long TestBlockPool(void)
{
	printf("---------------BlockPool Test----------------\r\n");

	static unsigned long uBlockBaseSize = 32 * 4 * 4;
	unsigned long arrBlockSize[] = 
	{
		uBlockBaseSize, //! BLOCK_BSAE_SIZE
		uBlockBaseSize * 2,
		uBlockBaseSize * 4,
		uBlockBaseSize * 8,
		uBlockBaseSize * 16,
		uBlockBaseSize * 32,
		uBlockBaseSize * 64,
	};

	unsigned long uUseTime = 0;
	for (LONG i = 0; i < sizeof(arrBlockSize) / sizeof(long); ++i)
	{
		unsigned long uAllocCount = arrBlockSize[i] / uBlockBaseSize + ((arrBlockSize[i] % uBlockBaseSize) ? 1 : 0);
		printf("分配范围（byte）：%u ~ %u 快数：%u 耗时（ms）：", arrBlockSize[i] - 20, arrBlockSize[i], uAllocCount);

		tagParam *pParam = (tagParam*)M_ALLOC(sizeof(tagParam));
		pParam->uRandBroud = 20;
		pParam->uBaseBroud = arrBlockSize[i] - 19;
		uUseTime += (unsigned long)testMemPool(pParam);
		M_FREE(pParam, sizeof(tagParam));
	}
	return uUseTime / (sizeof(arrBlockSize) / sizeof(long));
}

unsigned long GeneralTest(void)
{
	printf("---------------General Test----------------\r\n");

	const unsigned long uTestCount = 1024 * 1024 * 1024 / 2048 / 512;
	unsigned long uUseTime = 0;
	for (unsigned long i = 0; i < uTestCount; ++i)
	{
		unsigned long uTmpSize = (i + 1) * 2048;
		printf("分配范围（byte）：1 ~ %u 耗时（ms）：", uTmpSize);

		tagParam *pParam = (tagParam*)M_ALLOC(sizeof(tagParam));
		pParam->uRandBroud = uTmpSize;
		pParam->uBaseBroud = 1;
		uUseTime += (unsigned long)testMemPool(pParam);
		M_FREE(pParam, sizeof(tagParam));
	}

	return uUseTime / uTestCount;
}

LPVOID testMemPool(LPVOID pInParam)
{
	static long c_count = 0;
	static long c_time = 0;
	static long c_thrdtime = 0;
	static XM_Tools::LockOfWinWhile MyLock;

	MyLock.Lock();
	if(2 == c_count)
	{
		c_time = timeGetTime();
	}
	MyLock.UnLock();

	tagParam *pParam = (tagParam*)pInParam;
	assert(1 <= pParam->uRandBroud && 1 <= pParam->uBaseBroud);

	LONG re = 0;

	for (LONG j = 0; j < 1000; ++j)
	{
		map<LPVOID, LONG> mapP;
		long ltime = timeGetTime();
		for (LONG i = 0; i < 100; ++i)
		{
			LONG size = (rand() % pParam->uRandBroud) + pParam->uBaseBroud;
#ifdef USE_C_LIB_ALLOC
			LPVOID p = malloc(size);
			//LPVOID p = new char[size];
#else
			LPVOID p = M_ALLOC(size);
#endif
			lastSize = size;
			memset(p, 0xbb, size);
			mapP[p] = size;
		}

		for (map<LPVOID, LONG>::iterator ite = mapP.begin(); ite != mapP.end(); ++ite)
		{
#ifdef USE_C_LIB_ALLOC
			free(ite->first);
			//delete[] ite->first;
#else
			LPVOID pTemp = ite->first;
			M_FREE(pTemp, ite->second);
#endif
		}
		re += timeGetTime() - ltime;
		mapP.clear();
	}
	
	printf("%d\r\n", re);

	MyLock.Lock();
	++c_count;
	if(2 < c_count)
		c_thrdtime += re;
	if (g_ThreadNum + 2 <= c_count)
	{
		printf("所有任务完成，线程耗时%d 总耗时%d c_count=%d\r\n", c_thrdtime, timeGetTime() - c_time, c_count);
	}
	MyLock.UnLock();

	return (LPVOID)re;
}

LPVOID OutRunningInfo(LPVOID pInParam)
{
	while (!bExitInfoThd)
	{
		Sleep(200);
		//OUT_RUNNING_INFO();
	}

	return NULL;
}

class CObjA
{
	char szData[50];
};

class CObjB
{
	char szData[111];
};

class CObjC
{
	char szData[22];
};

class CObjD
{
	char szData[8];
};

unsigned long TestObjPool(void)
{
	set<CObjA*> mapObj;
	long ltime = timeGetTime();
	for (LONG i = 0; i < 50000; ++i)
	{
		CObjA *pCObjA1 = OBJ_CREATE(CObjA);
		mapObj.insert(pCObjA1);
	}

	for (set<CObjA*>::iterator ite = mapObj.begin(); ite != mapObj.end(); ++ite)
	{
		OBJ_RELEASE(CObjA, (*ite));
	}

	ltime = timeGetTime() - ltime;
	mapObj.clear();

	printf("%d\r\n", ltime);

	return ltime;

	return 0;
}


void Print(const char *pFileName, const char *pTxt)
{
	if(NULL == pTxt)
		return;

	//printf("%s", pTxt);

	if (NULL != pFileName)
	{
		char strFile[MAX_PATH];
		SYSTEMTIME stTime;
		GetLocalTime(&stTime);
		sprintf(strFile, "%04d-%02d-%02d.txt", (long)stTime.wYear, (long)stTime.wMonth, (long)stTime.wDay);

		FILE *fp;
		if( (fp=fopen(strFile, "a+")) == NULL )
		{
			return;
		}

		fseek(fp, 0, SEEK_END);
		fwrite(pTxt, strlen(pTxt), 1, fp);
		fclose(fp);
	}
}