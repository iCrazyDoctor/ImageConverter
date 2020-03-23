/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
         ==================== CYBERTEC ================== 
*/

// INCLUDE FILES
#include <AknUtils.h> // Pen support.
#include <ImageConversion.h>
#include "ShowInfoDialog.h"

const TInt KMaxInfoDescriptorLength = 100;

CShowInfoDialog::CShowInfoDialog( CFrameInfoStrings* aInfoStrings ) : 
    iInfoStrings( aInfoStrings )
    {
    }

CShowInfoDialog::~CShowInfoDialog()
    {
    iEikonEnv->ScreenDevice()->ReleaseFont(iFont);
    }

void CShowInfoDialog::PreLayoutDynInitL()
    {
    CAknQueryDialog::PreLayoutDynInitL();
    SetEditableL(ETrue);    
    
    // Create font
    _LIT( KArial, "Arial" );   // FONT NAME
    TFontSpec fontSpec( KArial, 140 );   // FONT SIZE
    fontSpec.iFontStyle.SetBitmapType(EAntiAliasedGlyphBitmap);
    User::LeaveIfError(iEikonEnv->ScreenDevice()->GetNearestFontInTwips( iFont, fontSpec ));
    }

void CShowInfoDialog::PostLayoutDynInitL()
    {
    CalculatePositionAndSize();
    }

TBool CShowInfoDialog::OkToExitL(TInt /*aButtonId*/)
    {
    return ETrue;
    }

TInt CShowInfoDialog::CountComponentControls() const
    {
    return( 0 ); 
    }

CCoeControl* CShowInfoDialog::ComponentControl(TInt /*aIndex*/) const
    {
    return NULL;
    }

void CShowInfoDialog::CalculatePositionAndSize()
    {
    TRect cRect;
    AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane,cRect);
    
   // cRect.Shrink(25, 25);  ORG
    cRect.Shrink(5, 5);
    SetExtent(cRect.iTl,cRect.Size());
    }

void CShowInfoDialog::SetSizeAndPosition(const TSize &aSize)
    {
    CAknQueryDialog::SetSizeAndPosition( aSize );
    
    CalculatePositionAndSize();
    }

void CShowInfoDialog::SizeChanged()
    {
    DrawNow();
    }

void CShowInfoDialog::HandleResourceChange(TInt aType)
    {
    CAknQueryDialog::HandleResourceChange(aType);   
    }

void CShowInfoDialog::Draw( const TRect& /*aRect*/ ) const
    {
    // Get the standard graphics context
    CWindowGc& gc = SystemGc();
    gc.SetPenStyle( CGraphicsContext::ESolidPen );
    gc.SetBrushColor( KRgbBlue);
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.IsFbsBitGc();

    // Clear screen
    gc.Clear(Rect());
    gc.SetBrushStyle( CGraphicsContext::ENullBrush );
    
    // Draw round rect
    gc.SetPenStyle( CGraphicsContext::ESolidPen );
    gc.SetPenColor(KRgbWhite);
    gc.DrawRect(Rect());
   // gc.DrawLineTo(TPoint(50, 46));
  //  utilOutputText(gc, _L("© CYBERTEC, 2017"), TPoint(50, 465), KRgbGray );

    // Draw image info texts
    gc.UseFont(iFont);
    TBuf<KMaxInfoDescriptorLength> desc;
    TPoint point = Rect().iTl;
    point.iX += 10;
    for( TInt i=0; i<iInfoStrings->Count(); i++ ) 
        {
        point.iY += 20;
        desc.Copy( iInfoStrings->String(i) );
        gc.DrawText(desc,point);
        }
    }

// end of file
