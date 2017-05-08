#include "stdafx.h"
#include "GetSysInfo.h"


#define  DFP_RECEIVE_DRIVE_DATA   CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
// ATA/ATAPIָ��
#define  IDE_ATA_IDENTIFY           0xEC     // ATA��IDָ��(IDENTIFY DEVICE)

GetSysInfo::GetSysInfo()
{
}


GetSysInfo::~GetSysInfo()
{
}

// �����е��ַ������ߵ�
// ԭ����ATA/ATAPI�е�WORD����Windows���õ��ֽ�˳���෴
// �����������Ѿ����յ�������ȫ��������������������������
void GetSysInfo::AdjustString(char* str, int len)
{
	char ch;
	int i;

	// �����ߵ�
	for (i = 0; i < len; i += 2)
	{
		ch = str[i];
		str[i] = str[i + 1];
		str[i + 1] = ch;
	}

	// �����Ҷ���ģ�����Ϊ����� (ȥ����ߵĿո�)
	i = 0;
	while ((i < len) && (str[i] == ' ')) i++;

	::memmove(str, &str[i], len - i);

	// ȥ���ұߵĿո�
	i = len - 1;
	while ((i >= 0) && (str[i] == ' '))
	{
		str[i] = '/0';
		i--;
	}
}

bool GetSysInfo::InitializeCpuInfo()
{
	{//������
		char str[13] = { 0 };
		unsigned int cpu_info[4] = {};
		__cpuidex(reinterpret_cast<int *>(cpu_info), 0, 0);
		memcpy_s(str, sizeof(str), &cpu_info[1], 4 * 3);
		strcpy(cpu_.Constructor, str);
	}
	{//�̱�
		char str[49] = { 0 };
		unsigned int cpu_info[4] = {};
		for (unsigned long i = 0; i < 3; i++) {
			__cpuidex(reinterpret_cast<int *>(cpu_info), 0x80000002 + i, 0);
			memcpy(str + i * 16, cpu_info, sizeof(cpu_info));
		}
		strcpy(cpu_.Trademarks, str);
	}
	{//����
		unsigned char str[16] = { 0 };
		unsigned int cpu_info[4] = {};
		__cpuidex(reinterpret_cast<int *>(cpu_info), 2, 0);
		memcpy_s(str, sizeof(str), cpu_info, sizeof(cpu_info));
	}
	{//CPU���к�
		char str[200] = { 0 };
		unsigned int cpu_info[4] = {};
		__cpuidex(reinterpret_cast<int *>(cpu_info), 1, 0);
		sprintf_s(str, "%08X%08X", cpu_info[3], cpu_info[0]);
		__cpuidex(reinterpret_cast<int *>(cpu_info), 3, 0);
		sprintf_s(str, "%s%08X%08X", str, cpu_info[3], cpu_info[0]);
		strcpy(cpu_.SerialNumber, str);
	}
	return true;
}

#define INTERFACE_DETAIL_SIZE (1024) 
bool GetSysInfo::InitialazeDiskInfo()
{
	SP_DEVICE_INTERFACE_DATA ifdata;
	PSENDCMDINPARAMS pSCIP;      // �������ݽṹָ��
	PSENDCMDOUTPARAMS pSCOP;     // ������ݽṹָ��
	unsigned long dwOutBytes;            // IOCTL������ݳ���
	bool bResult;                // IOCTL����ֵ
	LPGUID lpGuid = (LPGUID)&DiskClassGuid;

	auto hDevInfoSet = ::SetupDiGetClassDevs(lpGuid,
		/* class GUID*/  NULL,
		/* �޹ؼ��� */ NULL,
		/* ��ָ�������ھ�� */ DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	// Ŀǰ���ڵ��豸 
	// ʧ��... 
	if (hDevInfoSet == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	auto pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT, INTERFACE_DETAIL_SIZE);
	pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
	auto nCount = 0;
	bResult = false;
	// �豸���=0,1,2... ��һ�����豸�ӿڣ���ʧ��Ϊֹ 
	while (!bResult){
		ifdata.cbSize = sizeof(ifdata);
		// ö�ٷ��ϸ�GUID���豸�ӿ� 
		bResult = ::SetupDiEnumDeviceInterfaces(hDevInfoSet,
			/* �豸��Ϣ�����*/ NULL,
			/* ���������豸����*/ lpGuid,
			/* GUID */(ULONG)nCount,
			/* �豸��Ϣ������豸��� */&ifdata);

		if (!bResult) {
			break;
		}

		// ȡ�ø��豸�ӿڵ�ϸ��(�豸·��) 
		bResult = SetupDiGetInterfaceDeviceDetail(hDevInfoSet,
			/* �豸��Ϣ�����*/ &ifdata,
			/* �豸�ӿ���Ϣ*/ pDetail,
			/* �豸�ӿ�ϸ��(�豸·��)*/ INTERFACE_DETAIL_SIZE,
			/* �����������С*/ NULL,
			/* ������������������С(ֱ�����趨ֵ)*/ NULL);
		if (!bResult) {
			nCount++;
			continue;
		}

		if (IdentifyDevice(pDetail->DevicePath)) {
			break;
		}
		bResult = false;
		nCount++;
	}

	::GlobalFree(pDetail);
	// �ر��豸��Ϣ����� 
	::SetupDiDestroyDeviceInfoList(hDevInfoSet);
	return bResult;
}

// ����������IDENTIFY DEVICE���������豸��Ϣ
bool GetSysInfo::IdentifyDevice(wchar_t* szFileName)
{
	PSENDCMDINPARAMS pSCIP;      // �������ݽṹָ��
	PSENDCMDOUTPARAMS pSCOP;     // ������ݽṹָ��
	DWORD dwOutBytes;            // IOCTL������ݳ���
	BOOL bResult;                // IOCTL����ֵ

	auto hDevice = ::CreateFileW(szFileName, // �ļ���
		GENERIC_READ | GENERIC_WRITE,          // ��д��ʽ
		FILE_SHARE_READ | FILE_SHARE_WRITE,    // ����ʽ
		NULL,                    // Ĭ�ϵİ�ȫ������
		OPEN_EXISTING,           // ������ʽ
		0,                       // ���������ļ�����
		NULL);
	if (hDevice == INVALID_HANDLE_VALUE) {
		return false;
	}
								 // ��������/������ݽṹ�ռ�
	pSCIP = (PSENDCMDINPARAMS)::GlobalAlloc(LMEM_ZEROINIT, sizeof(SENDCMDINPARAMS) - 1);
	pSCOP = (PSENDCMDOUTPARAMS)::GlobalAlloc(LMEM_ZEROINIT, sizeof(SENDCMDOUTPARAMS) + sizeof(IDINFO) - 1);

	// ָ��ATA/ATAPI����ļĴ���ֵ
	//    pSCIP->irDriveRegs.bFeaturesReg = 0;
	//    pSCIP->irDriveRegs.bSectorCountReg = 0;
	//    pSCIP->irDriveRegs.bSectorNumberReg = 0;
	//    pSCIP->irDriveRegs.bCylLowReg = 0;
	//    pSCIP->irDriveRegs.bCylHighReg = 0;
	//    pSCIP->irDriveRegs.bDriveHeadReg = 0;
	pSCIP->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;

	// ָ������/������ݻ�������С
	pSCIP->cBufferSize = 0;
	pSCOP->cBufferSize = sizeof(IDINFO);

	// IDENTIFY DEVICE
	bResult = ::DeviceIoControl(hDevice,        // �豸���
		DFP_RECEIVE_DRIVE_DATA,                 // ָ��IOCTL
		pSCIP, sizeof(SENDCMDINPARAMS) - 1,     // �������ݻ�����
		pSCOP, sizeof(SENDCMDOUTPARAMS) + sizeof(IDINFO) - 1,    // ������ݻ�����
		&dwOutBytes,                // ������ݳ���
		(LPOVERLAPPED)NULL);        // ��ͬ��I/O

									// �����豸�����ṹ
	::memcpy(&disk_, pSCOP->bBuffer, sizeof(IDINFO));

	// �ͷ�����/������ݿռ�
	::GlobalFree(pSCOP);
	::GlobalFree(pSCIP);

	return bResult;
}