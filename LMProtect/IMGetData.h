#pragma once
#include <string>

enum GETINFOTYPE {
	Constructor = 0,//������
	Trademarks,//�̱�
	SerialNumber,//���к�
	FirmwareRev,//�̼��汾
	ModelNumber//�ڲ��ͺ�
};


union CpuFeaturesInfo1 {
	unsigned int all;
	struct {
		unsigned int sse3 : 1;       //�� <[0]simd��������չ3(SSE3) 
		unsigned int pclmulqdq : 1;  //�� <[1]PCLMULQDQ 
		unsigned int dtes64 : 1;     //�� <[2]64λDS����
		unsigned int monitor : 1;    //�� <[3]��ʾ��/��
		unsigned int ds_cpl : 1;     //�� <[4]CPL�ϸ�ĵ��Դ洢 
		unsigned int vmx : 1;        //�� <[5]��������� 
		unsigned int smx : 1;        //�� <[6]��ȫģʽ��չ
		unsigned int est : 1;        //�� <[7]��ǿ��Ӣ�ض�Speedstep���� 
		unsigned int tm2 : 1;        //�� <[8]ɢ�ȼ��2 
		unsigned int ssse3 : 1;      //�� <[9]����simd��������չ3
		unsigned int cid : 1;        //�� <[10]L1������ID 
		unsigned int sdbg : 1;       //�� <[11]IA32_DEBUG_INTERFACE MSR 
		unsigned int fma : 1;        //�� <[12]ʹ��YMM״̬FMA��չ
		unsigned int cx16 : 1;       //�� <[13]CMPXCHG16B 
		unsigned int xtpr : 1;       //�� <[14]xTPR���¿���
		unsigned int pdcm : 1;       //�� <[15]����/��������MSR 
		unsigned int reserved : 1;   //�� <[16]���� 
		unsigned int pcid : 1;       //�� <[17]����������ı�ʶ�� 
		unsigned int dca : 1;        //�� <[18]��Ǩ���ڴ�ӳ���豸
		unsigned int sse4_1 : 1;     //�� <[19]SSE4.1 
		unsigned int sse4_2 : 1;     //�� <[20]SSE4.2 
		unsigned int x2_apic : 1;    //�� <[21]x2APIC���� 
		unsigned int movbe : 1;      //�� <[22]MOVBEָ�� 
		unsigned int popcnt : 1;     //�� <[23]POPCNTָ��
		unsigned int reserved3 : 1;  //�� <[24]ʹ��TSC����һ���Բ��� 
		unsigned int aes : 1;        //�� <[25]AESNIָ�� 
		unsigned int xsave : 1;      //�� <[26]XSAVE/XRSTOR���� 
		unsigned int osxsave : 1;    //�� <[27]ʹXSETBV/XGETBV˵�� 
		unsigned int avx : 1;        //�� <[28]AVXָ����չ 
		unsigned int f16c : 1;       //�� <[29]16λ����ת�� 
		unsigned int rdrand : 1;     //�� <[30]RDRANDָ�� 
		unsigned int not_used : 1;   //�� <[31]0(a.k.һ�� HypervisorPresent)
	} fields;
};
static_assert(sizeof(CpuFeaturesInfo1) == 4, "Size check");

union CpuFeaturesInfo2 {
	unsigned int all;
	struct {
		unsigned int fpu : 1;        //�� <[0]���㵥Ԫ��Ƭ�� 
		unsigned int vme : 1;        //�� <[1]����8086ģʽ��ǿ 
		unsigned int de : 1;         //�� <[2]������չ����
		unsigned int pse : 1;        //�� <[3]ҳ��С��չ 
		unsigned int tsc : 1;        //�� <[4]ʱ��������� 
		unsigned int msr : 1;        //�� <[5]RDMSR��WRMSR˵��
		unsigned int mce : 1;        //�� <[7]��������쳣 
		unsigned int cx8 : 1;        //�� <[8]ɢ�ȼ��2 
		unsigned int apic : 1;       //�� <[9]APICƬ�� 
		unsigned int reserved1 : 1;  //�� <[10]����
		unsigned int sep : 1;        //�� <[11]SYSENTER��SYSEXIT˵�� 
		unsigned int mtrr : 1;       //�� <[12]�ڴ淶Χ�Ĵ��� 
		unsigned int pge : 1;        //�� <[13]ҳȫ��λ
		unsigned int mca : 1;        //�� <[14]�������ܹ� 
		unsigned int cmov : 1;       //�� <[15]���������ƶ�ָ�� 
		unsigned int pat : 1;        //�� <[16]ҳ���Ա�
		unsigned int pse36 : 1;      //�� <[17]36λҳ���С��չ 
		unsigned int psn : 1;        //�� <[18]���������к� 
		unsigned int clfsh : 1;      //�� <[19]CLFLUSHָ��
		unsigned int reserved2 : 1;  //�� <[20]���� 
		unsigned int ds : 1;         //�� <[21]�ĵ��Դ洢 
		unsigned int acpi : 1;       //�� <[22]TM���������ʱ�� 
		unsigned int mmx : 1;        //�� <[23]Ӣ�ض�MMX����
		unsigned int fxsr : 1;       //�� <[24]FXSAVE��FXRSTOR˵�� 
		unsigned int sse : 1;        //�� <[25]SSE 
		unsigned int sse2 : 1;       //�� <[26]SSE2 
		unsigned int ss : 1;         //�� <[27]��̽�� 
		unsigned int htt : 1;        //�� <[28]���������APIC id�ֶ���Ч 
		unsigned int tm : 1;         //�� <[29]ɢ�ȼ�� 
		unsigned int reserved3 : 1;  //�� <[30]���� 
		unsigned int pbe : 1;        //�� <[31]����ķ��з�����
	} fields;
};
static_assert(sizeof(CpuFeaturesInfo2) == 4, "Size check");

class IMGetData
{
public:
	IMGetData();
	~IMGetData();

	std::string GetCpuInfo(GETINFOTYPE GetDataType);
	CpuFeaturesInfo1 IMGetData::GetCpuFeaturesInfo1();
	CpuFeaturesInfo2 IMGetData::GetCpuFeaturesInfo2();
	std::string GetDiskInfo(GETINFOTYPE GetDataType);

};

