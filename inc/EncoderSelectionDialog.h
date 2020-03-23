/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    ============= CYBERTEC =================
 */

#ifndef __ENCODERSELECTIONDIALOG_H__
#define __ENCODERSELECTIONDIALOG_H__

// INCLUDES
#include <aknview.h>
#include <eikdialg.h>
#include <akndialog.h>
#include <eiklbo.h>
#include <AknsDrawUtils.h>

// FORWARD DECLARATIONS
class CImageConverterEngine;
class CAknSingleStyleListBox;


// CLASS DECLARATION

/**
*  Dialog class
*  Test various controls.
*/
class CEncoderSelectionDialog : public CAknDialog, public MEikListBoxObserver
    {
    public: // Constructors and destructor
        /**
        * Constructor
        * @param aImageTypes Image types to choose from.
        * @param sSelectedIdx The selected index is stored here.
        */      
        CEncoderSelectionDialog( 
            RImageTypeDescriptionArray& aImageTypes, 
            TInt& aSelectedIdx );
        
        /**
        * Destructor
        */
        ~CEncoderSelectionDialog();

    public: // from base classes
        /**
        * From CAknDialog set parameters before and do cleanup after showing 
        * dialog.
        */
        void PreLayoutDynInitL();

        void PostLayoutDynInitL();

        /**
        * From CCoeControl. Pass key events to contained listbox.
        */
        TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,
            TEventCode aType);

        /**
        * From CAknDialog update member variables of CAknExEditorDialog.
        * @param aButtonId The ID of the button that was activated.
        * @return Should return ETrue if the dialog should exit,
        *    and EFalse if it should not
        */
        TBool OkToExitL(TInt aButtonId);

    private: // from base classes
        /**
        * From CoeControl,CountComponentControls.
        */
        TInt CountComponentControls() const;

        /**
        * From CCoeControl,ComponentControl.
        */
        CCoeControl* ComponentControl(TInt aIndex) const;
    
        void SizeChanged();
        void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
        void HandleResourceChange(TInt aType);

        static TInt PeriodicTimerCallBack(TAny* aAny);

    private: // data

        /*! @var iImageTypes the image types to choose from. */
        RImageTypeDescriptionArray& iImageTypes;

        /*! @var iSelectedIdx the selected index from image types */
        TInt& iSelectedIdx;
    
        /*! @var iListBox the control used to display the results */
        CAknSingleStyleListBox* iListBox;

        /*! @var iMessageList the list of messages to display */
        CDesCArrayFlat* iMessageList;
        
        CPeriodic*      iPeriodic;
        
    };

#endif //__ENCODERSELECTIONDIALOG_H__
