/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    ============ CYBERTEC ==============
 */

#include "ImageConverterEngine.h"

#include <fbs.h>
#include <ImageConversion.h>
#include <BitmapTransforms.h>

#include <eikdef.h>
#include <eikenv.h>

const TInt KMaxInfoMsgLength = 50;
const TInt KFixImageHeight = 70;
const TInt KFixImageWidth = 10;

CImageConverterEngine* CImageConverterEngine::NewL( 
    MConverterController* aController )
    {
    CImageConverterEngine* self = 
        new(ELeave) CImageConverterEngine( aController );
    CleanupStack::PushL( self );
    
    self->ConstructL();

    CleanupStack::Pop(self);
    return self; 
    }

CImageConverterEngine::CImageConverterEngine( 
    MConverterController* aController ) : 
    CActive( EPriorityStandard ), iController( aController )
    {
    }

void CImageConverterEngine::ConstructL()
    {
    User::LeaveIfError( iFs.Connect() );
    iRotator = CBitmapRotator::NewL();
    iScaler = CBitmapScaler::NewL();
    iGlobalNote = CAknGlobalNote::NewL();   // START NOTE, SYMBIANC
    iState = EIdle;
    CActiveScheduler::Add( this );
    }

CImageConverterEngine::~CImageConverterEngine()
    {
    Cancel();
    delete iImageEncoder; 
    delete iImageDecoder; // CImageDecoder must be deleted before the 
    iFs.Close();          //   related RFs is closed, 
    delete iBitmap;       //   otherwise a related thread might panic   
    delete iRotator;
    delete iScaler;
    //delete iGlobalNote;
    }

void CImageConverterEngine::StartToDecodeL( TFileName& aFileName )
    {
    if (IsActive())
        {
        return;
        }

    delete iImageDecoder; iImageDecoder = NULL;
    delete iBitmap; iBitmap = NULL;

    iFilename.Copy(aFileName);
    
    // create the decoder
    iImageDecoder = CImageDecoder::FileNewL( iFs, aFileName );
    
    // create the destination bitmap
    iBitmap = new (ELeave) CFbsBitmap();
    iBitmap->Create( iImageDecoder->FrameInfo().iOverallSizeInPixels,
                     iImageDecoder->FrameInfo().iFrameDisplayMode ); 

    // start conversion to bitmap
    iState = EDecoding;
    iImageDecoder->Convert( &iStatus, *iBitmap );
    SetActive();
    }

void CImageConverterEngine::StartToEncodeL( 
    TFileName& aFileName, TUid aImageType, TUid aImageSubType )
    {
    if (!iBitmap)
        {
        return;
        }
    
    if (iImageEncoder)
        {
        delete iImageEncoder; 
        iImageEncoder = NULL;
        }

    // create the encoder
    iImageEncoder = CImageEncoder::FileNewL( iFs, aFileName, 
        CImageEncoder::EOptionNone, aImageType, aImageSubType );

    iState = EEncoding;
    iImageEncoder->Convert( &iStatus, *iBitmap );
    SetActive();
    
    ShowProgress();
    }

void CImageConverterEngine::DoCancel()
    {
    CancelProgress();
    
    if( iState == EDecoding )
        {
        iImageDecoder->Cancel();
        }
    else if( iState == EEncoding )
        {
        iImageEncoder->Cancel();
        }
    else if( iState == ERotating )
        {
        iRotator->Cancel();
        }
    else if( iState == EScaling )
        {
        iScaler->Cancel();
        }
    else
        {
        // State is EIdle, do nothing
        }
    }
    
void CImageConverterEngine::RunL()
    {
    CancelProgress();

    switch( iState ) 
        {
        case EDecoding:
            {
            iScaleDownCounter = 0;
            if( iStatus == KErrNone ) 
                {
                if (FitToScreen())
                    {
                    iState = EIdle;
                    _LIT(KImageReady, "Image ready");
                    iController->NotifyCompletion( KErrNone, KImageReady );
                    }
                else
                    {
                    _LIT(KImageReady, "Image not ready");
                    iController->NotifyCompletion( KErrNotReady, KImageReady );
                    DoFitToScreen();
                    }
                break;
                }
            else if( iStatus == KErrUnderflow ) 
                {
                ShowProgress();
                iImageDecoder->ContinueConvert( &iStatus );
                SetActive();
                break;
                }
            else if ( iStatus == KErrCorrupt )
                {
                iState = EIdle;
                _LIT(KImageCorrupt, "Image file corrupted");
                iController->NotifyCompletion( KErrCorrupt, KImageCorrupt );
                break;
                }
            else
                {
                // Unknown error
                iState = EIdle;
                _LIT( KErrorMsg, "Error opening image: %d" );
                TBuf<KMaxInfoMsgLength> buf;
                buf.Format( KErrorMsg, iStatus.Int() );
                iController->NotifyCompletion( iStatus.Int(), KErrorMsg );
                break;
                }
            }
        case EEncoding:
            {
            _LIT(KImageSaved, "Image saved");
            iState = EIdle;
            iController->NotifyCompletion( iStatus.Int(), KImageSaved);
            break;
            }
        case EScaling:
            {
            iState = EIdle;
            iController->NotifyCompletion( iStatus.Int(), KNullDesC );
            break;
            }
        default:
        case ERotating:
            {
            iState = EIdle;
            iController->NotifyCompletion( iStatus.Int(), KNullDesC );
            break;
            }
        }   
    }

TInt CImageConverterEngine::RunError(TInt /*aError*/)   
    {
    CancelProgress();
    // Our RunL does not contain any method calls that would leave, so this method
    // should never be called.
    return KErrNone;
    }

void CImageConverterEngine::GetEncoderImageTypesL( 
    RImageTypeDescriptionArray& aImageTypeArray )
    {
    CImageEncoder::GetImageTypesL( aImageTypeArray );
    }

CFrameInfoStrings* CImageConverterEngine::GetImageInfoL()
    {
    // notice that the information is returned from the original image
    //  hence the scaling of the bitmap does not affect the info until
    //  the bitmap is encoded back to image file and opened again
    if( iImageDecoder )
        {
        return( iImageDecoder->FrameInfoStringsL() );
        }
    else
        {
        return NULL ;
        }
    }

void CImageConverterEngine::Rotate( TDirection aDirection )
    {
    if (iState != EIdle || !iBitmap)
        {
        // Do nothing, if we are in the middle of processing a previous command
        return;
        }
        
    if( aDirection == EClockwise90 ) 
        {
        iRotator->Rotate( &iStatus, *iBitmap, 
            CBitmapRotator::ERotation90DegreesClockwise );  
        }
    else 
        { 
        // 270 clockwise == 90 anticlockwise
        iRotator->Rotate( &iStatus, *iBitmap, 
            CBitmapRotator::ERotation270DegreesClockwise ); 
        }

    iState = ERotating;
    SetActive();
    
    ShowProgress();
    }

TBool CImageConverterEngine::FitToScreen()
    {
    TBool ret = ETrue;
    TPixelsTwipsAndRotation sizeAndRotation;
    CEikonEnv::Static()->ScreenDevice()->GetDefaultScreenSizeAndRotation(sizeAndRotation);
    if (iBitmap->SizeInPixels().iHeight > sizeAndRotation.iPixelSize.iHeight-KFixImageHeight || 
        iBitmap->SizeInPixels().iWidth > sizeAndRotation.iPixelSize.iWidth-KFixImageWidth)
        {
        ret = EFalse;
        }
    return ret;
    }

void CImageConverterEngine::DoFitToScreen()
    {
    TPixelsTwipsAndRotation sizeAndRotation;
    CEikonEnv::Static()->ScreenDevice()->GetDefaultScreenSizeAndRotation(sizeAndRotation);

    TInt newWidth = (sizeAndRotation.iPixelSize.iWidth-KFixImageWidth);
    TInt newHeight = (sizeAndRotation.iPixelSize.iHeight-KFixImageHeight);
        
    // the maintain aspect ratio is by default set to true
    iScaler->Scale( &iStatus, *iBitmap, TSize(newWidth,newHeight) );

    iState = EScaling;
    SetActive();
    }

TBool CImageConverterEngine::CanDownScaleMore()
    {
    if (iScaleDownCounter > 2)
        {
        return EFalse;
        }
    else
        {
        return ETrue;
        }
    }

TInt CImageConverterEngine::Scale( TInt aPercent )
    {
    if (iState != EIdle || !iBitmap)
        {
        // Do nothing, if we are in the middle of processing a previous command         
        return KErrNotReady;
        }

    // Scaling down
    if (aPercent > 100)
        {
        if (!CanDownScaleMore())
            {
            return KErrCancel;
            }
        iScaleDownCounter++;

        TReal32 multiplier; 
    
        multiplier = aPercent/(TReal32)100.0;
    
        TInt newWidth = (TInt)(iBitmap->SizeInPixels().iWidth * multiplier);
        TInt newHeight = (TInt)(iBitmap->SizeInPixels().iHeight * multiplier);
            
        // the maintain aspect ratio is by default set to true
        iScaler->Scale( &iStatus, *iBitmap, TSize(newWidth,newHeight) );

        iState = EScaling;
        SetActive();
        ShowProgress();
        }
    // Scaling up gives original picture
    else
        {
        TRAPD(err,
                StartToDecodeL(iFilename));
        if (err)
            {
            return err;
            }
        }

    return KErrNone;
    }
    
CFbsBitmap* CImageConverterEngine::GetBitmap()
    {
    if (iState != EIdle)
        return NULL;
    else
        return iBitmap;
    }

void CImageConverterEngine::ShowProgress()
  {
  TRAPD(err, iNoteId = iGlobalNote->ShowNoteL(EAknGlobalConfirmationNote,_L("Completed ...")););
  }

void CImageConverterEngine::CancelProgress()
    {
   // TRAPD(err, iNoteId = iGlobalNote->ShowNoteL(EAknGlobalErrorNote,_L("ERROR ...")););
    }

//END
