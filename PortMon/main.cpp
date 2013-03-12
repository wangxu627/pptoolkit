#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <commctrl.h>
#include "resource.h"
#pragma comment(lib,"comctl32.lib")

#define BUFSIZE 1024 * 1024

UINT_PTR g_hTimer;

void FillItem(HWND hList)
{
	SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(saAttr);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    
    HANDLE ChildIn_Read,ChildIn_Write;
    ::CreatePipe(&ChildIn_Read,&ChildIn_Write,&saAttr,0);
    ::SetHandleInformation(ChildIn_Write,HANDLE_FLAG_INHERIT,0);
 
    HANDLE ChildOut_Read,ChildOut_Write;
    ::CreatePipe(&ChildOut_Read,&ChildOut_Write,&saAttr,0);
    ::SetHandleInformation(ChildOut_Read,HANDLE_FLAG_INHERIT,0);


    STARTUPINFO si = {sizeof(si)};
    si.hStdError = ChildOut_Write;
    si.hStdOutput = ChildOut_Write;
    si.hStdInput = ChildIn_Read;
    si.dwFlags |= STARTF_USESTDHANDLES;
    
    PROCESS_INFORMATION pi;
    TCHAR szCmd[] = TEXT("netstat -an");
    ::CreateProcess(
        NULL,
        szCmd,
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi);
        
    char* buffer = new char[BUFSIZE];
    DWORD dwReaded = 0;
           
    BOOL bRet = ::ReadFile(ChildOut_Read,buffer,BUFSIZE,&dwReaded,NULL);  
    
    int i = 4;
    char* pb = buffer;
    while(i-- > 0)
    {
        pb = strchr(pb,'\n');
        ++pb;
    }
    
	int line_idx = 0;
    pb--;
    while(pb)
    {
        char line_buf[128];
        char* pbo = pb;
        pb = strchr(pb + 1,'\n');
        if(pb == NULL)
        {
            break;
        }
        int diff = pb - pbo;
        memcpy(line_buf,pbo,diff);
        line_buf[diff] = 0;
        //printf(line_buf);

        char proc[4] = {0};
        char local_addr[23] = {0};
        char forei_addr[23] = {0};
        char state[12] = {0};
        memcpy(proc,        line_buf + 3,    4  ); proc[3] = 0;
        memcpy(local_addr,  line_buf + 10,   23 ); local_addr[22] = 0;
        memcpy(forei_addr,  line_buf + 33,   23 ); forei_addr[22] = 0;
        memcpy(state,       line_buf + 56,   12 ); state[11] = 0;
        
        //printf("%s | %s | %s | %s\n",proc,local_addr,forei_addr,state);

		LVITEM lvItem = {0};
		lvItem.mask = LVIF_TEXT;
		lvItem.cchTextMax = 256;
		lvItem.iItem = line_idx;
		lvItem.iSubItem = 0;
		lvItem.pszText = proc;
		ListView_InsertItem(hList,&lvItem);

		lvItem.iSubItem = 1;
		lvItem.pszText = local_addr;
		ListView_SetItem(hList,&lvItem);

		lvItem.iSubItem = 2;
		lvItem.pszText = forei_addr;
		ListView_SetItem(hList,&lvItem);

		lvItem.iSubItem = 3;
		lvItem.pszText = state;
		ListView_SetItem(hList,&lvItem);

		line_idx++;
    } 
}

BOOL RefreshList(HWND hWnd,UINT)
{
	HWND hList = ::GetDlgItem(hWnd,IDC_LIST1);
	ListView_DeleteAllItems(hList);
	FillItem(hList);
	return FALSE;
}

BOOL Handle_Init(HWND hWnd,HWND,LPARAM)
{
    ::InitCommonControls();
    HWND hList = ::GetDlgItem(hWnd,IDC_LIST1);
    
    RECT rect;
    ::GetWindowRect(hList,&rect);
    int w = rect.right - rect.left;
   
    LVCOLUMN lvCol = {0};
    lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lvCol.pszText = TEXT("PROTOCAL");
    lvCol.cx = w * 0.15;
    ListView_InsertColumn(hList,0,&lvCol);
    
    lvCol.pszText = TEXT("LOCAL ADDR");
    lvCol.cx = w * 0.34 ;
    ListView_InsertColumn(hList,1,&lvCol);
    
    lvCol.pszText = TEXT("FOREIGN ADDR");
    lvCol.cx = w * 0.34;
    ListView_InsertColumn(hList,2,&lvCol);
    
    lvCol.pszText = TEXT("STATE");
    lvCol.cx = w * 0.15;
    ListView_InsertColumn(hList,3,&lvCol);

	FillItem(hList);

	g_hTimer = ::SetTimer(hWnd,0,5000,NULL);
	
    return TRUE;
}
        
void Handle_Resize(HWND hWnd,UINT wParam, int cx,int cy)
{
	 HWND hList = ::GetDlgItem(hWnd,IDC_LIST1);

	 RECT rect;
	 ::GetWindowRect(hList,&rect);

	 ::SetWindowPos(hList,HWND_TOP,0,0,cx - 20,cy - 20,SWP_NOMOVE);
}

INT_PTR CALLBACK Dlg_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        return HANDLE_WM_INITDIALOG(hDlg,wParam,lParam,Handle_Init);
	case WM_TIMER:
		return HANDLE_WM_TIMER(hDlg,wParam,lParam,RefreshList);
	case WM_SIZE:
		return HANDLE_WM_SIZE(hDlg,wParam,lParam,Handle_Resize);
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_REFRESH:
				return RefreshList(hDlg,0);
			case ID_40004:
				::KillTimer(hDlg,g_hTimer);
				g_hTimer = ::SetTimer(hDlg,0,1000,NULL);
				return TRUE;
			case ID_40005:
				::KillTimer(hDlg,g_hTimer);
				g_hTimer = ::SetTimer(hDlg,0,3000,NULL);
				return TRUE;
			case ID_40006:
				::KillTimer(hDlg,g_hTimer);
				g_hTimer = ::SetTimer(hDlg,0,5000,NULL);
				return TRUE;
			case ID_40007:
				::KillTimer(hDlg,g_hTimer);
				g_hTimer = ::SetTimer(hDlg,0,10000,NULL);
				return TRUE;
			}
		}
    case IDOK:
    case IDCANCEL:
    case WM_CLOSE:
        return ::EndDialog(hDlg,0);
    }
//    return ::DefDlgProc(hDlg,uMsg,wParam,lParam);
    return NULL;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE,LPSTR lpCmdLine,int nShowCmd)
{
    DialogBox(hInstance,MAKEINTRESOURCE(IDD_DIALOG1),NULL,Dlg_Proc);

    return 0;
}

#if 0

#include <stdio.h>
#include <windows.h>

#define BUFSIZE 4096

int main(int argc,char* argv[])
{
 //   char* pp = NULL;
 //   pp = (char*)malloc(0);
    
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(saAttr);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    
    HANDLE ChildIn_Read,ChildIn_Write;
    ::CreatePipe(&ChildIn_Read,&ChildIn_Write,&saAttr,0);
    ::SetHandleInformation(ChildIn_Write,HANDLE_FLAG_INHERIT,0);
 
    HANDLE ChildOut_Read,ChildOut_Write;
    ::CreatePipe(&ChildOut_Read,&ChildOut_Write,&saAttr,0);
    ::SetHandleInformation(ChildOut_Read,HANDLE_FLAG_INHERIT,0);


    STARTUPINFO si = {sizeof(si)};
    si.hStdError = ChildOut_Write;
    si.hStdOutput = ChildOut_Write;
    si.hStdInput = ChildIn_Read;
    si.dwFlags |= STARTF_USESTDHANDLES;
    
    PROCESS_INFORMATION pi;
    WCHAR szCmd[] = TEXT("netstat -an");
    ::CreateProcess(
        NULL,
        szCmd,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi);
        
    //char buffer[BUFSIZE] = {0};
    char* buffer = new char[BUFSIZE];
    DWORD dwReaded = 0;
    
  /*  string buf;
    do
    {
        BOOL bRet = ::ReadFile(ChildOut_Read,buffer,BUFSIZE,&dwReaded,NULL);  
        buf += buffer;
    }while(dwReaded != 0);*/
       
    BOOL bRet = ::ReadFile(ChildOut_Read,buffer,BUFSIZE,&dwReaded,NULL);  
    
    int i = 4;
    char* pb = buffer;
    while(i-- > 0)
    {
        pb = strchr(pb,'\n');
        ++pb;
    }
    
    pb--;
    while(pb)
    {
        char line_buf[128];
        char* pbo = pb;
        pb = strchr(pb + 1,'\n');
        if(pb == NULL)
        {
            break;
        }
        int diff = pb - pbo;
        memcpy(line_buf,pbo,diff);
        line_buf[diff] = 0;
        //printf(line_buf);

        char proc[4] = {0};
        char local_addr[23] = {0};
        char forei_addr[23] = {0};
        char state[12] = {0};
        memcpy(proc,        line_buf + 3,    4  ); proc[3] = 0;
        memcpy(local_addr,  line_buf + 10,   23 ); local_addr[22] = 0;
        memcpy(forei_addr,  line_buf + 33,   23 ); forei_addr[22] = 0;
        memcpy(state,       line_buf + 56,   12 ); state[11] = 0;
        
        printf("%s | %s | %s | %s\n",proc,local_addr,forei_addr,state);

    } 
   
    FILE* fp = fopen("output.txt","w");
    fwrite(buffer,1,dwReaded,fp);
    fclose(fp);
        
    delete [] buffer;

    
    getchar();
}
#endif