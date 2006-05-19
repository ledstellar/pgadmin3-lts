//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id: ctlSQLBox.cpp 4983 2006-01-31 21:42:25Z dpage $
// Copyright (C) 2002 - 2006, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// ctlSQLBox.cpp - SQL syntax highlighting textbox
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>
#include <wx/stc/stc.h>

// App headers
#include "pgAdmin3.h"
#include "pgSet.h"
#include "ctl/ctlSQLBox.h"


#include "menu.h"


wxString ctlSQLBox::sqlKeywords;


#if 0

#include "../src/stc/scintilla/include/Platform.h"
#include "../src/stc/scintilla/include/PropSet.h"
#include "../src/stc/scintilla/include/Accessor.h"
#include "../src/stc/scintilla/include/KeyWords.h"
#include "../src/stc/scintilla/include/Scintilla.h"
#include "../src/stc/scintilla/include/SciLexer.h"


static void classifyWordSQL(unsigned int start, unsigned int end, 
                            WordList &keywords, Accessor &styler, wxString& lastWord)
{
	char s[100];
	bool wordIsNumber = isdigit(styler[start]) || (styler[start] == '.');
    unsigned int i;

	for (i = 0; i < end - start + 1 && i < 30; i++)
    {
		s[i] = static_cast<char>(toupper(styler[start + i]));
		s[i + 1] = '\0';
	}
    lastWord = s;

    char chAttr = SCE_C_IDENTIFIER;
	if (wordIsNumber)
		chAttr = SCE_C_NUMBER;
	else
    {
        if (keywords.InList(s))
			chAttr = SCE_C_WORD;
	}
	styler.ColourTo(end, chAttr);
}


static void ColouriseSQLDoc(unsigned int startPos, int length,
                            int initStyle, WordList *keywordlists[], Accessor &styler)
{
	WordList &keywords = *keywordlists[0];

	styler.StartAt(startPos);

	bool fold = styler.GetPropertyInt("fold") != 0;
	int lineCurrent = styler.GetLine(startPos);
	int spaceFlags = 0;

	wxString lastWord;

	int state = initStyle;

    bool bInFunctionDefinition;
	if (lineCurrent > 0)
    {
		styler.SetLineState(lineCurrent, styler.GetLineState(lineCurrent-1));
		bInFunctionDefinition = (styler.GetLineState(lineCurrent) == 1);
	}
    else
    {
		styler.SetLineState(lineCurrent, 0);
		bInFunctionDefinition = false;
	}

	char chPrev = ' ';
	char chNext = styler[startPos];
	styler.StartSegment(startPos);
	unsigned int lengthDoc = startPos + length;
    unsigned int i;

    for (i = startPos; i < lengthDoc; i++)
    {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);

		if ((ch == '\r' && chNext != '\n') || (ch == '\n'))
        {
			int indentCurrent = styler.IndentAmount(lineCurrent, &spaceFlags);
			int lev = indentCurrent;
			if (!(indentCurrent & SC_FOLDLEVELWHITEFLAG))
            {
				// Only non whitespace lines can be headers
				int indentNext = styler.IndentAmount(lineCurrent + 1, &spaceFlags);

				if (indentCurrent < (indentNext & ~SC_FOLDLEVELWHITEFLAG))
                {
					lev |= SC_FOLDLEVELHEADERFLAG;
				}
			}
			if (fold)
            {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			styler.SetLineState(lineCurrent, (bInFunctionDefinition? 1 : 0));
		}

		if (styler.IsLeadByte(ch))
        {
			chNext = styler.SafeGetCharAt(i + 2);
			chPrev = ' ';
			i += 1;
			continue;
		}

		if (state == SCE_C_DEFAULT)
        {
			if (iswordstart(ch))
            {
				styler.ColourTo(i - 1, state);
				state = SCE_C_WORD;
			}
            else if (ch == '/' && chNext == '*')
            {
				styler.ColourTo(i - 1, state);
				state = SCE_C_COMMENT;
			}
            else if (ch == '-' && chNext == '-')
            {
				styler.ColourTo(i - 1, state);
				state = SCE_C_COMMENTLINE;
			}
            else if (ch == '\'')
            {
				styler.ColourTo(i - 1, state);
				if (bInFunctionDefinition && chPrev != '\\')
                {
					bInFunctionDefinition = false;
					styler.SetLineState(lineCurrent,0);

				}
                else if (!bInFunctionDefinition && lastWord.IsSameAs(wxT("AS"), false))
                {
					bInFunctionDefinition = true;
					styler.SetLineState(lineCurrent, 1);

				}
                else
                {
					state = SCE_C_STRING;
					if (chPrev == '\\')
                    {
						styler.ColourTo(i - 1, state);
					}
				}
			}
            else if (isoperator(ch))
            {
				styler.ColourTo(i - 1, state);
				styler.ColourTo(i, SCE_C_OPERATOR);
			}
		} 
        else if (state == SCE_C_WORD)
        {
			if (!iswordchar(ch))
            {
			   classifyWordSQL(styler.GetStartSegment(), i - 1, keywords, styler, lastWord);

				state = SCE_C_DEFAULT;
				if (ch == '/' && chNext == '*')
					state = SCE_C_COMMENT;
				else if (ch == '-' && chNext == '-')
					state = SCE_C_COMMENTLINE;
				else if (ch == '\'')
					state = SCE_C_STRING;
				else if (isoperator(ch))
					styler.ColourTo(i, SCE_C_OPERATOR);
			}
		}
        else
        {
			if (state == SCE_C_COMMENT)
            {
				if (ch == '/' && chPrev == '*')
                {
					if (((i > (styler.GetStartSegment() + 2)) 
                        || ((initStyle == SCE_C_COMMENT) && (styler.GetStartSegment() == startPos))))
                    {
						styler.ColourTo(i, state);
						state = SCE_C_DEFAULT;
					}
				}
			}
            else if (state == SCE_C_COMMENTLINE)
            {
				if (ch == '\r' || ch == '\n')
                {
					styler.ColourTo(i - 1, state);
					state = SCE_C_DEFAULT;
				}
			} 
            else if (state == SCE_C_STRING)
            {
				if (ch == '\'')
                {
					if ( chNext == '\'' )
                    {
						i++;
					}
                    else
                    {
						styler.ColourTo(i,state);

						state = SCE_C_DEFAULT;
						i++;
					}
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);

				}
			}
			if (state == SCE_C_DEFAULT) 
            {    // One of the above succeeded

				if (ch == '/' && chNext == '*')
					state = SCE_C_COMMENT;
				else if (ch == '-' && chNext == '-')
					state = SCE_C_COMMENTLINE;
				else if (ch == '\'')
                {
				    if (bInFunctionDefinition && chPrev != '\\')
                    {
					    bInFunctionDefinition = false; 
                        styler.SetLineState(lineCurrent, 0);  
				    }
                    else if (!bInFunctionDefinition && lastWord.IsSameAs(wxT("AS"), false))
                    {
					    bInFunctionDefinition = true;
					    styler.SetLineState(lineCurrent, 1);
					} 
                    else 
                    {
						state = SCE_C_STRING;
					}
				} 
                else if (iswordstart(ch))
					state = SCE_C_WORD;
				else if (isoperator(ch))
					styler.ColourTo(i, SCE_C_OPERATOR);
			}
		}
		chPrev = ch;
	}
	styler.ColourTo(lengthDoc - 1, state);
}


LexerModule lmPostgreSQL(SCLEX_AUTOMATIC, ColouriseSQLDoc, "sql");

#endif


BEGIN_EVENT_TABLE(ctlSQLBox, wxStyledTextCtrl)
    EVT_KEY_DOWN(ctlSQLBox::OnKeyDown)
    EVT_MENU(MNU_FIND,ctlSQLBox::OnFind)
	EVT_MENU(MNU_AUTOCOMPLETE,ctlSQLBox::OnAutoComplete)
    EVT_FIND(-1, ctlSQLBox::OnFindDialog)
    EVT_FIND_NEXT(-1, ctlSQLBox::OnFindDialog)
    EVT_FIND_REPLACE(-1, ctlSQLBox::OnFindDialog)
    EVT_FIND_REPLACE_ALL(-1, ctlSQLBox::OnFindDialog)
    EVT_FIND_CLOSE(-1, ctlSQLBox::OnFindDialog)
	EVT_KILL_FOCUS(ctlSQLBox::OnKillFocus)
    EVT_STC_UPDATEUI(-1,  ctlSQLBox::OnPositionStc)
END_EVENT_TABLE()



IMPLEMENT_DYNAMIC_CLASS(ctlSQLBox, wxStyledTextCtrl)


ctlSQLBox::ctlSQLBox()
{
    m_dlgFind=0;
#ifndef __WXMSW__
	findDlgLast = false;
#endif
}


ctlSQLBox::ctlSQLBox(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
    m_dlgFind=0;
	m_database=NULL;
#ifndef __WXMSW__
    findDlgLast = false;
#endif
    Create(parent, id, pos, size, style);
}


void ctlSQLBox::Create(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
    wxLogInfo(wxT("Creating a ctlSQLBox"));

    wxStyledTextCtrl::Create(parent,id , pos, size, style);

    // Clear all styles
    StyleClearAll();
    
    // Font
    extern sysSettings *settings;
    wxFont fntSQLBox = settings->GetSQLFont();

    StyleSetFont(wxSTC_STYLE_DEFAULT, fntSQLBox);
    StyleSetFont(0, fntSQLBox);
    StyleSetFont(1, fntSQLBox);
    StyleSetFont(2, fntSQLBox);
    StyleSetFont(3, fntSQLBox);
    StyleSetFont(4, fntSQLBox);
    StyleSetFont(5, fntSQLBox);
    StyleSetFont(6, fntSQLBox);
    StyleSetFont(7, fntSQLBox);
    StyleSetFont(8, fntSQLBox);
    StyleSetFont(9, fntSQLBox);
    StyleSetFont(10, fntSQLBox);
    StyleSetFont(11, fntSQLBox);

    SetMarginWidth(1, 0);
    SetIndent(settings->GetIndentSpaces());
    
    // Setup the different highlight colurs
    StyleSetForeground(0,  wxColour(0x80, 0x80, 0x80));
    StyleSetForeground(1,  wxColour(0x00, 0x7f, 0x00));
    StyleSetForeground(2,  wxColour(0x00, 0x7f, 0x00));
    StyleSetForeground(3,  wxColour(0x7f, 0x7f, 0x7f));
    StyleSetForeground(4,  wxColour(0x00, 0x7f, 0x7f));
    StyleSetForeground(5,  wxColour(0x00, 0x00, 0x7f));
    StyleSetForeground(6,  wxColour(0x7f, 0x00, 0x7f));
    StyleSetForeground(7,  wxColour(0x7f, 0x00, 0x7f));
    StyleSetForeground(8,  wxColour(0x00, 0x7f, 0x7f));
    StyleSetForeground(9,  wxColour(0x7f, 0x7f, 0x7f));
    StyleSetForeground(10, wxColour(0x00, 0x00, 0x00));
    StyleSetForeground(11, wxColour(0x00, 0x00, 0x00));

	// Brace maching styles
    StyleSetBackground(34, wxColour(0x99, 0xF9, 0xFF));
    StyleSetBackground(35, wxColour(0xFF, 0xCF, 0x27));

    // SQL Lexer and keywords.
//    SetLexer(lmPostgreSQL.GetLanguage());
    if (sqlKeywords.IsEmpty())
        FillKeywords(sqlKeywords);
    SetLexer(wxSTC_LEX_SQL);
    SetKeyWords(0, sqlKeywords);

    wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_CTRL, (int)'F', MNU_FIND);
	entries[1].Set(wxACCEL_CTRL, (int)' ', MNU_AUTOCOMPLETE);
    wxAcceleratorTable accel(2, entries);
    SetAcceleratorTable(accel);

    m_findData.SetFlags(wxFR_DOWN);

	// Autocompletion configuration
	AutoCompSetSeparator('\t');
	AutoCompSetChooseSingle(true);
	AutoCompSetIgnoreCase(true);
	AutoCompSetFillUps(wxT(" \t"));
	AutoCompSetDropRestOfWord(true);
}

void ctlSQLBox::SetDatabase(pgConn *db)
{
	m_database = db;
}

void ctlSQLBox::OnFind(wxCommandEvent& ev)
{
#ifndef __WXMSW__
	if (m_dlgFind && !findDlgLast)
	{
		m_dlgFind->Destroy();
		m_dlgFind = NULL;
	}
#endif
	
    if (!m_dlgFind)
    {
        m_dlgFind = new wxFindReplaceDialog(this, &m_findData, _("Find text"), 0);
        m_dlgFind->Show(true);
#ifndef __WXMSW__
		findDlgLast = true;
#endif
    }
    else
        m_dlgFind->SetFocus();
}


void ctlSQLBox::OnReplace(wxCommandEvent& ev)
{
#ifndef __WXMSW__
	if (m_dlgFind && findDlgLast)
	{
		m_dlgFind->Destroy();
		m_dlgFind = NULL;
	}
#endif
	
    if (!m_dlgFind)
    {
        m_dlgFind = new wxFindReplaceDialog(this, &m_findData, _("Find text"), wxFR_REPLACEDIALOG);
        m_dlgFind->Show(true);
#ifndef __WXMSW__
		findDlgLast = false;
#endif
    }
    else
        m_dlgFind->SetFocus();
}


void ctlSQLBox::OnFindDialog(wxFindDialogEvent& event)
{
    wxEventType type = event.GetEventType();

    if (type == wxEVT_COMMAND_FIND || type == wxEVT_COMMAND_FIND_NEXT)
    {
        int flags = 0;
        if (event.GetFlags() & wxFR_MATCHCASE)
            flags |= wxSTC_FIND_MATCHCASE;

        if (event.GetFlags() & wxFR_WHOLEWORD)
            flags |= wxSTC_FIND_WHOLEWORD;

        int startPos = GetSelectionStart();
        int endPos = GetTextLength();
        
        if (event.GetFlags() & wxFR_DOWN)
        {
            startPos += 1;
        }
        else
        {
            endPos = 0;
            startPos -= 1;
        }

        int pos = FindText(startPos, endPos, event.GetFindString().c_str(), flags);

        if (pos >= 0)
        {
            SetSelectionStart(pos);
            SetSelectionEnd(pos + event.GetFindString().Length());
            EnsureCaretVisible();
        }
        else
        {
            wxMessageBox(_("Reached end of the document"), _("Find text"),
                        wxICON_EXCLAMATION | wxOK, this);
        }
    }
    else if (type == wxEVT_COMMAND_FIND_REPLACE)
    {
        int flags = 0;
        if (event.GetFlags() & wxFR_MATCHCASE)
            flags |= wxSTC_FIND_MATCHCASE;

        if (event.GetFlags() & wxFR_WHOLEWORD)
            flags |= wxSTC_FIND_WHOLEWORD;

        int startPos = GetSelectionStart();
        int endPos = GetTextLength();

        int pos = FindText(startPos, endPos, event.GetFindString().c_str(), flags);

        if (pos >= 0)
        {
            SetSelectionStart(pos);
            SetSelectionEnd(pos + event.GetFindString().Length());
            ReplaceSelection(event.GetReplaceString().c_str());
            EnsureCaretVisible();
        }
        else
        {
            wxMessageBox(_("Reached end of the document"), _("Replace text"),
                        wxICON_EXCLAMATION | wxOK, this);
        }
    }
    else if (type == wxEVT_COMMAND_FIND_REPLACE_ALL)
    {
        int flags = 0;
        if (event.GetFlags() & wxFR_MATCHCASE)
            flags |= wxSTC_FIND_MATCHCASE;

        if (event.GetFlags() & wxFR_WHOLEWORD)
            flags |= wxSTC_FIND_WHOLEWORD;

        int initialPos = GetCurrentPos();
        int startPos = 0;
        int endPos = GetTextLength();

        int pos = FindText(startPos, endPos, event.GetFindString().c_str(), flags);

        while (pos >= 0)
        {
            SetSelectionStart(pos);
            SetSelectionEnd(pos + event.GetFindString().Length());
            ReplaceSelection(event.GetReplaceString().c_str());
            pos = pos + event.GetReplaceString().Length();
            pos = FindText(pos, endPos, event.GetFindString().c_str(), flags);
        }

        GotoPos(initialPos);
    }
    else if (type == wxEVT_COMMAND_FIND_CLOSE)
    {
        wxFindReplaceDialog *dlg = event.GetDialog();
        m_dlgFind = NULL;
        dlg->Destroy();
    }
    else
    {
        wxLogError(__("Unknown find dialog event!"));
    }
}


void ctlSQLBox::OnKeyDown(wxKeyEvent& event)
{
#ifdef __WXGTK__
	event.m_metaDown=false;
#endif
    event.Skip();

	if (!AutoCompActive() &&
		 ( settings->GetTabForCompletion() && /* autocomplete on tab only if specifically configured */
		  !event.AltDown() && !event.CmdDown() && !event.ControlDown() && event.GetKeyCode() == '\t'
		 ))
	{
		wxCommandEvent e;
		OnAutoComplete(e);
	}
}

void ctlSQLBox::OnKillFocus(wxFocusEvent& event)
{
	AutoCompCancel();
}

void ctlSQLBox::OnPositionStc(wxStyledTextEvent& event)
{
	int pos = GetCurrentPos();
	wxChar ch = GetCharAt(pos);
	int match;

	// Clear all highlighting
	BraceBadLight(wxSTC_INVALID_POSITION);

	// Highlight the previous set if the match
	ch = GetCharAt(pos-1);

	if (ch == '{' || ch == '}' ||
		ch == '[' || ch == ']' ||
		ch == '(' || ch == ')')
	{
		match = BraceMatch(pos-1);
		if (match != wxSTC_INVALID_POSITION)
			BraceHighlight(pos-1, match);
	}

	// Roll back through the doc and highlight any unmatched braces
	while ((pos--) >= 0)
	{
		ch = GetCharAt(pos);

		if (ch == '{' || ch == '}' ||
			ch == '[' || ch == ']' ||
			ch == '(' || ch == ')')
		{
			match = BraceMatch(pos);
			if (match == wxSTC_INVALID_POSITION)
			{
				BraceBadLight(pos);
				break;
			}
		}
	}
    
	event.Skip();
}

extern "C" char *tab_complete(const char *allstr, const int startptr, const int endptr, void *dbptr);
void ctlSQLBox::OnAutoComplete(wxCommandEvent& rev)
{
	if (GetReadOnly())
		return;
	if (m_database == NULL)
		return;

	wxString what = GetCurLine().Left(GetCurrentPos()-PositionFromLine(GetCurrentLine()));;
	int spaceidx = what.Find(' ',true);
	
	char *tab_ret;
	if (spaceidx == -1)
		tab_ret = tab_complete(what.mb_str(wxConvUTF8), 0, what.Len()+1, m_database);
	else
		tab_ret = tab_complete(what.mb_str(wxConvUTF8), spaceidx+1, what.Len()+1, m_database);

	if (tab_ret == NULL || tab_ret[0] == '\0')
		return; /* No autocomplete available for this string */

	wxString wxRet = wxString(tab_ret, wxConvUTF8);
	free(tab_ret);

	if (spaceidx == -1)
		AutoCompShow(what.Len(), wxRet);
	else
		AutoCompShow(what.Len()-spaceidx-1, wxRet);
}


ctlSQLBox::~ctlSQLBox()
{
    wxLogInfo(wxT("Destroying a ctlSQLBox"));
    if (m_dlgFind)
    {
        m_dlgFind->Destroy();
        delete m_dlgFind;
    }
}


/*
 * Callback function from tab-complete.c, bridging the gap between C++ and C.
 * Execute a query using the C++ APIs, returning it as a tab separated
 * "char*-string"
 * The query is expected to return only one column, and will have an ORDER BY
 * clause for this column added automatically.
 */
extern "C"
char *pg_query_to_single_ordered_string(char *query, void *dbptr)
{
	pgConn *db = (pgConn *)dbptr;
	pgSet *res = db->ExecuteSet(wxString(query, wxConvUTF8) + wxT(" ORDER BY 1"));
	if (!res)
		return NULL;

	wxString ret = wxString();
    wxString tmp;

	while (!res->Eof())
	{
        tmp =  res->GetVal(0);
        if (tmp.Mid(tmp.Length() - 1) == wxT("."))
		    ret += tmp + wxT("\t");
        else
		    ret += tmp + wxT(" \t");

		res->MoveNext();
	}

	ret.Trim();
	// Trims both space and tab, but we want to keep the space!
	if (ret.Length() > 0)
	    ret += wxT(" ");

	return strdup(ret.mb_str(wxConvUTF8));
}
