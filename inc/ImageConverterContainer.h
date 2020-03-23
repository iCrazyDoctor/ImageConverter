/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    =========== CYBERTEC =============
 */

#ifndef IMAGECONVERTERCONTAINER_H
#define IMAGECONVERTERCONTAINER_H

// INCLUDES
#include <mw/coecntrl.h>
#include <sensrvdatalistener.h> // ABOUTL

#ifdef __TUI_SUPPORTED__
#include <touchfeedback.h>
#include <touchlogicalfeedback.h>
#include <aknlongtapdetector.h> 
#endif

// Minium amount of pointer drag events received
const TInt KMIN_POINTER_COUNT = 10;

#define DELAY 30000
#define SLEEP_DELAY 1*1000*1000

struct TApplicationInfo
{
TFileName iCaption;
	TUid iUid;
	static TInt Compare(const TApplicationInfo& aInfo1, const TApplicationInfo& aInfo2)
	{
		return aInfo1.iCaption.CompareC(aInfo2.iCaption);
	}
};
// FORWARD DECLARATIONS
class CEikLabel;        // for example labels
class CImageConverterAppUi;

class AknsUtils;
class AknTextUtils;

enum EPointerEvents
    {
    ENoValidPointerEvent = 0,
    EZoomIn,
    EZoomOut,
    ENextImage,
    EPrevImage,
    ERotateRight,
    ERotateLeft
    };

enum ETitleState
    {
    ETitleNo = 0,
    ETitleZoomIn,
    ETitleZoomOut,
    ETitleNextImage,
    ETitlePrevImage,
    ETitleRotateRight,
    ETitleRotateLeft
    };

enum EAnimationEvents
    {
    ENoAnimation = 0,
    EAnimateToRight,
    EAnimateToLeft,
    EAnimateFromRight,
    EAnimateFromLeft
    };

// CLASS DECLARATION

/**
*  CImageConverterContainer  container control class.
*  
*/
class CImageConverterContainer : public CCoeControl, 
MCoeControlObserver
#ifdef __TUI_SUPPORTED__
, MAknLongTapDetectorCallBack
#endif    
    {
    public: // Constructors and destructor
        
        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect);

        /**
        * Destructor.
        */
        ~CImageConverterContainer();

    public: // New functions
        void SetBitmap(CFbsBitmap* aBitmap);
        void StartAnimateL();
        void StartNewPictureAnimate();
        TBool IsAnimating();
        void SetNextAnimationWaiting(TBool aValue);
     //   void OpenBrowser(const TDesC& aUrl); // CYBERTEC
        TUid Id() const;  // CYBERTEC
        void ChangeNextPictureL();
        void ChangePrevPictureL();
        void SetPictureScreenCenter(TBool aDraw);
        
    public:
        void DrawToBackBuffer();
        void SetTitleText(ETitleState aState, TBool aDrawNow);
        void ResetTitleText(TBool aDrawNow);

    private:

        /**
        * From CoeControl,SizeChanged.
        */
        void SizeChanged();
        void AboutL( const TDesC& aText ) const;

        /**
        * From CoeControl,HandleResourceChange.
        */
        void HandleResourceChange(TInt aType);

        /**
        * From CoeControl,CountComponentControls.
        */
        TInt CountComponentControls() const;

        /**
        * From CCoeControl,ComponentControl.
        */
        CCoeControl* ComponentControl(TInt aIndex) const;

        /**
        * From CCoeControl,Draw.
        */
        void Draw(const TRect& aRect) const;
        
        /**
        * From MCoeControlObserver
        * Acts upon changes in the hosted control's state. 
        *
        * @param    aControl    The control changing its state
        * @param    aEventType  The type of control event 
        */
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
        
        #ifdef __TUI_SUPPORTED__
        void HandlePointerEventL(const TPointerEvent& aPointerEvent);
        void MovePicture(const TPoint& aPoint);        
        #endif
        
        TBool IsZooming();

        TBool IsChangingPicture();
        
        TBool IsRotate();
        
    //    TBool SIStore( TInt aCommand ); // CYBERTEC

        TPoint FindCenterPointInRotate(TPoint& aFirstPoint);
        
        void FindPointsForRotate(TPoint& aTopPoint, TPoint& aCenterPoint, TPoint& aBottomPoint, TBool& aDrawDirectionIsFromTop);

        void StopAnimate();

        static TInt Animate(TAny* aObj);
        static TInt Sleep(TAny* aObj);
        
        void DoFrame();
        
        void CreateBackBufferL();
        void ReleaseBackBuffer();
        
        
        

    #ifdef __TUI_SUPPORTED__    
    private: // From MAknLongTapDetectorCallBack
        void HandleLongTapEventL( const TPoint& aPenEventLocation, const TPoint& aPenEventScreenLocation );
    #endif        

    private: //data
       CFbsBitmap*          iBitmap;        
       TPoint               iPicturePoint;
       TBuf<100>            iPictureInfo;
       TBuf<100>            iStateInfo;
       CFont*               iFont;
       CFont*               iCbaFont;
       CFont*               iFont2;      // CYBERTEC

       // Pointer events
       RArray<TPoint>       iDragPoints;
       EPointerEvents       iPointerEvent;

       // Animation
       CPeriodic*           iPeriodic; 
       TInt                 iAnimateX;
       TInt                 iPixelsToMove;
       TBool                iNextAnimationWaiting;
       EAnimationEvents     iAnimationEvent;

       #ifdef __TUI_SUPPORTED__       
       // Long tap detection
       CAknLongTapDetector* iLongTapDetector;
       TBool                iLongTapDetected;
       TPoint               iLongTapPoint;
       
       MTouchFeedback*      iTouchFeedBack;
       #endif       
       
       CImageConverterAppUi*            iAppUi;
       
       TRect                            iOptionsRect;
       TRect                            iExitRect;
       TRect                            iCyberTec;  // CYBERTEC TEXT CONTAINER

       // double buffer bitmapsystem
       CFbsBitmap*                      iBackBuffer;
       CFbsBitmapDevice*                iBackBufferDevice;
       CFbsBitGc*                       iBackBufferContext;
       TSize                            iBackBufferSize;
       
       ETitleState                      iTitleState;
    };

#endif

// End of File
