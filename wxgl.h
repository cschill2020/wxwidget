// Evolving from learnopengl and wxWidgets pyramid sample.

#ifndef WXGL_H
#define WXGL_H

#include <chrono>

// Due to oglpfuncs.h needs to be included before gl.h (to avoid some declarations),
// we include glcanvas.h after oglstuff.h
#include "graphics/ogl.h"
#include "graphics/xlOGL3GraphicsContext.h"
#include "wx/glcanvas.h"

// Define a new application
class MyApp: public wxApp
{
public:
    MyApp(){}
    bool OnInit() wxOVERRIDE;
};


class MyGLCanvas;

// The main frame class
class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title);

    void OnAbout(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
#if wxUSE_LOGWINDOW
    void OnLogWindow(wxCommandEvent& event);
#endif // wxUSE_LOGWINDOW
    void SetOGLString(const wxString& ogls)
        { _OGLString = ogls; }
    bool OGLAvailable();

private:
#if wxUSE_LOGWINDOW
    wxLogWindow* _LogWin;
#endif // wxUSE_LOGWINDOW
    wxString     _OGLString;
    MyGLCanvas*  _mycanvas;

    wxDECLARE_EVENT_TABLE();
};


// The canvas window
class MyGLCanvas : public xlGLCanvas
{
public:
    MyGLCanvas(MyFrame* parent, const wxGLAttributes& canvasAttrs, bool a, int styles);
    ~MyGLCanvas();

    //Used just to know if we must end now because OGL 3.2 isn't available
    bool OglCtxAvailable()
        {return _oglContext != NULL;}

    //Init the OpenGL stuff
    bool oglInit();

private:
    // wxEvent Bindings
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftMotion(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);
    void OnMouseWheel(wxMouseEvent& event);

    // Timer for the renderloop.  Refresh the canvas regularly.
    void OnRenderLoopTimer(wxTimerEvent& event);

    // Members
    MyFrame*      _parent;
    wxGLContext*  _oglContext;
    myOGLManager* _oglManager;
    // Screensaver Timer (not used yet)
    wxTimer _renderTimer;

    wxPoint _mousePrev;
    bool _mouseDragging;
    bool _spacePressed;

    int           _winHeight; // We use this var to know if we have been sized

    unsigned long _frameCnt;
    std::chrono::high_resolution_clock::time_point _previousTime;
};


// IDs for the controls and the menu commands
enum
{
    wxGL_Quit = wxID_EXIT,
    wxGL_About = wxID_ABOUT,
    wxGL_LogW = wxID_HIGHEST + 10
};

#endif // WXGL_H

