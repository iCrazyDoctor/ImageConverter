/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    =========== CYBERTEC =============
 */

#ifndef IMAGECONVERTERAPPUI_H
#define IMAGECONVERTERAPPUI_H

// INCLUDES
#include <eikapp.h>
#include <eikdoc.h>
#include <e32std.h>
#include <coeccntx.h>
#include <aknappui.h>
#include <aknutils.h>

#include "ImageConverterEngine.h"
#include "ImageConverterContainer.h"

const TUid KUidHelpFile = {0x2000e192};  // From help

// FORWARD DECLARATIONS
class CImageConverterContainer;


// CLASS DECLARATION

class CAknListQueryDialog;
class CShowInfoDialog;
class CFrameInfoStrings;

/**
* Application UI class.
* Provides support for the following features:
* - EIKON control architecture
* 
*/
class CImageConverterAppUi : public CAknAppUi, MConverterController
    {
    public: // // Constructors and destructor

        /**
        * EPOC default constructor.
        */      
        void ConstructL();

        /**
        * Destructor.
        */      
        ~CImageConverterAppUi();
        
    public: // New functions
        void ShowMessage(const TDesC& aMsg) const;
        void DoEvent(EPointerEvents aEvent);
        void ReadImage(TInt aDirection);
        TBool IsEngineBusy();
        TInt ImageIndex();
        TInt ImageCount();        
        void ImageName(TFileName& aFilename);
        void PlainImageName(TFileName& aFilename);
        /**
        * From CEikAppUi, takes care of command handling.
        * @param aCommand command to be handled
        */
        void HandleCommandL(TInt aCommand);
        
    public: // Functions from base classes
        // from MConverterController
        void NotifyCompletion( TInt aErr, const TDesC& aMsg  );
        TBool IsAnimating();
        TBool IsOptionsButtonOnTop();
        void SearchOptionsButtonPosition();
        TState EngineState();


        
    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

    private:

        /**
        * From CEikAppUi, handles key events.
        * @param aKeyEvent Event to handled.
        * @param aType Type of the key event. 
        * @return Response code (EKeyWasConsumed, EKeyWasNotConsumed). 
        */
        virtual TKeyResponse HandleKeyEventL(
            const TKeyEvent& aKeyEvent,TEventCode aType);

        CArrayFix<TCoeHelpContext>* HelpContextL() const;
        
    private: // internal methods
        /**
        * Handles Open menu command
        */
        void HandleOpenL();

        /**
        * Handles Save as menu command
        */
        void HandleSaveAsL();
        
        /**
        * Handles Info menu command
        */
        void HandleInfoL();
        
        /**
        * Handles Rotate menu command
        */
        void HandleRotate();
        
        /**
        * Handles Scale menu command
        */
        void HandleScale();
        
        /**
        * Shows an error message.
        */
        void ShowMessageL(const TDesC& aMsg) const;
        
        void ReadImageDirectoryL();
        
        void HandleResourceChangeL( TInt aType );

        
    private: //Data
        /*! @var iImageLoaded true if an image has been opened/decoded
        and is to be shown. 
        Used to enable/disable save as and info menu commands */
        TBool                       iImageLoaded;

        /*! @var iAppContainer A reference to the converter container
        */
        CImageConverterContainer*   iAppContainer; 

        /*! @var iConverter the actuall calls to ICL are done from this engine 
        */
        CImageConverterEngine*      iConverter;
        RArray<TFileName>           iFiles;
        TInt                        iOpenFileIndex;
        TBool                       iShiftDown;
        
        TBool                       iOptionButtonOnTop;
        
        CAknListQueryDialog*        iSaveAs;
        CShowInfoDialog*            iInfoDialog;
        CFrameInfoStrings*          iInfoStrings;
        CFont*               iFont;
    };

#endif

// End of File
