#include "stdafx.h"
#include <windows.h>
#include <Shlwapi.h>
#include <winioctl.h>
#include <string>
#include "WinSvc.h"   //shupb
#include "IMProtect.h"
//#include "Function.h" //shupb

typedef struct
{
	ULONGLONG m_hEvent;
	ULONGLONG m_hNotify;
	ULONGLONG m_uPass;
}NOTIFY_HANDLE;

typedef struct  
{
	unsigned long ProcessId;
	WCHAR ProcessInfo[260];
	WCHAR VisitInfo[5][260];
}TRY_SOKE;

typedef bool(WINAPI *Wow64DisableWow64FsRedirectionFun)(PVOID *OldValue);
typedef bool(WINAPI *Wow64RevertWow64FsRedirectionFun)(PVOID *OldValue);

#define IOCTL_MPROTECT_EVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_UNEVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_USERCHOICE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_GET_TRY_SOKE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MPROTECT_RESET_PROCESS_HIDE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_ADD_PROTECTION_HIDE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_RESET_PROCESS_POTECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_ADD_PROCESS_POTECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x813, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_TRY_SOKE 0x80000000

NOTIFY_HANDLE m_NotifyHandle;
PVOID m_pOldValue;
std::wstring m_DriverPath;
std::wstring GetAppFolder();

IMProtect::IMProtect()
{
}


IMProtect::~IMProtect()
{
	Close();
}

/*
����ֵ��
NULL	����ȡʧ�ܣ���ʹ��GetLastError��ȡ������Ϣ
Windows 8				��62
Windows 7				��61
Windows Server 2008 R2	��61
Windows Server 2008		��60
Windows Vista			��60
Windows Server 2003 R2	��52
Windows Server 2003		��52
Windows XP				��51
Windows 2000			��50
ע�⣬�˺����ر���Error(4996)�Ĵ��󾯸�
*/
float GetVistaOrLater()
{
	float vi = 0.0;
	__try
	{
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (GetVersionEx(&osvi)) {
			vi = osvi.dwMajorVersion * 10 + osvi.dwMinorVersion;
			return vi;
		}else
			return NULL;
	}
	__except (1)
	{
		//MessageBox(NULL, "���ش����쳣", "__try(IsVistaOrLater)", MB_OK);
	}
}

/*
����ֵ���ж�ϵͳ�Ƿ���64λ
true��64λϵͳ
false��32λϵͳ
*/
BOOL Is64Bit_OS()
{
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);

	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		return true;
	else
		return false;
}

bool IMProtect::LoadDriver()
{
	unsigned long dwRetBytes = 0;

	if (GetVistaOrLater() < 51){
		return false;
	}

	if (Is64Bit_OS()){
		m_DriverPath = GetAppFolder() + TEXT("IntoProtect.sys");
	}
	else {
		m_DriverPath = GetAppFolder() + TEXT("IntoProtect32.sys");
	}

	
	if (_waccess(m_DriverPath.c_str(), 0) == -1) {
		return false;
	}

	m_hMProtect = CreateFile(L"\\\\.\\IntoProtect", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == m_hMProtect) {
		LoadNTDriver(L"IntoProtect", m_DriverPath.c_str());
		m_hMProtect = CreateFile(L"\\\\.\\IntoProtect", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ |
			FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (INVALID_HANDLE_VALUE == m_hMProtect) {
			return 0;
		}
	}
	else{
		//DebugOutput(hListView,L"��������ʧ�ܣ���Ϊ�����Ѿ�����"); 
	}

	m_NotifyHandle.m_hEvent = (ULONGLONG)CreateEvent(NULL, false, false, NULL);
	m_NotifyHandle.m_hNotify = (ULONGLONG)CreateEvent(NULL, false, false, NULL);
	m_NotifyHandle.m_uPass = false;

	//�����¼���r0
	if (0 == DeviceIoControl(m_hMProtect, IOCTL_MPROTECT_EVENT, &m_NotifyHandle, sizeof(NOTIFY_HANDLE), NULL, 0, &dwRetBytes, NULL)){
		CloseHandle(m_hMProtect);
		CloseHandle((HANDLE)m_NotifyHandle.m_hEvent);
		CloseHandle((HANDLE)m_NotifyHandle.m_hNotify);
		Wow64RevertWow64FsRedirectionFun Wow64RevertWow64FsRedirection = (Wow64RevertWow64FsRedirectionFun)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "Wow64RevertWow64FsRedirection");
		if (Wow64RevertWow64FsRedirection)
			Wow64RevertWow64FsRedirection(&m_pOldValue);
		return 0;
	}
	return true;
}

std::wstring GetAppFolder()
{
	wchar_t moduleName[MAX_PATH];
	GetModuleFileNameW(NULL, moduleName, sizeof(moduleName));
	std::wstring strPath = moduleName;
	int pos = strPath.rfind(L"\\");
	if (!(pos == std::wstring::npos || pos == strPath.length() - 1))
		strPath = strPath.substr(0, pos);
	strPath += L"\\";
	return strPath;
}
                             //const wchar_t* lpDriverName, const wchar_t* lpDriverPathName
bool IMProtect::LoadNTDriver(const wchar_t*  lpszServerName,const wchar_t*  lpszDriverPath)
{
	wchar_t driver_image_path[256];
	//�õ�����������·��
	GetFullPathName(lpszDriverPath, 256, driver_image_path, NULL);

	bool b_ret = true;

	//�򿪷�����ƹ�����
	auto service_mgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( service_mgr == NULL )  {
		//DebugOutput(hListView, L"��������ʧ�ܣ���ΪOpenSCManager() ʧ�ܣ�������:%d ! \n", GetLastError() );
		b_ret = false;
		goto BeforeLeave;
	}

	//������������Ӧ�ķ���
	auto service_ddk = CreateService( service_mgr,
		lpszServerName, //�����������ע����е�����  
		lpszServerName, // ע������������ DisplayName ֵ  
		SERVICE_ALL_ACCESS, // ������������ķ���Ȩ��  
		SERVICE_KERNEL_DRIVER,// ��ʾ���صķ�������������  
		SERVICE_DEMAND_START, // ע������������ Start ֵ  
		SERVICE_ERROR_IGNORE, // ע������������ ErrorControl ֵ  
		driver_image_path, // ע������������ ImagePath ֵ  
		NULL,  
		NULL,  
		NULL,  
		NULL,  
		NULL);  
	if( service_ddk == NULL )  //�жϷ����Ƿ�ʧ��
{  
		unsigned long dwRtn = GetLastError();
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS )  {  
			//��������ԭ�򴴽�����ʧ��
			//DebugOutput(hListView,L"��������ʧ�ܣ���ΪCrateService() ʧ�ܣ�������:%d ! \n", dwRtn );  
			b_ret = false;
			//goto BeforeLeave;
		}  
		else  {
			//DebugOutput(hListView, L"��������ʧ�ܣ���ΪCrateService() ʧ��,ʧ��ԭ��ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n" );  
			b_ret = false;
			//goto BeforeLeave;
		}

		// ���������Ѿ����أ�ֻ��Ҫ��  
		service_ddk = OpenService( service_mgr, lpszServerName, SERVICE_ALL_ACCESS );  
		if( service_ddk == NULL )  {
			dwRtn = GetLastError();  
			//DebugOutput(hListView, L"��������ʧ�ܣ���ΪOpenService() ʧ�ܣ�������:%d ! \n", dwRtn );  
			b_ret = false;
			goto BeforeLeave;
		} 
	}  

	//�����������
	b_ret= StartService( service_ddk, NULL, NULL );  
	if( !b_ret )  {  
		unsigned long dwRtn = GetLastError();  
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING )  {  
			//DebugOutput(hListView,L"��������ʧ�ܣ���ΪStartService() ʧ�ܣ�������:%d ! \n", dwRtn );  
			b_ret = false;
			goto BeforeLeave;
		}  
		else  {  
			if( dwRtn == ERROR_IO_PENDING )  {  
				//DebugOutput(hListView,L"��������ʧ�ܣ���ΪStartService() ʧ�ܣ�����ԭ��:ERROR_IO_PENDING ! \n");
				b_ret = true;
				goto BeforeLeave;
			}  
			else  {  
				//DebugOutput(hListView,L"��������ʧ�ܣ���ΪStartService()ʧ�ܣ�����ԭ��:ERROR_SERVICE_ALREADY_RUNNING ! \n");
				b_ret = true;
				goto BeforeLeave;
			}  
		}  
	}
	else{
		//DebugOutput(hListView,L"���������ɹ�"); 
	}

BeforeLeave:
	if(service_ddk){
		CloseServiceHandle(service_ddk);
	}
	if(service_mgr){
		CloseServiceHandle(service_mgr);
	}
	return b_ret;
}

bool IMProtect::ProcessHideRemoval()
{
	unsigned long reg_bytes = 0;
	unsigned int pass = false;

	if (!DeviceIoControl(m_hMProtect, IOCTL_MPROTECT_RESET_PROCESS_HIDE, &pass, sizeof(UINT), NULL, 0, &reg_bytes, NULL)){
		//DebugOutput(hListView,L"ֹͣ�������ع��ܣ�ʧ�ܡ�������:%d ! \n", GetLastError() ); 
		return 1063;
	}
	//DebugOutput(hListView,L"ֹͣ�������ع��ܣ��ɹ�\n"); 
	return S_OK;
}


bool IMProtect::ProcessHideAdd(const wchar_t * ProcessFileName, unsigned long ProcessId)
{
	TRY_SOKE soke;
	unsigned long reg_bytes = 0;

	::memset(&soke,0,sizeof(TRY_SOKE));
	wcscpy(soke.ProcessInfo, ProcessFileName);
	soke.ProcessId = ProcessId;
	if (0 == DeviceIoControl(m_hMProtect, IOCTL_MPROTECT_ADD_PROTECTION_HIDE, &soke, sizeof(TRY_SOKE), NULL, 0, &reg_bytes, NULL)){
		//DebugOutput(hListView,L"�����������ع��ܣ�ʧ�ܡ�������:%d ! \n", GetLastError() ); 
		return 1063;
	}

	//DebugOutput(hListView,L"�����������ع��ܣ��ɹ�\n"); 
	return S_OK;
}

bool IMProtect::ProcessProtectionRemoval()
{
	unsigned long reg_bytes = 0;
	unsigned int pass = false;

	if (0 == DeviceIoControl(m_hMProtect, IOCTL_MPROTECT_RESET_PROCESS_POTECT, &pass, sizeof(UINT), NULL, 0, &reg_bytes, NULL))
	{
		//DebugOutput(hListView,L"ֹͣ���̱������棬ʧ�ܡ�������:%d ! \n", GetLastError() ); 
		return 1063;
	}
	//DebugOutput(hListView,L"ֹͣ���̱������ܣ��ɹ�\n");
	return S_OK;
}


bool IMProtect::ProcessProtectionAdd(const wchar_t * ProcessFileName, unsigned long ProcessId)
{
	TRY_SOKE soke;
	unsigned long reg_bytes = 0;

	::memset(&soke,0,sizeof(TRY_SOKE));
	wcscpy(soke.ProcessInfo, ProcessFileName);
	soke.ProcessId = ProcessId;
	if (0 == DeviceIoControl(m_hMProtect, IOCTL_MPROTECT_ADD_PROCESS_POTECT, &soke, sizeof(TRY_SOKE), NULL, 0, &reg_bytes, NULL)){
		//DebugOutput(hListView,L"�������̱������ܣ�ʧ�ܡ�������:%d !", GetLastError() ); 
		return 1063;
	}
	//DebugOutput(hListView,L"�������̱������ܣ��ɹ�");
	return S_OK;
}

bool IMProtect::Close()
{
	//if (m_hMProtect <= 0){
	//	return false;
	//}
	if (CloseHandle(m_hMProtect)) {
		m_hMProtect = 0;
		return true;
	}
	return false;
}

bool IMProtect::UnloadNTDriver()    //shupb 
{   
	bool b_ret = true;
	SERVICE_STATUS svr_sta;
	
	//��SCM������
	auto service_mgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if(service_mgr == NULL )  {
		printf( "OpenSCManager() Faild %d ! \n", GetLastError() );  
		b_ret = false;
		goto BeforeLeave;
	}  
	else  {
		printf( "OpenSCManager() ok ! \n" );  
	}

	//����������Ӧ�ķ���
	auto service_ddk = OpenService(service_mgr, L"IntoProtect", SERVICE_ALL_ACCESS );
	if(service_ddk == NULL )  {
		printf( "OpenService() Faild %d ! \n", GetLastError() );  
		b_ret = false;
		goto BeforeLeave;
	}  
	else  {  
		printf( "OpenService() ok ! \n" );  
	} 

	//ֹͣ�����������ֹͣʧ�ܣ�ֻ�������������ܣ��ٶ�̬���ء�  
	if( !ControlService(service_ddk, SERVICE_CONTROL_STOP , &svr_sta) )  {
		printf( "ControlService() Faild %d !\n", GetLastError() );  
	}  
	else  {
		printf( "ControlService() ok !\n" );  
	}  

	//��̬ж����������  
	if( !DeleteService(service_ddk) )  {
		printf( "DeleteSrevice() Faild %d !\n", GetLastError() );  
		b_ret = false;
	}  
	else  {  
		printf( "DelServer:eleteSrevice() ok !\n" );  
		b_ret = true;
	}  
	
BeforeLeave:
	if(service_ddk){
		CloseServiceHandle(service_ddk);
	}
	if(service_mgr){
		CloseServiceHandle(service_mgr);
	}
	return b_ret;
}