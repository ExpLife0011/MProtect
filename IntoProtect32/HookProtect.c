/**********************************************************************************************
 *    ����˵����																				  *
 *                                                                                            *
 * ��������Ҫʵ���ˣ�                                                                          *
 * ���̱������������ء��ļ�Ŀ¼������ע�����ؼ���ֵ�ı��������̵ļ�ء������������еĽ��̡�      *
 *																						      *
 *																							  *
 *                                                                               *
 *																							   *
 * ���뿪��ʱ�䣺 												   *
 *																							   *
 ***********************************************************************************************/

//#ifndef _WIN32_WINNT				                  
//#define _WIN32_WINNT 0x0501	 //Windows����ϵͳ�İ汾��
//#endif						
#include "HookProtect.h"

extern KSPIN_LOCK  HideSpinLock;
extern KIRQL HideIrql;

extern KSPIN_LOCK  PotectSpinLock;
extern KIRQL PotectIrql;


/*
PROCESSINFO  psInfo;
PROCESSINFO  processInfoArray[MAXPROCESSCOUNT]={ 0 };//���������Ϣ������

ULONG countProcessID=0;//ͳ�ƽ��̵ĸ���
ULONG CurrentProcessCount=0;//��¼�µ�ǰϵͳ���ж��ٸ�����������

PDEVICE_OBJECT  g_pDriverObject;//����ȫ���豸
*/
ULONG g_PIDHideArray[MAX_PROCESS_ARRARY_LENGTH];
ULONG g_PIDProtectArray[MAX_PROCESS_ARRARY_LENGTH];
ULONG g_currHideArrayLen = 0;
ULONG g_currProtectArrayLen = 0;/*
ULONG g_OffsetEprocessName = NULL;

WCHAR ProtectFilePath[MAXBUF]={0};//����Ҫ�������ļ�Ŀ¼�����ֵ����Ӧ�ò㴫��������
WCHAR ProtectFilePathArray[PATHCOUNT][MAXBUF] = {0}; //�������ļ�Ŀ¼�Ķ�ά���飬ÿ���ļ�Ŀ¼����ProtectFilePath

ULONG iCountFilePath=0;		 //iCountFilePath����ͳ��Ҫ�������ļ�Ŀ¼�����������벻����PATHCOUNT   
ULONG iCountRegistryKey=0;	 //ͳ��ע�������ֵ�ĸ���
ULONG iCountRegistryPath=0;  //ͳ��ע���ľ���·���ĸ���

WCHAR ProtectKey[REGISTRY_DATA_MAXLEN] ={0}; //����һ������Ҫ�����ļ���  ���ֵ��Ӧ�ò㴫��������
WCHAR ProtectKeyArray[REGISTRY_MAX_PATH][REGISTRY_DATA_MAXLEN]={0}; //����һ����� ����ProtectKey�ļ���

WCHAR ProtectKeyDirectory[REGISTRY_DATA_MAXLEN]={0}; //����һ��ע����·���� ���ֵ��Ӧ�ò㴫��������
WCHAR ProtectKeyDirectoryArray[REGISTRY_MAX_PATH][REGISTRY_DATA_MAXLEN]={0}; //����һ��ע����·��  ����ProtectKeyDirectory�ļ���
*/
NTQUERYSYSTEMINFORMATION pOldNtQuerySystemInformation;
ZWTERMINATEPROCESS pOldNtTerminateProcess;
PMDL          pMdl=NULL;
PULONG        plMapped=NULL;
#define        SSDT_INDEX(_Func)                (*(PULONG)((PUCHAR)_Func + 1))

int HookProcessProtect()
{
	/*DbgPrint("��DriverEntry���������ǻ�ȡ���Ľ��̸�����: %d\n",countProcessID);

	DbgPrint("���� PsSetCreateProcessNotifyRoutine��������ؽ��̵Ĵ����ͽ��̵��˳�\r\n");

	rc = PsSetCreateProcessNotifyRoutine(ProcessNotifyRoutine, FALSE);  //���ý��̵Ĵ����ͽ��̵��˳� �ص�����
	if (!NT_SUCCESS(rc))
	{
		DbgPrint("���� PsSetCreateProcessNotifyRoutine ʧ��!\r\n");
		DbgPrint("Status Code: 0x%08X", rc);
		return rc;
	}

	rc = GetProcessNameOffset(&g_OffsetEprocessName);
	if (!NT_SUCCESS(rc))
	{
		KdPrint(("���� GetProcessNameOffsetʧ��!\r\n"));
		KdPrint(("Status Code: 0x%08X", rc));
		return rc;
	}
	KdPrint(("g_OffsetEprocessName��ֵΪ��0x%X\r\n", g_OffsetEprocessName));*/

	KIRQL Irql;
	DbgPrint("�޸�SSDT��....\n");
	//_asm 
	//{
	//	cli
	//	mov eax,cr0
	//	and eax,not 10000h  //���cr0��WPλ
	//	mov cr0,eax
	//}
	///////////////////////////////////
	pMdl = IoAllocateMdl(KeServiceDescriptorTable.ServiceTableBase, KeServiceDescriptorTable.NumberOfServices * 4,
		FALSE, FALSE, NULL);
	if (pMdl == NULL)
	{
		return 0;
	}

	MmBuildMdlForNonPagedPool(pMdl);
	pMdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;        //Write SSDT

	plMapped = (PULONG)MmMapLockedPagesSpecifyCache(pMdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
	if (plMapped == NULL)
	{
		IoFreeMdl(pMdl);
		return 0;
	}
	///////////////////////////////////
	//����IRQL�жϼ�
	//Irql = KeRaiseIrqlToDpcLevel();
	//����ԭʼ���ں˺��� ZwQuerySystemInformation�ĵ�ַ,���������Զ���ĺ��������
	//�û�ָ���ġ�����Windowsϵͳ���еĽ��� ��������
	pOldNtQuerySystemInformation=(NTQUERYSYSTEMINFORMATION)(SYSTEMSERVICE(ZwQuerySystemInformation));
	//(NTQUERYSYSTEMINFORMATION)(SYSTEMSERVICE(ZwQuerySystemInformation)) = HookNtQuerySystemInformation;
	plMapped[SSDT_INDEX(ZwQuerySystemInformation)] = (ULONG)HookNtQuerySystemInformation;
	
	//����ԭʼ���ں˺��� ZwTerminateProcess�ĵ�ַ,���������Զ���ĺ��������
	//�û�ָ���ġ�����Windowsϵͳ���еĽ��� ����������������ֹ����
	pOldNtTerminateProcess=(ZWTERMINATEPROCESS)(SYSTEMSERVICE(ZwTerminateProcess));
	//(ZWTERMINATEPROCESS)(SYSTEMSERVICE(ZwTerminateProcess)) = HookZwTerminateProcess;
	plMapped[SSDT_INDEX(ZwTerminateProcess)] = (ULONG)HookZwTerminateProcess;

	//����ԭʼ���ں˺��� ZwSetValueKey�ĵ�ַ,���������Զ���ĺ��������
	//�ڹ�˾�������Ʒ����Ӧ��ע��������棬��ֹ�û��޸�ע���ı����Լ���ֵ
//	RealZwSetValueKey =(ZWSETVALUEKEY)(SYSTEMSERVICE(ZwSetValueKey));
//	(ZWSETVALUEKEY)(SYSTEMSERVICE(ZwSetValueKey)) = HookZwSetValueKey;

	
	//����ԭʼ���ں˺��� ZwDeleteValueKey�ĵ�ַ,���������Զ���ĺ��������
	//�ڹ�˾�������Ʒ����Ӧ��ע��������棬��ֹ�û�ɾ��ע����ֵ
//	RealZwDeleteValueKey=(ZWDELETEVALUEKEY)(SYSTEMSERVICE(ZwDeleteValueKey));
//	(ZWDELETEVALUEKEY)(SYSTEMSERVICE(ZwDeleteValueKey)) = HookZwDeleteValueKey;

	
	//����ԭʼ���ں˺��� ZwDeleteKey�ĵ�ַ,���������Զ���ĺ��������
	//�ڹ�˾�������Ʒ����Ӧ��ע��������棬��ֹ�û�ɾ��ע���ı���
//	RealZwDeleteKey=(ZWDELETEKEY)(SYSTEMSERVICE(ZwDeleteKey));
//	(ZWDELETEKEY)(SYSTEMSERVICE(ZwDeleteKey)) = HookZwDeleteKey;

	
	//����ԭʼ���ں˺��� ZwCreateKey�ĵ�ַ,���������Զ���ĺ��������
	//�ڹ�˾�������Ʒ����Ӧ��ע��������棬��ֹ�û�����ע������ֵ
//	RealZwCreateKey=(ZWCREATEKEY)(SYSTEMSERVICE(ZwCreateKey));
//	(ZWCREATEKEY)(SYSTEMSERVICE(ZwCreateKey)) = HookZwCreateKey;

	
	//����ԭʼ���ں˺��� ZwSetInformationFile�ĵ�ַ,���������Զ���ĺ��������
	//��ֹ�û�ɾ���ļ����ļ��С������������ļ�����������
	//RealZwSetInformationFile=(ZWSETINFORMATIONFILE)(SYSTEMSERVICE(ZwSetInformationFile));
	//(ZWSETINFORMATIONFILE)(SYSTEMSERVICE(ZwSetInformationFile)) = HookZwSetInformationFile;

	
	//����ԭʼ���ں˺��� ZwCreateFile�ĵ�ַ,���������Զ���ĺ��������
	//���ܱ������ļ�Ŀ¼��  �������û������ļ����ļ��С����ơ�ճ��������
	//RealZwCreateFile=(ZWCREATEFILE)(SYSTEMSERVICE(ZwCreateFile));
	//(ZWCREATEFILE)(SYSTEMSERVICE(ZwCreateFile)) = HookZwCreateFile;

	
	//����ԭʼ���ں˺��� ZwOpenProcess�ĵ�ַ,���������Զ���ĺ��������
	//��ȡ�û�����Windowsϵͳ���еĽ��̵�ID�źͽ��̵�·��,�����Ǵ��ݸ�Ӧ�ò�
//	RealZwOpenProcess=(ZWOPENPROCESS)(SYSTEMSERVICE(ZwOpenProcess));
//	(ZWOPENPROCESS)(SYSTEMSERVICE(ZwOpenProcess))=HookZwOpenProcess;

	//����ԭʼ���ں˺��� ZwCreateThread�ĵ�ַ,���������Զ���ĺ��������
	//��ֹԶ��ע�룬Hook��Ӧ�ò��CreateRemoteThread����
	//RealZwCreateThreadEx=(ZWCREATETHREADEX)(KeServiceDescriptorTable.ServiceTableBase[0x58]);//Windows 7 32  ZwCreateThreadEx-->0x58
	//(ZWCREATETHREADEX)(KeServiceDescriptorTable.ServiceTableBase[0x58]) = HookZwCreateThreadEx;
//	RealZwCreateThread=(ZWCREATETHREAD)(KeServiceDescriptorTable.ServiceTableBase[0x35]);//windowsXP ZwCreateThread--->0x35
//	(ZWCREATETHREAD)(KeServiceDescriptorTable.ServiceTableBase[0x35]) = HookZwCreateThread;
	//KeLowerIrql(Irql);
	/*_asm
	{
		mov eax,cr0
		or eax,not 10000h  //�ָ�cr0��WPλ
		mov cr0,eax
		sti
	}*/
	MmUnmapLockedPages(plMapped, pMdl);
	IoFreeMdl(pMdl);
	pMdl = NULL;

	DbgPrint("*********Leave DriverEntry()*********\n");

	return 1;
}

void UnHookProcessProtect()
{
	/*
	������if�жϣ�����ȷ���ˣ�
		(1)��Ӧ�ò��EXE�ļ�û�����е�ʱ�򣬻��ߣ�
		(2)��Ӧ�ò�û�е���DeviceIoControl�����������¼��������������ʱ��	
		Windowsϵͳ����������
	*/
	/*_asm
	{
	cli
	mov eax,cr0
	and eax,not 10000h  //���cr0��WPλ
	mov cr0,eax
	}*/

	DbgPrint("�޸�SSDT��\n");

	pMdl = IoAllocateMdl(KeServiceDescriptorTable.ServiceTableBase, KeServiceDescriptorTable.NumberOfServices * 4,
		FALSE, FALSE, NULL);
	if (pMdl == NULL)
	{
		return ;
	}

	MmBuildMdlForNonPagedPool(pMdl);
	pMdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;        

	plMapped = (PULONG)MmMapLockedPagesSpecifyCache(pMdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
	if (plMapped == NULL)
	{
		IoFreeMdl(pMdl);
		return ;
	}
	//���濪ʼ�ָ�Windows�ں�ԭʼ��ZwXXX����
	//(NTQUERYSYSTEMINFORMATION)(SYSTEMSERVICE(ZwQuerySystemInformation))=pOldNtQuerySystemInformation;
	plMapped[SSDT_INDEX(ZwQuerySystemInformation)] = (ULONG)pOldNtQuerySystemInformation;
	//(ZWTERMINATEPROCESS)(SYSTEMSERVICE(ZwTerminateProcess))=pOldNtTerminateProcess;
	plMapped[SSDT_INDEX(ZwTerminateProcess)] = (ULONG)pOldNtTerminateProcess;
	//	(ZWSETVALUEKEY)(SYSTEMSERVICE(ZwSetValueKey))=RealZwSetValueKey;
	//	(ZWDELETEVALUEKEY)(SYSTEMSERVICE(ZwDeleteValueKey))=RealZwDeleteValueKey;
	//	(ZWDELETEKEY)(SYSTEMSERVICE(ZwDeleteKey))=RealZwDeleteKey;
	//	(ZWCREATEKEY)(SYSTEMSERVICE(ZwCreateKey))=RealZwCreateKey; 
	//	(ZWSETINFORMATIONFILE)(SYSTEMSERVICE(ZwSetInformationFile)) =RealZwSetInformationFile;
	//	(ZWCREATEFILE)(SYSTEMSERVICE(ZwCreateFile))=RealZwCreateFile;
	//	(ZWOPENPROCESS)(SYSTEMSERVICE(ZwOpenProcess)) = RealZwOpenProcess;
	//	(ZWCREATETHREAD)(KeServiceDescriptorTable.ServiceTableBase[0x35]) = RealZwCreateThread;	//windows XP
	//	(ZWCREATETHREADEX)(KeServiceDescriptorTable.ServiceTableBase[0x58]) = RealZwCreateThreadEx;	//windows 7	
	
	//_asm
	//{
	//	mov eax,cr0
	//	or eax,not 10000h  //�ָ�cr0��WPλ
	//	mov cr0,eax
	//	sti
	//}
	MmUnmapLockedPages(plMapped, pMdl);
	IoFreeMdl(pMdl);
	pMdl = NULL;
	DbgPrint("*********�޸�SSDT�����*********\n");
}

//=========================================================================================================//
//Name: ULONG ValidateProcessNeedHide()											                           //
//                                                                                                         //
//Descripion: ���� uPID�����������б��е�����������ý����������б��в����ڣ��򷵻�-1                      //
//            				                            						                           //
//=========================================================================================================//

ULONG ValidateProcessNeedHide(ULONG uPID)
{
	ULONG i = 0;

	if(uPID == 0)
	{
		return -1;
	}

	for(i=0; i<g_currHideArrayLen && i<MAX_PROCESS_ARRARY_LENGTH; i++)
	{
		if(g_PIDHideArray[i] == uPID)
		{
			return i;
		}
	}
	return -1;
}


//=========================================================================================================//
//Name: ULONG ValidateProcessNeedProtect()										                           //
//                                                                                                         //
//Descripion: ����uPID�����ڱ����б��е�����������ý����ڱ����б��в����ڣ��򷵻� -1                      //
//            				                            						                           //
//=========================================================================================================//
ULONG ValidateProcessNeedProtect(ULONG uPID)
{
	ULONG i = 0;

	if(uPID == 0)
	{
		return -1;
	}

	for(i=0; i<g_currProtectArrayLen && i<MAX_PROCESS_ARRARY_LENGTH;i++)
	{
		if(g_PIDProtectArray[i] == uPID)
		{
			return i;
		}
	}
	return -1;
}


//=========================================================================================================//
//Name: ULONG InsertHideProcess()												                           //
//                                                                                                         //
//Descripion: �ڽ��������б��в����µĽ��� ID										                       //
//            				                            						                           //
//=========================================================================================================//
ULONG InsertHideProcess(ULONG uPID)
{
	if(ValidateProcessNeedHide(uPID) == -1 && g_currHideArrayLen < MAX_PROCESS_ARRARY_LENGTH)
	{
		g_PIDHideArray[g_currHideArrayLen] = uPID;
		g_currHideArrayLen++;
		return TRUE;
	}

	return FALSE;
}


//=========================================================================================================//
//Name: ULONG RemoveHideProcess()												                           //
//                                                                                                         //
//Descripion: �ӽ��������б����Ƴ����� ID											                       //
//            				                            						                           //
//=========================================================================================================//
ULONG RemoveHideProcess(ULONG uPID)
{
	ULONG uIndex = ValidateProcessNeedHide(uPID);
	ULONG i=uIndex;
	if(uIndex != -1)
	{
		//g_PIDHideArray[uIndex] = g_PIDHideArray[g_currHideArrayLen--];
		for(i=uIndex;i<g_currHideArrayLen;i++)
		{
			g_PIDHideArray[i] = g_PIDHideArray[i+1];
		}
		g_currHideArrayLen--;
		return TRUE;
	}
	
	return FALSE;
}


//=========================================================================================================//
//Name: ULONG InsertProtectProcess()											                           //
//                                                                                                         //
//Descripion: �ڽ��̱����б��в����µĽ��� ID										                       //
//            				                            						                           //
//=========================================================================================================//
ULONG InsertProtectProcess(ULONG uPID)
{
	if(ValidateProcessNeedProtect(uPID) == -1 && g_currProtectArrayLen < MAX_PROCESS_ARRARY_LENGTH)
	{
		g_PIDProtectArray[g_currProtectArrayLen] = uPID;
		g_currProtectArrayLen++;
		return TRUE;
	}
	return FALSE;
}


//=========================================================================================================//
//Name: ULONG RemoveProtectProcess()											                           //
//                                                                                                         //
//Descripion: �ڽ��̱����б����Ƴ�һ������ID                                                               //
//            				                            						                           //
//=========================================================================================================//
ULONG RemoveProtectProcess(ULONG uPID)
{
	ULONG uIndex = ValidateProcessNeedProtect(uPID);
	if(uIndex != -1)
	{
		g_PIDProtectArray[uIndex] = g_PIDProtectArray[g_currProtectArrayLen--];

		return TRUE;
	}
	return FALSE;
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////        �����Ǹ���Hook�����ľ������ʵ��            ////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
/*
PVOID GetPointer( HANDLE handle )
{

	PVOID         pKey;
	DbgPrint("*******Enter GetPointer()*******\n");

	if(!handle) return NULL;
	//ȡ��ָ��
	if( ObReferenceObjectByHandle( handle, 0, NULL, KernelMode, &pKey, NULL ) != STATUS_SUCCESS ) 
	{
		pKey = NULL;
	} 

	DbgPrint("*******Leave GetPointer()*******\n");
	return pKey;
}




 //����������м�ؽ��̣�����ȡ���̵�����·��



//=============================================================================================//
//Name: ULONG PsGetProcessPathByPid()														   //
//                                                                                             //
//Descripion: ���ݽ��̵�ID������ȡ����·��				                                       //
//            				                            						               //
//=============================================================================================//
ULONG PsGetProcessPathByPid( IN ULONG Pid ,WCHAR FilePath[MAXPATHLEN])
{
	NTSTATUS       status;

	UNICODE_STRING		uni_path;
	UNICODE_STRING		uni_disk;

	PEPROCESS			pEprocess;
	PFILE_OBJECT		FileObject;
	PVOID				Object;

	ULONG current_build;    
	ULONG ans = 0;    


	DbgPrint("Enter  PsGetProcessPathByPid()����\n");

	status = PsLookupProcessByProcessId(Pid,&pEprocess);

	if(!NT_SUCCESS(status))
	{
		DbgPrint("EPROCESS Error");
		return 0;
	} 
	
	DbgPrint("EPROCESS 0x%0.8X",pEprocess);

	PsGetVersion(NULL, NULL, &current_build, NULL); 

	// if (current_build == 2195)  ans = 0; ��ǰ��֧��Windows 2000   
	if (current_build == 2600)  ans = 0x138;   //Windows XP
	if (current_build == 3790)  ans = 0x124;   //Windows 2003

	if( !MmIsAddressValid( (PULONG)( (ULONG)pEprocess+ans ) ) )//EPROCESS+0x138 -> SectionObject

	{   DbgPrint("SectionObject Error");
		return 0;
	} 
	
	Object = (PVOID)(*(PULONG)((ULONG)pEprocess+ans));

	if( !MmIsAddressValid( (PULONG)( (ULONG)Object+0x014 ) ) )//SectionObject+0x014 -> Segment
	{
		DbgPrint("Segment Error");
		return 0;
	} 
	
	Object = (PVOID)(*(PULONG)( (ULONG)Object+0x014 ));

	if( !MmIsAddressValid( (PULONG)((ULONG)Object+0x000) ) )//Segment+0x000 -> ControlAera
	{
		DbgPrint("ControlAera Error");
		return 0;
	} 
	
	Object = (PVOID)(*(PULONG)( (ULONG)Object+0x000 ));

	if( !MmIsAddressValid( (PULONG)( (ULONG)Object+0x024 ) ) )//ControlAera+0x024 -> FilePointer(FileObject)
	{
		DbgPrint("FilePointer Error");
		return 0;
	} 
	
	DbgPrint("++++++++++PsGetProcessPathByPid�����Ѿ��ɹ�ִ��������Ҫ���ǲ���\n");

	Object = (PVOID)(*(PULONG)( (ULONG)Object+0x024 ));

	FileObject = Object;
	ObReferenceObjectByPointer((PVOID)FileObject,0,NULL,KernelMode);

	RtlInitUnicodeString(&uni_path,FileObject->FileName.Buffer); //��ȡ·����
	RtlVolumeDeviceToDosName(FileObject->DeviceObject,&uni_disk); //��ȡ�̷���
	ObDereferenceObject(FileObject);

	if( wcslen(uni_path.Buffer)+wcslen(uni_disk.Buffer) < MAXPATHLEN+10  )
	{
		wcscat(FilePath,uni_disk.Buffer);
		wcscat(FilePath,uni_path.Buffer);

	}
	else 
	{
		wcscat(FilePath,uni_disk.Buffer);
		wmemcpy(FilePath,uni_path.Buffer,MAXPATHLEN-wcslen(uni_disk.Buffer)-1);

		*(FilePath + MAXPATHLEN) = 0;
	}

	DbgPrint("Leave  PsGetProcessPathByPid()����\n");

	return 1;
}




ULONG GetPlantformDependentInfo(ULONG dwFlag)   
{    
	ULONG current_build;    
	ULONG ans = 0;    

	PsGetVersion(NULL, NULL, &current_build, NULL);    

	
	switch ( dwFlag )   
	{    
	case EPROCESS_SIZE:    
		if (current_build == 2195) ans = 0 ;        // Windows 2000����ǰ��֧��2000����ͬ   
		if (current_build == 2600) ans = 0x25C;     // Windows XP   
		if (current_build == 3790) ans = 0x270;     // Windows 2003   
		break;    
	case PEB_OFFSET:    
		if (current_build == 2195)  ans = 0;    
		if (current_build == 2600)  ans = 0x1b0;    
		if (current_build == 3790)  ans = 0x1a0;   
		break;    
	case FILE_NAME_OFFSET:    
		if (current_build == 2195)  ans = 0;    
		if (current_build == 2600)  ans = 0x174;    
		if (current_build == 3790)  ans = 0x164;   
		break;    
	case PROCESS_LINK_OFFSET:    
		if (current_build == 2195)  ans = 0;    
		if (current_build == 2600)  ans = 0x088;    
		if (current_build == 3790)  ans = 0x098;   
		break;    
	case PROCESS_ID_OFFSET:    
		if (current_build == 2195)  ans = 0;    
		if (current_build == 2600)  ans = 0x084;    
		if (current_build == 3790)  ans = 0x094;   
		break;    
	case EXIT_TIME_OFFSET:    
		if (current_build == 2195)  ans = 0;    
		if (current_build == 2600)  ans = 0x078;    
		if (current_build == 3790)  ans = 0x088;   
		break;    
	}    
	return ans;    
}



//=============================================================================================//
//Name: NTSTATUS GetProcessPath()													           //
//                                                                                             //
//Descripion: ����Windows�ں˽��̵����ݽṹ��EPROCESS����ȡ����·��				               //
//            				                            						               //
//=============================================================================================//
VOID GetProcessPath(ULONG eprocess, PUNICODE_STRING pFilePath)
{
	ULONG object;
	ULONG current_build;    
	ULONG ans = 0;  

	PFILE_OBJECT	FilePointer;
	UNICODE_STRING	name;  //�̷�

	DbgPrint("Enter GetProcessPath����\n");

	PsGetVersion(NULL, NULL, &current_build, NULL);    

	// if (current_build == 2195)  ans = 0; ��ǰ��֧��Windows2000   
	 if (current_build == 2600)  ans = 0x138; //Windows XP   
	 if (current_build == 3790)  ans = 0x124; //Windows 7 


	if(MmIsAddressValid((PULONG)(eprocess + ans)))// EPROCESS -> SectionObject
	{
		object = (*(PULONG)(eprocess + ans));
		
		if(MmIsAddressValid((PULONG)((ULONG)object + 0x014)))// SECTION_OBJECT -> Segment
		{
			object = *(PULONG)((ULONG)object + 0x014);
				
			if(MmIsAddressValid((PULONG)((ULONG)object + 0x0)))// SEGMENT_OBJECT -> ControlArea ����0x018
			{
				object = *(PULONG)((ULONG_PTR)object + 0x0);
				
				if(MmIsAddressValid((PULONG)((ULONG)object + 0x024)))// CONTROL_AREA -> FilePointer
				{
					object=*(PULONG)((ULONG)object + 0x024);
				}
				else 
				{ 
					DbgPrint("Leave GetProcessPath����\n");
					return;
				}
				
			}
			else
			{
				DbgPrint("Leave GetProcessPath����\n");
				return;
			} 
		}
		else 
		{
			DbgPrint("Leave GetProcessPath����\n");
			return;
		}
	}
	else
	{
		DbgPrint("Leave GetProcessPath����\n");
		return;

	} 
	FilePointer = (PFILE_OBJECT)object;

	ObReferenceObjectByPointer((PVOID)FilePointer,0,NULL,KernelMode);
	RtlVolumeDeviceToDosName(FilePointer->DeviceObject, &name); //��ȡ�̷���
	RtlCopyUnicodeString(pFilePath, &name); //�̷�����
	RtlAppendUnicodeStringToString(pFilePath, &FilePointer->FileName); //·������
	ObDereferenceObject(FilePointer);		//�رն�������

	DbgPrint("Leave GetProcessPath����\n");

}



//==================================================================================================//
//Name: VOID ProcessNotifyRoutine()													                //
//                                                                                                  //
//Descripion: ������أ���Windowsϵͳ���е�ʱ������Щ�µĽ��̱��������Լ�����Щ���̱��˳�		    //
//            				                            						                    //
//==================================================================================================//
VOID ProcessNotifyRoutine(
						  IN HANDLE	ParentId,
						  IN HANDLE	ProcessId,
						  IN BOOLEAN	Create
						  )
{
	NTSTATUS		status = STATUS_SUCCESS;
	PEPROCESS		pEprocess = NULL;
	UNICODE_STRING 	uniPath;
	ULONG  i,j,uPid;

	uniPath.Length = 0;
	uniPath.MaximumLength = MAXPATHLEN * 2;
	uniPath.Buffer = (PWSTR)ExAllocatePool(NonPagedPool, uniPath.MaximumLength);


	DbgPrint("Enter ProcessNotifyRoutine����\n");

	// ��������
	if (Create)
	{
		DbgPrint("*******----��----��----��----��----��----��----*******\r\n");
		// ������
		// DbgPrint("��������Ϣ\r\n");
		// DbgPrint("        PID:  %d\r\n", ParentId);
		// status = PsLookupProcessByProcessId(ParentId, &pEprocess);
		// if (NT_SUCCESS(status))
		// {
		// GetProcessPath(pEprocess, &uniPath);
		// DbgPrint("        ·��: %wZ\r\n", &uniPath);
		// }
		
		DbgPrint("����������Ϣ\r\n");
		DbgPrint("        ����PID:  %d\r\n", ProcessId);
		status = PsLookupProcessByProcessId(ProcessId, &pEprocess);
		if (NT_SUCCESS(status))
		{
			GetProcessPath((ULONG)pEprocess, &uniPath);
			DbgPrint("        ����·��: %wZ\r\n", &uniPath);
		}
	}
	// ��������
	else
	{
		DbgPrint("*******----��----��----��----��----��----��----*******\r\n");
		// ������
		//DbgPrint("��������Ϣ\r\n");
		//DbgPrint("        PID:  %d\r\n", ParentId);
		//status = PsLookupProcessByProcessId(ParentId, &pEprocess);
		//if (NT_SUCCESS(status))
		//{
		//	GetProcessPath(pEprocess, &uniPath);
		//	DbgPrint("        ·��: %wZ\r\n", &uniPath);
		//}
		// ����
		DbgPrint("�˳�������Ϣ\r\n");
		DbgPrint("        ����PID:  %d\r\n", ProcessId);
		status = PsLookupProcessByProcessId(ProcessId, &pEprocess);
		if (NT_SUCCESS(status))
		{
			GetProcessPath(pEprocess, &uniPath);
			DbgPrint("        ����·��: %wZ\r\n", &uniPath);
		}

		for(i=0;i<CurrentProcessCount;i++)
		{
			uPid=processInfoArray[i].pid;

			if(uPid ==ProcessId)
			{
				//����Ҫɾ��processInfoArray���������Ӧ��Ԫ��
				for(j=i;j<CurrentProcessCount-1;j++)
				{
					//�����е�ÿһ��Ԫ����ǰ�ƶ�
					processInfoArray[j].pid=processInfoArray[j+1].pid;
					wcscpy(processInfoArray[j].psPath,processInfoArray[j+1].psPath);
					//strcpy(processInfoArray[j].psPath,processInfoArray[j+1].psPath);
				}

				//�������һ��Ԫ�ص�ֵ����Ϊ��ֵ
				processInfoArray[j].pid=0;
				//memset(processInfoArray[j].psPath,0,sizeof(processInfoArray[j].psPath));
				wmemset(processInfoArray[j].psPath,0,sizeof(processInfoArray[j].psPath)/sizeof(WCHAR));

				CurrentProcessCount--;
				DbgPrint("-----�˳�һ�����̺�ʣ�µĽ��̻���-----\n");
				for(j=0;j<CurrentProcessCount;j++)
				{
					DbgPrint("����ID %d\n",processInfoArray[j].pid);
					DbgPrint("����·�� %S\n",processInfoArray[j].psPath);
				}
				break;
			}
		}
	}
	ExFreePool(uniPath.Buffer);

	DbgPrint("Leave ProcessNotifyRoutine����\n");
}



ULONG EnumProcessList()
{
	PROCESS_INFO     ProcessInfo = {0};
	ULONG            EProcess;
	ULONG            FirstProcess;
	ULONG            dwCount = 0;
	LIST_ENTRY*      ActiveProcessLinks;

	ULONG    dwPIdOffset = GetPlantformDependentInfo(PROCESS_ID_OFFSET);
	ULONG    dwPNameOffset = GetPlantformDependentInfo(FILE_NAME_OFFSET);
	ULONG    dwPLinkOffset = GetPlantformDependentInfo(PROCESS_LINK_OFFSET);

	DbgPrint("Enter EnumProcessList����\n");

	DbgPrint("PidOff=0x%X  NameOff=0x%X  LinkOff=0x%X", dwPIdOffset, dwPNameOffset, dwPLinkOffset);



	// ��ȡ��ǰ���̵ĵ�ַ
	FirstProcess = EProcess = (ULONG)PsGetCurrentProcess();

	do
	{
		ProcessInfo.dwProcessId = *((ULONG *)(EProcess + dwPIdOffset));
		ProcessInfo.pImageFileName= (PUCHAR)(EProcess + dwPNameOffset);

		dwCount++;


		if(ProcessInfo.dwProcessId<=0 || ProcessInfo.dwProcessId==4) //��ͳ�ƽ��̺���0���߽��̺���4�Ľ���
		{ 
			dwCount--;

			ActiveProcessLinks = (LIST_ENTRY *)(EProcess + dwPLinkOffset);		
			EProcess = (ULONG)ActiveProcessLinks->Flink - dwPLinkOffset;

		} 
		else
		{
			DbgPrint("[Pid=%6d] %s ", ProcessInfo.dwProcessId, ProcessInfo.pImageFileName);

			ActiveProcessLinks = (LIST_ENTRY *)(EProcess + dwPLinkOffset);		
			EProcess = (ULONG)ActiveProcessLinks->Flink - dwPLinkOffset;

		}


		if (EProcess == FirstProcess)
		{
			break;
		}
	}while (EProcess != 0);


	return dwCount;
}


unsigned long __fastcall SizeOfCode(void *Code, unsigned char **pOpcode)
{
	PUCHAR cPtr;
	UCHAR Flags;
	BOOLEAN PFX66, PFX67;
	BOOLEAN SibPresent;
	UCHAR iMod, iRM, iReg;
	UCHAR OffsetSize, Add;
	UCHAR Opcode;
	if (!MmIsAddressValid(Code)) return 0;
	OffsetSize = 0;
	PFX66 = FALSE;
	PFX67 = FALSE;
	cPtr = (PUCHAR)Code;
	while ( (*cPtr == 0x2E) || (*cPtr == 0x3E) || (*cPtr == 0x36) ||
		(*cPtr == 0x26) || (*cPtr == 0x64) || (*cPtr == 0x65) || 
		(*cPtr == 0xF0) || (*cPtr == 0xF2) || (*cPtr == 0xF3) ||
		(*cPtr == 0x66) || (*cPtr == 0x67) ) 
	{
		if (*cPtr == 0x66) PFX66 = TRUE;
		if (*cPtr == 0x67) PFX67 = TRUE;
		cPtr++;
		if (cPtr > (PUCHAR)Code + 16) return 0; 
	}
	Opcode = *cPtr;
	if (pOpcode) *pOpcode = cPtr; 
	if (*cPtr == 0x0F)
	{
		cPtr++;
		Flags = OpcodeFlagsExt[*cPtr];
	} else 
	{
		Flags = OpcodeFlags[Opcode];
		if (Opcode >= 0xA0 && Opcode <= 0xA3) PFX66 = PFX67;
	}
	cPtr++;
	if (Flags & OP_WORD) cPtr++;	
	if (Flags & OP_MODRM)
	{
		iMod = *cPtr >> 6;
		iReg = (*cPtr & 0x38) >> 3;  
		iRM  = *cPtr &  7;
		cPtr++;
		if ((Opcode == 0xF6) && !iReg) Flags |= OP_DATA_I8;    
		if ((Opcode == 0xF7) && !iReg) Flags |= OP_DATA_PRE66_67; 
		SibPresent = !PFX67 & (iRM == 4);
		switch (iMod)
		{
		case 0: 
			if ( PFX67 && (iRM == 6)) OffsetSize = 2;
			if (!PFX67 && (iRM == 5)) OffsetSize = 4; 
			break;
		case 1: OffsetSize = 1;
			break; 
		case 2: if (PFX67) OffsetSize = 2; else OffsetSize = 4;
			break;
		case 3: SibPresent = FALSE;
		}
		if (SibPresent)
		{
			if (((*cPtr & 7) == 5) && ( (!iMod) || (iMod == 2) )) OffsetSize = 4;
			cPtr++;
		}
		cPtr = (PUCHAR)(ULONG)cPtr + OffsetSize;
	}
	if (Flags & OP_DATA_I8)  cPtr++;
	if (Flags & OP_DATA_I16) cPtr += 2;
	if (Flags & OP_DATA_I32) cPtr += 4;
	if (PFX66) Add = 2; else Add = 4;
	if (Flags & OP_DATA_PRE66_67) cPtr += Add;
	return (ULONG)cPtr - (ULONG)Code;
}



//�õ�PsSuspendProcess�ĵ�ַ
ULONG process_getPsSuspendProcessAddress()
{
	UCHAR *cPtr, *pOpcode;
	ULONG ulLength;
	ULONG ulCallCount = 1;

	ULONG ulNtSuspendProcess,ulPsSuspendProcessAddr;

	DbgPrint("******Enter process_getPsSuspendProcessAddress����******\n");

	__try
	{
        //winXP                                
		//���ulNtSuspendProcess�ĵ�ַ
		ulNtSuspendProcess = *((PULONG)(KeServiceDescriptorTable.ServiceTableBase) + NtSuspendProcess_XP);

		//����
		DbgPrint("process_getPsSuspendProcessAddress  ulNtSuspendProcess :%08X",ulNtSuspendProcess);

		/*
		kd> uf nt!NtSuspendProcess
		nt!NtSuspendProcess:
		83ef88d7 8bff            mov     edi,edi
		83ef88d9 55              push    ebp
		83ef88da 8bec            mov     ebp,esp
		83ef88dc 51              push    ecx
		83ef88dd 51              push    ecx
		83ef88de 64a124010000    mov     eax,dword ptr fs:[00000124h]
		83ef88e4 8a803a010000    mov     al,byte ptr [eax+13Ah]
		83ef88ea 56              push    esi
		83ef88eb 6a00            push    0
		83ef88ed 8845f8          mov     byte ptr [ebp-8],al
		83ef88f0 8d45fc          lea     eax,[ebp-4]
		83ef88f3 50              push    eax
		83ef88f4 ff75f8          push    dword ptr [ebp-8]
		83ef88f7 ff350431d883    push    dword ptr [nt!PsProcessType (83d83104)]
		83ef88fd 6800080000      push    800h
		83ef8902 ff7508          push    dword ptr [ebp+8]
		83ef8905 e8fa34f4ff      call    nt!ObReferenceObjectByHandle (83e3be04)
		83ef890a 8bf0            mov     esi,eax
		83ef890c 85f6            test    esi,esi
		83ef890e 7c12            jl      nt!NtSuspendProcess+0x4b (83ef8922)

		nt!NtSuspendProcess+0x39:
		83ef8910 ff75fc          push    dword ptr [ebp-4]
		83ef8913 e837feffff      call    nt!PsSuspendProcess (83ef874f)
		83ef8918 8b4dfc          mov     ecx,dword ptr [ebp-4]
		83ef891b 8bf0            mov     esi,eax
		83ef891d e8a1a3d9ff      call    nt!ObfDereferenceObject (83c92cc3)
		*/
/*
		//���PsSuspendProcess�ĵ�ַ
		for (cPtr = (PUCHAR)ulNtSuspendProcess; cPtr < (PUCHAR)ulNtSuspendProcess + PAGE_SIZE;) 
		{
			//��ñ��������ֽ���
			ulLength = SizeOfCode(cPtr, &pOpcode);

			//��������ʧ�ܵĻ�
			if (ulLength == 0)
			{
				return NULL;
			}

			//������E8��ʾcallָ��
			if (*pOpcode == 0xE8)
			{
				//�ڶ��ε���callָ��
				if (ulCallCount == 2)
				{
					ulPsSuspendProcessAddr = (*(PULONG)(pOpcode + 1) + (ULONG)cPtr + 5);
					break;
				}

				ulCallCount ++;
			}

			//����
			cPtr = cPtr + ulLength;
		}

		//����
		DbgPrint("process_getPsSuspendProcessAddress  ulPsSuspendProcessAddr :%08X",ulPsSuspendProcessAddr);

		DbgPrint("******Leave process_getPsSuspendProcessAddress����******\n");

		//���ص�ַ
		return ulPsSuspendProcessAddr;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("process_getPsSuspendProcessAddress EXCEPTION_EXECUTE_HANDLER error!");
		return NULL;
	}
}



//=========================================================================================================//
//Name: BOOLEAN process_suspendProcess()										                           //
//                                                                                                         //
//Descripion: Hook������NtSuspendProcess�����ݽ��̵�ID�����������                                         //
//            				                            						                           //
//=========================================================================================================//
//�������
BOOLEAN process_suspendProcess(ULONG ulPID)
{
	NTSTATUS status;
	PEPROCESS pEpr;

	DbgPrint("******Enter process_suspendProcess����******\n");
	__try
	{
		PPsSuspendProcess PsSuspendProcess = (PPsSuspendProcess)process_getPsSuspendProcessAddress();

		if (!PsSuspendProcess)
		{
			DbgPrint("process_suspendProcess process_getPsSuspendProcessAddress error!");
			return FALSE;
		}

		status = PsLookupProcessByProcessId((HANDLE)ulPID,&pEpr);

		if (!NT_SUCCESS(status))
		{
			DbgPrint("process_suspendProcess PsLookupProcessByProcessId error!");
			return FALSE;
		}

		status = PsSuspendProcess(pEpr);

		if (!NT_SUCCESS(status))
		{
			DbgPrint("process_suspendProcess PsSuspendProcess error!");
			return FALSE;
		}


		DbgPrint("******Leave process_suspendProcess����******\n");

		return TRUE;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("process_suspendProcess EXCEPTION_EXECUTE_HANDLER error!");
		return FALSE;
	}
}

//=========================================================================================================//
//Name: NTSTATUS HookZwCreateFile()                                                                        //
//											                                                               //        
//Descripion: ���ܱ������ļ�Ŀ¼��  �������û������ļ����ļ��С����ơ�ճ��������                           //
//            				                            						                           //
//=========================================================================================================//
NTSTATUS HookZwCreateFile(
						  OUT PHANDLE FileHandle,
						  IN ACCESS_MASK DesiredAccess,
						  IN POBJECT_ATTRIBUTES ObjectAttributes,
						  OUT PIO_STATUS_BLOCK IoStatusBlock,
						  IN PLARGE_INTEGER AllocationSize  OPTIONAL,
						  IN ULONG FileAttributes,
						  IN ULONG ShareAccess,
						  IN ULONG CreateDisposition,
						  IN ULONG CreateOptions,
						  IN PVOID EaBuffer  OPTIONAL,
						  IN ULONG EaLength)
{

	ANSI_STRING ansiDirName;
	PUNICODE_STRING uniFileName;
	UNICODE_STRING uDirName;

	NTSTATUS rc;
	ULONG i;

	DbgPrint("*********Enter HookZwCreateFile()************\n");

	//���û�������·�������ڱ���ansiDirName��
	RtlUnicodeStringToAnsiString( &ansiDirName, ObjectAttributes->ObjectName, TRUE);
	_strupr(ansiDirName.Buffer);
	DbgPrint("AnsiDirNameBuffer=%s\n",ansiDirName.Buffer);

	RtlAnsiStringToUnicodeString(&uDirName,&ansiDirName,TRUE); //��ansiDirNameת����UNICODE_STRING����
	DbgPrint("UnicodeDirName= %wZ\n",&uDirName);

	for(i=0;i<iCountFilePath;i++)
	{
		if(ProtectFilePathArray[i][0]!=NULL && ProtectFilePathArray[i][0]!=0x0)
		{	 	        
			DbgPrint("ʹ��wcsstr����ִ���˺�����Ԫ��ProtectFilePathArray[i]�ıȽ�\n");

			if(wcsstr(uDirName.Buffer,ProtectFilePathArray[i]))//�ж�ProtectFilePathArray[i]�Ƿ���uDirName.Buffer���Ӵ�
			{
				DbgPrint("**����ִ��return STATUS_ACCESS_DENIED��䣬��Ҫ�뿪HookZwCreateFile����\n");   
				return STATUS_ACCESS_DENIED; 
			}   
		}

	}

	//��������������Ҫ������Ŀ¼����ô�͵���Windows�ں��Լ���ZwCreateFile������ʵ�ִ����ļ��Ĺ���

	DbgPrint("ִ�� RealZwCreateFile\n");

	return RealZwCreateFile(
			FileHandle,
			DesiredAccess,
			ObjectAttributes,
			IoStatusBlock,
			AllocationSize,
			FileAttributes,
			ShareAccess,
			CreateDisposition,
			CreateOptions,
			EaBuffer,
			EaLength);

}


//=========================================================================================================//
//Name: NTSTATUS HookZwSetInformationFile()                                                                //
//											                                                               //
//Descripion: ��ֹ�û�ɾ���ļ����ļ��С������������ļ�����������                                         //
//            				                            						                           //
//=========================================================================================================//
NTSTATUS HookZwSetInformationFile(
								  IN HANDLE  FileHandle,
								  OUT PIO_STATUS_BLOCK  IoStatusBlock,
								  IN PVOID  FileInformation,
								  IN ULONG  Length,
								  IN FILE_INFORMATION_CLASS  FileInformationClass
								  )

{
	PFILE_OBJECT pFileObject;
	NTSTATUS ret;
	ULONG i;

	UNICODE_STRING uDosName={0};
	UNICODE_STRING pFilePath={0};

	pFilePath.MaximumLength=MAXPATHLEN*2;
	pFilePath.Buffer=(PWSTR)ExAllocatePool(PagedPool,MAXPATHLEN*2);

	uDosName.MaximumLength=40;
	uDosName.Buffer=(PWSTR)ExAllocatePool(PagedPool,40);

	ret = ObReferenceObjectByHandle(FileHandle, GENERIC_READ,*IoFileObjectType,
									KernelMode, (PVOID*)&pFileObject, 0);

	DbgPrint("*********Enter HookZwSetInformationFile()*********\n");

	if (NT_SUCCESS(ret))
	{
		ret=IoVolumeDeviceToDosName(pFileObject->DeviceObject, &uDosName); //��ȡ�̷���
		RtlCopyUnicodeString(&pFilePath, &uDosName); //�����̷�    
		RtlAppendUnicodeStringToString(&pFilePath,&(pFileObject->FileName));//�����̷���·��

		DbgPrint("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		DbgPrint("pFilePath: %wZ\n",&pFilePath); 
		DbgPrint("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

		if (NT_SUCCESS(ret))
		{
			RtlUpcaseUnicodeString(&pFilePath,&pFilePath,FALSE);
			DbgPrint("ת���ɴ�д���pFilePath:  %wZ\n",&pFilePath);
			DbgPrint("ProtectFilePath: %S\n",ProtectFilePath);

			for(i=0;i<iCountFilePath;i++)
			{
				if(ProtectFilePathArray[i][0]!=NULL && ProtectFilePathArray[i][0]!=0x0)
				{
					if(wcsstr(pFilePath.Buffer,ProtectFilePathArray[i]))//�ж�ProtectFilePathArray[i]�Ƿ���pFilePath.Buffer���Ӵ�
					{  
						RtlFreeUnicodeString(&pFilePath);
						RtlFreeUnicodeString(&uDosName);

						DbgPrint("**����ִ��return STATUS_ACCESS_DENIED��䣬��Ҫ�뿪Leave HookZwSetInformationFile()\n");
						return STATUS_ACCESS_DENIED;
					}
				}
			} 
		}
	}

	if (pFileObject)
	{
		ObDereferenceObject(pFileObject);
	}

	if(pFilePath.Buffer[0]!=NULL && pFilePath.Buffer[0]!=0x0)
	{
		RtlFreeUnicodeString(&pFilePath);
	}

	if(uDosName.Buffer[0]!=NULL && uDosName.Buffer[0]!=0x0)
	{
		RtlFreeUnicodeString(&uDosName);
	}

	DbgPrint("*********Leave HookZwSetInformationFile()*********\n");

	return RealZwSetInformationFile(FileHandle, 
			IoStatusBlock,
			FileInformation,
			Length,
			FileInformationClass);

}


//=========================================================================================================//
//Name: NTSTATUS HookZwCreateKey()                                                                         //
//											                                                               //
//Descripion:�ڹ�˾�������Ʒ����Ӧ��ע��������棬��ֹ�û�����ע������ֵ                              //
//            				                            						                           //
//=========================================================================================================//

NTSTATUS HookZwCreateKey(
						 OUT PHANDLE  KeyHandle,
						 IN ACCESS_MASK  DesiredAccess,
						 IN POBJECT_ATTRIBUTES  ObjectAttributes,
						 IN ULONG  TitleIndex,
						 IN PUNICODE_STRING  Class  OPTIONAL,
						 IN ULONG  CreateOptions,
						 OUT PULONG  Disposition  OPTIONAL
						 )

{

	ULONG i;
	ULONG actualLen;
	UNICODE_STRING *pUniName; 
	NTSTATUS rc;
	PVOID Object;
	WCHAR str[REGISTRY_DATA_MAXLEN]={0}; //���WCHAR���ͱ���str������������ǽ�����ע���ı���

	DbgPrint("******Enter HookZwCreateKey()******\n");

	//ObjectAttributes->ObjectName���½�����ע��� �����·������������������·������ֻ��·���ĺ�벿��
	DbgPrint("���½�����ע���ı����·����:   %wZ\n",ObjectAttributes->ObjectName); 
	
	RtlUpcaseUnicodeString(ObjectAttributes->ObjectName,ObjectAttributes->ObjectName,FALSE);
	DbgPrint("ObjectNameת���ɴ�д���ֵ��:   %wZ\n",ObjectAttributes->ObjectName);
	DbgPrint("-- RootDirectory 0x%X\n", ObjectAttributes->RootDirectory);

	pUniName = ExAllocatePool( NonPagedPool, 512*2+2*sizeof(ULONG));
	pUniName->MaximumLength = 512*2;


	if(ObjectAttributes->RootDirectory != 0) 
	{
		rc=ObReferenceObjectByHandle(ObjectAttributes->RootDirectory,0,0,KernelMode,&Object,NULL);
		if (rc==STATUS_SUCCESS) 
		{
			if( NT_SUCCESS( ObQueryNameString(Object, pUniName, MAXPATHLEN, &actualLen)))
			{
				DbgPrint("In CreateKey Path is  %wZ\n",pUniName); //����·����ǰ�벿��
			}
			ObDereferenceObject(Object);
			wcscpy(str,pUniName->Buffer);
			wcscat(str,"\\");
			wcscat(str,ObjectAttributes->ObjectName->Buffer);
			_wcsupr(str);
			DbgPrint("�ϲ����ע���·����    %S\n",str); 

			//��������forѭ����������2��ע���·���ıȽϲ�����
			//����һ��·���ǣ�Ӧ�ò�����ͨ��DeviceioControl�������������ġ�����һ��·���ǣ��������ﱣ����ַ���str�����ֵ��
			//��������Ӧ�ö�������strstr��wcsstr�ıȽϲ�����

			for(i=0;i<iCountRegistryPath;i++)
			{
				if(ProtectKeyDirectoryArray[i]!=NULL && ProtectKeyDirectoryArray[i][0]!=0x0)
				{
					if(wcsstr(str,ProtectKeyDirectoryArray[i]))
					{
						DbgPrint("��������� return STATUS_ACCESS_DENIED��� �����뿪HookZwCreateKey()\n");
						DbgPrint("*******Leave HookZwCreateKey()*******\n");
						ExFreePool(pUniName);
						return STATUS_ACCESS_DENIED;
					}
				}
			}//end for

		}//end  if(rc==STATUS_SUCCESS)
	}

	ExFreePool(pUniName);

	DbgPrint("******Leave HookZwCreateKey()******\n");

	return  RealZwCreateKey(
				KeyHandle,
				DesiredAccess,
				ObjectAttributes,
				TitleIndex,
				Class,
				CreateOptions,
				Disposition 
				);
	
}


//=========================================================================================================//
//Name: NTSTATUS HookZwDeleteValueKey()                                                                    //
//											                                                               //
//Descripion:�ڹ�˾�������Ʒ����Ӧ��ע��������棬��ֹ�û�ɾ��ע����ֵ                                  //
//            				                            						                           //
//=========================================================================================================//
NTSTATUS HookZwDeleteValueKey(IN HANDLE KeyHandle,PUNICODE_STRING ValueName)
{      
	NTSTATUS nStatus = STATUS_SUCCESS;
	UNICODE_STRING yyo;
	UNICODE_STRING dut;
	ANSI_STRING tbb;
	ULONG i;

	DbgPrint("*******Enter HookZwDeleteValueKey()*******\n");

	RtlUnicodeStringToAnsiString(&tbb,ValueName,TRUE); //�ѽػ�ϵͳҪɾ����ע���ļ������ַ���
	RtlAnsiStringToUnicodeString(&dut,&tbb,TRUE);      //�� tbb ת��ΪUNICODE_STRING 
	for(i=0;i<iCountRegistryKey;i++)
	{
		if(ProtectKeyArray[i][0]!=NULL && ProtectKeyArray[i][0]!=0x0 )
		{
			RtlInitUnicodeString(&yyo,ProtectKeyArray[i]); //��ʼ���ַ������� 

			if(RtlEqualUnicodeString(&yyo,&dut,TRUE))  //�ж�ϵͳҪɾ���ļ����Ƿ�������Ҫ������
			{
				DbgPrint("��������� return STATUS_ACCESS_DENIED��� �����뿪HookZwDeleteValueKey()\n");
				DbgPrint("*******Leave HookZwDeleteValueKey()*******\n");
				return STATUS_ACCESS_DENIED; 
			}

		}
	}

	//�������������Ǳ�����Ŀ¼����ô�͵���Windows�ں��Լ���ZwDeleteValueKey����	  
	DbgPrint("���� RealZwDeleteValueKey()\n");
	nStatus = RealZwDeleteValueKey(KeyHandle,ValueName); 

	DbgPrint("*******Leave HookZwDeleteValueKey()*******\n");

	return nStatus;
}


//=========================================================================================================//
//Name: NTSTATUS HookZwDeleteKey()                                                                         //
//											                                                               //
//Descripion:�ڹ�˾�������Ʒ����Ӧ��ע��������棬��ֹ�û�ɾ��ע���ı���                                //
//            				                            						                           //
//=========================================================================================================//
NTSTATUS HookZwDeleteKey(IN HANDLE KeyHandle)
{
	NTSTATUS rc;
	UNICODE_STRING *pUniName;  //����õ��޸�ע����UNI·��
	ULONG actualLen;   
	ULONG i;  
	PVOID pKey;
	UNICODE_STRING  KeyValueData={0};//���ǵ�ǰ��ȡ����ע���Ŀ¼

	DbgPrint("*******Enter HookZwDeleteKey()*******\n");

	KeyValueData.Buffer=(PWSTR)ExAllocatePool(NonPagedPool,512*2+2*sizeof(ULONG));
	KeyValueData.MaximumLength = 512*2;

	pKey = GetPointer( KeyHandle);

	if(pKey)
	{
		pUniName = ExAllocatePool( NonPagedPool, 512*2+2*sizeof(ULONG));
		pUniName->MaximumLength = 512*2;

		if( NT_SUCCESS( ObQueryNameString( pKey, pUniName, MAXPATHLEN, &actualLen)))
		{	
			RtlCopyUnicodeString(&KeyValueData,pUniName);

			for(i=0;i<iCountRegistryPath;i++)
			{
				if(ProtectKeyDirectoryArray[i]!=NULL && ProtectKeyDirectoryArray[i][0]!=0x0)
				{
					//�Ƚϵ�ǰ��ȡ����ע���Ŀ¼�Ƿ� ���� ��Ӧ�ò㴫��������ע���Ŀ¼
					DbgPrint("===�������_wcsicmp���������Ƚ�ע����Ŀ¼�Ƿ���ڴ�Ӧ�ò㴫��������ע���Ŀ¼\n");

					if(_wcsicmp(KeyValueData.Buffer,ProtectKeyDirectoryArray[i])==0)
					{
						DbgPrint("��������� return STATUS_ACCESS_DENIED��� �����뿪HookZwDeleteKey()\n");
						DbgPrint("*******Leave HookZwDeleteKey()*******\n");
						ExFreePool(pUniName);
						RtlFreeUnicodeString(&KeyValueData);
						return STATUS_ACCESS_DENIED;
					}
				}
			}//end for
		}

	} //end if(pKey)

	ExFreePool(pUniName);
	//RtlFreeUnicodeString(pUniName);
	RtlFreeUnicodeString(&KeyValueData);

	if(pKey)  
	{
		ObDereferenceObject(pKey);
	}

	DbgPrint("���� RealZwDeleteKey()\n");

	rc=RealZwDeleteKey(KeyHandle);

	DbgPrint("*******Leave HookZwDeleteKey()*******\n");

	return rc;

}


//=========================================================================================================//
//Name: NTSTATUS HookZwSetValueKey()                                                                       //
//											                                                               //
//Descripion:�ڹ�˾�������Ʒ����Ӧ��ע��������棬��ֹ�û��޸�ע���ı����Լ���ֵ                        //
//            				                            						                           //
//=========================================================================================================//
NTSTATUS HookZwSetValueKey( IN HANDLE  KeyHandle,
						   IN PUNICODE_STRING  ValueName,
						   IN ULONG  TitleIndex  OPTIONAL,
						   IN ULONG  Type,
						   IN PVOID  Data,
						   IN ULONG  DataSize)

{
	
	NTSTATUS rc;
	UNICODE_STRING *pUniName;  //����õ��޸�ע����UNI·��
	ULONG actualLen;
	ULONG i;
	PVOID pKey;
	UNICODE_STRING  keyname={0};

	DbgPrint("*******Enter HookZwSetValueKey()*******\n");

	keyname.Buffer=(PWSTR)ExAllocatePool(NonPagedPool,512*2+2*sizeof(ULONG));
	keyname.MaximumLength = 512*2;

	pKey = GetPointer( KeyHandle);

	if(pKey)
	{
		pUniName = ExAllocatePool( NonPagedPool, 512*2+2*sizeof(ULONG));
		pUniName->MaximumLength = 512*2;

		if( NT_SUCCESS( ObQueryNameString( pKey, pUniName, MAXPATHLEN, &actualLen)))
		{
			RtlCopyUnicodeString( &keyname, pUniName, TRUE);
			DbgPrint("===���������_wcsicmp���������Ƚ�ע���ļ�ֵ�Ƿ���ڴ�Ӧ�ò㴫�������ļ�ֵ \n");

			for(i=0;i<iCountRegistryPath;i++)
			{
				if(ProtectKeyDirectoryArray[i]!=NULL && ProtectKeyDirectoryArray[i][0]!=0x0)
				{  
					if(_wcsicmp(keyname.Buffer,ProtectKeyDirectoryArray[i]) == 0)
					{
						DbgPrint("��������� return STATUS_ACCESS_DENIED��� �����뿪HookZwSetValueKey()\n");
						DbgPrint("*******Leave HookZwSetValueKey()*******\n");
						ExFreePool(pUniName);
						RtlFreeUnicodeString(&keyname); 
						return STATUS_ACCESS_DENIED;    
					}
				}
			}
		}

	}//end if(pKey)

	if(pKey)  
	{
		ObDereferenceObject(pKey);
	}

	ExFreePool(pUniName);
	RtlFreeUnicodeString(&keyname); 

	DbgPrint("���� RealZwSetValueKey()\n");
	rc=RealZwSetValueKey(KeyHandle,ValueName,TitleIndex,Type,Data,DataSize);

	DbgPrint("*******Leave HookZwSetValueKey()*******\n");

	return rc;
	
}*/



//=========================================================================================================//
//Name: NTSTATUS HookNtQuerySystemInformation()                                                            //
//											                                                               //
//Descripion: �û�ָ���ġ�����Windowsϵͳ���еĽ��� ��������                                               //
//            				                            						                           //
//=========================================================================================================//
NTSTATUS HookNtQuerySystemInformation (
									   __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
									   __out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
									   __in ULONG SystemInformationLength,
									   __out_opt PULONG ReturnLength
									   )
{
	
	NTSTATUS rtStatus=STATUS_SUCCESS;
	NTSTATUS hThreadStatus;
	HANDLE hSystemHandle;
	ULONG ulSize = 0x1000;
	PVOID pBuffer;
	NTSTATUS status;
	//���������Ϣ�Ľṹ��ָ��
	PSYSTEM_PROCESS_INFORMATION pSystemProcessInformation;

	ULONG uPID; 
	UNICODE_STRING ProcessName;
	int inRt=-1;

	DbgPrint("*********Enter HookNtQuerySystemInformation()*********\n");

	rtStatus = pOldNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
	if(NT_SUCCESS(rtStatus))
	{
		if(SystemProcessInformation== SystemInformationClass)
		{
			PSYSTEM_PROCESS_INFORMATION pPrevProcessInfo = NULL;
			PSYSTEM_PROCESS_INFORMATION pCurrProcessInfo = (PSYSTEM_PROCESS_INFORMATION)SystemInformation; 

			while(pCurrProcessInfo != NULL)
			{
				//��ȡ��ǰ������ SYSTEM_PROCESS_INFORMATION �ڵ�Ľ������ƺͽ��� ID
			
				 uPID = (ULONG)pCurrProcessInfo->UniqueProcessId;

				// RtlInitUnicodeString(&ProcessName, L"HavkAv.exe");
				// if(!RtlCompareUnicodeString(&ProcessName, &(pCurrProcessInfo->ImageName), TRUE))
				//{
				//	KdPrint(("HookNtQuerySystemInformation------------------>>>>>>>>>>>\n"));
				//	//ZwTerminateProcess((HANDLE)uPID,0);
			
				//	return STATUS_ACCESS_DENIED;
				//}

				//hThreadStatus=PsCreateSystemThread(&hSystemHandle,0,NULL,NULL,NULL,SystemThread, (PVOID)uPID);

				//�жϵ�ǰ��������������Ƿ�Ϊ��Ҫ���صĽ���
				KeAcquireSpinLock(&HideSpinLock,&HideIrql);
				inRt=ValidateProcessNeedHide(uPID);
				KeReleaseSpinLock(&HideSpinLock,HideIrql);

				if(inRt != -1)
				{
					if(pPrevProcessInfo)
					{
						if(pCurrProcessInfo->NextEntryOffset)
						{
							//����ǰ�������(��Ҫ���صĽ���)�� SystemInformation ��ժ��(��������ƫ��ָ��ʵ��)
							pPrevProcessInfo->NextEntryOffset += pCurrProcessInfo->NextEntryOffset;
						}
						else
						{
							//˵����ǰҪ���ص���������ǽ��������е����һ��
							pPrevProcessInfo->NextEntryOffset = 0;
						}
					}
					else
					{
						//��һ���������ý��̾�����Ҫ���صĽ���
						if(pCurrProcessInfo->NextEntryOffset)
						{
							(PCHAR)SystemInformation += pCurrProcessInfo->NextEntryOffset;
						}
						else
						{
							SystemInformation = NULL;
						}
					}
				}

				//������һ�� SYSTEM_PROCESS_INFORMATION �ڵ�
				pPrevProcessInfo = pCurrProcessInfo;

				//��������
				if(pCurrProcessInfo->NextEntryOffset)
				{
					pCurrProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(((PCHAR)pCurrProcessInfo) + pCurrProcessInfo->NextEntryOffset);
				}
				else
				{
					pCurrProcessInfo = NULL;
				}
			}//End while
		}
	}

	DbgPrint("*********Leave HookNtQuerySystemInformation()*********\n");

	return rtStatus;
}



/*
//====================================================================================================//
//Name: NTSTATUS HookZwOpenProcess()                                                                  //
//											                                                          //
//Descripion: ��ȡ�û�����Windowsϵͳ���еĽ��̵�ID�źͽ��̵�·��,�����Ǵ��ݸ�Ӧ�ò�                  //
//            				                            						                      //
//====================================================================================================//
NTSTATUS HookZwOpenProcess(OUT PHANDLE ProcessHandle,
						   IN ACCESS_MASK DesiredAccess,
						   IN POBJECT_ATTRIBUTES ObjectAttributes,
						   IN PCLIENT_ID ClientId OPTIONAL)

{

	/* Windows�ں˽ṹ�Ķ���
	typedef struct _CLIENT_ID
	{
	PVOID UniqueProcess;
	PVOID UniqueThread;
	} CLIENT_ID, *PCLIENT_ID;
	*/
/*
	NTSTATUS nStatus = STATUS_SUCCESS;
    WCHAR    fullname[MAXPATHLEN]={0};
	ULONG uPID;
	ULONG rt;

	ULONG i;
    ULONG flag=0;

//	DbgPrint("*********Enter HookZwOpenProcess()*********\n");

	nStatus = RealZwOpenProcess(ProcessHandle,DesiredAccess,ObjectAttributes,ClientId);
	
	if(ClientId!=NULL)  //����һ�����̵�ID
	{
		uPID=(long)ClientId->UniqueProcess;
	}

// 	DbgPrint("��HookZwOpenProcess������̵�ID�ǣ�%d\n",uPID);

	if((uPID!=0)  &&  (uPID!=4))
	{
		rt=PsGetProcessPathByPid(uPID, fullname); //���ݽ��̵�ID��uPID����ȡ������̵�·��
	
		if(!rt){ goto label; }


		//ȫ�ֱ���CurrentProcessCount�����˵�ǰϵͳ���ж��ٸ�����������

		if(CurrentProcessCount==0) //��ʾWindowsϵͳ��һ�ν���HookZwOpenProcess�����Ĵ���������
		{
			processInfoArray[CurrentProcessCount].pid=uPID;
			wcscpy(processInfoArray[CurrentProcessCount].psPath,fullname);
		}

		for( i=0; (i<CurrentProcessCount) && (CurrentProcessCount<MAXPROCESSCOUNT); i++)
		{
			
			//���Ԥ�ȱ���������processInfoArray�������Ϣ�͸ո�ץȡ��������Ϣ��ͬ
			//��ô�� ִ��break��䣬���˳�forѭ���塣
			if( (processInfoArray[i].pid == uPID) &&				 
				(_wcsicmp(processInfoArray[i].psPath,fullname) == 0) 				
			  )

			{
				
				 //��forѭ�����������Ϣ�ȶ��ж��Ժ�����û�з�����ȵ�ֵ����ô��ִ��CurrentProcessCount++
				 //��ˣ����������Լ���������Ϊ�˱���ƽ�⡣
				
				CurrentProcessCount--;
				break;
			}
			else 
			{
				if(i==(CurrentProcessCount-1)) //�Ƚϵ���processInfoArray��������һ��Ԫ��
				{
				  
				//��������������flag=1����ʾ����: ������processInfoArray����һֱ��û���ҵ���ͬ��ֵ�����Ǿ��˳���forѭ����
					flag=1; 
				}
				else  continue;
			}

		}

		if(flag==1) 
		{
			//���»�õĽ���ID�źͽ���·�����浽ȫ����������
			processInfoArray[CurrentProcessCount].pid=uPID;
			wcscpy(processInfoArray[CurrentProcessCount].psPath,fullname);
		}

		//�������ۼ�
		CurrentProcessCount++;

		for(i=0;i<CurrentProcessCount;i++)
		{
			DbgPrint("���̵�ID��processInfoArrayPid�� %d\n",processInfoArray[i].pid);
			DbgPrint("$$$$$$$$$$$$���̵�Full Path Name ��: %S\n", processInfoArray[i].psPath);
		}

		if(CurrentProcessCount==MAXPROCESSCOUNT) //����Ԫ�ظ���������ɵ����ֵ
		{
			CurrentProcessCount=0;
		}

		
		
		//·������󳤶����ڴ��������޶�Ϊ1024���ֽڣ���ÿһ���ڻ�ȡ·��ʱ������ʹ��strcat���������ӣ�
		//������ͣ��ʹ��strcat�����������ַ������ܻᳬ��1024�ֽڡ����ʹ��memset���������fullname���������ֵ��
		
		wmemset(fullname,0,sizeof(fullname)/sizeof(WCHAR));
		//memset(fullname,0,sizeof(fullname));
	}


label:

//	DbgPrint("*********Leave HookZwOpenProcess()*********\n");

	return nStatus;
}
*/


//=========================================================================================================//
//Name: NTSTATUS HookZwTerminateProcess()                                                                  //
//											                                                               //
//Descripion: �û�ָ���ġ�����Windowsϵͳ���еĽ��� ����������������ֹ����                                 //
//            				                            						                           //
//=========================================================================================================//
NTSTATUS HookZwTerminateProcess(
								IN HANDLE ProcessHandle,
								IN NTSTATUS ExitStatus
								)
{
	ULONG uPID;
	NTSTATUS rtStatus;
	PCHAR pStrProcName;
	PEPROCESS pEProcess;
	ANSI_STRING strProcName;

	//	DbgPrint("*********Enter HookZwTerminateProcess()*********\n");
	
	//ͨ�����̾������øý�������Ӧ FileObject�������������ǽ��̶�����Ȼ��õ��� EPROCESS ����
	rtStatus = ObReferenceObjectByHandle(ProcessHandle, FILE_READ_DATA, NULL, KernelMode, &pEProcess, NULL);
	if(!NT_SUCCESS(rtStatus))
	{
		rtStatus = pOldNtTerminateProcess(ProcessHandle, ExitStatus);
		return rtStatus;
	}

	//ͨ���ú������Ի�ȡ���������ƺͽ��� ID���ú������ں���ʵ���ǵ�����(�� WRK �п��Կ���)
	//���� ntddk.h �в�û�е�����������Ҫ�Լ���������ʹ��
	uPID = (ULONG)PsGetProcessId(pEProcess);
	//pStrProcName = (PCHAR)PsGetProcessImageFileName(pEProcess);
	//ͨ������������ʼ��һ�� ASCII �ַ���
	//RtlInitAnsiString(&strProcName, pStrProcName);

	KeAcquireSpinLock(&PotectSpinLock,&PotectIrql);
	if(ValidateProcessNeedProtect(uPID) != -1)
	{
		//ȷ�������߽����ܹ�����(������Ҫ��ָ taskmgr.exe)
		if(uPID != (ULONG)PsGetProcessId(PsGetCurrentProcess()))
		{
			//����ý������������Ľ��̣��򷵻�Ȩ�޲������쳣����
			KeReleaseSpinLock(&PotectSpinLock,PotectIrql);
			return STATUS_ACCESS_DENIED;
		}
	}
	KeReleaseSpinLock(&PotectSpinLock,PotectIrql);
	
	//���ڷǱ����Ľ��̿���ֱ�ӵ���ԭ�� SSDT �е� NtTerminateProcess ����������
	rtStatus = pOldNtTerminateProcess(ProcessHandle, ExitStatus);

	//	DbgPrint("*********Leave HookZwTerminateProcess()*********\n");

	return rtStatus;
}

/*
//======================================================================================================//
//Name: NTSTATUS HookZwCreateThread()                                                                   //
//											                                                            //
//Descripion: ��ֹԶ��ע�룬Hook��Ӧ�ò��CreateRemoteThread����                                        //
//            				                            						                        //
//======================================================================================================//
NTSTATUS HookZwCreateThread(
							 OUT PHANDLE             ThreadHandle,
							 IN  ACCESS_MASK         DesiredAccess,
							 IN  POBJECT_ATTRIBUTES  ObjectAttributes,
							 IN  HANDLE              ProcessHandle,//�����������̾��
							 OUT PCLIENT_ID          ClientId,
							 IN  PCONTEXT            ThreadContext,
							 IN  PVOID				UserStack,
							 IN  BOOLEAN             CreateSuspended
							 )
{
	ULONG uPID;
//	ULONG uInjectPid;
	ULONG k;
	PEPROCESS pEProcess;
	NTSTATUS rc;

	rc=STATUS_SUCCESS;

	DbgPrint("*******Enter HookZwCreateThread����*******----------------------------->\n");
	DbgPrint("��ȡȨ��DesiredAccess��ֵ��:    %x\n",DesiredAccess);
	DbgPrint("ProcessHandle��ֵ��:  %x\n",ProcessHandle);
	DbgPrint("ThreadContext��һ�������Ĳ���\n");
	DbgPrint("UserStack��ֵ��:    %x\n",UserStack);


	//ͨ�����̾������øý�������Ӧ FileObject�������������ǽ��̶�����Ȼ��õ��� EPROCESS ����
	rc = ObReferenceObjectByHandle(ProcessHandle, FILE_READ_DATA, NULL, KernelMode, &pEProcess, NULL);
	if(!NT_SUCCESS(rc))
	{
		DbgPrint("****ObReferenceObjectByHandle�������ò��ɹ��������˳�HookZwCreateThread����****\n");
		return rc;
	}

	uPID = (ULONG)PsGetProcessId(pEProcess); //�����������̵ľ��ProcessHandle���õ����Ľ���ID
	DbgPrint("�������̵Ľ���ID:%d", &uPID);

	if(!_stricmp("HavkAv.exe", (PCHAR) pEProcess + g_OffsetEprocessName))			//windows xp 0x174
	{
		KdPrint(("-------------------------------->>>>>>>>>>>"));
		ZwTerminateProcess(ProcessHandle,0);
			
		return STATUS_ACCESS_DENIED;
	}
	if(ValidateProcessNeedProtect(uPID) != -1)		//�����������uPID ��ָ����������g_PIDProtectArray�У���ôֱ�ӷ���
	{
		//ȷ�������߽����ܹ������߳�
		if(uPID != (ULONG)PsGetProcessId(PsGetCurrentProcess()))
		{
			KdPrint(("-----�ж��������Ҫע�뱣���Ľ���---"));

			//����ý������������Ľ��̣��򷵻�Ȩ�޲������쳣����
			return STATUS_ACCESS_DENIED;
		}
	}

	//for(k=0; k<g_currProtectArrayLen; k++)		
	//{
	//	if(uPID == g_PIDProtectArray[k++])
	//	{
	//		KdPrint(("-----�ж��������Ҫע�뱣���Ľ���---"));
	//		return STATUS_ACCESS_DENIED;
	//	}
	//}

	DbgPrint("*******Leave HookZwCreateThread����*******------------------------------->\n");

	rc=RealZwCreateThread(ThreadHandle,
		DesiredAccess,
		ObjectAttributes,
		ProcessHandle,
		ClientId,
		ThreadContext,
		UserStack,
		CreateSuspended
		);

	return  rc;
}


//======================================================================================================//
//Name: NTSTATUS HookZwCreateThreadEx() windows 7                                                       //
//											                                                            //
//Descripion: ��ֹԶ��ע�룬Hook��Ӧ�ò��CreateRemoteThread����                                        //
//            				                            						                        //
//======================================================================================================//
NTSTATUS HookZwCreateThreadEx(
								  OUT PHANDLE ThreadHandle,
								  IN ACCESS_MASK DesiredAccess,
								  IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
								  IN HANDLE ProcessHandle,		//�����������̾��
								  IN PVOID StartRoutine,
								  IN PVOID StartContext,
								  IN ULONG CreateThreadFlags,
								  IN SIZE_T ZeroBits OPTIONAL,
								  IN SIZE_T StackSize OPTIONAL,
								  IN SIZE_T MaximumStackSize OPTIONAL,
								  IN PVOID AttributeList
							 )
{
	ULONG uPID;
//	ULONG uInjectPid;
	ULONG k;
	PEPROCESS pEProcess;
	NTSTATUS rc;

	rc=STATUS_SUCCESS;

	DbgPrint("*******Enter HookZwCreateThreadEx����*******----------------------------->\n");
	//DbgPrint("��ȡȨ��DesiredAccess��ֵ��:    %x\n",DesiredAccess);
	//DbgPrint("ProcessHandle��ֵ��:  %x\n",ProcessHandle);
	//DbgPrint("ThreadContext��һ�������Ĳ���\n");
	//DbgPrint("UserStack��ֵ��:    %x\n",UserStack);


	//ͨ�����̾������øý�������Ӧ FileObject�������������ǽ��̶�����Ȼ��õ��� EPROCESS ����
	rc = ObReferenceObjectByHandle(ProcessHandle, FILE_READ_DATA, NULL, KernelMode, &pEProcess, NULL);
	if(!NT_SUCCESS(rc))
	{
		DbgPrint("****ObReferenceObjectByHandle�������ò��ɹ��������˳�HookZwCreateThread����****\n");
		return rc;
	}

	uPID = (ULONG)PsGetProcessId(pEProcess); //�����������̵ľ��ProcessHandle���õ����Ľ���ID
	DbgPrint("�������̵Ľ���ID:%d\n", &uPID);

	//if( !strncmp( "HavkAv.exe", (PCHAR) pEProcess + 0x16c, strlen("HavkAv.exe") ))
	//{
	//	KdPrint(("-------------------------------->>>>>>>>>>>"));
	//	ZwTerminateProcess(ProcessHandle,0);
	//		
	//	return STATUS_ACCESS_DENIED;
	//}

	if(!_stricmp("HavkAv.exe", (PCHAR) pEProcess + g_OffsetEprocessName))					//windows 7  0x16c
	{
		KdPrint(("-------------------------------->>>>>>>>>>>\n"));
		ZwTerminateProcess(ProcessHandle,0);
			
		return STATUS_ACCESS_DENIED;
	}

	if(ValidateProcessNeedProtect(uPID) != -1)		//�����������uPID ��ָ����������g_PIDProtectArray�У���ôֱ�ӷ���
	{
		//ȷ�������߽����ܹ������߳�
		if(uPID != (ULONG)PsGetProcessId(PsGetCurrentProcess()))
		{
			KdPrint(("-----�ж��������Ҫע�뱣���Ľ���---\n"));

			//����ý������������Ľ��̣��򷵻�Ȩ�޲������쳣����
			return STATUS_ACCESS_DENIED;
		}
	}

	//for(k=0; k<g_currProtectArrayLen; k++)		//�����������uPID ��ָ����������g_PIDProtectArray�У���ôֱ�ӷ���
	//{
	//	if(uPID == g_PIDProtectArray[k++])
	//	{
	//		KdPrint(("-----�ж��������Ҫע�뱣���Ľ���---"));
	//		return STATUS_ACCESS_DENIED;
	//	}
	//}

	DbgPrint("*******Leave HookZwCreateThreadEx����*******------------------------------->\n");

	rc=RealZwCreateThreadEx(ThreadHandle,
							DesiredAccess,
							ObjectAttributes ,
							ProcessHandle,		//�����������̾��
							StartRoutine,
							StartContext,
							CreateThreadFlags,
							ZeroBits,
							StackSize,
							MaximumStackSize,
							AttributeList
		);

	return  rc;
}

//======================================================================================================//
//Name: NTSTATUS GetProcessNameOffset()																    //
//											                                                            //
//Descripion: ��̬��ȡEPROCESS�ṹ�н�������ƫ�ƣ�����Windows�汾                                       //
//            				                            						                        //
//======================================================================================================//
NTSTATUS GetProcessNameOffset(
	OUT PULONG	Offset OPTIONAL )
{
	NTSTATUS	status;
	PEPROCESS	curproc;
	ULONG			i;

	if (!MmIsAddressValid((PVOID)Offset))
	{
		status = STATUS_INVALID_PARAMETER;
		return status;
	}

	curproc = PsGetCurrentProcess();

	//
	// Ȼ������KPEB���õ�ProcessName���KPEB��ƫ����
	// ƫ��174h��λ�ã��������ǽ��̵Ķ��ļ����������ط��ã�
	// ����SoftIce��addr��proc���������Ƴ���16���ַ�ֱ�ӽض�

	// Scan for 12KB, hopping the KPEB never grows that big!
	//
	for( i = 0; i < 3 * PAGE_SIZE; i++ ) {

		if(!strncmp( "System", (PCHAR) curproc + i, strlen("System"))) {
			*Offset = i;
			status = STATUS_SUCCESS;
			break;
		}
	}
	return status;
}*/


