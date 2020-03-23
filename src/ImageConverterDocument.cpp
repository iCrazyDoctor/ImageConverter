/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    ============== CYBERTEC ===============
 */

#include "ImageConverterDocument.h"
#include "ImageConverterAppUi.h"

// ================= MEMBER FUNCTIONS =======================

// constructor
CImageConverterDocument::CImageConverterDocument(CEikApplication& aApp)
: CAknDocument(aApp)    
    {
    }

// destructor
CImageConverterDocument::~CImageConverterDocument()
    {
    }

// EPOC default constructor can leave.
void CImageConverterDocument::ConstructL()
    {
    }

// Two-phased constructor.
CImageConverterDocument* CImageConverterDocument::NewL(
        CEikApplication& aApp)     // CImageConverterApp reference
    {
    CImageConverterDocument* self = 
        new (ELeave) CImageConverterDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);

    return self;
    }
    
// ----------------------------------------------------
// CImageConverterDocument::CreateAppUiL()
// constructs CImageConverterAppUi
// ----------------------------------------------------
//
CEikAppUi* CImageConverterDocument::CreateAppUiL()
    {
    return new (ELeave) CImageConverterAppUi;
    }

// End of File  
