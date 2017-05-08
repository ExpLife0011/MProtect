
// IMProtectDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "IMProtect.h"
#include "IMProtectDlg.h"
#include "afxdialogex.h"
#include <tlhelp32.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
#define VNAME(name,name2) AddExtensionInfo(#name,name2,name)
#define VNAME2(name,name2) AddExtensionInfo2(#name,name2,name)
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CIMProtectDlg �Ի���



CIMProtectDlg::CIMProtectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IMPROTECT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIMProtectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, list_process_protect_);
	DDX_Control(pDX, IDC_COMBO1, combox_processlist_);
	DDX_Control(pDX, IDC_LIST2, list_process_hide_);
	DDX_Control(pDX, IDC_COMBO2, combox_processlist2_);
	DDX_Control(pDX, IDC_LIST3, list_infoshow_);
}

BEGIN_MESSAGE_MAP(CIMProtectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_DROPDOWN(IDC_COMBO1, &CIMProtectDlg::OnCbnDropdownCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CIMProtectDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON1, &CIMProtectDlg::OnBnClickedButton1)
	ON_CBN_DROPDOWN(IDC_COMBO2, &CIMProtectDlg::OnCbnDropdownCombo2)
	ON_BN_CLICKED(IDC_BUTTON3, &CIMProtectDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, &CIMProtectDlg::OnBnClickedButton2)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CIMProtectDlg::OnNMRClickList1)
	ON_BN_CLICKED(IDC_BUTTON4, &CIMProtectDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON_GET_CPUINO, &CIMProtectDlg::OnBnClickedButtonGetCpuino)
	ON_BN_CLICKED(IDC_BUTTON_GET_DISKINFO, &CIMProtectDlg::OnBnClickedButtonGetDiskinfo)
END_MESSAGE_MAP()


// CIMProtectDlg ��Ϣ�������

BOOL CIMProtectDlg::OnInitDialog()
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

	
	DWORD dwStyle = list_process_protect_.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��

	list_process_protect_.SetExtendedStyle(dwStyle); //������չ���
	list_process_protect_.InsertColumn(0, L"ProcessName", LVCFMT_LEFT, 140);
	list_process_hide_.SetExtendedStyle(dwStyle); //������չ���
	list_process_hide_.InsertColumn(0, L"ProcessName", LVCFMT_LEFT, 140);

	dwStyle = list_infoshow_.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;// ѡ��ĳ��ʹ���и�����ֻ������report ����listctrl �� 
	dwStyle |= LVS_EX_GRIDLINES;// �����ߣ�ֻ������report ����listctrl �� 
								//dwStyle |= LVS_EX_CHECKBOXES;//item ǰ����checkbox �ؼ� 
	dwStyle |= LVS_EX_SUBITEMIMAGES;
	list_infoshow_.SetExtendedStyle(dwStyle); // ������չ��� 
	list_infoshow_.InsertColumn(0, L"English", LVCFMT_LEFT, 125);
	list_infoshow_.InsertColumn(1, L"Chinese", LVCFMT_LEFT, 200);
	list_infoshow_.InsertColumn(2, L"Support", LVCFMT_LEFT, 250);

	MessageBox(L"", NULL);

	Improtect_ = new IMProtect;
	Imgetdata_ = new IMGetData;
	GetShadowSsdtSym_ = new GetShadowSsdtSym;

	//���64λϵͳʹ��32λ�ͻ����򱨴��˳�
	if (GetShadowSsdtSym_->Is64Bit_OS() && sizeof(void*) == 4){
		MessageBox(L"64λϵͳ��ʹ��64λ�ͻ��˳���", NULL);
		exit(0);
	}

	if (!GetShadowSsdtSym_->Init()){
		auto error = GetLastError();
		if (error) {
			wchar_t ser[125];
			_itow(error, ser, 10);
			CString s = L"Win32k.sys�ķ�������ʧ�ܣ�";
			s += ser;
			MessageBox(s, NULL);
		}
		exit(0);
	}
	

	MessageBox(L"", NULL);
	if (!Improtect_->LoadDriver()){
		auto error = GetLastError();
		if (error) {
			wchar_t ser[125];
			_itow(error, ser, 10);
			CString s = L"������������";
			s += ser;
			MessageBox(s, NULL);
		}
	}
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CIMProtectDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CIMProtectDlg::OnPaint()
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
HCURSOR CIMProtectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CIMProtectDlg::OnCbnDropdownCombo1()
{
	// ���������Ϣ�ṹ  
	PROCESSENTRY32 pe32 = { sizeof(pe32) };
	auto hProcessShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessShot == INVALID_HANDLE_VALUE) {
		return;
	}

	combox_processlist_.ResetContent();

	if (Process32First(hProcessShot, &pe32)) {
		do {
			if (combox_processlist_.FindString(-1, pe32.szExeFile) == CB_ERR) {
				combox_processlist_.AddString(pe32.szExeFile);
			}

		} while (Process32Next(hProcessShot, &pe32));
	}
	CloseHandle(hProcessShot);
}


void CIMProtectDlg::OnCbnSelchangeCombo1()
{
	
}


void CIMProtectDlg::OnBnClickedButton1()
{
	LVFINDINFO info;
	CString str;
	combox_processlist_.GetLBText(combox_processlist_.GetCurSel(), str);
	if (str != L"") {
		info.flags = LVFI_PARTIAL | LVFI_STRING;
		info.psz = str;
		if (list_process_protect_.FindItem(&info) == CB_ERR) {
			list_process_protect_.InsertItem(0, str);
		}
	}
}


void CIMProtectDlg::OnCbnDropdownCombo2()
{
	// ���������Ϣ�ṹ  
	PROCESSENTRY32 pe32 = { sizeof(pe32) };
	auto hProcessShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessShot == INVALID_HANDLE_VALUE) {
		return;
	}

	combox_processlist2_.ResetContent();

	if (Process32First(hProcessShot, &pe32)) {
		do {
			if (combox_processlist2_.FindString(-1, pe32.szExeFile) == CB_ERR) {
				combox_processlist2_.AddString(pe32.szExeFile);
			}

		} while (Process32Next(hProcessShot, &pe32));
	}
	CloseHandle(hProcessShot);
}


void CIMProtectDlg::OnBnClickedButton3()
{
	LVFINDINFO info;
	CString str;
	combox_processlist2_.GetLBText(combox_processlist2_.GetCurSel(), str);
	if (str != L"") {
		info.flags = LVFI_PARTIAL | LVFI_STRING;
		info.psz = str;
		if (list_process_hide_.FindItem(&info) == CB_ERR) {
			list_process_hide_.InsertItem(0, str);
		}
	}
}


void CIMProtectDlg::OnBnClickedButton2()
{
	//�����¼���r0
	if (Improtect_->ProcessHideRemoval() ||
		Improtect_->ProcessProtectionRemoval() ){
		auto error = GetLastError();
		AfxMessageBox(L"Ӧ��ʧ��!");
		return;
	}

	int Num = list_process_hide_.GetItemCount();
	for (int i = 0; i < Num; i++) {
		auto Text = list_process_hide_.GetItemText(i, 0);
		if (Improtect_->ProcessHideAdd(Text.GetBuffer(0), 0))
		{
			AfxMessageBox(L"���ݴ���ʧ��!");
			return;
		}
	}

	Num = list_process_protect_.GetItemCount();
	for (int i = 0; i < Num; i++) {
		auto Text = list_process_protect_.GetItemText(i, 0);
		if (Improtect_->ProcessProtectionAdd(Text.GetBuffer(0), 0))
		{
			AfxMessageBox(L"���ݴ���ʧ��!");
			return;
		}
	}
	AfxMessageBox(L"Ӧ�óɹ�!");
}



void CIMProtectDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	CMenu menu; //��������Ҫ�õ���cmenu����
	menu.LoadMenu(IDR_MENU1); //װ���Զ�����Ҽ��˵� 
	CMenu *pPopup = menu.GetSubMenu(0); //��ȡ��һ�������˵������Ե�һ���˵��������Ӳ˵�
	CPoint point1;//����һ������ȷ�����λ�õ�λ�� 
	GetCursorPos(&point1);//��ȡ��ǰ����λ�ã��Ա�ʹ�ò˵����Ը����� 
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, GetParent());//��ָ��λ����ʾ�����˵�

	*pResult = 0;
}


void CIMProtectDlg::OnBnClickedButton4()
{
	if (Improtect_->UnloadNTDriver()){
		AfxMessageBox(L"ж�سɹ�!");
	}
	else {
		AfxMessageBox(L"ж��ʧ��!");
	}
}


void CIMProtectDlg::AddExtensionInfo(char *English, char* Chinese, bool Support)
{
	CString s;
	s = English;
	auto Index = list_infoshow_.InsertItem(list_infoshow_.GetItemCount(), s);
	s = Chinese;
	list_infoshow_.SetItemText(Index, 1, s);
	if (Support) {
		list_infoshow_.SetItemText(Index, 2, L"yes");
	}
	else {
		list_infoshow_.SetItemText(Index, 2, L"no");
	}
}

void CIMProtectDlg::AddExtensionInfo2(char *English, char* Chinese, ULONGLONG Support)
{
	char cs[100];
	CString s;

	s = English;
	auto Index = list_infoshow_.InsertItem(list_infoshow_.GetItemCount(), s);
	s = Chinese;
	list_infoshow_.SetItemText(Index, 1, s);
	itoa(Support, cs, 10);
	s = cs;
	list_infoshow_.SetItemText(Index, 2, s);
}

void CIMProtectDlg::OnBnClickedButtonGetCpuino()
{
	CStringW aTow;
	list_infoshow_.DeleteAllItems();
	auto ecx = Imgetdata_->GetCpuFeaturesInfo1();
	auto edx = Imgetdata_->GetCpuFeaturesInfo2();
	aTow = Imgetdata_->GetCpuInfo(GETINFOTYPE::Constructor).c_str();
	auto Index = list_infoshow_.InsertItem(list_infoshow_.GetItemCount(), L"Constructor");
	list_infoshow_.SetItemText(Index, 1, L"������");
	list_infoshow_.SetItemText(Index, 2, aTow);

	aTow = Imgetdata_->GetCpuInfo(GETINFOTYPE::SerialNumber).c_str();
	Index = list_infoshow_.InsertItem(list_infoshow_.GetItemCount(), L"SerialNumber");
	list_infoshow_.SetItemText(Index, 1, L"���к�");
	list_infoshow_.SetItemText(Index, 2, aTow);

	aTow = Imgetdata_->GetCpuInfo(GETINFOTYPE::Trademarks).c_str();
	Index = list_infoshow_.InsertItem(list_infoshow_.GetItemCount(), L"Trademarks");
	list_infoshow_.SetItemText(Index, 1, L"�̱�");
	list_infoshow_.SetItemText(Index, 2, aTow);

	VNAME(ecx.fields.sse3, "simd��������չ3(SSE3) ");
	VNAME(ecx.fields.pclmulqdq, "PCLMULQDQ ");
	VNAME(ecx.fields.dtes64, "64λDS����");
	VNAME(ecx.fields.monitor, "��ʾ��/��");
	VNAME(ecx.fields.ds_cpl, "CPL�ϸ�ĵ��Դ洢");
	VNAME(ecx.fields.vmx, "���������");
	VNAME(ecx.fields.smx, "��ȫģʽ��չ");
	VNAME(ecx.fields.est, "��ǿ��Ӣ�ض�Speedstep����");
	VNAME(ecx.fields.tm2, "ɢ�ȼ��2");
	VNAME(ecx.fields.ssse3, "����simd��������չ3");
	VNAME(ecx.fields.cid, "L1������ID");
	VNAME(ecx.fields.sdbg, "IA32_DEBUG_INTERFACE MSR");
	VNAME(ecx.fields.fma, "ʹ��YMM״̬FMA��չ");
	VNAME(ecx.fields.cx16, "CMPXCHG16B");
	VNAME(ecx.fields.xtpr, "xTPR���¿���");
	VNAME(ecx.fields.pdcm, "����/��������MSR");
	VNAME(ecx.fields.reserved, "����");
	VNAME(ecx.fields.pcid, "����������ı�ʶ��");
	VNAME(ecx.fields.dca, "��Ǩ���ڴ�ӳ���豸");
	VNAME(ecx.fields.sse4_1, "SSE4.1");
	VNAME(ecx.fields.sse4_2, "SSE4.2");
	VNAME(ecx.fields.x2_apic, "x2APIC����");
	VNAME(ecx.fields.movbe, "MOVBEָ��");
	VNAME(ecx.fields.popcnt, "POPCNTָ��");
	VNAME(ecx.fields.reserved3, "ʹ��TSC����һ���Բ���");
	VNAME(ecx.fields.aes, "AESNIָ��");
	VNAME(ecx.fields.xsave, "XSAVE/XRSTOR����");
	VNAME(ecx.fields.osxsave, "ʹXSETBV/XGETBV˵��");
	VNAME(ecx.fields.avx, "AVXָ����չ");
	VNAME(ecx.fields.f16c, "16λ����ת��");
	VNAME(ecx.fields.rdrand, "RDRANDָ��");
	VNAME(ecx.fields.not_used, "0(a.k.һ�� HypervisorPresent)");


	VNAME(edx.fields.fpu, "���㵥Ԫ��Ƭ��");
	VNAME(edx.fields.vme, "����8086ģʽ��ǿ");
	VNAME(edx.fields.de, "������չ����");
	VNAME(edx.fields.pse, "ҳ��С��չ");
	VNAME(edx.fields.tsc, "ʱ���������");
	VNAME(edx.fields.msr, "RDMSR��WRMSR˵��");
	VNAME(edx.fields.mce, "��������쳣");
	VNAME(edx.fields.cx8, "ɢ�ȼ��2");
	VNAME(edx.fields.apic, "APICƬ��");
	VNAME(edx.fields.reserved1, "����");
	VNAME(edx.fields.sep, "SYSENTER��SYSEXIT˵��");
	VNAME(edx.fields.mtrr, "�ڴ淶Χ�Ĵ���");
	VNAME(edx.fields.pge, "ҳȫ��λ");
	VNAME(edx.fields.mca, "�������ܹ�");
	VNAME(edx.fields.cmov, "���������ƶ�ָ��");
	VNAME(edx.fields.pat, "ҳ���Ա�");
	VNAME(edx.fields.pse36, "36λҳ���С��չ");
	VNAME(edx.fields.psn, "���������к�");
	VNAME(edx.fields.clfsh, "CLFLUSHָ��");
	VNAME(edx.fields.reserved2, "����");
	VNAME(edx.fields.ds, "�ĵ��Դ洢");
	VNAME(edx.fields.acpi, "TM���������ʱ��");
	VNAME(edx.fields.mmx, "Ӣ�ض�MMX����");
	VNAME(edx.fields.fxsr, "FXSAVE��FXRSTOR˵��");
	VNAME(edx.fields.sse, "SSE");
	VNAME(edx.fields.sse2, "SSE2");
	VNAME(edx.fields.ss, "��̽��");
	VNAME(edx.fields.htt, "���������APIC id�ֶ���Ч");
	VNAME(edx.fields.tm, "ɢ�ȼ��");
	VNAME(edx.fields.reserved3, "����");
	VNAME(edx.fields.pbe, "����ķ��з�����");
}


void CIMProtectDlg::OnBnClickedButtonGetDiskinfo()
{
	CStringW aTow;
	list_infoshow_.DeleteAllItems();

	aTow = Imgetdata_->GetDiskInfo(GETINFOTYPE::FirmwareRev).c_str();
	auto Index = list_infoshow_.InsertItem(list_infoshow_.GetItemCount(), L"Constructor");
	list_infoshow_.SetItemText(Index, 1, L"�̼��汾");
	list_infoshow_.SetItemText(Index, 2, aTow);

	aTow = Imgetdata_->GetDiskInfo(GETINFOTYPE::SerialNumber).c_str();
	Index = list_infoshow_.InsertItem(list_infoshow_.GetItemCount(), L"SerialNumber");
	list_infoshow_.SetItemText(Index, 1, L"���к�");
	list_infoshow_.SetItemText(Index, 2, aTow);

	aTow = Imgetdata_->GetDiskInfo(GETINFOTYPE::ModelNumber).c_str();
	Index = list_infoshow_.InsertItem(list_infoshow_.GetItemCount(), L"Trademarks");
	list_infoshow_.SetItemText(Index, 1, L"�ڲ��ͺ�");
	list_infoshow_.SetItemText(Index, 2, aTow);
}