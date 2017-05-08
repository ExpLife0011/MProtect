//#include "stdafx.h"
#include "Driver.h"
#include "HookProtect.h"
/*
#define IOCTL_MPROTECT_EVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_USERCHOICE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_GET_TRY_SOKE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MPROTECT_RESET_PROCESS_HIDE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_ADD_PROTECTION_HIDE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_RESET_PROCESS_POTECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_ADD_PROCESS_POTECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)*/
#define IOCTL_MPROTECT_EVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_UNEVENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_USERCHOICE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_GET_TRY_SOKE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MPROTECT_RESET_PROCESS_HIDE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_ADD_PROTECTION_HIDE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_RESET_PROCESS_POTECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MPROTECT_ADD_PROCESS_POTECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x813, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_TRY_SOKE 0x80000000

#define DEVICE_NAME_PROCESS				L"\\Device\\IntoProtect"		//ProtectProgram
#define SYMBOLINK_NAME_PROCESS			L"\\??\\IntoProtect"			//ProtectProgram

typedef struct  
{
	unsigned long ProcessId;
	WCHAR ProcessInfo[260];
	WCHAR VisitInfo[5][260];
}TRY_SOKE;

PDEVICE_OBJECT  g_pDriverObject=NULL;//����ȫ���豸
KSPIN_LOCK  HideSpinLock;
KIRQL HideIrql;
KSPIN_LOCK  PotectSpinLock;
KIRQL PotectIrql;


NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING  pRegistryPath)
{
	ULONG i;
	NTSTATUS status;
	NTSTATUS rc;

	UNICODE_STRING strDeviceName;
	UNICODE_STRING strSymbolLinkName;

	UNICODE_STRING	FileEventString;
	UNICODE_STRING	FileAppEventString;

	PDEVICE_OBJECT pDeviceObject;

	pDeviceObject = NULL;

	RtlInitUnicodeString(&strDeviceName, DEVICE_NAME_PROCESS);
	RtlInitUnicodeString(&strSymbolLinkName, SYMBOLINK_NAME_PROCESS);

	DbgPrint("*********Enter DriverEntry()*********\n");

	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DispatcherGeneral;
	}

	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatcherCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatcherClose;
	pDriverObject->MajorFunction[IRP_MJ_READ] = DispatcherRead;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = DispatcherWrite;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatcherDeviceIoControl;

	pDriverObject->DriverUnload =DriverUnload;

	status = IoCreateDevice(pDriverObject, 0, &strDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	if (!pDeviceObject)
	{
		return STATUS_UNEXPECTED_IO_ERROR;
	}

	status = IoCreateSymbolicLink(&strSymbolLinkName, &strDeviceName);

	g_pDriverObject=pDeviceObject;


	InitGlobalVar();
	
	HookProcessProtect();
	DbgPrint("*********Leave DriverEntry()*********\n");

	return STATUS_SUCCESS;
}

void InitGlobalVar()
{
	KeInitializeSpinLock(&HideSpinLock);
	KeInitializeSpinLock(&PotectSpinLock);
}

void DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING strSymbolLinkName;
	NTSTATUS status;
	PDEVICE_OBJECT DeviceObjectTemp1=NULL;
	PDEVICE_OBJECT DeviceObjectTemp2=NULL;

	DbgPrint("*********Enter DriverUnload()*********");

	RtlInitUnicodeString(&strSymbolLinkName, SYMBOLINK_NAME_PROCESS);
	IoDeleteSymbolicLink(&strSymbolLinkName);

	DbgPrint("ֹͣ���̵ļ�ز���\r\n");
	DbgPrint("���� PsSetCreateProcessNotifyRoutine\r\n");

	/*status = PsSetCreateProcessNotifyRoutine(ProcessNotifyRoutine, TRUE);	//ȡ�����̵Ĵ����ͽ��̵��˳� �ص�����
	if (!NT_SUCCESS(status))
	{
		DbgPrint("���� PsSetCreateProcessNotifyRoutine ʧ��!\r\n");
		DbgPrint("Status Code: 0x%08X", status);
	}*/

	if(pDriverObject)
	{
		DeviceObjectTemp1=pDriverObject->DeviceObject;
		while(DeviceObjectTemp1)
		{
			DeviceObjectTemp2=DeviceObjectTemp1;
			DeviceObjectTemp1=DeviceObjectTemp1->NextDevice;
			IoDeleteDevice(DeviceObjectTemp2);
		}
	}

	UnHookProcessProtect();

	DbgPrint("*********Leave DriverUnload()*********\n");
}


//=====================================================================================//
//Name: NTSTATUS DispatcherCreate()										               //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS DispatcherCreate(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


//=====================================================================================//
//Name: NTSTATUS DispatcherClose()										               //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS DispatcherClose(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


//=====================================================================================//
//Name: NTSTATUS DispatcherGeneral()										           //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS DispatcherGeneral(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}


//=====================================================================================//
//Name: NTSTATUS DispatcherRead()											           //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS DispatcherRead(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	NTSTATUS rtStatus;

	rtStatus = STATUS_NOT_SUPPORTED;

	return rtStatus;
}


//=====================================================================================//
//Name: NTSTATUS DispatcherWrite()										               //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS DispatcherWrite(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	NTSTATUS rtStatus;

	rtStatus = STATUS_NOT_SUPPORTED;

	return rtStatus;
}


//=====================================================================================//
//Name: NTSTATUS DispatcherDeviceIoControl()								           //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS DispatcherDeviceIoControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	NTSTATUS rtStatus=STATUS_SUCCESS;
	PIO_STACK_LOCATION pStack;

	//ULONG uPID=0;
	ULONG uInLen;
	ULONG uOutLen;
	ULONG uCtrlCode;

	PCHAR pInBuffer;
	//PWSTR pInBufferRegistry;
	//PWSTR pInBufferFileFolder;
	
	//PUCHAR Buffer = 0;
	//TRY_SOKE * pSoke = 0;
	TRY_SOKE Soke;
	//unsigned long uPass = FALSE;
	
	pStack = IoGetCurrentIrpStackLocation(pIrp);

	uInLen = pStack->Parameters.DeviceIoControl.InputBufferLength;
	uOutLen = pStack->Parameters.DeviceIoControl.OutputBufferLength;
	uCtrlCode = pStack->Parameters.DeviceIoControl.IoControlCode;

	DbgPrint("*********Enter DispatcherDeviceIoControl()*********");

	switch(uCtrlCode)
	{
		case IOCTL_MPROTECT_ADD_PROTECTION_HIDE: //�ѽ�����������
			{
				DbgPrint("���������ڴ����Ӧ�ò㴫�������� HIDE PROCESSֵ\n");

				/*Buffer = (PUCHAR)pIrp->AssociatedIrp.SystemBuffer;
				pSoke = (TRY_SOKE*)Buffer;*/
				if (uInLen < sizeof(TRY_SOKE)) 
				{
					rtStatus = STATUS_UNSUCCESSFUL;
					//rtStatus = STATUS_PROCESS_IS_TERMINATING;
					break;
				}
				else
				{
					RtlZeroMemory(&Soke,sizeof(TRY_SOKE));
					pInBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
					RtlCopyMemory(&Soke,pInBuffer,sizeof(TRY_SOKE));
				}
				
				KeAcquireSpinLock(&HideSpinLock,&HideIrql);
				if(InsertHideProcess(Soke.ProcessId) == FALSE)
				{
					//rtStatus = STATUS_PROCESS_IS_TERMINATING;
					rtStatus = STATUS_UNSUCCESSFUL;
				}
				else
				{
					rtStatus = STATUS_SUCCESS;
				}
				KeReleaseSpinLock(&HideSpinLock,HideIrql);

				//pSoke = nullptr;
				/*pIrp->IoStatus.Status = STATUS_SUCCESS;
				pIrp->IoStatus.Information = sizeof(ULONG);
				RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, &uPass, sizeof(ULONG));
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);*/

				break;
			}
		case IOCTL_MPROTECT_RESET_PROCESS_HIDE: //�������صĽ���
			{
				DbgPrint("���������ڴ����Ӧ�ò㴫�������� IO_REMOVE_HIDE_PROCESSֵ\n");

				//pInBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
				//uPID = atol(pInBuffer);
				//uPID = 1;

				if (uInLen < sizeof(TRY_SOKE)) 
				{
					rtStatus = STATUS_UNSUCCESSFUL;
					//rtStatus = STATUS_PROCESS_IS_TERMINATING;
					break;
				}
				else
				{
					RtlZeroMemory(&Soke,sizeof(TRY_SOKE));
					pInBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
					RtlCopyMemory(&Soke,pInBuffer,sizeof(TRY_SOKE));
				}

				KeAcquireSpinLock(&HideSpinLock,&HideIrql);
				if(RemoveHideProcess(Soke.ProcessId) == FALSE)
				{
					rtStatus = STATUS_UNSUCCESSFUL;
				}
				else
				{
					rtStatus = STATUS_SUCCESS;
				}
				KeReleaseSpinLock(&HideSpinLock,HideIrql);

				DbgPrint("REMOVE_HIDE_PROCESSֵ�Ѿ��������\n");

				break;
			}
		case IOCTL_MPROTECT_ADD_PROCESS_POTECT: //�������̣���ֹ�����û�����
			{
				DbgPrint("���������ڴ����Ӧ�ò㴫�������� IO_INSERT_PROTECT_PROCESSֵ\n");
				
				if (uInLen < sizeof(TRY_SOKE)) 
				{
					rtStatus = STATUS_UNSUCCESSFUL;
					//rtStatus = STATUS_PROCESS_IS_TERMINATING;
					break;
				}
				else
				{
					RtlZeroMemory(&Soke,sizeof(TRY_SOKE));
					pInBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
					RtlCopyMemory(&Soke,pInBuffer,sizeof(TRY_SOKE));
				}

				KeAcquireSpinLock(&PotectSpinLock,&PotectIrql);
				if(InsertProtectProcess(Soke.ProcessId) == FALSE)
				{
					//rtStatus = STATUS_PROCESS_IS_TERMINATING;
					rtStatus = STATUS_UNSUCCESSFUL;
				}
				else
				{
					rtStatus = STATUS_SUCCESS;
				}
				KeReleaseSpinLock(&PotectSpinLock,PotectIrql);

				DbgPrint("PROTECT_PROCESSֵ�Ѿ��������\n");

				break;
			}
		case IOCTL_MPROTECT_RESET_PROCESS_POTECT: //���������Ľ���
			{
				DbgPrint("���������ڴ����Ӧ�ò㴫�������� IO_REMOVE_PROTECT_PROCESSֵ\n");
				/*
				pInBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
				uPID = atol(pInBuffer);*/

				if (uInLen < sizeof(TRY_SOKE)) 
				{
					rtStatus = STATUS_UNSUCCESSFUL;
					//rtStatus = STATUS_PROCESS_IS_TERMINATING;
					break;
				}
				else
				{
					RtlZeroMemory(&Soke,sizeof(TRY_SOKE));
					pInBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
					RtlCopyMemory(&Soke,pInBuffer,sizeof(TRY_SOKE));
				}

				KeAcquireSpinLock(&PotectSpinLock,&PotectIrql);
				if(RemoveProtectProcess(Soke.ProcessId) == FALSE)
				{
					//rtStatus = STATUS_PROCESS_IS_TERMINATING;
					rtStatus = STATUS_UNSUCCESSFUL;
				}
				else
				{
					rtStatus = STATUS_SUCCESS;
				}
				KeReleaseSpinLock(&PotectSpinLock,PotectIrql);

			
				DbgPrint("REMOVE_PROTECT_PROCESSֵ�Ѿ��������\n");

				break; 
			}/*
		case IOCTL_PROTECT_FILEFOLDER: //����Ӧ�ó���ָ�����ļ��� ��ֹ����ļ��б�ɾ������ֹ������ļ������洴���ļ����ļ���
			{
				pInBufferFileFolder = (PWSTR)pIrp->AssociatedIrp.SystemBuffer;

				wcscpy(ProtectFilePath,pInBufferFileFolder);
				_wcsupr(ProtectFilePath);

				if(iCountFilePath<PATHCOUNT)//���ܴ������ܱ������ļ��еĸ���
				{ 
					wcscpy(ProtectFilePathArray[iCountFilePath++],ProtectFilePath);//�����ļ�·����ȫ������
				}
				else
				{
					iCountFilePath=PATHCOUNT;
				}

				DbgPrint("Ҫ�������ļ����� ProtectFilePath=%S\n",ProtectFilePath);			
				DbgPrint("=========================================\n");
				DbgPrint("IOCTL_PROTECT_FILEFOLDER ������,ͨѶ�ɹ�!\n");			
				DbgPrint("=========================================\n");	

				rtStatus = STATUS_SUCCESS;
				pIrp->IoStatus.Information = uOutLen;		

				break;
			}
		case IOCTL_PROTECT_REGISTRY_VALUEKEY: //����ע���ļ�ֵ
			{
				pInBufferRegistry = (PWSTR)pIrp->AssociatedIrp.SystemBuffer;
				wcscpy(ProtectKey,pInBufferRegistry);

				if(iCountRegistryKey < REGISTRY_MAX_PATH)//�������ļ������ܱ����ĸ���
				{ 
					wcscpy(ProtectKeyArray[iCountRegistryKey++],ProtectKey);//����ע����ֵ��ȫ������
				}
				else
				{
					iCountRegistryKey=REGISTRY_MAX_PATH;
				}

				DbgPrint("ProtectKey��ֵ��:   %S\n",ProtectKey);
				DbgPrint("*------------------------------------------------*\n");
				DbgPrint("*IOCTL_PROTECT_REGISTRY_VALUEKEY ������,ͨѶ�ɹ�!*\n");			
				DbgPrint("*------------------------------------------------*\n");	

				break;
			}
		case IOCTL_PROTECT_REGISTRY_DIRECTORY: //����ָ����ע���Ŀ¼
			{
				pInBufferRegistry = (PWSTR)pIrp->AssociatedIrp.SystemBuffer;
				wcscpy(ProtectKeyDirectory,pInBufferRegistry);

				if(iCountRegistryPath < REGISTRY_MAX_PATH)
				{
					wcscpy(ProtectKeyDirectoryArray[iCountRegistryPath++],ProtectKeyDirectory);//����ע����·����ȫ������
				}
				else
				{
					iCountRegistryPath=REGISTRY_MAX_PATH;
				}

				DbgPrint("ProtectKeyDirectory��ֵ��:   %S\n",ProtectKeyDirectory);
				DbgPrint("*-------------------------------------------------*\n");
				DbgPrint("*IOCTL_PROTECT_REGISTRY_DIRECTORY ������,ͨѶ�ɹ�!*\n");			
				DbgPrint("*-------------------------------------------------*\n");	

				break;
			}*/
		default:
			{
				rtStatus = STATUS_SUCCESS;

				DbgPrint("���������ڴ���default���\n");
				break;
			}
	}

	pIrp->IoStatus.Status = rtStatus;
	pIrp->IoStatus.Information =uOutLen;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	
	DbgPrint("*********Leave DispatcherDeviceIoControl()*********\n");

	return rtStatus;
}

