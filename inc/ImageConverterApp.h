/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    ========== CYBERTEC ===========
 */

#ifndef IMAGECONVERTERAPP_H
#define IMAGECONVERTERAPP_H

// INCLUDES
#include <aknapp.h>

// CONSTANTS
// UID of the application
const TUid KUidImageConverter = { 0xE01F5467 };

// CLASS DECLARATION

/**
* CImageConverterApp application class.
* Provides factory to create concrete document object.
* 
*/
class CImageConverterApp : public CAknApplication
    {
    
    public: // Functions from base classes
    private:

        /**
        * From CApaApplication, 
        * creates CImageConverterDocument document object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
        
        /**
        * From CApaApplication,
        * returns application's UID (KUidImageConverter).
        * @return The value of KUidImageConverter.
        */
        TUid AppDllUid() const;
    };

#endif

// End of File

