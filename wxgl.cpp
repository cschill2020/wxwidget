#define wxUSE_LOGWINDOW true
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_GLCANVAS
#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#include "wxgl.h"

#include <chrono>
#include <filesystem>

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>

#include <wx/dir.h>

void InitialiseLogging(bool fromMain) {
  static bool loggingInitialised = false;

  if (!loggingInitialised) {
    std::string initFileName = "../log4cpp.properties";
    if (!std::filesystem::exists(initFileName)) {
      if (fromMain) {
        return;
      }
    }
    loggingInitialised = true;
    // make sure the default logging location is actually created
    std::string ld = std::getenv("HOME");
    ld += "/Library/Logs/";
    wxDir logDir(ld);
    if (!wxDir::Exists(ld)) {
      wxDir::Make(ld);
    }
    try {
      std::cout<<"Configuring logs."<<std::endl;
      log4cpp::PropertyConfigurator::configure(initFileName);
      static log4cpp::Category &logger_base =
          log4cpp::Category::getInstance(std::string("log_base"));

      wxDateTime now = wxDateTime::Now();
      int millis = wxGetUTCTimeMillis().GetLo() % 1000;
      wxString ts =
          wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(),
                           now.GetMonth() + 1, now.GetDay(), now.GetHour(),
                           now.GetMinute(), now.GetSecond(), millis);
      logger_base.info("Start Time: %s.", (const char *)ts.c_str());

      logger_base.info("Log4CPP config read from %s.",
                       (const char *)initFileName.c_str());
      logger_base.info("Current working directory %s.",
                       (const char *)wxGetCwd().c_str());

      auto categories = log4cpp::Category::getCurrentCategories();

      for (auto it = categories->begin(); it != categories->end(); ++it) {
        std::string levels = "";

        if ((*it)->isAlertEnabled())
          levels += "ALERT ";
        if ((*it)->isCritEnabled())
          levels += "CRIT ";
        if ((*it)->isDebugEnabled())
          levels += "DEBUG ";
        if ((*it)->isEmergEnabled())
          levels += "EMERG ";
        if ((*it)->isErrorEnabled())
          levels += "ERROR ";
        if ((*it)->isFatalEnabled())
          levels += "FATAL ";
        if ((*it)->isInfoEnabled())
          levels += "INFO ";
        if ((*it)->isNoticeEnabled())
          levels += "NOTICE ";
        if ((*it)->isWarnEnabled())
          levels += "WARN ";

        logger_base.info("    %s : %s", (const char *)(*it)->getName().c_str(),
                         (const char *)levels.c_str());
      }
      delete categories;
    } catch (log4cpp::ConfigureFailure &e) {
      // ignore config failure ... but logging wont work
      printf("Log issue:  %s\n", e.what());
    } catch (const std::exception &ex) {
      printf("Log issue: %s\n", ex.what());
    }
  }
}

// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
// #ifndef wxHAS_IMAGES_IN_RESOURCES
//     #include "../../sample.xpm"
// #endif

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame) EVT_MENU(wxGL_Quit, MyFrame::OnQuit)
    EVT_MENU(wxGL_About, MyFrame::OnAbout)
#if wxUSE_LOGWINDOW
        EVT_MENU(wxGL_LogW, MyFrame::OnLogWindow)
#endif // wxUSE_LOGWINDOW
            wxEND_EVENT_TABLE()

                wxIMPLEMENT_APP(MyApp);

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit() {
  if (!wxApp::OnInit())
    return false;

  InitialiseLogging(true);

  // create the main application window
  MyFrame *frame = new MyFrame("wxWidgets OpenGL wxGL Sample");

  // Exit if the required visual attributes or OGL context couldn't be created
  if (!frame->OGLAvailable())
    return false;

  // As of October 2015 GTK+ needs the frame to be shown before we call
  // SetCurrent()
  frame->Show(true);

  return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MyFrame::MyFrame(const wxString &title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)) {
  // set the frame icon
  // SetIcon(wxICON(sample));

#if wxUSE_MENUS
  // create a menu bar
  wxMenu *fileMenu = new wxMenu;

  // the "About" item should be in the help menu
  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxGL_About, "&About\tF1", "Show about dialog");

#if wxUSE_LOGWINDOW
  fileMenu->Append(wxGL_LogW, "&Log window", "Open the log window");
  fileMenu->AppendSeparator();
#endif // wxUSE_LOGWINDOW
  fileMenu->Append(wxGL_Quit, "E&xit\tAlt-X", "Quit this program");

  // now append the freshly created menu to the menu bar...
  wxMenuBar *menuBar = new wxMenuBar();
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(helpMenu, "&Help");

  // ... and attach this menu bar to the frame
  SetMenuBar(menuBar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
  // create a status bar just for fun (by default with 1 pane only)
  CreateStatusBar(2);
  SetStatusText("Welcome to wxWidgets!");
#endif // wxUSE_STATUSBAR

#if wxUSE_LOGWINDOW
  // Open a log window, don't show it though
  _LogWin = new wxLogWindow(NULL, "wxGL log window", false, false);
  wxLog::SetActiveTarget(_LogWin);
#endif // wxUSE_LOGWINDOW

  // The canvas
  _mycanvas = NULL;
  wxGLAttributes vAttrs;
  // Defaults should be accepted
  vAttrs.PlatformDefaults().Defaults().EndList();
  bool accepted = wxGLCanvas::IsDisplaySupported(vAttrs);

  if (accepted) {
#if wxUSE_LOGWINDOW
    wxLogMessage("The display supports required visual attributes.");
#endif // wxUSE_LOGWINDOW
  } else {
#if wxUSE_LOGWINDOW
    wxLogMessage("First try with OpenGL default visual attributes failed.");
#endif // wxUSE_LOGWINDOW
       // Try again without sample buffers
    vAttrs.Reset();
    vAttrs.PlatformDefaults().RGBA().DoubleBuffer().Depth(16).EndList();
    accepted = wxGLCanvas::IsDisplaySupported(vAttrs);

    if (!accepted) {
      wxMessageBox("Visual attributes for OpenGL are not accepted.\nThe app "
                   "will exit now.",
                   "Error with OpenGL", wxOK | wxICON_ERROR);
    } else {
#if wxUSE_LOGWINDOW
      wxLogMessage("Second try with other visual attributes worked.");
#endif // wxUSE_LOGWINDOW
    }
  }

  if (accepted)
    _mycanvas = new MyGLCanvas(this, vAttrs, true, 0);

  SetMinSize(wxSize(250, 200));
}

// event handlers

void MyFrame::OnQuit(wxCommandEvent &WXUNUSED(event)) {
  // true is to force the frame to close
  Close(true);
}

void MyFrame::OnAbout(wxCommandEvent &WXUNUSED(event)) {
  wxMessageBox(wxString::Format("Welcome to %s!\n"
                                "\n"
                                "This is the wxWidgets OpenGL wxGL sample.\n"
                                "%s\n",
                                wxVERSION_STRING, _OGLString),
               "About wxWidgets wxGL sample", wxOK | wxICON_INFORMATION,
               this);
}

#if wxUSE_LOGWINDOW
void MyFrame::OnLogWindow(wxCommandEvent &WXUNUSED(event)) {
  if (_LogWin->GetFrame()->IsIconized())
    _LogWin->GetFrame()->Restore();

  if (!_LogWin->GetFrame()->IsShown())
    _LogWin->Show();

  _LogWin->GetFrame()->SetFocus();
}
#endif // wxUSE_LOGWINDOW

bool MyFrame::OGLAvailable() {
  // Test if visual attributes were accepted.
  if (!_mycanvas)
    return false;

  // Test if OGL context could be created.
  return _mycanvas->OglCtxAvailable();
}

// ----------------------------------------------------------------------------
// Function for receiving messages from OGLstuff and passing them to the log
// window
// ----------------------------------------------------------------------------
void fOGLErrHandler(int err, int glerr, const GLchar *glMsg) {
  std::cout << "log window" << std::endl;
#if wxUSE_LOGWINDOW
  wxString msg;

  switch (err) {
  case OGL_ERR_SHADERCREATE:
    msg = _("Error in shader creation.");
    break;
  case OGL_ERR_SHADERCOMPILE:
    msg = _("Error in shader compilation.");
    break;
  case OGL_ERR_SHADERLINK:
    msg = _("Error in shader linkage.");
    break;
  case OGL_ERR_SHADERLOCATION:
    msg = _("Error: Can't get uniforms locations.");
    break;
  case OGL_ERR_BUFFER:
    msg = _("Error: Can't load buffer. Likely out of GPU memory.");
    break;
  case OGL_ERR_TEXTIMAGE:
    msg = _("Error: Can't load texture. Likely out of GPU memory.");
    break;
  case OGL_ERR_DRAWING_TRI:
    msg = _("Error: Can't draw the triangles.");
    break;
  case OGL_ERR_DRAWING_STR:
    msg = _("Error: Can't draw the string.");
    break;
  case OGL_ERR_JUSTLOG:
    msg = _("Log info: ");
    break;
  default:
    msg = _("Not a GL message.");
  }

  if (glerr != GL_NO_ERROR)
    msg += wxString::Format(_(" GL error %d. "), glerr);
  else if (err == 0)
    msg = _("Information: ");
  else if (err != OGL_ERR_JUSTLOG)
    msg += _(" GL reports: ");

  if (glMsg != NULL)
    msg += wxString::FromUTF8(reinterpret_cast<const char *>(glMsg));

  wxLogMessage(msg);
  std::cout << "msg: " << msg << std::endl;
#endif // wxUSE_LOGWINDOW
}

// ----------------------------------------------------------------------------
// These two functions allow us to convert a wxString into a RGBA pixels array
// ----------------------------------------------------------------------------

// Creates a 4-bytes-per-pixel, RGBA array from a wxImage.
// If the image has alpha channel, it's used. If not, pixels with 'cTrans' color
// get 'cAlpha' alpha; and the rest of pixels get alpha=255 (opaque).
//
// NOTE: The returned pointer must be deleted somewhere in the app.
unsigned char *MyImgToArray(const wxImage &img, const wxColour &cTrans,
                            unsigned char cAlpha) {
  int w = img.GetWidth();
  int h = img.GetHeight();
  int siz = w * h;
  unsigned char *resArr = new unsigned char[siz * 4];
  unsigned char *res = resArr;
  unsigned char *sdata = img.GetData();
  unsigned char *alpha = NULL;
  if (img.HasAlpha())
    alpha = img.GetAlpha();
  // Pixel by pixel
  for (int i = 0; i < siz; i++) { // copy the colour
    res[0] = sdata[0];
    res[1] = sdata[1];
    res[2] = sdata[2];
    if (alpha != NULL) { // copy alpha
      res[3] = alpha[i];
    } else { // Colour cTrans gets cAlpha transparency
      if (res[0] == cTrans.Red() && res[1] == cTrans.Green() &&
          res[2] == cTrans.Blue())
        res[3] = cAlpha;
      else
        res[3] = 255;
    }
    sdata += 3;
    res += 4;
  }

  return resArr;
}

// Creates an array of bytes that defines the pixels of the string.
// The background color has cAlpha transparency. 0=transparent, 255=opaque
//
// NOTE: The returned pointer must be deleted somewhere in the app.
unsigned char *MyTextToPixels(const wxString &sText,     // The string
                              const wxFont &sFont,       // Font to use
                              const wxColour &sForeColo, // Foreground colour
                              const wxColour &sBackColo, // Background colour
                              unsigned char cAlpha, // Background transparency
                              int *width, int *height) // Image sizes
{
  if (sText.IsEmpty())
    return NULL;

  // The dc where we temporally draw
  wxMemoryDC mdc;

  mdc.SetFont(sFont);

  // Measure
  mdc.GetMultiLineTextExtent(sText, width, height);

  /* This code should be used for old graphics cards.
     But this sample uses OGL Core Profile, so the card is not that old.

  // Adjust sizes to power of two. Needed for old cards.
  int sizP2 = 4;
  while ( sizP2 < *width )
      sizP2 *= 2;
  *width = sizP2;
  sizP2 = 4;
  while ( sizP2 < *height )
      sizP2 *= 2;
  *height = sizP2;
  */

  // Now we know dimensions, let's draw into a memory dc
  wxBitmap bmp(*width, *height, 24);
  mdc.SelectObject(bmp);
  // If we have multiline string, perhaps not all of the bmp is used
  wxBrush brush(sBackColo);
  mdc.SetBackground(brush);
  mdc.Clear(); // Make sure all of bmp is cleared
  // Colours
  mdc.SetBackgroundMode(wxPENSTYLE_SOLID);
  mdc.SetTextBackground(sBackColo);
  mdc.SetTextForeground(sForeColo);
  // We draw the string and get it as an image.
  // NOTE: OpenGL axis are bottom to up. Be aware when setting the texture
  // coords.
  mdc.DrawText(sText, 0, 0);
  mdc.SelectObject(wxNullBitmap); // bmp must be detached from wxMemoryDC

  // Bytes from the image. Background pixels become transparent with the
  // cAlpha transparency value.
  unsigned char *res = MyImgToArray(bmp.ConvertToImage(), sBackColo, cAlpha);

  return res;
}

// ----------------------------------------------------------------------------
// The canvas inside the frame. Our OpenGL connection
// ----------------------------------------------------------------------------

// wxBEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
//     EVT_PAINT(MyGLCanvas::OnPaint)
//     EVT_SIZE(MyGLCanvas::OnSize)
//     EVT_MOUSE_EVENTS(MyGLCanvas::OnMouse)
// wxEND_EVENT_TABLE()

// We create a wxGLContext in this constructor.
// We do OGL initialization at OnSize().
MyGLCanvas::MyGLCanvas(MyFrame *parent, const wxGLAttributes &canvasAttrs, bool a, int styles)
    : xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, styles, a ? "Layout" : "Preview", false),
    _renderTimer(this), _frameCnt(0)
    //wxGLCanvas(parent, canvasAttrs), _renderTimer(this), _frameCnt(0) {
{    
  Bind(wxEVT_SIZE, &MyGLCanvas::OnSize, this);
  Bind(wxEVT_PAINT, &MyGLCanvas::OnPaint, this);

  Bind(wxEVT_LEFT_DOWN, &MyGLCanvas::OnLeftDown, this);
  Bind(wxEVT_TIMER, &MyGLCanvas::OnRenderLoopTimer, this, _renderTimer.GetId());
  Bind(wxEVT_MOUSEWHEEL, &MyGLCanvas::OnMouseWheel, this);

  Bind(wxEVT_KEY_DOWN, &MyGLCanvas::OnKeyDown, this);
  Bind(wxEVT_KEY_UP, &MyGLCanvas::OnKeyUp, this);

  _parent = parent;

  _oglManager = NULL;
  _winHeight = 0; // We have not been sized yet

  // Explicitly create a new rendering context instance for this canvas.
  wxGLContextAttrs ctxAttrs;
#ifndef __WXMAC__
  // An impossible context, just to test IsOk()
  ctxAttrs.PlatformDefaults().OGLVersion(99, 2).EndList();
  _oglContext = new wxGLContext(this, NULL, &ctxAttrs);

  if (!_oglContext->IsOK()) {
#if wxUSE_LOGWINDOW
    wxLogMessage("Trying to set OpenGL 99.2 failed, as expected.");
#endif // wxUSE_LOGWINDOW
    delete _oglContext;
    ctxAttrs.Reset();
#endif //__WXMAC__
    ctxAttrs.PlatformDefaults().CoreProfile().OGLVersion(3, 2).EndList();
    _oglContext = new wxGLContext(this, NULL, &ctxAttrs);
#ifndef __WXMAC__
  }
#endif //__WXMAC__

  if (!_oglContext->IsOK()) {
    wxMessageBox("This sample needs an OpenGL 3.2 capable driver.\nThe app "
                 "will end now.",
                 "OpenGL version error", wxOK | wxICON_INFORMATION, this);
    delete _oglContext;
    _oglContext = NULL;
  } else {
#if wxUSE_LOGWINDOW
    wxLogMessage("OpenGL Core Profile 3.2 successfully set.");
#endif // wxUSE_LOGWINDOW
  }

  _renderTimer.Start(17);
}

MyGLCanvas::~MyGLCanvas() {
  if (_oglContext)
    SetCurrent(*_oglContext);

  if (_oglManager) {
    delete _oglManager;
    _oglManager = NULL;
  }

  if (_oglContext) {
    delete _oglContext;
    _oglContext = NULL;
  }
}

bool MyGLCanvas::oglInit() {
  if (!_oglContext)
    return false;

  // The current context must be set before we get OGL pointers
  SetCurrent(*_oglContext);

  // Initialize our OGL pointers
  if (!myOGLManager::Init()) {
    wxMessageBox("Error: Some OpenGL pointer to function failed.",
                 "OpenGL initialization error", wxOK | wxICON_INFORMATION,
                 this);
    return false;
  }

  // Create our OGL manager, pass our OGL error handler
  _oglManager = new myOGLManager(&fOGLErrHandler);

  // Get the GL version for the current OGL context
  wxString sglVer = "\nUsing OpenGL version: ";
  sglVer += wxString::FromUTF8(
      reinterpret_cast<const char *>(_oglManager->GetGLVersion()));
  // Also Vendor and Renderer
  sglVer += "\nVendor: ";
  sglVer += wxString::FromUTF8(
      reinterpret_cast<const char *>(_oglManager->GetGLVendor()));
  sglVer += "\nRenderer: ";
  sglVer += wxString::FromUTF8(
      reinterpret_cast<const char *>(_oglManager->GetGLRenderer()));
  // For the menu "About" info
  _parent->SetOGLString(sglVer);

  // Load some data into GPU
  _oglManager->SetShadersAndTriangles();

  // This string will be placed on a face of the pyramid
  int swi = 0, shi = 0; // Image sizes
  wxString stg("wxWidgets");
  // Set the font. Use a big pointsize so as to smoothing edges.
  wxFont font(wxFontInfo(48).Family(wxFONTFAMILY_MODERN));
  if (!font.IsOk())
    font = *wxSWISS_FONT;
  wxColour bgrdColo(*wxBLACK);
  wxColour foreColo(160, 0, 200); // Dark purple
  // Build an array with the pixels. Background fully transparent
  unsigned char *sPixels =
      MyTextToPixels(stg, font, foreColo, bgrdColo, 0, &swi, &shi);
  // Send it to GPU
  //_oglManager->SetStringOnPyr(sPixels, swi, shi);
  delete[] sPixels; // That memory was allocated at MyTextToPixels

  // This string is placed at left bottom of the window. Its size doesn't
  // change with window size.
  stg = "Rotate the pyramid with\nthe left mouse button";
  font.SetPointSize(14);
  bgrdColo = wxColour(40, 40, 255);
  foreColo = wxColour(*wxWHITE);
  unsigned char *stPixels =
      MyTextToPixels(stg, font, foreColo, bgrdColo, 80, &swi, &shi);
  //_oglManager->SetImmutableString(stPixels, swi, shi);
  delete[] stPixels;

  return true;
}

void MyGLCanvas::OnPaint(wxPaintEvent &event) {
  // This is a dummy, to avoid an endless succession of paint messages.
  // OnPaint handlers must always create a wxPaintDC.
  wxPaintDC dc(this);

  // Avoid painting when we have not yet a size
  if (_winHeight < 1 || !_oglManager)
    return;

  // This should not be needed, while we have only one canvas
  SetCurrent(*_oglContext);

  // Do the magic
  _oglManager->Render();

  SwapBuffers();
}

// Note:
//  You may wonder why OpenGL initialization was not done at wxGLCanvas ctor.
//  The reason is due to GTK+/X11 working asynchronously, we can't call
//  SetCurrent() before the window is shown on screen (GTK+ doc's say that the
//  window must be realized first).
//  In wxGTK, window creation and sizing requires several size-events. At least
//  one of them happens after GTK+ has notified the realization. We use this
//  circumstance and do initialization then.

void MyGLCanvas::OnSize(wxSizeEvent &event) {
  event.Skip();

  // If this window is not fully initialized, dismiss this event
  if (!IsShownOnScreen())
    return;

  if (!_oglManager) {
    // Now we have a context, retrieve pointers to OGL functions
    if (!oglInit())
      return;
    // Some GPUs need an additional forced paint event
    PostSizeEvent();
  }

  // This is normally only necessary if there is more than one wxGLCanvas
  // or more than one wxGLContext in the application.
  SetCurrent(*_oglContext);

  // It's up to the application code to update the OpenGL viewport settings.
  const wxSize size = event.GetSize() * GetContentScaleFactor();
  _winHeight = size.y;
  _oglManager->SetViewport(0, 0, size.x, _winHeight);

  // Generate paint event without erasing the background.
  Refresh(false);
}

void MyGLCanvas::OnLeftDown(wxMouseEvent &event) {
  wxCursor cursor(wxCURSOR_HAND);
  SetCursor(cursor);

  // GL 0 Y-coordinate is at bottom of the window
  _mousePrev = wxPoint(event.GetX(), _winHeight - event.GetY());
  _mouseDragging = true;

  Bind(wxEVT_MOTION, &MyGLCanvas::OnLeftMotion, this);
  Bind(wxEVT_LEFT_UP, &MyGLCanvas::OnLeftUp, this);

  CaptureMouse();
}

void MyGLCanvas::OnLeftMotion(wxMouseEvent &event) {
  _oglManager->GetCamera().MouseRotation(
      _mousePrev.x, _mousePrev.y, event.GetX(), _winHeight - event.GetY());
  _mousePrev = wxPoint(event.GetX(), _winHeight - event.GetY());
  Refresh(false);
}

void MyGLCanvas::OnLeftUp(wxMouseEvent &event) {
  if (_mouseDragging) {
    SetCursor(wxNullCursor);

    if (HasCapture()) {
      ReleaseMouse();
    }

    Unbind(wxEVT_MOTION, &MyGLCanvas::OnLeftMotion, this);
    Unbind(wxEVT_LEFT_UP, &MyGLCanvas::OnLeftUp, this);

    _mouseDragging = false;

    Refresh(false);
  }
}

void MyGLCanvas::OnKeyDown(wxKeyEvent &event) {
  //event.Skip();

  _previousTime = std::chrono::high_resolution_clock::now();

  if (event.GetUnicodeKey() == 'W') {
    _oglManager->GetCamera().KeyboardPan(FORWARD, true);
  } else if (event.GetUnicodeKey() == 'S') {
    _oglManager->GetCamera().KeyboardPan(BACKWARD, true);
  } else if (event.GetUnicodeKey() == 'D') {
    _oglManager->GetCamera().KeyboardPan(RIGHT, true);
  } else if (event.GetUnicodeKey() == 'A') {
    _oglManager->GetCamera().KeyboardPan(LEFT, true);
  }

    if(event.GetUnicodeKey() == WXK_SPACE) {
        _spacePressed = !_spacePressed;
    }
}

void MyGLCanvas::OnKeyUp(wxKeyEvent &event) {
  //event.Skip();

  if (event.GetUnicodeKey() == 'W') {
    _oglManager->GetCamera().KeyboardPan(FORWARD, false);
  } else if (event.GetUnicodeKey() == 'S') {
    _oglManager->GetCamera().KeyboardPan(BACKWARD, false);
  } else if (event.GetUnicodeKey() == 'D') {
    _oglManager->GetCamera().KeyboardPan(RIGHT, false);
  } else if (event.GetUnicodeKey() == 'A') {
    _oglManager->GetCamera().KeyboardPan(LEFT, false);
  }
}

void MyGLCanvas::OnMouseWheel(wxMouseEvent &event) {
  _oglManager->GetCamera().MouseScroll(event.GetWheelRotation());
  Refresh(false);
}

void MyGLCanvas::OnRenderLoopTimer(wxTimerEvent &event) {
  _oglManager->SetFrameCount(_frameCnt++);
  auto currentTime = std::chrono::high_resolution_clock::now();
  auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - _previousTime);
  _previousTime = currentTime;
  _oglManager->GetCamera().SetDeltaT(dur.count());
  Refresh(false);
}
