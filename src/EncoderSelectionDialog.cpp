/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    =========== CYBERTEC ============
 */

// INCLUDE FILES
#include <aknlists.h>
#include <ImageConversion.h>

#include "ImageConverter.hrh"
#include "EncoderSelectionDialog.h"
#include "ImageConverterEngine.h"

const TInt KNumMessages = 10;
const TInt KMaxEncoderListLength = 80;
const TInt KPeriodicTimerInterval(100000);

// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code, that
// might leave.
//
CEncoderSelectionDialog::CEncoderSelectionDialog( 
    RImageTypeDescriptionArray& aImageTypes, 
    TInt& aSelectedIdx ) : 
    iImageTypes( aImageTypes ), iSelectedIdx( aSelectedIdx )
    {
    }

CEncoderSelectionDialog::~CEncoderSelectionDialog()
    {
    if(iPeriodic)
        {
        iPeriodic->Cancel();
        }
    delete iPeriodic;
    }

// ----------------------------------------------------------
// CEncoderSelectionDialog::PreLayoutDynInitL()
// Initializing the dialog dynamically
// ----------------------------------------------------------
//
void CEncoderSelectionDialog::PreLayoutDynInitL()
    {
    SetEditableL(ETrue);

    // Initialize component array
    InitComponentArrayL();
    
    // Create a control to display a list of messages
    iListBox = new (ELeave) CAknSingleStyleListBox;
    iListBox->SetContainerWindowL(*this);
    iListBox->ConstructL(this);
    iListBox->SetListBoxObserver(this);
    
    iListBox->CreateScrollBarFrameL(ETrue);
    iListBox->ScrollBarFrame()->
        SetScrollBarVisibilityL(CEikScrollBarFrame::EAuto, 
                                CEikScrollBarFrame::EAuto);

    Components().AppendLC(iListBox);
    CleanupStack::Pop(iListBox);
    
    
    // Create an array to hold the messages
    iMessageList = new (ELeave) CDesCArrayFlat(KNumMessages);

    // Give it to the control
    CTextListBoxModel* model = iListBox->Model();
    model->SetItemTextArray(iMessageList);
    model->SetOwnershipType(ELbmOwnsItemArray); // transfer ownership
    }

void CEncoderSelectionDialog::PostLayoutDynInitL()
    {
    // add the message to the list
    iListBox->SetRect( Rect() );    
 
    _LIT( KTab, "\t" );

    for( TInt i=0; i<iImageTypes.Count(); i++ ) 
        {
        TBuf<KMaxEncoderListLength> desc;

        desc.Append( KTab );
        desc.Append( iImageTypes[i]->Description() );
    
        iMessageList->AppendL( desc );
        }

    // tell the control about the change
    iListBox->HandleItemAdditionL();
    }

// ----------------------------------------------------------
// CEncoderSelectionDialog::OkToExitL()
// This function ALWAYS returns ETrue
// ----------------------------------------------------------
//
TBool CEncoderSelectionDialog::OkToExitL(TInt /*aButtonId*/)
    {
    iSelectedIdx = iListBox->CurrentItemIndex();
    return ETrue;
    }

TInt CEncoderSelectionDialog::CountComponentControls() const
    {
    TInt count = 0;
    if( iListBox )
        count = 1;

    return count;
    }

CCoeControl* CEncoderSelectionDialog::ComponentControl(TInt /*aIndex*/) const
    {
    return iListBox;
    }

TKeyResponse CEncoderSelectionDialog::OfferKeyEventL(
    const TKeyEvent& aKeyEvent,TEventCode aType)
    {
    if( iListBox )
        {
        if (aKeyEvent.iCode == EKeyOK)
            {
            iSelectedIdx = iListBox->CurrentItemIndex();
            TryExitL(1);
       		return( EKeyWasConsumed );
            }
        else
            {
            return( iListBox->OfferKeyEventL(aKeyEvent, aType) );
            }
        }
    else
       return( EKeyWasNotConsumed );
    }

void CEncoderSelectionDialog::SizeChanged()
    {
    iListBox->SetRect(Rect());
    }

void CEncoderSelectionDialog::HandleResourceChange(TInt aType)
    {
    CAknDialog::HandleResourceChange(aType);
    if ( aType==KEikDynamicLayoutVariantSwitch )
        {
        TRect rect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane,rect);

        SetRect(rect);
        }     
    }

void CEncoderSelectionDialog::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
    {
    if (aEventType==EEventEnterKeyPressed || aEventType==EEventItemDoubleClicked)
        {
        iSelectedIdx = iListBox->CurrentItemIndex();
        
        // NOTE: Need to cut method calling flow
        // Wait as asynchronoysly 10ms and call TryExitL() after that  
        if (!iPeriodic)
            {
            iPeriodic = CPeriodic::NewL(CActive::EPriorityIdle);
            iPeriodic->Start(KPeriodicTimerInterval, KPeriodicTimerInterval,TCallBack(PeriodicTimerCallBack, this));
            }
        }
    }

TInt CEncoderSelectionDialog::PeriodicTimerCallBack(TAny* aAny)
    {
    CEncoderSelectionDialog* self = static_cast<CEncoderSelectionDialog*>( aAny );
    self->iPeriodic->Cancel();
    TRAPD(err,
            self->TryExitL(1);
    );
    return KErrNone;
    }

// END OF FILE
