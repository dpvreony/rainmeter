/*
  Copyright (C) 2011 Birunthan Mohanathas, Kimmo Pekkola

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "StdAfx.h"
#include "Rainmeter.h"
#include "System.h"
#include "MeterWindow.h"
#include "TrayWindow.h"
#include "Measure.h"
#include "resource.h"
#include "DialogAbout.h"
#include "../Version.h"

extern CRainmeter* Rainmeter;

WINDOWPLACEMENT CDialogAbout::c_WindowPlacement = {0};
CDialogAbout* CDialogAbout::c_Dialog = NULL;

/*
** Constructor.
**
*/
CDialogAbout::CDialogAbout() : CDialog()
{
}

/*
** Destructor.
**
*/
CDialogAbout::~CDialogAbout()
{
}

/*
** Opens the About dialog.
**
*/
void CDialogAbout::Open(int tab)
{
	if (!c_Dialog)
	{
		c_Dialog = new CDialogAbout();
	}

	c_Dialog->ShowDialogWindow(
		GetString(ID_STR_ABOUTRAINMETER),
		0, 0, 400, 210,
		DS_CENTER | WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
		WS_EX_APPWINDOW | WS_EX_CONTROLPARENT | ((*GetString(ID_STR_ISRTL) == L'1') ? WS_EX_LAYOUTRTL : 0),
		Rainmeter->GetWindow());

	// Fake WM_NOTIFY to change tab
	NMHDR nm;
	nm.code = TCN_SELCHANGE;
	nm.idFrom = Id_Tab;
	nm.hwndFrom = c_Dialog->GetControl(Id_Tab);
	TabCtrl_SetCurSel(nm.hwndFrom, tab);
	c_Dialog->OnNotify(0, (LPARAM)&nm);
}

/*
** Opens the About dialog by tab name.
**
*/
void CDialogAbout::Open(const WCHAR* name)
{
	int tab = 0;

	if (name)
	{
		if (_wcsicmp(name, L"Skins") == 0 ||
			_wcsicmp(name, L"Measures") == 0)	// For backwards compatibility
		{
			tab = 1;
		}
		else if (_wcsicmp(name, L"Plugins") == 0)
		{
			tab = 2;
		}
		else if (_wcsicmp(name, L"Version") == 0)
		{
			tab = 3;
		}
	}

	Open(tab);
}

/*
** Shows log if dialog isn't already open.
**
*/
void CDialogAbout::ShowAboutLog()
{
	if (!c_Dialog)
	{
		Open();
	}
}

void CDialogAbout::AddLogItem(int level, LPCWSTR time, LPCWSTR message)
{
	if (c_Dialog && c_Dialog->m_TabLog.IsInitialized())
	{
		c_Dialog->m_TabLog.AddItem(level, time, message);
	}
}

void CDialogAbout::UpdateSkins()
{
	if (c_Dialog && c_Dialog->m_TabSkins.IsInitialized())
	{
		c_Dialog->m_TabSkins.UpdateSkinList();
	}
}

void CDialogAbout::UpdateMeasures(CMeterWindow* meterWindow)
{
	if (c_Dialog && c_Dialog->m_TabSkins.IsInitialized())
	{
		c_Dialog->m_TabSkins.UpdateMeasureList(meterWindow);
	}
}

CDialog::CTab& CDialogAbout::GetActiveTab()
{
	int sel = TabCtrl_GetCurSel(GetControl(Id_Tab));
	if (sel == 0)
	{
		return m_TabLog;
	}
	else if (sel == 1)
	{
		return m_TabSkins;
	}
	else if (sel == 2)
	{
		return m_TabPlugins;
	}
	else // if (sel == 3)
	{
		return m_TabVersion;
	}
}

INT_PTR CDialogAbout::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return OnInitDialog(wParam, lParam);

	case WM_ACTIVATE:
		return OnActivate(wParam, lParam);

	case WM_COMMAND:
		return OnCommand(wParam, lParam);

	case WM_NOTIFY:
		return OnNotify(wParam, lParam);

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = 550;
			mmi->ptMinTrackSize.y = 350;
		}
		return TRUE;

	case WM_SIZE:
		{
			if (wParam != SIZE_MINIMIZED)
			{
				int w = LOWORD(lParam);
				int h = HIWORD(lParam);
				RECT r;

				HWND item = GetControl(Id_Tab);
				SetWindowPos(item, NULL, 0, 0, w - 18, h - 47, SWP_NOMOVE | SWP_NOZORDER);

				item = GetControl(Id_CloseButton);
				GetClientRect(item, &r);
				SetWindowPos(item, NULL, w - r.right - 9, h - r.bottom - 8, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				w -= 48;
				h -= 100;
				m_TabLog.Resize(w, h);
				m_TabSkins.Resize(w, h);
				m_TabPlugins.Resize(w, h);
				m_TabVersion.Resize(w, h);
			}
		}
		return TRUE;

	case WM_CLOSE:
		{
			GetWindowPlacement(m_Window, &c_WindowPlacement);
			if (c_WindowPlacement.showCmd == SW_SHOWMINIMIZED)
			{
				c_WindowPlacement.showCmd = SW_SHOWNORMAL;
			}

			delete c_Dialog;
			c_Dialog = NULL;
		}
		return TRUE;
	}

	return FALSE;
}

INT_PTR CDialogAbout::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	static const ControlTemplate::Control s_Controls[] =
	{
		CT_BUTTON(Id_CloseButton, ID_STR_CLOSE,
			344, 191, 50, 14,
			WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 0),
		CT_TAB(Id_Tab, 0,
			6, 6, 388, 181,
			WS_VISIBLE | WS_TABSTOP | TCS_FIXEDWIDTH, 0)  // Last for correct tab order.
	};

	CreateControls(s_Controls, _countof(s_Controls), m_Font, GetString);

	HWND item = GetControl(Id_Tab);
	m_TabLog.Create(m_Window);
	m_TabSkins.Create(m_Window);
	m_TabPlugins.Create(m_Window);
	m_TabVersion.Create(m_Window);

	TCITEM tci = {0};
	tci.mask = TCIF_TEXT;
	tci.pszText = GetString(ID_STR_LOG);
	TabCtrl_InsertItem(item, 0, &tci);
	tci.pszText = GetString(ID_STR_SKINS);
	TabCtrl_InsertItem(item, 1, &tci);
	tci.pszText = GetString(ID_STR_PLUGINS);
	TabCtrl_InsertItem(item, 2, &tci);
	tci.pszText = GetString(ID_STR_VERSION);
	TabCtrl_InsertItem(item, 3, &tci);

	HICON hIcon = GetIcon(IDI_RAINMETER);
	SendMessage(m_Window, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	item = GetControl(Id_CloseButton);
	SendMessage(m_Window, WM_NEXTDLGCTL, (WPARAM)item, TRUE);

	if (CSystem::GetOSPlatform() >= OSPLATFORM_VISTA)
	{
		item = m_TabLog.GetControl(CTabLog::Id_ItemsListView);
		SetWindowTheme(item, L"explorer", NULL);
		item = m_TabSkins.GetControl(CTabSkins::Id_ItemsListView);
		SetWindowTheme(item, L"explorer", NULL);
	}

	if (c_WindowPlacement.length == 0)
	{
		c_WindowPlacement.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(m_Window, &c_WindowPlacement);
	}
	SetWindowPlacement(m_Window, &c_WindowPlacement);

	return TRUE;
}

INT_PTR CDialogAbout::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case Id_CloseButton:
		PostMessage(m_Window, WM_CLOSE, 0, 0);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

INT_PTR CDialogAbout::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR nm = (LPNMHDR)lParam;
	switch (nm->idFrom)
	{
	case Id_Tab:
		if (nm->code == TCN_SELCHANGE)
		{
			// Disable all tab windows first
			EnableWindow(m_TabLog.GetWindow(), FALSE);
			EnableWindow(m_TabSkins.GetWindow(), FALSE);
			EnableWindow(m_TabPlugins.GetWindow(), FALSE);
			EnableWindow(m_TabVersion.GetWindow(), FALSE);

			GetActiveTab().Activate();
		}
		break;

	default:
		return 1;
	}

	return 0;
}

// -----------------------------------------------------------------------------------------------
//
//                                Log tab
//
// -----------------------------------------------------------------------------------------------

/*
** Constructor.
**
*/
CDialogAbout::CTabLog::CTabLog() : CTab(),
	m_Error(true),
	m_Warning(true),
	m_Notice(true),
	m_Debug(true)
{
}

void CDialogAbout::CTabLog::Create(HWND owner)
{
	CTab::CreateTabWindow(15, 30, 370, 148, owner);

	static const ControlTemplate::Control s_Controls[] =
	{
		CT_LISTVIEW(Id_ItemsListView, 0,
			0, 0, 368, 135,
			WS_VISIBLE | WS_TABSTOP | WS_BORDER | LVS_ICON | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER, 0),
		CT_CHECKBOX(Id_ErrorCheckBox, ID_STR_ERROR,
			0, 139, 70, 9,
			WS_VISIBLE | WS_TABSTOP, 0),
		CT_CHECKBOX(Id_WarningCheckBox, ID_STR_WARNING,
			70, 139, 70, 9,
			WS_VISIBLE | WS_TABSTOP, 0),
		CT_CHECKBOX(Id_NoticeCheckBox, ID_STR_NOTICE,
			140, 139, 70, 9,
			WS_VISIBLE | WS_TABSTOP, 0),
		CT_CHECKBOX(Id_DebugCheckBox, ID_STR_DEBUG,
			210, 139, 70, 9,
			WS_VISIBLE | WS_TABSTOP, 0)
	};

	CreateControls(s_Controls, _countof(s_Controls), c_Dialog->m_Font, GetString);
}

/*
** Called when tab is displayed.
**
*/
void CDialogAbout::CTabLog::Initialize()
{
	// Add columns to the list view
	HWND item = GetControl(Id_ItemsListView);
	ListView_SetExtendedListViewStyleEx(item, 0, LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	// Set folder/.ini icons for tree list
	HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32, 3, 1);
	HMODULE hDLL = GetModuleHandle(L"user32");

	HICON hIcon = (HICON)LoadImage(hDLL, MAKEINTRESOURCE(103), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ImageList_AddIcon(hImageList, hIcon);
	DeleteObject(hIcon);

	hIcon = (HICON)LoadImage(hDLL, MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ImageList_AddIcon(hImageList, hIcon);
	DeleteObject(hIcon);

	hIcon = (HICON)LoadImage(hDLL, MAKEINTRESOURCE(104), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR); 
	ImageList_AddIcon(hImageList, hIcon);
	DeleteObject(hIcon);

	ListView_SetImageList(item, (WPARAM)hImageList, LVSIL_SMALL);

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;  // left-aligned column
	lvc.iSubItem = 0;
	lvc.cx = 75;
	lvc.pszText = GetString(ID_STR_TYPE);
	ListView_InsertColumn(item, 0, &lvc);
	lvc.iSubItem = 1;
	lvc.cx = 85;
	lvc.pszText = GetString(ID_STR_TIME);
	ListView_InsertColumn(item, 1, &lvc);
	lvc.iSubItem = 2;
	lvc.cx = 370;
	lvc.pszText = GetString(ID_STR_MESSAGE);
	ListView_InsertColumn(item, 2, &lvc);

	// Add stored entires
	std::list<CRainmeter::LogInfo>::const_iterator iter = Rainmeter->GetAboutLogData().begin();
	for ( ; iter != Rainmeter->GetAboutLogData().end(); ++iter)
	{
		AddItem((*iter).level, (*iter).timestamp.c_str(), (*iter).message.c_str());
	}

	item = GetControl(Id_ErrorCheckBox);
	Button_SetCheck(item, BST_CHECKED);

	item = GetControl(Id_WarningCheckBox);
	Button_SetCheck(item, BST_CHECKED);

	item = GetControl(Id_NoticeCheckBox);
	Button_SetCheck(item, BST_CHECKED);

	item = GetControl(Id_DebugCheckBox);
	Button_SetCheck(item, BST_CHECKED);

	m_Initialized = true;
}

/*
** Resizes window and repositions controls.
**
*/
void CDialogAbout::CTabLog::Resize(int w, int h)
{
	SetWindowPos(m_Window, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);

	RECT r;
	HWND item = GetControl(Id_ErrorCheckBox);
	GetClientRect(item, &r);

	SetWindowPos(item, NULL, 0, h - r.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	item = GetControl(Id_WarningCheckBox);
	SetWindowPos(item, NULL, r.right, h - r.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	item = GetControl(Id_NoticeCheckBox);
	SetWindowPos(item, NULL, r.right * 2, h - r.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	item = GetControl(Id_DebugCheckBox);
	SetWindowPos(item, NULL, r.right * 3, h - r.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	item = GetControl(Id_ItemsListView);
	SetWindowPos(item, NULL, 0, 0, w, h - r.bottom - 7, SWP_NOMOVE | SWP_NOZORDER);

	// Adjust third colum
	LVCOLUMN lvc;
	lvc.mask = LVCF_WIDTH;
	lvc.cx = w - 183;
	ListView_SetColumn(item, 2, &lvc);
}

/*
** Adds item to log.
**
*/
void CDialogAbout::CTabLog::AddItem(int level, LPCWSTR time, LPCWSTR message)
{
	WCHAR buffer[32];
	LVITEM vitem;
	vitem.mask = LVIF_IMAGE | LVIF_TEXT;
	vitem.iItem = 0;
	vitem.iSubItem = 0;
	vitem.pszText = buffer;
	HWND item;

	switch (level)
	{
	case LOG_ERROR:
		if (!m_Error) return;
		item = GetControl(Id_ErrorCheckBox);
		vitem.iImage = 0;
		break;

	case LOG_WARNING:
		if (!m_Warning) return;
		item = GetControl(Id_WarningCheckBox);
		vitem.iImage = 1;
		break;

	case LOG_NOTICE:
		if (!m_Notice) return;
		item = GetControl(Id_NoticeCheckBox);
		vitem.iImage = 2;
		break;

	case LOG_DEBUG:
		if (!m_Debug) return;
		item = GetControl(Id_DebugCheckBox);
		vitem.iImage = I_IMAGENONE;
		break;
	}

	GetWindowText(item, buffer, 32);
	item = GetControl(Id_ItemsListView);
	ListView_InsertItem(item, &vitem);
	ListView_SetItemText(item, vitem.iItem, 1, (WCHAR*)time);
	ListView_SetItemText(item, vitem.iItem, 2, (WCHAR*)message);
	if (!ListView_IsItemVisible(item, 0))
	{
		ListView_Scroll(item, 0, 16);
	}
}

INT_PTR CDialogAbout::CTabLog::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		return OnCommand(wParam, lParam);

	case WM_NOTIFY:
		return OnNotify(wParam, lParam);
	}

	return FALSE;
}

INT_PTR CDialogAbout::CTabLog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case Id_ErrorCheckBox:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			m_Error = !m_Error;
		}
		break;

	case Id_WarningCheckBox:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			m_Warning = !m_Warning;
		}
		break;

	case Id_NoticeCheckBox:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			m_Notice = !m_Notice;
		}
		break;

	case Id_DebugCheckBox:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			m_Debug = !m_Debug;
		}
		break;

	default:
		return 1;
	}

	return 0;
}

INT_PTR CDialogAbout::CTabLog::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR nm = (LPNMHDR)lParam;
	switch (nm->code)
	{
	case LVN_KEYDOWN:
		{
			NMLVKEYDOWN* lvkd = (NMLVKEYDOWN*)nm;
			if (lvkd->wVKey == 0x43 && // C key.
				IsCtrlKeyDown())
			{
				int sel = ListView_GetNextItem(nm->hwndFrom, -1, LVNI_FOCUSED | LVNI_SELECTED);
				if (sel != -1)
				{
					std::wstring tmpSz(512, L'0');
					ListView_GetItemText(nm->hwndFrom, sel, 2, &tmpSz[0], 512);
					CSystem::SetClipboardText(tmpSz);
				}
			}
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------------
//
//                                Measures tab
//
// -----------------------------------------------------------------------------------------------

/*
** Constructor.
**
*/
CDialogAbout::CTabSkins::CTabSkins() : CTab(),
	m_SkinWindow()
{
}

void CDialogAbout::CTabSkins::Create(HWND owner)
{
	CTab::CreateTabWindow(15, 30, 370, 148, owner);

	static const ControlTemplate::Control s_Controls[] =
	{
		CT_LISTBOX(Id_SkinsListBox, 0,
			0, 0, 120, 148,
			WS_VISIBLE | WS_TABSTOP | LBS_NOTIFY | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_HSCROLL, WS_EX_CLIENTEDGE),
		CT_LISTVIEW(Id_ItemsListView, 0,
			125, 0, 242, 148,
			WS_VISIBLE | WS_TABSTOP | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER, 0)
	};

	CreateControls(s_Controls, _countof(s_Controls), c_Dialog->m_Font, GetString);
}

void CDialogAbout::CTabSkins::Initialize()
{
	// Add columns to the list view
	HWND item = GetControl(Id_ItemsListView);
	ListView_SetExtendedListViewStyleEx(item, 0, LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	LVGROUP lvg;
	lvg.cbSize = sizeof(LVGROUP);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_STATE;
	lvg.state = (CSystem::GetOSPlatform() >= OSPLATFORM_VISTA) ? LVGS_COLLAPSIBLE : LVGS_NORMAL;
	lvg.iGroupId = 0;
	lvg.pszHeader = GetString(ID_STR_MEASURES);
	ListView_InsertGroup(item, 0, &lvg);
	lvg.iGroupId = 1;
	lvg.pszHeader = GetString(ID_STR_VARIABLES);
	ListView_InsertGroup(item, 1, &lvg);

	ListView_EnableGroupView(item, TRUE);

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.iSubItem = 0;
	lvc.cx = 120;
	lvc.pszText = GetString(ID_STR_NAME);
	ListView_InsertColumn(item, 0, &lvc);
	lvc.iSubItem = 1;
	lvc.cx = 90;
	lvc.pszText = GetString(ID_STR_RANGE);
	ListView_InsertColumn(item, 1, &lvc);
	lvc.iSubItem = 2;
	lvc.cx = 130;
	lvc.pszText = GetString(ID_STR_VALUE);
	ListView_InsertColumn(item, 2, &lvc);

	UpdateSkinList();

	m_Initialized = true;
}

/*
** Resizes window and repositions controls.
**
*/
void CDialogAbout::CTabSkins::Resize(int w, int h)
{
	SetWindowPos(m_Window, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);

	HWND item = GetControl(Id_SkinsListBox);
	int wList = (w < 650) ? (w - 373) : 277;
	SetWindowPos(item, NULL, 0, 0, wList, h, SWP_NOMOVE | SWP_NOZORDER);

	item = GetControl(Id_ItemsListView);
	SetWindowPos(item, NULL, (w < 650) ? (w - 365) : 285, 0, w - wList - 10, h, SWP_NOZORDER);

	// Adjust third column
	LVCOLUMN lvc;
	lvc.mask = LVCF_WIDTH;
	lvc.cx = w - wList - 243;
	ListView_SetColumn(item, 2, &lvc);
}

/*
** Updates the list of skins.
**
*/
void CDialogAbout::CTabSkins::UpdateSkinList()
{
	// Delete all entries
	HWND item = GetControl(Id_SkinsListBox);
	ListBox_ResetContent(item);

	// Add entries for each skin
	std::wstring::size_type maxLength = 0;
	const std::map<std::wstring, CMeterWindow*>& windows = Rainmeter->GetAllMeterWindows();
	std::map<std::wstring, CMeterWindow*>::const_iterator iter = windows.begin();
	bool found = false;
	for ( ; iter != windows.end(); ++iter)
	{
		const std::wstring& skinName = (*iter).first;
		std::wstring::size_type curLength = skinName.length();
		if (curLength > maxLength)
		{
			maxLength = curLength;
		}
		
		const WCHAR* name = skinName.c_str();
		int index = ListBox_AddString(item, name);
		if (!found && m_SkinWindow == (*iter).second)
		{
			found = true;
			m_SkinWindow = (*iter).second;
			ListBox_SetCurSel(item, index);
		}
	}

	ListBox_SetHorizontalExtent(item, 6 * maxLength);

	if (!found)
	{
		if (windows.empty())
		{
			m_SkinWindow = NULL;
			item = GetControl(Id_ItemsListView);
			ListView_DeleteAllItems(item);
		}
		else
		{
			// Default to first skin
			m_SkinWindow = (*windows.begin()).second;
			ListBox_SetCurSel(item, 0);
			UpdateMeasureList(m_SkinWindow);
		}
	}
}

/*
** Updates the list of measures and values.
**
*/
void CDialogAbout::CTabSkins::UpdateMeasureList(CMeterWindow* meterWindow)
{
	if (!meterWindow)
	{
		// Find selected skin
		HWND item = GetControl(Id_SkinsListBox);
		int selected = (int)SendMessage(item, LB_GETCURSEL, NULL, NULL);

		const std::map<std::wstring, CMeterWindow*>& windows = Rainmeter->GetAllMeterWindows();
		std::map<std::wstring, CMeterWindow*>::const_iterator iter = windows.begin();
		while (selected && iter != windows.end())
		{
			++iter;
			--selected;
		}

		m_SkinWindow = (*iter).second;
	}
	else if (meterWindow != m_SkinWindow)
	{
		// Called by a skin other than currently visible one, so return
		return;
	}

	HWND item = GetControl(Id_ItemsListView);
	SendMessage(item, WM_SETREDRAW, FALSE, 0);
	int count = ListView_GetItemCount(item);

	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_GROUPID | LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.iItem = 0;
	lvi.lParam = 0;

	lvi.iGroupId = 0;
	const std::vector<CMeasure*>& measures = m_SkinWindow->GetMeasures();
	std::vector<CMeasure*>::const_iterator j = measures.begin();
	for ( ; j != measures.end(); ++j)
	{
		lvi.pszText = (WCHAR*)(*j)->GetName();

		if (lvi.iItem < count)
		{
			ListView_SetItem(item, &lvi);
		}
		else
		{
			ListView_InsertItem(item, &lvi);
		}

		WCHAR buffer[256];
		CMeasure::GetScaledValue(AUTOSCALE_ON, 1, (*j)->GetMinValue(), buffer, _countof(buffer));
		std::wstring range = buffer;
		range += L" - ";
		CMeasure::GetScaledValue(AUTOSCALE_ON, 1, (*j)->GetMaxValue(), buffer, _countof(buffer));
		range += buffer;

		ListView_SetItemText(item, lvi.iItem, 1, (WCHAR*)range.c_str());
		ListView_SetItemText(item, lvi.iItem, 2, (WCHAR*)(*j)->GetStringValue(AUTOSCALE_OFF, 1, -1, false));
		++lvi.iItem;
	}

	lvi.iGroupId = 1;
	const auto& variables = m_SkinWindow->GetParser().GetVariables();
	for (auto iter = variables.cbegin(); iter != variables.cend(); ++iter)
	{
		const WCHAR* name = (*iter).first.c_str();
		lvi.lParam = (LPARAM)name;

		if (wcscmp(name, L"@") == 0)
		{
			// Ignore reserved variables
			continue;
		}

		std::wstring tmpStr = (*iter).first;
		wcslwr(&tmpStr[0]);
		lvi.pszText = (WCHAR*)tmpStr.c_str();

		if (lvi.iItem < count)
		{
			ListView_SetItem(item, &lvi);
		}
		else
		{
			ListView_InsertItem(item, &lvi);
		}

		ListView_SetItemText(item, lvi.iItem, 1, L"");
		ListView_SetItemText(item, lvi.iItem, 2, (WCHAR*)(*iter).second.c_str());
		++lvi.iItem;
	}

	// Delete unnecessary items
	while (count > lvi.iItem)
	{
		ListView_DeleteItem(item, lvi.iItem);
		--count;
	}

	int selIndex = ListView_GetNextItem(item, -1, LVNI_FOCUSED | LVNI_SELECTED);

	ListView_SortItems(item, ListSortProc, 0);

	if (selIndex != -1)
	{
		// Re-select previously selected item
		ListView_SetItemState(item, selIndex, LVIS_FOCUSED | LVNI_SELECTED, LVIS_FOCUSED | LVNI_SELECTED);
	}

	SendMessage(item, WM_SETREDRAW, TRUE, 0);
}

int CALLBACK CDialogAbout::CTabSkins::ListSortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// Measures
	if (!lParam1 && !lParam2) return 0;
	if (!lParam1) return -1;
	if (!lParam2) return 1;

	// Variables
	return wcscmp((const WCHAR*)lParam1, (const WCHAR*)lParam2);
}

INT_PTR CDialogAbout::CTabSkins::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		return OnCommand(wParam, lParam);

	case WM_NOTIFY:
		return OnNotify(wParam, lParam);
	}

	return FALSE;
}

INT_PTR CDialogAbout::CTabSkins::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case Id_SkinsListBox:
		if (HIWORD(wParam) == LBN_SELCHANGE)
		{
			UpdateMeasureList(NULL);
		}
		break;

	default:
		return 1;
	}

	return 0;
}

INT_PTR CDialogAbout::CTabSkins::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR nm = (LPNMHDR)lParam;
	switch (nm->code)
	{
	case LVN_KEYDOWN:
		{
			NMLVKEYDOWN* lvkd = (NMLVKEYDOWN*)nm;
			if (lvkd->wVKey == 0x43 && IsCtrlKeyDown()) // CTRL + C.
			{
				int sel = ListView_GetNextItem(nm->hwndFrom, -1, LVNI_FOCUSED | LVNI_SELECTED);
				if (sel != -1)
				{
					std::wstring tmpSz(512, L'0');
					ListView_GetItemText(nm->hwndFrom, sel, 2, &tmpSz[0], 512);
					CSystem::SetClipboardText(tmpSz);
				}
			}
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------------
//
//                                Plugins tab
//
// -----------------------------------------------------------------------------------------------

CDialogAbout::CTabPlugins::CTabPlugins() : CTab()
{
}

void CDialogAbout::CTabPlugins::Create(HWND owner)
{
	CTab::CreateTabWindow(15, 30, 370, 148, owner);

	static const ControlTemplate::Control s_Controls[] =
	{
		CT_LISTVIEW(Id_ItemsListView, 0,
			0, 0, 368, 148,
			WS_VISIBLE | WS_TABSTOP | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER, 0)
	};

	CreateControls(s_Controls, _countof(s_Controls), c_Dialog->m_Font, GetString);
}

void CDialogAbout::CTabPlugins::Initialize()
{
	// Add columns to the list view
	HWND item = GetControl(Id_ItemsListView);

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;  // left-aligned column
	lvc.iSubItem = 0;
	lvc.cx = 140;
	lvc.pszText = GetString(ID_STR_NAME);
	ListView_InsertColumn(item, 0, &lvc);
	lvc.iSubItem = 1;
	lvc.cx = 80;
	lvc.pszText = GetString(ID_STR_VERSION);
	ListView_InsertColumn(item, 1, &lvc);
	lvc.iSubItem = 2;
	lvc.cx = 310;
	lvc.pszText = GetString(ID_STR_AUTHOR);
	ListView_InsertColumn(item, 2, &lvc);

	LVITEM vitem;
	vitem.mask = LVIF_TEXT;
	vitem.iItem = 0;
	vitem.iSubItem = 0;

	auto findPlugins = [&](const std::wstring& path)
	{
		std::wstring filter = path + L"*.dll";

		WIN32_FIND_DATA fd;
		HANDLE hSearch = FindFirstFile(filter.c_str(), &fd);
		if (hSearch == INVALID_HANDLE_VALUE)
		{
			return;
		}

		int index = 0;
		do
		{
			// Try to get the version and author
			std::wstring tmpSz = path + fd.cFileName;
			const WCHAR* path = tmpSz.c_str();

			vitem.iItem = index;
			vitem.pszText = fd.cFileName;

			// Try to get version and author from file resources first
			DWORD handle;
			DWORD versionSize = GetFileVersionInfoSize(path, &handle);
			if (versionSize)
			{
				bool found = false;
				void* data = new BYTE[versionSize];
				if (GetFileVersionInfo(path, 0, versionSize, data))
				{
					UINT len;
					struct LANGCODEPAGE
					{
						WORD wLanguage;
						WORD wCodePage;
					} *lcp;

					if (VerQueryValue(data, L"\\VarFileInfo\\Translation", (LPVOID*)&lcp, &len))
					{
						WCHAR key[64];
						LPWSTR value;

						_snwprintf_s(key, _TRUNCATE, L"\\StringFileInfo\\%04x%04x\\ProductName", lcp[0].wLanguage, lcp[0].wCodePage);
						if (VerQueryValue(data, (LPTSTR)(LPCTSTR)key, (void**)&value, &len) &&
							wcscmp(value, L"Rainmeter") == 0)
						{
							ListView_InsertItem(item, &vitem);
							++index;
							found = true;

							_snwprintf_s(key, _TRUNCATE, L"\\StringFileInfo\\%04x%04x\\FileVersion", lcp[0].wLanguage, lcp[0].wCodePage);
							if (VerQueryValue(data, (LPTSTR)(LPCTSTR)key, (void**)&value, &len))
							{
								ListView_SetItemText(item, vitem.iItem, 1, value);
							}

							_snwprintf_s(key, _TRUNCATE, L"\\StringFileInfo\\%04x%04x\\LegalCopyright", lcp[0].wLanguage, lcp[0].wCodePage);
							if (VerQueryValue(data, (LPTSTR)(LPCTSTR)key, (void**)&value, &len))
							{
								ListView_SetItemText(item, vitem.iItem, 2, value);
							}
						}
					}
				}

				delete [] data;
				if (found) continue;
			}

			// Try old calling GetPluginVersion/GetPluginAuthor for backwards compatibility
			DWORD err = 0;
			HMODULE dll = CSystem::RmLoadLibrary(path, &err);
			if (dll)
			{
				ListView_InsertItem(item, &vitem);
				++index;

				GETPLUGINVERSION GetVersionFunc = (GETPLUGINVERSION)GetProcAddress(dll, "GetPluginVersion");
				if (GetVersionFunc)
				{
					UINT version = GetVersionFunc();
					WCHAR buffer[64];
					_snwprintf_s(buffer, _TRUNCATE, L"%u.%u", version / 1000, version % 1000);
					ListView_SetItemText(item, vitem.iItem, 1, buffer);
				}

				GETPLUGINAUTHOR GetAuthorFunc = (GETPLUGINAUTHOR)GetProcAddress(dll, "GetPluginAuthor");
				if (GetAuthorFunc)
				{
					LPCTSTR author = GetAuthorFunc();
					if (author && *author)
					{
						ListView_SetItemText(item, vitem.iItem, 2, (LPWSTR)author);
					}
				}

				FreeLibrary(dll);
			}
			else
			{
				LogWithArgs(LOG_ERROR, L"Unable to load plugin: %s (%u)", tmpSz.c_str(), err);
			}
		}
		while (FindNextFile(hSearch, &fd));
		FindClose(hSearch);
	};

	findPlugins(Rainmeter->GetPluginPath());
	if (Rainmeter->HasUserPluginPath())
	{
		findPlugins(Rainmeter->GetUserPluginPath());
	}

	m_Initialized = true;
}

/*
** Resizes window and repositions controls.
**
*/
void CDialogAbout::CTabPlugins::Resize(int w, int h)
{
	SetWindowPos(m_Window, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);

	HWND item = GetControl(Id_ItemsListView);
	SetWindowPos(item, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);

	// Adjust third colum
	LVCOLUMN lvc;
	lvc.mask = LVCF_WIDTH;
	lvc.cx = w - 242;
	ListView_SetColumn(item, 2, &lvc);
}

// -----------------------------------------------------------------------------------------------
//
//                                Version tab
//
// -----------------------------------------------------------------------------------------------

/*
** Constructor.
**
*/
CDialogAbout::CTabVersion::CTabVersion() : CTab()
{
}

void CDialogAbout::CTabVersion::Create(HWND owner)
{
	CTab::CreateTabWindow(15, 30, 370, 148, owner);

	// FIXME: Temporary hack.
	short buttonWidth = (short)_wtoi(GetString(ID_STR_NUM_BUTTONWIDTH));

	const ControlTemplate::Control s_Controls[] =
	{
		CT_ICON(Id_AppIcon, 0,
			0, 8, 24, 24,
			WS_VISIBLE, 0),
		CT_LABEL(Id_VersionLabel, 0,
			28, 0, 300, 9,
			WS_VISIBLE, 0),
		CT_LINKLABEL(Id_HomeLink, ID_STR_GETLATESTVERSION,
			28, 13, 300, 9,
			WS_VISIBLE, 0),
		CT_LINKLABEL(Id_LicenseLink, ID_STR_COPYRIGHTNOTICE,
			28, 26, 300, 9,
			WS_VISIBLE, 0),
		CT_LABEL(Id_PathLabel, 0,
			0, 43, 360, 9,
			WS_VISIBLE | SS_ENDELLIPSIS | SS_NOPREFIX, 0),
		CT_LABEL(Id_IniFileLabel, 0,
			0, 56, 360, 9,
			WS_VISIBLE | SS_ENDELLIPSIS | SS_NOPREFIX, 0),
		CT_LABEL(Id_SkinPathLabel, 0,
			0, 69, 360, 9,
			WS_VISIBLE | SS_ENDELLIPSIS | SS_NOPREFIX, 0),
		CT_BUTTON(Id_CopyButton, ID_STR_COPYTOCLIPBOARD,
			0, 85, buttonWidth + 25, 14,
			WS_VISIBLE | WS_TABSTOP, 0)
	};

	CreateControls(s_Controls, _countof(s_Controls), c_Dialog->m_Font, GetString);
}

void CDialogAbout::CTabVersion::Initialize()
{
	HWND item = GetControl(Id_AppIcon);
	HICON icon = GetIcon(IDI_RAINMETER, true);
	Static_SetIcon(item, icon);

	item = GetControl(Id_VersionLabel);
	WCHAR tmpSz[64];
	_snwprintf_s(tmpSz, _TRUNCATE, L"%s%s r%i %s (%s)", APPVERSION, revision_beta ? L" beta" : L"", revision_number, APPBITS, APPDATE);
	SetWindowText(item, tmpSz);

	item = GetControl(Id_PathLabel);
	std::wstring text = L"Path: " + Rainmeter->GetPath();
	SetWindowText(item, text.c_str());

	item = GetControl(Id_IniFileLabel);
	text = L"IniFile: " + Rainmeter->GetIniFile();
	SetWindowText(item, text.c_str());

	item = GetControl(Id_SkinPathLabel);
	text = L"SkinPath: " + Rainmeter->GetSkinPath();
	SetWindowText(item, text.c_str());

	m_Initialized = true;
}

/*
** Resizes window and repositions controls.
**
*/
void CDialogAbout::CTabVersion::Resize(int w, int h)
{
	SetWindowPos(m_Window, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
}

INT_PTR CDialogAbout::CTabVersion::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		return OnCommand(wParam, lParam);

	case WM_NOTIFY:
		return OnNotify(wParam, lParam);
	}

	return FALSE;
}

INT_PTR CDialogAbout::CTabVersion::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case Id_CopyButton:
		{
			WCHAR tmpSz[64];
			int len = _snwprintf_s(tmpSz, _TRUNCATE, L"%s%s r%i %s (%s)", APPVERSION, revision_beta ? L" beta" : L"", revision_number, APPBITS, APPDATE);
			std::wstring text(tmpSz, len);
			text += L"\nPath: ";
			text += Rainmeter->GetPath();
			text += L"\nIniFile: ";
			text += Rainmeter->GetIniFile();
			text += L"\nSkinPath: ";
			text += Rainmeter->GetSkinPath();
			CSystem::SetClipboardText(text);
		}
		break;

	default:
		return 1;
	}

	return 0;
}

INT_PTR CDialogAbout::CTabVersion::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR nm = (LPNMHDR)lParam;
	switch (nm->code)
	{
	case NM_CLICK:
		if (nm->idFrom == Id_HomeLink)
		{
			RunFile(L"http://rainmeter.net");
		}
		else if (nm->idFrom == Id_HomeLink)
		{
			RunFile(L"http://gnu.org/licenses");
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}
