/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    ============ CYBERTEC ============
 */


// INCLUDE FILES
#include <eikstart.h>
#include "ImageConverterApp.h"
#include "ImageConverterDocument.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CImageConverterApp::AppDllUid()
// Returns application UID
// ---------------------------------------------------------
//
TUid CImageConverterApp::AppDllUid() const
    {
    return KUidImageConverter;
    }
 
// ---------------------------------------------------------
// CImageConverterApp::CreateDocumentL()
// Creates CImageConverterDocument object
// ---------------------------------------------------------
//
CApaDocument* CImageConverterApp::CreateDocumentL()
    {
    return CImageConverterDocument::NewL( *this );
    }

// ================= OTHER EXPORTED FUNCTIONS ==============
//
// ---------------------------------------------------------
// NewApplication() 
// Constructs CImageConverterApp
// Returns: created application object
// ---------------------------------------------------------
//
EXPORT_C CApaApplication* NewApplication()
    {
    return new CImageConverterApp;
    }

// -----------------------------------------------------------------------------
//  Entry point function for Symbian Apps, separate function for
//  S60 3rd Ed and 1st/2nd Ed
// -----------------------------------------------------------------------------
GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication( NewApplication );
    }

// End of File  
