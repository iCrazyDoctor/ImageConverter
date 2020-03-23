/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    ============ CYBERTEC ================
 */

// INCLUDE FILES
#include "ImageConverterContainer.h"
#include "ImageConverterAppUi.h"

#include <mw/eiklabel.h>  // for example label control
#include <mw/aknutils.h>  // for scalable UI
#include <mw/eikdef.h>
#include <mw/eikenv.h>
#include <e32cmn.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CImageConverterContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CImageConverterContainer::ConstructL(const TRect& /*aRect*/)
    {
    iAppUi = (static_cast<CImageConverterAppUi*>(iEikonEnv->AppUi()));

    CreateWindowL();

    EnableDragEvents();
    
    #ifdef __TUI_SUPPORTED__    
    iTouchFeedBack = MTouchFeedback::Instance();
    iTouchFeedBack->SetFeedbackEnabledForThisApp(ETrue);
    iLongTapDetector = CAknLongTapDetector::NewL(this);
    #endif
    
    // Create font, FOR FILE NAME
    _LIT( KGeorgia, "Georgia" );   // FONT TYPE
    TFontSpec fontSpec( KGeorgia, 200 );   // FONT SIZE
    fontSpec.iFontStyle.SetBitmapType(EAntiAliasedGlyphBitmap);
    User::LeaveIfError(iEikonEnv->ScreenDevice()->GetNearestFontInTwips( iFont, fontSpec ));

    // create FONT FOR MENU
    _LIT( KArial, "Arial" );
    TFontSpec fontSpec2( KArial, 220 );           // 120
    fontSpec2.iFontStyle.SetBitmapType(EAntiAliasedGlyphBitmap);
    User::LeaveIfError(iEikonEnv->ScreenDevice()->GetNearestFontInTwips( iCbaFont, fontSpec2 ));
    
    SetExtentToWholeScreen();
    ActivateL();
    }

// Destructor
CImageConverterContainer::~CImageConverterContainer()
    {
    iDragPoints.Close();
    delete iPeriodic;
    
    ReleaseBackBuffer();

    #ifdef __TUI_SUPPORTED__
    delete iLongTapDetector;
    #endif

    // Release font
    iEikonEnv->ScreenDevice()->ReleaseFont(iFont);
    iEikonEnv->ScreenDevice()->ReleaseFont(iCbaFont);
    }

// ---------------------------------------------------------
// CImageConverterContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CImageConverterContainer::SizeChanged()
    {
    // Delete back buffer and create a new one with new size
    ReleaseBackBuffer();
    CreateBackBufferL();
    SetPictureScreenCenter(ETrue);
    }

// ---------------------------------------------------------
// CImageConverterContainer::HandleResourceChange()
// Called by framework when resource is changed.
// ---------------------------------------------------------
//
void CImageConverterContainer::HandleResourceChange(TInt aType)
    {
    CCoeControl::HandleResourceChange(aType);
    if ( aType==KEikDynamicLayoutVariantSwitch )
        {
        SetExtentToWholeScreen();
        }
    }

// ---------------------------------------------------------
// CImageConverterContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CImageConverterContainer::CountComponentControls() const
    {
    return 0; // return nbr of controls inside this container
    }

// ---------------------------------------------------------
// CImageConverterContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CImageConverterContainer::ComponentControl(TInt /*aIndex*/) const
    {
    return NULL;
    }

// ---------------------------------------------------------
// CImageConverterContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CImageConverterContainer::Draw(const TRect& /*aRect*/) const
    {
    CWindowGc& gc = SystemGc();
    
    // Copy backbuffer into the screen
 // utilOutputText(gc, _L("© CYBERTEC, 2017"), TPoint(50, 465), KRgbGray );
    gc.BitBlt(TPoint(0, 0), iBackBuffer);
   // iBackBuffer->ExecuteLD(R_IMAGECONVERTER_MENU);
    }

// ---------------------------------------------------------
// CImageConverterContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CImageConverterContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }

// ---------------------------------------------------------
// CImageConverterContainer::SetBitmap(CFbsBitmap* aBitmap)
// ---------------------------------------------------------
//
void CImageConverterContainer::SetBitmap(CFbsBitmap* aBitmap)
    {
    iBitmap = aBitmap;

    if (iBitmap)
        {
        // Set picture first to screen center
        SetPictureScreenCenter(EFalse);

        // If this picture is animated and comes from right or left
       if (iAnimationEvent != ENoAnimation)
            {
            iNextAnimationWaiting = ETrue;
            StartNewPictureAnimate();
            }
        }
    }

#ifdef __TUI_SUPPORTED__
void CImageConverterContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    iLongTapDetector->PointerEventL(aPointerEvent);
    
    if (aPointerEvent.iType == TPointerEvent::EButton1Down)
        {
        iDragPoints.Reset();
        // Start dragging
        iDragPoints.Append(aPointerEvent.iPosition);
        // Give feedback to user (vibration)
        iTouchFeedBack->InstantFeedback(ETouchFeedbackBasic);
        }
    else if (aPointerEvent.iType == TPointerEvent::EDrag)
        {
        // Dragging...
        // If long tap detection is running user is moving picture location then
        // we do not have to store pointer positions
        if (!iLongTapDetected)
            {
            iDragPoints.Append(aPointerEvent.iPosition);
            }
        else
            {
            // Long tap detection is activated so user is moving the picture
            MovePicture(aPointerEvent.iPosition);
            }
        }
    else if (aPointerEvent.iType == TPointerEvent::EButton1Up)
        {
        // Long tap detection ends when pen is released (EButton1Up)
        if (iLongTapDetected)
            {
            // If long tap detection is running user is moving picture location then
            // we do not have to handle pointer positions
            iLongTapDetected = EFalse;
            return;
            }
        
        // Check CBA buttons
        if (iOptionsRect.Contains(aPointerEvent.iPosition))
            {
            iEikonEnv->AppUiFactory()->MenuBar()->TryDisplayMenuBarL();
            return;
            }
        else if (iExitRect.Contains(aPointerEvent.iPosition))
            {
            iAppUi->HandleCommandL(EEikCmdExit);
            return;
            }
        
        // Dragging stops
        iDragPoints.Append(aPointerEvent.iPosition);

        // Clear pointer event
        iPointerEvent = ENoValidPointerEvent;

        // Check user pointer event, how user move pointer
        if (iDragPoints.Count() > KMIN_POINTER_COUNT)
            {
            if (!IsZooming())
                {
                if (!IsChangingPicture())
                    {
                    if (!IsRotate())
                        {
                        // Did not found anything
                        return;
                        }
                    }
                }
            }
        // Do first animation and then event
        if (iAnimationEvent != ENoAnimation)
            {
            // Changing picture
            // appUI->DoEvent() is called when current picture animation ends
            }
        // Do event only (no animation)
        else if (iPointerEvent != ENoValidPointerEvent && iAnimationEvent == ENoAnimation)
            {
            DrawToBackBuffer();
            DrawNow();
            iAppUi->DoEvent(iPointerEvent);
            }
        }
    }
#endif

void CImageConverterContainer::ResetTitleText(TBool aDrawNow)
    {
    iStateInfo.Zero();
    if (aDrawNow)
        {
        DrawToBackBuffer();
        DrawNow();
        }
    }

void CImageConverterContainer::SetTitleText(ETitleState aState, TBool aDrawNow)
    {
    iStateInfo.Zero();

    iTitleState = aState;
    
    switch (iTitleState)
        {
        case ETitleNextImage:
            {
            iStateInfo.Copy(_L("Next"));
            break;
            }
        case ETitlePrevImage:
            {
            iStateInfo.Copy(_L("Previuos"));
            break;
            }
        case ETitleZoomIn:
        case ETitleZoomOut:
            {
            iStateInfo.Copy(_L("Scaled"));
            break;
            }
        case ETitleRotateRight:
        case ETitleRotateLeft:
            {
            iStateInfo.Copy(_L("Rotated"));
            break;
            }
        default:
            {
            break;
            }
        }
    if (aDrawNow)
        {
        DrawToBackBuffer();
        DrawNow();
        }
    }

TBool CImageConverterContainer::IsZooming()
    {
    TPoint firstPoint;
    TPoint lastPoint;
    firstPoint = iDragPoints[0];
    lastPoint = iDragPoints[iDragPoints.Count()-1];

    // Max difference 33%
    TInt max_x_difference = Abs(firstPoint.iY - lastPoint.iY) / 3;
    
    // Is line straight?
    // Goes all points throught and finds is some over max_x_difference
    // if found this is not anymore vertical line
    for (TInt i=1;i<iDragPoints.Count();i++)
        {
        TPoint point = iDragPoints[i];
        TInt value = Abs(point.iX - firstPoint.iX);
        if (value > max_x_difference)
            {
            return EFalse;
            }
        }

    // Is direction up or down?
    lastPoint = iDragPoints[iDragPoints.Count()-1];
    if (firstPoint.iY > lastPoint.iY)
        {
        iPointerEvent = EZoomIn;
        }
    else
        {
        iPointerEvent = EZoomOut;
        }
    return ETrue;
    }

TBool CImageConverterContainer::IsChangingPicture()
    {
    TPoint firstPoint;
    TPoint lastPoint;
    firstPoint = iDragPoints[0];
    lastPoint = iDragPoints[iDragPoints.Count()-1];

    // Max difference 33%
    TInt max_y_difference = Abs(firstPoint.iX - lastPoint.iX) / 3;
    
    // Is line straight?
    // Goes all points throught and finds is some over max_y_difference
    // if found this is not anymore horizontal line
    for (TInt i=1;i<iDragPoints.Count();i++)
        {
        TPoint point = iDragPoints[i];
        TInt value = Abs(point.iY - firstPoint.iY);
        if (value > max_y_difference)
            {
            return EFalse;
            }
        }

    // Is direction right or left?
    lastPoint = iDragPoints[iDragPoints.Count()-1];
    if (firstPoint.iX > lastPoint.iX)
        {
        iPointerEvent = ENextImage;
        iAnimationEvent = EAnimateToLeft;
        }
    else
        {
        iPointerEvent = EPrevImage;
        iAnimationEvent = EAnimateToRight;
        }

    StartAnimateL();
    return ETrue;
    }

void CImageConverterContainer::ChangeNextPictureL()
    {
    iAnimationEvent = EAnimateToLeft;
    iPointerEvent = ENextImage;
    StartAnimateL();
    }

void CImageConverterContainer::ChangePrevPictureL()
    {
    iAnimationEvent = EAnimateToRight;
    iPointerEvent = EPrevImage;
    StartAnimateL();
    }

TPoint CImageConverterContainer::FindCenterPointInRotate(TPoint& aFirstPoint)
    {
    TPoint center;
    TInt  farist(0);

    for (TInt i=0;i<iDragPoints.Count();i++)
        {
        TPoint point = iDragPoints[i];
        TInt value = Abs(point.iX - aFirstPoint.iX);
        if (value > farist)
            {
            farist = value;
            center = point;
            }
        }
    
    return center;
    }

void CImageConverterContainer::FindPointsForRotate(TPoint& aTopPoint, 
        TPoint& aCenterPoint, TPoint& aBottomPoint, TBool& aDrawDirectionIsFromTop)
    {
    aTopPoint = TPoint(Size().iWidth/2,Size().iHeight);
    aBottomPoint = TPoint(Size().iWidth/2,0);
    TInt topIndex=0;
    TInt bottomIndex=0;

    // Find top and bottom points
    for (TInt i=0;i<iDragPoints.Count();i++)
        {
        if (aTopPoint.iY > iDragPoints[i].iY)
            {
            aTopPoint = iDragPoints[i];
            topIndex = i;
            }
        if (aBottomPoint.iY < iDragPoints[i].iY)
            {
            aBottomPoint = iDragPoints[i];
            bottomIndex = i;
            }
        }
    
    // Find center point
    aCenterPoint = FindCenterPointInRotate(aTopPoint);
    
    // Is direction to left or right?
    if (topIndex < bottomIndex)
        {
        aDrawDirectionIsFromTop = ETrue;
        }
    else
        {
        aDrawDirectionIsFromTop = EFalse;
        }
    }

TBool CImageConverterContainer::IsRotate()
    {
    TPoint firstPoint;
    TPoint centerPoint;
    TPoint lastPoint;
    TBool drawFromTop;
    
    // Find points locations (top, bottom, center(left/right))
    FindPointsForRotate(firstPoint,centerPoint,lastPoint,drawFromTop);
    
    // Max difference is height of the drawed thing
    TInt height = Abs(firstPoint.iY - lastPoint.iY);
    
    // Start and end point x difference can not be more than
    // height of the whole thing
    TInt xSE = Abs(firstPoint.iX - lastPoint.iX);
    if (xSE > height)
        {
        return EFalse;
        }
    
    // Check center point in x-axel
    TInt maxXcenterLine = Max(Abs(firstPoint.iX-centerPoint.iX),Abs(lastPoint.iX-centerPoint.iX));
    // Must draw to right or left at least 33% from height
    if (maxXcenterLine < height / 3) // 33%
        {
        return EFalse;
        }
    
    // Rotate to left or right?
    if (centerPoint.iX < firstPoint.iX)
        {
        // Left
        if (drawFromTop)
            {
            iPointerEvent = ERotateLeft;
            }
        else
            {
            iPointerEvent = ERotateRight;
            }
        }
    else
        {
        // Right
        if (drawFromTop)
            {
            iPointerEvent = ERotateRight;
            }
        else
            {
            iPointerEvent = ERotateLeft;
            }
        }

    return ETrue;
    }

void CImageConverterContainer::StartAnimateL()
    {
    // Start animation timer, image is ready
    if (iAnimationEvent != ENoAnimation && iBitmap && !iAppUi->IsEngineBusy())
        {
        if (!iPeriodic)
            {
            iPeriodic = CPeriodic::NewL(CActive::EPriorityStandard);
            }
        if (iPeriodic)
            {
            iPeriodic->Cancel();
            }

        // Update title
        if (iAnimationEvent == EAnimateToLeft)
            {
            SetTitleText(ETitleNextImage,EFalse);
            }
        else if(iAnimationEvent == EAnimateToRight)
            {
            SetTitleText(ETitlePrevImage,EFalse);
            }
        
        
        if (iAnimationEvent == EAnimateToLeft  || iAnimationEvent == EAnimateToRight)
            {
            iAnimateX = 0;
            iPixelsToMove = 1;
            }
        else if (iAnimationEvent == EAnimateFromRight)
            {
            iAnimateX = 0;
            iPixelsToMove = (Size().iWidth - iBitmap->SizeInPixels().iWidth)/2 + iBitmap->SizeInPixels().iWidth;
            }
        else if (iAnimationEvent == EAnimateFromLeft)
            {
            iAnimateX = 0;
            iPixelsToMove = (Size().iWidth - iBitmap->SizeInPixels().iWidth)/2 + iBitmap->SizeInPixels().iWidth;
            }
        iPeriodic->Start(TTimeIntervalMicroSeconds32(0), DELAY, TCallBack(CImageConverterContainer::Animate, this));
        }
    // Engine is busy, there is image conversion or scaling ongoing, wait for these
    else if (iAppUi->IsEngineBusy())
        {
        if (!iPeriodic)
            {
            iPeriodic = CPeriodic::NewL(CActive::EPriorityStandard);
            }
        if (iPeriodic)
            {
            iPeriodic->Cancel();
            }
        iPeriodic->Start(SLEEP_DELAY, SLEEP_DELAY, TCallBack(CImageConverterContainer::Sleep, this));
        }
    // No image, no animation
    else if (!iAppUi->IsEngineBusy() && !iBitmap)
        {
        iAnimationEvent = ENoAnimation;
        StopAnimate();
        }
    }

void CImageConverterContainer::StartNewPictureAnimate()
    {
    if (iNextAnimationWaiting)
        {
        iNextAnimationWaiting = EFalse;
        if (iAnimationEvent == EAnimateToLeft)
            {
            iAnimationEvent = EAnimateFromRight;
            }
        else if (iAnimationEvent == EAnimateToRight)
            {
            iAnimationEvent = EAnimateFromLeft;
            }
        
        if (iAnimationEvent == EAnimateFromRight)
            {
            iPicturePoint.iX = Size().iWidth;
            }
        else if (iAnimationEvent == EAnimateFromLeft)
            {
            iPicturePoint.iX = iBitmap->SizeInPixels().iWidth*-1;
            }
        
        StartAnimateL();
        }
    }

TBool CImageConverterContainer::IsAnimating()
    {
    if (iPeriodic && iPeriodic->IsActive())
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }

void CImageConverterContainer::StopAnimate()
    {
    // Stop animation
    if (iPeriodic)
        {
        iPeriodic->Cancel();
        }

    // Call event now (animation is ended)
    iAppUi->DoEvent(iPointerEvent);
    // Clear pointer event
    iPointerEvent = ENoValidPointerEvent;

    // Set animation event to ENoAnimation after new picture is also animated
    if (iAnimationEvent == EAnimateFromRight || iAnimationEvent == EAnimateFromLeft)
        {
        iAnimationEvent = ENoAnimation;
        }
    
    if (iAnimationEvent==ENoAnimation)
        {
        ResetTitleText(EFalse);
        }
    }

void CImageConverterContainer::SetNextAnimationWaiting(TBool aValue)
    {
    iNextAnimationWaiting = aValue;
    }

TInt CImageConverterContainer::Animate(TAny* aObj)
    {
    static_cast<CImageConverterContainer*>(aObj)->DoFrame();
    return 1;
    }

TInt CImageConverterContainer::Sleep(TAny* aObj)
    {
    static_cast<CImageConverterContainer*>(aObj)->SetNextAnimationWaiting(EFalse);
    static_cast<CImageConverterContainer*>(aObj)->StartAnimateL();
    return 0;
    }

void CImageConverterContainer::SetPictureScreenCenter(TBool aDraw)
    {
    if (iBitmap)
        {
        iPicturePoint.SetXY((Size().iWidth - iBitmap->SizeInPixels().iWidth) / 2, (Size().iHeight - iBitmap->SizeInPixels().iHeight) / 2);
        if (aDraw)
            {
            DrawToBackBuffer();
            DrawNow();
            }
        }
    }

void CImageConverterContainer::DoFrame()
    {
    if (iAnimationEvent == EAnimateToLeft)
        {
        if (iPicturePoint.iX < iBitmap->SizeInPixels().iWidth*-1)
            {
            StopAnimate();
            }
        else
            {
            iPixelsToMove = iPixelsToMove*2;
            iAnimateX = iAnimateX - iPixelsToMove;
            }
        }
    else if (iAnimationEvent == EAnimateToRight)
        {
        if (iPicturePoint.iX > Size().iWidth)
            {
            StopAnimate();
            }
        else
            {
            iPixelsToMove = iPixelsToMove*2;
            iAnimateX = iAnimateX + iPixelsToMove;
            }
        }
    else if (iAnimationEvent == EAnimateFromLeft)
        {
        // Stop animate if picture is about screen center
        iPixelsToMove = iPixelsToMove/2;
        iPixelsToMove++;
        TInt tmp = iPixelsToMove + iPicturePoint.iX;
        if (tmp >= (Size().iWidth - iBitmap->SizeInPixels().iWidth)/2)
            {
            StopAnimate();
            SetPictureScreenCenter(EFalse);
            }
        else
            {
            iAnimateX = iPixelsToMove;
            }
        }
    else if (iAnimationEvent == EAnimateFromRight)
        {
        // Stop animate if picture is about screen center
        iPixelsToMove = iPixelsToMove/2;
        iPixelsToMove++;
        TInt tmp = iPicturePoint.iX - iPixelsToMove;
        if (tmp <= (Size().iWidth - iBitmap->SizeInPixels().iWidth)/2)
            {
            StopAnimate();
            SetPictureScreenCenter(EFalse);
            }
        else
            {
            iAnimateX = iPixelsToMove*-1;
            }
        }
    else
        {
        StopAnimate();
        }
    
    DrawToBackBuffer();
    DrawNow();
    }

#ifdef __TUI_SUPPORTED__
void CImageConverterContainer::HandleLongTapEventL( const TPoint& aPenEventLocation, const TPoint& /*aPenEventScreenLocation*/ )
    {
    iLongTapDetected = ETrue;
    iLongTapPoint.SetXY(-10,-10);
    MovePicture(aPenEventLocation);
    }

void CImageConverterContainer::MovePicture(const TPoint& aPoint)
    {
    if (iLongTapPoint.iX == -10)
        {
        // First move received
        iLongTapPoint = aPoint;
        return;
        }
    TInt x = iLongTapPoint.iX - aPoint.iX;
    TInt y = iLongTapPoint.iY - aPoint.iY;
    iPicturePoint.iX = iPicturePoint.iX - x; 
    iPicturePoint.iY = iPicturePoint.iY - y; 
    iLongTapPoint = aPoint;
    DrawToBackBuffer();
    DrawNow();
    }
#endif

void CImageConverterContainer::CreateBackBufferL()
    {
    // create back buffer bitmap
    iBackBuffer = new (ELeave) CFbsBitmap;
    
    User::LeaveIfError( iBackBuffer->Create(Size(),
                        iEikonEnv->DefaultDisplayMode()));
    
    // create back buffer graphics context
    iBackBufferDevice = CFbsBitmapDevice::NewL(iBackBuffer);
    User::LeaveIfError(iBackBufferDevice->CreateContext(iBackBufferContext));
    iBackBufferContext->SetPenStyle(CGraphicsContext::ESolidPen);
    
    iBackBufferSize = iBackBuffer->SizeInPixels();
    }

void CImageConverterContainer::ReleaseBackBuffer()
    {
    if (iBackBufferContext)
        {
        delete iBackBufferContext;
        iBackBufferContext = NULL;
        }
    if (iBackBufferDevice)
        {
        delete iBackBufferDevice;
        iBackBufferDevice = NULL;
        }
    if (iBackBuffer)
        {
        delete iBackBuffer;
        iBackBuffer = NULL;
        }
    iBackBufferSize = TSize(0, 0);
    }

void CImageConverterContainer::DrawToBackBuffer()
    {
    if (!iBackBufferContext)
        {
        return;
        }         
    iBackBufferContext->SetBrushColor(KRgbBlack);
    iBackBufferContext->Clear();
    
    TRgb color = KRgbWhite;
    color.SetAlpha(70);
    
    iBackBufferContext->SetPenColor(KRgbYellow);
    TInt textWidth = 0;
    
    TFileName filename;

    iBackBufferContext->UseFont(iFont);

    // Draw status text
    textWidth = iFont->TextWidthInPixels(iStateInfo);
    iBackBufferContext->DrawText(iStateInfo,TPoint((Size().iWidth-textWidth)/2,iFont->FontMaxHeight()+3));
    
    // Draw bitmap and picture counter and filename
    if (iBitmap)
        {
        if (iAnimationEvent != ENoAnimation)    
            {
            iPicturePoint.iX = iPicturePoint.iX + iAnimateX;
            iBackBufferContext->BitBlt(iPicturePoint, iBitmap);
            }
        else
            {
            iBackBufferContext->BitBlt(iPicturePoint, iBitmap);
            iBackBufferContext->SetPenColor(KRgbBlue);
            iBackBufferContext->DrawRect(TRect(iPicturePoint.iX-3,
                                               iPicturePoint.iY-3,
                                               iPicturePoint.iX + iBitmap->SizeInPixels().iWidth + 3,
                                               iPicturePoint.iY + iBitmap->SizeInPixels().iHeight + 3));

            iBackBufferContext->SetPenColor(KRgbWhite);

            // Draw filename
            iAppUi->ImageName(filename);
            textWidth = iFont->TextWidthInPixels(filename);
            iBackBufferContext->DrawText(filename,TPoint(5,iFont->FontMaxHeight()+3));

            // Draw counter
            iPictureInfo.Zero();
            iPictureInfo.AppendNum(iAppUi->ImageIndex()+1);
            iPictureInfo.Append(_L(" of "));
            iPictureInfo.AppendNum(iAppUi->ImageCount());
            textWidth = iFont->TextWidthInPixels(iPictureInfo);
            iBackBufferContext->DrawText(iPictureInfo,TPoint(Size().iWidth-textWidth-5,iFont->FontMaxHeight()+3));
            }
        }

    // Draw round rect for filename and counter
    iBackBufferContext->SetBrushColor(color);
    iBackBufferContext->SetBrushStyle(CWindowGc::ESolidBrush);
    iBackBufferContext->DrawRoundRect(TRect(2,2,Size().iWidth-2,iFont->FontMaxHeight()+7),TSize(2,2));
    iBackBufferContext->SetBrushStyle(CWindowGc::ENullBrush);

    //Finding correct location for CBA and drawing them.
    // Note that there is a known issue detected in the S60 5th Edition SDK v0.9. With an empty status pane,            
    // AknLayoutUtils::CbaLocation() does not return correct locations for CBA in 
    // the landscape orientation. This method has been tested to work fine on S60 3rd Edition and
    // 5th Edition devices.
    
    // CYBERTEC, DRAW TEXT MENU PANE
          //  iBackBufferContext->SetPenColor(KRgbGreen);  // TEXT COLOR
           // iPictureInfo.Zero();
           // iPictureInfo.Append(_L("<-prev"));   // TEXT-1
          //  textWidth = iFont->TextWidthInPixels(iPictureInfo); // TEXT FONT
          //  iBackBufferContext->DrawText(iPictureInfo,TPoint(Size().iWidth-textWidth-300,Size().iHeight-65)); // TEXT CORDINATE
    {
    iBackBufferContext->SetPenColor(KRgbDarkGray);
    iPictureInfo.Zero();
    iPictureInfo.Append(_L("v4.3.5"));   // TEXT-2
    textWidth = iFont->TextWidthInPixels(iPictureInfo);
    iBackBufferContext->DrawText(iPictureInfo,TPoint(Size().iWidth-textWidth-10,Size().iHeight-60));
    // ADD BUTTON AND SIZE, iCyberTec.SetRect(Size().iWidth-textWidth-50,Size().iHeight-50,Size().iWidth,Size().iHeight);
    }
    
    iBackBufferContext->UseFont(iCbaFont);
    if (AknLayoutUtils::CbaLocation() == AknLayoutUtils::EAknCbaLocationRight)
    {
    // Landscape
    if (iAppUi->IsOptionsButtonOnTop())
            {
      //      iBackBufferContext->SetPenColor(KRgbRed);
                            // Options selection is on top (Nokia E90)
                            // Draw menu item "Exit"
       //     iPictureInfo.Zero();
      //      iPictureInfo.Append(_L("X"));
      //      textWidth = iCbaFont->TextWidthInPixels(iPictureInfo);
      //      iBackBufferContext->DrawText(iPictureInfo,TPoint(Size().iWidth-textWidth-15,Size().iHeight-7));
                            // Save exit rect for Touch
      //      iExitRect.SetRect(Size().iWidth-textWidth-50,Size().iHeight-50,Size().iWidth,Size().iHeight);

       //     iBackBufferContext->SetPenColor(KRgbGreen);
                            // Draw menu item "Options"
       //     iPictureInfo.Zero();
       //     iPictureInfo.Append(_L("|||"));
       //     textWidth = iCbaFont->TextWidthInPixels(iPictureInfo);
      //      iBackBufferContext->DrawText(iPictureInfo,TPoint(Size().iWidth-textWidth-15,iCbaFont->FontMaxHeight()+30));
                            // Save option rect for Touch
      //      iOptionsRect.SetRect(Size().iWidth-textWidth-50,iCbaFont->FontMaxHeight()+50,Size().iWidth,iCbaFont->FontMaxHeight()+40);
            }
        else
            {
       //     iBackBufferContext->SetPenColor(KRgbGreen);
                              // Options selection is on bottom (Nokia N95)
                              // Draw menu item "Options"
       //     iPictureInfo.Zero();
        //    iPictureInfo.Append(_L("|||"));
        //    textWidth = iCbaFont->TextWidthInPixels(iPictureInfo);
        //    iBackBufferContext->DrawText(iPictureInfo,TPoint(Size().iWidth-textWidth-15,Size().iHeight-7));
                              // Save exit rect for Touch
       //     iOptionsRect.SetRect(Size().iWidth-textWidth-50,Size().iHeight-50,Size().iWidth,Size().iHeight);

        //    iBackBufferContext->SetPenColor(KRgbRed);
                              // Draw menu item "Exit"
       //     iPictureInfo.Zero();
      //      iPictureInfo.Append(_L("X"));
      //      textWidth = iCbaFont->TextWidthInPixels(iPictureInfo);
       //     iBackBufferContext->DrawText(iPictureInfo,TPoint(Size().iWidth-textWidth-15,iCbaFont->FontMaxHeight()+30));
                              // Save option rect for Touch
       //     iExitRect.SetRect(Size().iWidth-textWidth-50,iCbaFont->FontMaxHeight()+50,Size().iWidth,iCbaFont->FontMaxHeight()+40);
        }
        }
    
    // CREATE BOTTOM BUTTONS, OPTION and EXIT
    else if(AknLayoutUtils::CbaLocation() == AknLayoutUtils::EAknCbaLocationBottom)
    {
        iBackBufferContext->SetPenColor(KRgbGray);
        // Portrait
        // Draw menu item "Options"
        iPictureInfo.Zero();
       iPictureInfo.Append(_L("Options"));         // CYBERTEC
        textWidth = iCbaFont->TextWidthInPixels(iPictureInfo);
        iBackBufferContext->DrawText(iPictureInfo,TPoint(15,Size().iHeight-7));
        // Save option rect for Touch
        iOptionsRect.SetRect(0,Size().iHeight-50,textWidth+50,Size().iHeight);
                      
        iBackBufferContext->SetPenColor(KRgbRed);     // CYBERTEC
        // Draw menu item "Exit"
        iPictureInfo.Zero();
        iPictureInfo.Append(_L("Exit"));
        textWidth = iCbaFont->TextWidthInPixels(iPictureInfo);
        iBackBufferContext->DrawText(iPictureInfo,TPoint(Size().iWidth-textWidth-15,Size().iHeight-7));
        // Save exit rect for Touch
        iExitRect.SetRect(Size().iWidth-textWidth-50,Size().iHeight-50,Size().iWidth,Size().iHeight);
    	}  
    }

// End of File  
