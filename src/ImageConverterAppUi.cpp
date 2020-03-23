/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    ============ CYBERTEC =================
 */

// INCLUDE FILES
#include "ImageConverterApp.h"
#include "ImageConverterAppUi.h"
#include "ImageConverterContainer.h" 
#include "ImageConverterEngine.h"
#include "imageconverter.hrh"
#include "encoderselectiondialog.h"
#include "showinfodialog.h"
#include <ImageConverter.rsg>

#include <aknmessagequerydialog.h>
#include <hlplch.h>
#include <avkon.hrh>
#include <avkon.rsg>
#include <caknfileselectiondialog.h>
#include <akncommondialogs.h>
#include <imageconversion.h>
#include <eikmenup.h>
#include <aknglobalnote.h>
#include <pathinfo.h>
#include <aknquerydialog.h>

// TEST INC
#include <eiklabel.h>  // for example label control
#include <aknutils.h>  // for scalable UI
#include <eikdef.h>
#include <eikenv.h>
#include <e32cmn.h>

const TInt KScaleUpFactor = 150;    // Scale up by 50%
const TInt KScaleDownFactor = 50;   // Scale down by 50%
const TInt KMaxInfoMsgLength = 70;
const TInt KMaxInfoDescriptorLength = 80;

void CleanupRArray( TAny* object )
    {
    ((RImageTypeDescriptionArray*)object)->ResetAndDestroy();
    }

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CImageConverterAppUi::ConstructL()
// 
// ----------------------------------------------------------
//
void CImageConverterAppUi::ConstructL()
    {
    #ifdef __TUI_SUPPORTED__
    BaseConstructL(EAknEnableSkin | EAknTouchCompatible);
    #else
    BaseConstructL(EAknEnableSkin);
    #endif
    
    SearchOptionsButtonPosition();

    iImageLoaded = EFalse;
    iConverter = CImageConverterEngine::NewL( this );
    
    iAppContainer = new (ELeave) CImageConverterContainer;
    iAppContainer->SetMopParent( this );
    iAppContainer->ConstructL( ClientRect() );
    AddToStackL( iAppContainer );
    
    ReadImageDirectoryL();
    ReadImage(0);
    }

// ----------------------------------------------------
// CImageConverterAppUi::~CImageConverterAppUi()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CImageConverterAppUi::~CImageConverterAppUi()
    {
    if (iAppContainer)
        {
        RemoveFromStack( iAppContainer );
        delete iAppContainer;
        }
 
    if (iConverter)
        {
        iConverter->Cancel();
        delete iConverter;
        }

    delete iSaveAs;
    delete iInfoDialog;
    delete iInfoStrings;
    iFiles.Close();
    }

// ----------------------------------------------------
// CImageConverterAppUi::DynInitMenuPaneL(
//    TInt aResourceId,CEikMenuPane* aMenuPane)
// Handle dynamic menu initialization
// ----------------------------------------------------
//
void CImageConverterAppUi::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
    {
    if(aResourceId == R_IMAGECONVERTER_MENU )
        {
        if( !iImageLoaded ) 
            {
            // if there is no image loaded do not show these menu items
            aMenuPane->SetItemDimmed( EImageConverterCmdSaveAs, true);
            aMenuPane->SetItemDimmed( EImageConverterCmdInfo, true);
            aMenuPane->SetItemDimmed( EImageConverterCmdRotate, true);
           // aMenuPane->SetItemDimmed( EImageConverterCmdZoomOut, true );
           // aMenuPane->SetItemDimmed( EImageConverterCmdZoomIn, true );
            }
        else 
            {
            aMenuPane->SetItemDimmed( EImageConverterCmdSaveAs, false );
            aMenuPane->SetItemDimmed( EImageConverterCmdInfo, false );
            aMenuPane->SetItemDimmed( EImageConverterCmdRotate, false );
          //  aMenuPane->SetItemDimmed( EImageConverterCmdZoomOut, false );
            if (iConverter && iConverter->CanDownScaleMore())
                {
              //  aMenuPane->SetItemDimmed( EImageConverterCmdZoomIn, false );
                }
            else
                {
              //  aMenuPane->SetItemDimmed( EImageConverterCmdZoomIn, true );
            }
            }
            }
            }

void CImageConverterAppUi::DoEvent(EPointerEvents aEvent)
    {
    switch (aEvent)
        {
        case EZoomIn:
            {
            iAppContainer->SetTitleText(ETitleZoomIn,EFalse);
            if (iConverter->Scale( KScaleUpFactor )==KErrNone)
                {
                iImageLoaded = false;
                }
            break;
            }
        case EZoomOut:
            {
            iAppContainer->ResetTitleText(EFalse);
            if (iConverter->Scale( KScaleDownFactor )==KErrNone)
                {
                iImageLoaded = false;
                }
            break;
            }
        case ENextImage:
            {
            ReadImage(1);
            break;
            }
        case EPrevImage:
            {
            ReadImage(-1);
            break;
            }
        case ERotateRight:
            {
            iImageLoaded = false;
            iAppContainer->SetTitleText(ETitleRotateRight,EFalse);
            iConverter->Rotate( CImageConverterEngine::EClockwise90 );
            break;
            }
        case ERotateLeft:
            {
            iImageLoaded = false;
            iAppContainer->SetTitleText(ETitleRotateLeft,EFalse);
            iConverter->Rotate( CImageConverterEngine::EAntiClockwise90 );
            break;
            }
        default:
            {
            break;
            }
        };
    }

TState CImageConverterAppUi::EngineState()
    {
    return iConverter->EngineState();
    }

// ----------------------------------------------------
// CImageConverterAppUi::HandleKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
// takes care of key event handling
// ----------------------------------------------------
//
TKeyResponse CImageConverterAppUi::HandleKeyEventL(
    const TKeyEvent& aKeyEvent,TEventCode aType)
    {
    // if there is an image loaded, pressing arrow keys causes
    // scale and rotate
    if( iImageLoaded && (aType == EEventKey) ) 
        {
        if( aKeyEvent.iCode == EKeyLeftArrow ) 
            {
            iAppContainer->ChangePrevPictureL();
            return EKeyWasConsumed;
            }
        else if( aKeyEvent.iCode == EKeyRightArrow ) 
            {
            iAppContainer->ChangeNextPictureL();
            return EKeyWasConsumed;
            }
        else if( aKeyEvent.iCode == EKeyUpArrow ) 
            {
            iAppContainer->SetTitleText(ETitleZoomIn,ETrue);
            if (iConverter->Scale( KScaleUpFactor )==KErrNone) // scale up
                {
                iImageLoaded = false;
                }
            return EKeyWasConsumed;
            }
        else if( aKeyEvent.iCode == EKeyDownArrow ) 
            {
            iAppContainer->ResetTitleText(ETrue);
            if (iConverter->Scale( KScaleDownFactor )==KErrNone) // scale up
                {
                iImageLoaded = false;
                }
            return EKeyWasConsumed;
            }
        }
    else if ( iImageLoaded && aType == EEventKeyDown)
        {
        if (aKeyEvent.iScanCode == '1' || aKeyEvent.iScanCode == '*')
            {
            iAppContainer->SetTitleText(ETitleRotateLeft,ETrue);
            iImageLoaded = false;
            iConverter->Rotate( CImageConverterEngine::EAntiClockwise90 );
            return EKeyWasConsumed;
            }
        else if (aKeyEvent.iScanCode == '3' || aKeyEvent.iScanCode == EStdKeyHash)
            {
            iAppContainer->SetTitleText(ETitleRotateRight,ETrue);
            iImageLoaded = false;
            iConverter->Rotate( CImageConverterEngine::EClockwise90 );
            return EKeyWasConsumed;
            }
        }
    return EKeyWasNotConsumed;
    }

// ----------------------------------------------------
// CImageConverterAppUi::HandleCommandL(TInt aCommand)
// takes care of command handling
// ----------------------------------------------------
//
void CImageConverterAppUi::HandleCommandL(TInt aCommand)
    {
    switch ( aCommand )
        {	
       case EAknSoftkeyExit:
       case EEikCmdExit:
            {
            Exit();
            break;
            }
        case EImageConverterCmdOpen:
            {
            HandleOpenL();
            break;
            }
        case EImageConverterCmdSaveAs:
            {
            HandleSaveAsL();
            break;
            }
        case EImageConverterCmdInfo:
            {
         HandleInfoL();
            break;
            }
        case EImageConverterCmdOrientation:
            {
            if(Orientation() == CAknAppUiBase::EAppUiOrientationLandscape)
                {
                SetOrientationL(CAknAppUiBase::EAppUiOrientationPortrait);
                iAppContainer->SetPictureScreenCenter(ETrue);
                }
            else
                {
                SetOrientationL(CAknAppUiBase::EAppUiOrientationLandscape);
                iAppContainer->SetPictureScreenCenter(ETrue);
                }
            break;
            }
        case EImageConverterCmdRotateLeft:
            {
            iAppContainer->SetTitleText(ETitleRotateLeft,ETrue);
            iImageLoaded = false;
            iConverter->Rotate( CImageConverterEngine::EAntiClockwise90 );
            break;
            }
        case EImageConverterCmdRotateRight:
            {
            iAppContainer->SetTitleText(ETitleRotateRight,ETrue);
            iImageLoaded = false;
            iConverter->Rotate( CImageConverterEngine::EClockwise90 );
            break;
            }
        case EImageConverterCmdZoomIn:
            {
            iAppContainer->SetTitleText(ETitleZoomIn,ETrue);
            if (iConverter->Scale( KScaleUpFactor )==KErrNone) // scale down
                {
                iImageLoaded = false;
                }
            break;
            }
        case EImageConverterCmdZoomOut:
            {
            iAppContainer->ResetTitleText(ETrue);
            if (iConverter->Scale( KScaleDownFactor )==KErrNone) // scale down
                {
                iImageLoaded = false;
                }
            break;
            }
        case EImageConverterCmdNext:
            {
            iAppContainer->ChangeNextPictureL();
            break;
            }
        case EImageConverterCmdPrev:
            {
            iAppContainer->ChangePrevPictureL();
            break;
            }
        case EImageConverterCmdHelp:
            {
            
   // HELP MSG, RSC DIALOG
   CAknMessageQueryDialog* dlg = new (ELeave)CAknMessageQueryDialog();
                        dlg->PrepareLC (R_HELP_QUERY_DIALOG );
                        HBufC* title = iEikonEnv->AllocReadResourceLC ( R_HELP_DIALOG_TITLE);
                        dlg->QueryHeading()->SetTextL (*title );
                        CleanupStack::PopAndDestroy ( ); //title
                        HBufC* msg = iEikonEnv->AllocReadResourceLC (R_HELP_DIALOG_TEXT );
                        dlg->SetMessageTextL (*msg);
                      //  _LIT( "SYMBIANC, HELP");
                        CleanupStack::PopAndDestroy ( ); //msg
                        dlg->RunLD ( );
            break;
            }
        case EImageConverterCmdAbout:
            {    
            // ABOUT MSG
            CAknMessageQueryDialog* dlg = new (ELeave)CAknMessageQueryDialog();
            dlg->PrepareLC (R_ABOUT_QUERY_DIALOG );
            HBufC* title = iEikonEnv->AllocReadResourceLC ( R_ABOUT_DIALOG_TITLE);
            dlg->QueryHeading()->SetTextL (*title );
            CleanupStack::PopAndDestroy ( ); //title
            HBufC* msg = iEikonEnv->AllocReadResourceLC (R_ABOUT_DIALOG_TEXT );
            dlg->SetMessageTextL (*msg);
            CleanupStack::PopAndDestroy ( ); //msg
            dlg->RunLD ( );
            break;
            }
        default:
            break;      
        }
    }

CArrayFix<TCoeHelpContext>* CImageConverterAppUi::HelpContextL ( ) const
    {
    CArrayFixFlat<TCoeHelpContext>* array = new(ELeave)CArrayFixFlat<TCoeHelpContext>(1);
    CleanupStack::PushL (array );
    _LIT(KGeneral_Information, "General Information about the application"); 
    array->AppendL (TCoeHelpContext (KUidHelpFile, KGeneral_Information ) );
    CleanupStack::Pop (array );
    return array;
    }

void CImageConverterAppUi::ReadImageDirectoryL()
    {
    RFs& rfs = iEikonEnv->FsSession();
    CDir* fileList = NULL;
    
    iFiles.Reset();

    TFileName path = PathInfo::MemoryCardRootPath();        // CYBERTEC, DISK E
    path.Append(PathInfo::ImagesPath());
    
    User::LeaveIfError(rfs.GetDir(path,KEntryAttNormal,ESortByName,fileList));
    
    for( TInt i = 0; i < fileList->Count(); i++ )
        {
        // Read only files to array
        if (!fileList->operator[](i).IsDir())
            {
            TFileName filename;
            filename.Append(path);
            filename.Append(fileList->operator[](i).iName);
            filename.LowerCase();
            // We support only JPG, BMP,  GIF
            if ( filename.Find(_L(".jpg"))!=KErrNotFound || filename.Find(_L(".mbm"))!=KErrNotFound || filename.Find(_L(".bmp"))!=KErrNotFound || filename.Find(_L(".png"))!=KErrNotFound || filename.Find(_L(".gif"))!=KErrNotFound )
                {
                iFiles.Append(filename);
                }
            }
        }
    
    delete fileList;
    }

TInt CImageConverterAppUi::ImageIndex()
    {
    return iOpenFileIndex;
    }

TInt CImageConverterAppUi::ImageCount()
    {
    return iFiles.Count();
    }

void CImageConverterAppUi::ImageName(TFileName& aFilename)
    {
    aFilename.Copy(iFiles[iOpenFileIndex]);
    if (aFilename.Find(_L("\\")) > -1)
        {
        aFilename.Delete(0,aFilename.LocateReverse('\\')+1);
        }
    }

void CImageConverterAppUi::PlainImageName(TFileName& aFilename)
    {
    aFilename.Copy(iFiles[iOpenFileIndex]);
    if (aFilename.Find(_L("\\")) > -1)
        {
        aFilename.Delete(0,aFilename.LocateReverse('\\')+1);
        }
    if (aFilename.Find(_L(".")) > -1)
        {
        aFilename.SetLength(aFilename.Find(_L(".")));
        }
    }

void CImageConverterAppUi::ReadImage(TInt aDirection)
    {
    TFileName fileToOpen;
    
    if (iFiles.Count()==0)
        {
        return; // No images found
        }
    
    iOpenFileIndex = iOpenFileIndex + aDirection;
    if (iOpenFileIndex < 0)
        {
        iOpenFileIndex = iFiles.Count()-1;
        }
    else if(iOpenFileIndex > iFiles.Count()-1)
        {
        iOpenFileIndex = 0;
        }
    
    fileToOpen.Append(iFiles[iOpenFileIndex]);
    
    iImageLoaded = false;
    iAppContainer->SetBitmap(NULL);
    
    // request the actuall open/decode
    // asynchronous, the result is reported via callback
    // NotifyCompletion
    TRAPD( err, iConverter->StartToDecodeL(fileToOpen) );

    // report errors in issuing the asynchronous call
    if( err != KErrNone ) 
        {
        _LIT( KMsg, "Cannot decode: %d" );
        TBuf<KMaxInfoMsgLength> buf;
        buf.Format( KMsg, err );
        ShowMessage(buf);
        }
    }

// ----------------------------------------------------
// CImageConverterAppUi::HandleOpenL( )
// Handle open command
// ----------------------------------------------------
//
void CImageConverterAppUi::HandleOpenL( )
    { 
    TFileName fileToOpen;
   _LIT( KFileSelectionTitle, "Select image to open" );
    // show fileopen dialog from akncommondialogs
    if( !AknCommonDialogs::RunSelectDlgLD(fileToOpen, R_MEMORY_SELECTION_DIALOG, KFileSelectionTitle) ) 
        {
        return;
        }

    iImageLoaded = false;
    iAppContainer->SetBitmap(NULL); 

    // request the actuall open/decode
    // asynchronous, the result is reported via callback
    // NotifyCompletion
    TRAPD( err, iConverter->StartToDecodeL(fileToOpen) );

    // report errors in issuing the asynchronous call
    if( err != KErrNone ) 
        {
        _LIT( KMsg, "Invalid file selected: %d" );
        TBuf<KMaxInfoMsgLength> buf;
        buf.Format( KMsg, err );
        ShowMessage(buf);
        }
    else
        {
        iFiles.Append(fileToOpen);
        iOpenFileIndex = iFiles.Count()-1;
        }
    }

// ----------------------------------------------------
// CImageConverterAppUi::HandleSaveAsL()
// Handle save command
// ----------------------------------------------------
//
void CImageConverterAppUi::HandleSaveAsL()
    {
    TInt selectedIdx = 0;

    // get encoder types from engine
    RImageTypeDescriptionArray imageTypes; 
    iConverter->GetEncoderImageTypesL( imageTypes); 
    
    // Create a custom cleanup item and specify CleanupRArray to be the
    // method that is called once we call CleanupStack::PopAndDestroy() at the
    // end of this method.
    CleanupStack::PushL( TCleanupItem(CleanupRArray, &imageTypes) );

    // Create path for filename
    TFileName path = PathInfo::MemoryCardRootPath(); // DISK E
    path.Append(PathInfo::ImagesPath());
    // Get filename of current image for save image dialog
    TFileName fileToSave;
    PlainImageName(fileToSave);

    // Create image format array for the dialog
    CDesCArrayFlat* array = new (ELeave) CDesCArrayFlat(imageTypes.Count());
    CleanupStack::PushL(array);
    for( TInt i=0; i<imageTypes.Count(); i++ ) 
        {
        TBuf<KMaxInfoDescriptorLength> desc;
        desc.Append( imageTypes[i]->Description() );
        array->AppendL( desc );
        }
    
    // Show dialog
    TRAPD(err, 
        iSaveAs = new(ELeave)CAknListQueryDialog(&selectedIdx);
        iSaveAs->PrepareLC(R_SELECTION_QUERY);
        iSaveAs->SetItemTextArray(array); 
        iSaveAs->SetOwnershipType(ELbmDoesNotOwnItemArray); 
        if(iSaveAs->RunLD())
            {
            // Ask filename
            CAknTextQueryDialog* dlg = CAknTextQueryDialog::NewL(fileToSave,CAknQueryDialog::ENoTone);
            dlg->PrepareLC(R_QUERY_DIALOG);
            dlg->SetPromptL(_L("Save as"));
            if(dlg->RunLD())
                {
                fileToSave.Append(_L("."));
                fileToSave.Append(imageTypes[selectedIdx]->Description());
    
                path.Append(fileToSave);
                iConverter->StartToEncodeL(path, 
                    imageTypes[selectedIdx]->ImageType(),
                    imageTypes[selectedIdx]->SubType());
                }
            }
        );
    iSaveAs = NULL;
    CleanupStack::PopAndDestroy(); // array

    if (err)
        {
        User::Leave(err);
        }

    // This will lead to a call to CleanupRArray, our custom cleanup method.
    CleanupStack::PopAndDestroy(&imageTypes);
    }

// ----------------------------------------------------
// CImageConverterAppUi::HandleInfoL()
// Handle info command
// ----------------------------------------------------
//
void CImageConverterAppUi::HandleInfoL()
    {
    // Get the info strings
    delete iInfoStrings;
    iInfoStrings = NULL;
    iInfoStrings = iConverter->GetImageInfoL();

    if (iInfoStrings)
        {
        // Show the info strings in a separate dialog
    iInfoDialog = new(ELeave) CShowInfoDialog( iInfoStrings );      // ORG
     iInfoDialog->ExecuteLD( R_IMAGECONVERTER_SHOWINFO_DIALOG );
     iInfoDialog = NULL; 
        }
    }

TBool CImageConverterAppUi::IsAnimating()
    {
    return iAppContainer->IsAnimating();
    }

TBool CImageConverterAppUi::IsEngineBusy()
    {
    return iConverter->IsActive();
    }

// ----------------------------------------------------
// CImageConverterAppUi::NotifyCompletion( TInt aErr, 
//                const TDesC& aMsg )
// Handle completion notifications
// ----------------------------------------------------
//
void CImageConverterAppUi::NotifyCompletion( TInt aErr, const TDesC& aMsg ) 
    {
    if( aErr == KErrNone ) 
        {
        // we have succesfully decoded an image, now make the view
        // to display it
        iAppContainer->SetBitmap(iConverter->GetBitmap());
        iImageLoaded = true;
        }
    else 
        {
        iAppContainer->SetBitmap(NULL);
        iImageLoaded = false;
        }

        iAppContainer->DrawToBackBuffer();
        iAppContainer->DrawNow();

    if (aErr && aErr != KErrNotReady)
        {
        ShowMessage(aMsg);
        }
    }

// ----------------------------------------------------
// CImageConverterAppUi::ShowMessage(const TDesC& aMsg) const
// Show a message to the user
// ----------------------------------------------------
//
void CImageConverterAppUi::ShowMessage(const TDesC& aMsg) const
    {
    if (aMsg.Length())  // If there's something to show, otherwise do nothing
        {
        // No need to react if this leaves, just trap
        TRAPD(err, ShowMessageL(aMsg));
        }
    }
// ----------------------------------------------------
// CImageConverterAppUi::ShowMessageL(const TDesC& aMsg) const
// Show a message to the user
// ----------------------------------------------------
//
void CImageConverterAppUi::ShowMessageL(const TDesC& aMsg) const
    {
    CAknGlobalNote* note = CAknGlobalNote::NewLC();
    note->ShowNoteL(EAknGlobalErrorNote, aMsg);
    CleanupStack::PopAndDestroy(note);
    }

TBool CImageConverterAppUi::IsOptionsButtonOnTop()
    {
    return iOptionButtonOnTop;
    }

void CImageConverterAppUi::SearchOptionsButtonPosition()
    {
    iOptionButtonOnTop = EFalse;
    CEikButtonGroupContainer* cba = Cba();
    if(!cba)
        {
        return;
        }
    CCoeControl* options = cba->ControlOrNull(EAknSoftkeyOptions);
    CCoeControl* exit = cba->ControlOrNull(EAknSoftkeyExit);
    if( options && exit)
        {
        if( options->Position().iY < exit->Position().iY )
            {
            iOptionButtonOnTop = ETrue;
            }
        }
    }

void CImageConverterAppUi::HandleResourceChangeL( TInt aType )
    {
    CAknAppUi::HandleResourceChangeL( aType );
       
    if ( aType==KEikDynamicLayoutVariantSwitch )
        {
        SearchOptionsButtonPosition();

        TRect rect;
        AknLayoutUtils::LayoutMetricsRect(
        AknLayoutUtils::EMainPane,rect);
        if (iSaveAs)
            {
            iSaveAs->SetRect(rect);
            }
        if (iInfoDialog)
            {
            iInfoDialog->SetRect(rect);
            }
        }           
    }

// End of File

