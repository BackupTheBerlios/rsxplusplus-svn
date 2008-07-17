#ifndef _0C46500C_084D_44ac_8C26_37E38BED2714_
#define _0C46500C_084D_44ac_8C26_37E38BED2714_
#pragma once

#ifdef __ATLBASE_H__
template <class T> class CSnapWindow
#else
class CSnapWindow
#endif
{
public:
    int snap_Margin, snap_ModifierKey;

#ifdef __ATLBASE_H__
    BEGIN_MSG_MAP(CSnapWindow<T>)
        MESSAGE_HANDLER(WM_MOVING, OnSnapMoving)
        MESSAGE_HANDLER(WM_ENTERSIZEMOVE, OnSnapEnterSizeMove)
    END_MSG_MAP()
#endif

#ifdef __ATLBASE_H__ 
    virtual LRESULT OnSnapEnterSizeMove(UINT uMsg, WPARAM wParam, LPARAM lParam,
       BOOL& bHandled)
#else
    virtual LRESULT OnSnapEnterSizeMove(HWND hWnd, UINT, WPARAM,
       LPARAM)
#endif
            {
                snap_cur_pos.x=0;
                snap_cur_pos.y=0;

                snap_rcWindow.bottom=0;
                snap_rcWindow.left=0;
                snap_rcWindow.right=0;
                snap_rcWindow.top=0;

            #ifdef __ATLBASE_H__ 
                T* pT = static_cast<T*>(this);
                GetWindowRect(pT->m_hWnd, &snap_rcWindow );
            #else
                GetWindowRect(hWnd, &snap_rcWindow );
            #endif

                    GetCursorPos( &snap_cur_pos );

                    snap_x = snap_cur_pos.x - snap_rcWindow.left;
                    snap_y = snap_cur_pos.y - snap_rcWindow.top;
                return 0;
                }

#ifdef __ATLBASE_H__ 
    virtual LRESULT OnSnapMoving(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
#else
    virtual LRESULT OnSnapMoving(HWND, UINT, WPARAM, LPARAM lParam)
#endif
            {
                //no snap if modifier key pressed
                if (GetAsyncKeyState(snap_ModifierKey) < 0) return FALSE;

                snap_prc = (LPRECT)lParam;

                snap_cur_pos.x=0;
                snap_cur_pos.y=0;
                snap_rcWindow.bottom=0;
                snap_rcWindow.left=0;
                snap_rcWindow.right=0;
                snap_rcWindow.top=0;

                GetCursorPos( &snap_cur_pos );
                OffsetRect( snap_prc,
                        snap_cur_pos.x - (snap_prc->left + snap_x) ,
                        snap_cur_pos.y - (snap_prc->top + snap_y) );

                //working area may change during app lifetime
                SystemParametersInfo( SPI_GETWORKAREA, 0, &snap_wa, 0 );

                if (isSnapClose( snap_prc->left, snap_wa.left ))
                    {OffsetRect( snap_prc, snap_wa.left - snap_prc->left, 0);}
                else 
                    if (isSnapClose( snap_wa.right, snap_prc->right )) 
                        {OffsetRect( snap_prc, snap_wa.right - snap_prc->right, 0);}

                if (isSnapClose( snap_prc->top, snap_wa.top )) 
                    {OffsetRect( snap_prc, 0, snap_wa.top - snap_prc->top );}
                else 
                    if (isSnapClose( snap_wa.bottom, snap_prc->bottom )) 
                        {OffsetRect( snap_prc, 0, snap_wa.bottom - snap_prc->bottom );}
                return TRUE;
                }

    virtual BOOL isSnapClose( int a, int b ) { return (abs( a - b ) < snap_Margin);}

    CSnapWindow()
        {
        snap_ModifierKey=VK_SHIFT;
        NONCLIENTMETRICS ncm = { 0 };
        ncm.cbSize = sizeof ncm;
        SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, &ncm, 0 );
        snap_Margin=ncm.iCaptionHeight;
        }
private:
    POINT snap_cur_pos;
    RECT snap_rcWindow, snap_wa, *snap_prc;
    int snap_x, snap_y;
};
#endif//_0C46500C_084D_44ac_8C26_37E38BED2714_