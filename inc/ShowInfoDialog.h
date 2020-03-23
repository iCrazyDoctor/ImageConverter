/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    =========== CYBERTEC ============ 
 */

#ifndef __SHOWINFODIALOG_H__
#define __SHOWINFODIALOG_H__

// INCLUDES
#include <aknquerydialog.h>

// CLASS DECLARATION
class CShowInfoDialog : public CAknQueryDialog
    {
    public: // Constructors and destructor
        CShowInfoDialog( CFrameInfoStrings* aInfoStrings );
        
        virtual ~CShowInfoDialog();

        void PreLayoutDynInitL();

        void PostLayoutDynInitL();

        TBool OkToExitL(TInt aButtonId);

    public:
        void SizeChanged();
        void CalculatePositionAndSize();
        
    private: // from base classes
        TInt CountComponentControls() const;
        
        CCoeControl* ComponentControl(TInt aIndex) const;

        void Draw( const TRect& /*aRect*/ ) const;

    private: // internal methods
       void HandleResourceChange(TInt aType);
       void SetSizeAndPosition(const TSize &aSize);
       
    private: // data
        CFrameInfoStrings*      iInfoStrings;
        CFont*                  iFont;
    };

#endif //__SHOWINFODIALOG_H__
