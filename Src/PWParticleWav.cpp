/**
 * Copyright L. Spiro 2024
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: Let's convert and modify WAV files!
 */


#include "PWParticleWav.h"
#include "Files/PWStdFile.h"
#include "Utilities/PWUtilities.h"
#include "Wav/PWWavFile.h"


int wmain( int _iArgC, wchar_t const * _wcpArgV[] ) {
    --_iArgC;
    std::u16string sThisDir = pw::CFileBase::GetFilePath( std::u16string( reinterpret_cast<const char16_t *>((*_wcpArgV++)) ) );

#define PW_ERRORT( TXT, CODE )					pw::PrintError( reinterpret_cast<const char16_t *>(TXT), (pw::CODE) );					\
												if ( oOptions.bPause ) { ::system( "pause" ); }	                                        \
												return int( pw::CODE )
#define PW_ERROR( CODE )						PW_ERRORT( nullptr, CODE )

#define PW_CHECK( TOTAL, NAME )                _iArgC >= (TOTAL) && ::_wcsicmp( &(*_wcpArgV)[1], L ## #NAME ) == 0
#define PW_ADV( VAL )                          _iArgC -= (VAL); _wcpArgV += (VAL); continue


    pw::PW_OPTIONS oOptions;

    while ( _iArgC ) {
        if ( (*_wcpArgV)[0] == L'-' ) {
            if ( PW_CHECK( 2, file ) ) {
                try {
                    oOptions.vInputs.push_back( reinterpret_cast<const char16_t *>((_wcpArgV[1])) );
                }
                catch ( ... ) { PW_ERROR( PW_E_OUTOFMEMORY ); }
                PW_ADV( 2 );
            }
            if ( PW_CHECK( 2, dir ) ) {
                try {
                    pw::CFileBase::FindFiles( reinterpret_cast<const char16_t *>((_wcpArgV[1])), u"*.wav", false, oOptions.vInputs );
                }
                catch ( ... ) { PW_ERROR( PW_E_OUTOFMEMORY ); }
                PW_ADV( 2 );
            }
            if ( PW_CHECK( 2, outfile ) || PW_CHECK( 2, out_file ) ) {
                // Make sure the output list has at least 1 fewer entries than the input list.
				if ( oOptions.vOutputs.size() >= oOptions.vInputs.size() ) {
					// Too many outputs have already been submitted.
					PW_ERRORT( u"Too many outputs for the given number of inputs.\r\n", PW_E_INVALIDCALL );
				}
                try {
                    std::u16string sThis = reinterpret_cast<const char16_t *>((_wcpArgV[1]));
                    // Get the provided extension.
                    std::u16string sExt = pw::CFileBase::GetFileExtension( sThis );
                    std::u16string sPath = pw::CFileBase::GetFilePath( sThis );
                    std::u16string sName = pw::CFileBase::GetFileName( sThis );
                    if ( !sExt.size() ) {
                        sPath = sThis;
                        sName.clear();
                        if ( sPath.size() && sPath[sPath.size()-1] != u'\\' && sPath[sPath.size()-1] != u'/' ) {
                            sPath.push_back( u'\\' );
                        }
                    }
                    if ( sName.size() == 1 && sName.c_str()[0] == u'*' || sName.c_str()[0] == u'.' ) { sName.clear(); }
                    if ( !sPath.size() ) { sPath = sThisDir; }

                    size_t sTotal = sExt.size() ? oOptions.vInputs.size() - 1 : oOptions.vInputs.size();
                    for ( size_t J = oOptions.vOutputs.size(); oOptions.vOutputs.size() < sTotal; ++J ) {
                        std::u16string sSrc = sPath;
                        sSrc += pw::CFileBase::NoExtension( pw::CFileBase::GetFileName( oOptions.vInputs[J] ) );
                        sSrc += u".";
                        // If the string is empty, keep the extension of the inputs.
                        if ( !sExt.size() ) {
                            sSrc += pw::CFileBase::GetFileExtension( oOptions.vInputs[J] );
                        }
                        else {
                            sSrc += sExt;
                        }

                        oOptions.vOutputs.push_back( sSrc );
                    }
                    if ( oOptions.vOutputs.size() < oOptions.vInputs.size() ) {
                    
                        std::u16string sSrc = sPath;
                        if ( !sName.size() ) {
                            sSrc += pw::CFileBase::NoExtension( pw::CFileBase::GetFileName( oOptions.vInputs[oOptions.vOutputs.size()] ) );
                        }
                        else {
                            sSrc += pw::CFileBase::NoExtension( pw::CFileBase::GetFileName( sThis ) );
                        }
                        sSrc += u".";
                        // If the string is empty, keep the extension of the inputs.
                        if ( !sExt.size() ) {
                            sSrc += pw::CFileBase::GetFileExtension( oOptions.vInputs[oOptions.vOutputs.size()] );
                        }
                        else {
                            sSrc += sExt;
                        }
                        oOptions.vOutputs.push_back( sSrc );
                    }
                }
                catch ( ... ) { PW_ERROR( PW_E_OUTOFMEMORY ); }
                PW_ADV( 2 );
            }
        }
        else {
            PW_ERRORT( std::format( L"Invalid command: \"{}\".",
                _wcpArgV[0] ).c_str(), PW_E_INVALIDCALL );
        }


        --_iArgC;
        ++_wcpArgV;
    }



    if ( oOptions.vOutputs.size() != oOptions.vInputs.size() ) {
        PW_ERRORT( std::format( L"There must be the same number of inputs and outputs: \"{}\" inputs -> \"{}\" outputs.",
            oOptions.vInputs.size(), oOptions.vOutputs.size() ).c_str(), PW_E_INVALIDCALL );
    }

    std::vector<std::u16string>::size_type sSuccess = 0;
    for ( std::vector<std::u16string>::size_type I = 0; I < oOptions.vInputs.size(); ++I ) {
        pw::CWavFile wfWav;
        if ( !wfWav.Open( oOptions.vInputs[I].c_str() ) ) {
            std::wcout << std::format( L"Failed to load file: \"{}\"", reinterpret_cast<const wchar_t *>(oOptions.vInputs[I].c_str()) ) << std::endl;
            continue;
        }

        pw::CWavFile::lwaudio aSamples;
        if ( !wfWav.GetAllSamples( aSamples ) ) {
            std::wcout << std::format( L"Failed to get all samples from file: \"{}\"", reinterpret_cast<const wchar_t *>(oOptions.vInputs[I].c_str()) ) << std::endl;
            continue;
        }

        if ( !wfWav.SaveAsPcm( oOptions.vOutputs[I].c_str(), aSamples ) ) {
            std::wcout << std::format( L"Failed to save file: \"{}\"", reinterpret_cast<const wchar_t *>(oOptions.vOutputs[I].c_str()) ) << std::endl;
            continue;
        }

        std::wcout << std::format( L"Saved file: \"{}\"", reinterpret_cast<const wchar_t *>(oOptions.vOutputs[I].c_str()) ) << std::endl;
        ++sSuccess;
    }

    return 0;
}
