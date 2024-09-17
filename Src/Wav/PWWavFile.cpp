﻿/**
 * Copyright L. Spiro 2024
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: A WAV file.
 */

#include "PWWavFile.h"
#include "../Files/PWStdFile.h"
#include "../Utilities/PWUtilities.h"

#include <codecvt>
#include <string>

// warning C4309: 'static_cast': truncation of constant value
#pragma warning( disable : 4309 )


namespace pw {

	CWavFile::CWavFile() :
		m_uiNumChannels( 0 ),
		m_uiSampleRate( 0 ),
		m_uiBitsPerSample( 0 ),
		m_uiBytesPerSample( 0 ),
		m_uiBaseNote( 64 ) {
	}
	CWavFile::~CWavFile() {
		Reset();
	}

	// == Functions.
	/**
	 * Loads a WAV file.
	 *
	 * \param _pcPath The UTF-8 path to open.
	 * \return Returns true if the file was opened.
	 */
	bool CWavFile::Open( const char8_t * _pcPath ) {
		std::vector<uint8_t> vFile;
		{
			CStdFile sfFile;
			if ( !sfFile.Open( _pcPath ) ) { return false; }
			if ( !sfFile.LoadToMemory( vFile ) ) { return false; }
		}
		return LoadFromMemory( vFile );
		//return Open( std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes( _pcPath ).c_str() );
	}

	/**
	 * Loads a WAV file.
	 *
	 * \param _pwcPath The UTF-16 path to open.
	 * \return Returns true if the file was opened.
	 */
	bool CWavFile::Open( const char16_t * _pwcPath ) {
		std::vector<uint8_t> vFile;
		{
			CStdFile sfFile;
			if ( !sfFile.Open( _pwcPath ) ) { return false; }
			if ( !sfFile.LoadToMemory( vFile ) ) { return false; }
		}
		return LoadFromMemory( vFile );
	}

	/**
	 * Loads a WAV file from memory.  This is just an in-memory version of the file.
	 *
	 * \param _vData The in-memory file to load.
	 * \return Returns true if the file is a valid WAV file.
	 */
	bool CWavFile::LoadFromMemory( const std::vector<uint8_t> &_vData ) {
		Reset();
		size_t stOffset = 0;
		const uint32_t * pui32Scratch;
		//const uint16_t * pui16Scratch;

#define PW_PTR_SIZE( TYPE, OFFSET, SIZE )			((OFFSET) + SIZE) > _vData.size() ? nullptr : reinterpret_cast<const TYPE *>(&_vData[OFFSET])
#define PW_PTR( TYPE, OFFSET )						PW_PTR_SIZE( TYPE, OFFSET, sizeof( TYPE ) )
#define PW_READ_32( VAL )							pui32Scratch = PW_PTR( uint32_t, stOffset ); if ( !pui32Scratch ) { return false; } stOffset += sizeof( uint32_t ); VAL = (*pui32Scratch)
//#define PW_READ_16( VAL )						pui16Scratch = PW_PTR( uint16_t, stOffset ); if ( !pui16Scratch ) { return false; } stOffset += sizeof( uint16_t ); VAL = (*pui16Scratch)
#define PW_READ_STRUCT( TYPE, VAL )				VAL = PW_PTR( TYPE, stOffset ); if ( !VAL ) { return false; } stOffset += sizeof( TYPE )


		do {
			PW_CHUNK cCurChunk;
			PW_READ_32( cCurChunk.u.uiName );
			if ( cCurChunk.u.uiName != PW_C_RIFF ) { return false; }
			PW_READ_32( cCurChunk.uiSize );
			PW_READ_32( cCurChunk.u2.uiFormat );
			if ( cCurChunk.u2.uiFormat != PW_C_WAVE ) { return false; }
			size_t stStartOff = stOffset;
			std::vector<PW_CHUNK_ENTRY> ceChunks;
			PW_CHUNK_ENTRY ceThis = { 0 };
			while ( (stOffset - stStartOff) < cCurChunk.uiSize && stOffset < _vData.size() ) {
				ceThis.uiOffset = static_cast<uint32_t>(stOffset);
				PW_READ_32( ceThis.u.uiName );
				if ( ceThis.u.uiName == 0 ) { break; }
				PW_READ_32( ceThis.uiSize );

				ceChunks.push_back( ceThis );
				stOffset += ceThis.uiSize;
			}

			for ( size_t I = 0; I < ceChunks.size(); ++I ) {
				switch ( ceChunks[I].u.uiName ) {
					case PW_C_FMT_ : {		// "fmt "
						const PW_FMT_CHUNK * pfcFmt = PW_PTR_SIZE( PW_FMT_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize );
						if ( !pfcFmt ) { return false; }

						if ( !LoadFmt( pfcFmt ) ) { return false; }
						break;
					}
					case PW_C_DATA : {		// "data"
						const PW_DATA_CHUNK * pfcData = PW_PTR_SIZE( PW_DATA_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize );
						if ( !pfcData ) { return false; }

						if ( !LoadData( pfcData ) ) { return false; }
						break;
					}
					case PW_C_SMPL : {		// "smpl"
						const PW_SMPL_CHUNK * pfcSmpl = PW_PTR_SIZE( PW_SMPL_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize );
						if ( !pfcSmpl ) { return false; }

						if ( !LoadSmpl( pfcSmpl ) ) { return false; }
						break;
					}
					case PW_C_LIST : {		// "LIST"
						const PW_LIST_CHUNK * plcList = PW_PTR_SIZE( PW_LIST_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize );
						if ( !plcList ) { return false; }

						if ( !LoadList( plcList ) ) { return false; }
						break;
					}
					case PW_C_ID3_ : {		// "id3 "
						const PW_ID3_CHUNK * picList = PW_PTR_SIZE( PW_ID3_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize );
						if ( !picList ) { return false; }

						if ( !LoadId3( picList ) ) { return false; }
						break;
					}
					case PW_C_INST : {		// "inst"
						const PW_INST_CHUNK * picList = PW_PTR_SIZE( PW_INST_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize );
						if ( !picList ) { return false; }

						if ( !LoadInst( picList ) ) { return false; }
						break;
					}

				}
			}


			/*switch ( cCurChunk.u2.uiFormat ) {
				case 0x45564157 : {		// "WAVE"
					if ( !LoadWAVE( _vData, stOffset ) ) { return false; }
					break;
				}
				default : { return false; }
			}*/
			//m_vChunks.push_back( cCurChunk );
		} while ( stOffset < _vData.size() );


#undef PW_READ_STRUCT
//#undef PW_READ_16
#undef PW_READ_32
#undef PW_PTR
		return true;
	}

	/**
	 * Saves as a PCM WAV file.
	 *
	 * \param _pcPath The path to where the file will be saved.
	 * \param _vSamples The samples to convert and write to the file.
	 * \param _psdSaveSettings Settings to override this class's settings.
	 * \return Returns true if the file was created and saved.
	 */
	bool CWavFile::SaveAsPcm( const char8_t * _pcPath, const lwaudio &_vSamples,
		const PW_SAVE_DATA * _psdSaveSettings ) const {
		if ( !_vSamples.size() ) { return false; }
		std::u8string sPath = _pcPath;
		std::u8string sFolder = CFileBase::GetFilePath( sPath );
		std::u8string sName = CFileBase::GetFileName( sPath );
		

		std::u8string sCopy = sName;
		const struct PW_TABLE {
			const char8_t *					pcReplaceMe;
			const char8_t *					pcWithMe;
		} tTable[] = {
			{ u8"?", u8"-" },
			{ u8"*", u8"˙" },
			{ u8":", u8" -" },
			{ u8"\\", u8"-" },
			{ u8"/", u8"∕" },
			{ u8"<", u8"‹" },
			{ u8">", u8"›" },
			{ u8"|", u8"¦" },
			{ u8"\"", u8"‟" },
		};
		for ( auto I = sizeof( tTable ) / sizeof( tTable[0] ); I--; ) {
			sCopy = CUtilities::Replace<std::u8string>( sCopy, reinterpret_cast<const char8_t *>(tTable[I].pcReplaceMe), reinterpret_cast<const char8_t *>(tTable[I].pcWithMe) );
		};

		sPath = sFolder + sCopy;

		CStdFile sfFile;
		if ( !sfFile.Create( sPath.c_str() ) ) {
			std::wprintf( L"Failed to create PCM file: %s.\r\n", reinterpret_cast<const wchar_t *>(CUtilities::Utf8ToUtf16( sPath.c_str() ).c_str()) );
			return false;
		}
		PW_FMT_CHUNK fcChunk = CreateFmt( PW_F_PCM, static_cast<uint16_t>(_vSamples.size()),
			_psdSaveSettings );
		std::vector<uint8_t> vLoops, vList;
		if ( m_vLoops.size() ) {
			vLoops = CreateSmpl();
		}
		if ( m_vListEntries.size() ) {
			vList = CreateList();
		}
		

		uint32_t uiFmtSize = fcChunk.chHeader.uiSize + 8;
		uint32_t ui32DataSize = CalcSize( static_cast<PW_FORMAT>(fcChunk.uiAudioFormat), static_cast<uint32_t>(_vSamples[0].size()), static_cast<uint16_t>(_vSamples.size()), fcChunk.uiBitsPerSample );

		uint32_t ui32Size = 4 +						// "WAVE".
			uiFmtSize +								// "fmt " chunk.
			ui32DataSize + 8 +						// "data" chunk.
			static_cast<uint32_t>(vLoops.size()) +	// "smpl" chunk.
			static_cast<uint32_t>(vList.size()) +	// "LIST" chunk.
			0;

		std::vector<uint8_t> vRet;
		vRet.reserve( ui32Size + 8 );
#define PW_PUSH32( VAL )		vRet.push_back( static_cast<uint8_t>((VAL) >> 0) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 8) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 16) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 24) )
		PW_PUSH32( PW_C_RIFF );						// "RIFF"
		
		PW_PUSH32( ui32Size );
		PW_PUSH32( PW_C_WAVE );
		
		// Append the "fmt " chunk.
		for ( size_t I = 0; I < uiFmtSize; ++I ) {
			vRet.push_back( reinterpret_cast<const uint8_t *>(&fcChunk)[I] );
		}

		// Append the "data" chunk.
		PW_PUSH32( PW_C_DATA );
		PW_PUSH32( ui32DataSize );
		switch ( fcChunk.uiAudioFormat ) {
			case PW_F_PCM : {
				switch ( fcChunk.uiBitsPerSample ) {
					case 8 : {
						if ( !BatchF64ToPcm8( _vSamples, vRet ) ) {
							return false;
						}
						break;
					}
					case 16 : {
						if ( !BatchF64ToPcm16( _vSamples, vRet ) ) {
							return false;
						}
						break;
					}
					case 24 : {
						if ( !BatchF64ToPcm24( _vSamples, vRet ) ) {
							return false;
						}
						break;
					}
					case 32 : {
						if ( !BatchF64ToPcm32( _vSamples, vRet ) ) {
							return false;
						}
						break;
					}
					default : {
						return false;
					}
				}
				break;
			}
			default : {
				return false;
			}
		}

		// Append "smpl" chunk.
		for ( size_t I = 0; I < vLoops.size(); ++I ) {
			vRet.push_back( vLoops[I] );
		}

		// Append "LIST" chunk.
		for ( size_t I = 0; I < vList.size(); ++I ) {
			vRet.push_back( vList[I] );
		}
		if ( !sfFile.WriteToFile( vRet ) ) { return false; }

#undef PW_PUSH32
		return true;

	}

	/**
	 * Resets the object back to scratch.
	 */
	void CWavFile::Reset() {
		//m_vChunks.clear();
		m_vSamples.clear();
		m_vLoops.clear();
		m_vListEntries.clear();
		m_vId3Entries.clear();
		m_vDisp.clear();
		m_uiNumChannels = 0;
		m_uiSampleRate = 0;
		m_uiBytesPerSample = 0;
		m_uiBitsPerSample = 0;
		m_uiBaseNote = 64;
	}

#pragma optimize( "gt", on )

	/**
	 * Fills a vector with the whole range of samples for a given channel.
	 *
	 * \param _uiChan The channel whose sample data is to be obtained.
	 * \param _vResult The vector containing the samples.
	 * \return Returns true if the vector was able to fit all samples.
	 */
	bool CWavFile::GetAllSamples( uint16_t _uiChan, lwtrack &_vResult ) const {
		if ( _uiChan >= m_uiNumChannels ) { return false; }
		switch ( m_uiBitsPerSample ) {
			case 8 : {
				return Pcm8ToF64( 0, static_cast<uint32_t>(TotalSamples()), _uiChan, _vResult );
			}
			case 16 : {
				return Pcm16ToF64( 0, static_cast<uint32_t>(TotalSamples()), _uiChan, _vResult );
			}
			case 24 : {
				return Pcm24ToF64( 0, static_cast<uint32_t>(TotalSamples()), _uiChan, _vResult );
			}
			case 32 : {
				switch ( m_fFormat ) {
					case PW_F_IEEE_FLOAT : {
						return F32ToF64( 0, static_cast<uint32_t>(TotalSamples()), _uiChan, _vResult );
					}
					case PW_F_PCM : {
						return Pcm32ToF64( 0, static_cast<uint32_t>(TotalSamples()), _uiChan, _vResult );
					}
				}
			}

		}
		return false;
	}

	/**
	 * Fills a vector with the whole range of samples for a given channel.
	 *
	 * \param _uiChan The channel whose sample data is to be obtained.
	 * \param _vResult The vector containing the samples.
	 * \param _stFrom The starting sample to get.
	 * \param _stTo The ending sample to get, exclusive.
	 * \return Returns true if the vector was able to fit all samples.
	 */
	bool CWavFile::GetSamples( uint16_t _uiChan, lwtrack &_vResult, size_t _stFrom, size_t _stTo ) const {
		if ( _uiChan >= m_uiNumChannels ) { return false; }
		switch ( m_uiBitsPerSample ) {
			case 8 : {
				return Pcm8ToF64( static_cast<uint32_t>(_stFrom), static_cast<uint32_t>(_stTo), _uiChan, _vResult );
			}
			case 16 : {
				return Pcm16ToF64( static_cast<uint32_t>(_stFrom), static_cast<uint32_t>(_stTo), _uiChan, _vResult );
			}
			case 24 : {
				return Pcm24ToF64( static_cast<uint32_t>(_stFrom), static_cast<uint32_t>(_stTo), _uiChan, _vResult );
			}
			case 32 : {
				switch ( m_fFormat ) {
					case PW_F_IEEE_FLOAT : {
						return F32ToF64( static_cast<uint32_t>(_stFrom), static_cast<uint32_t>(_stTo), _uiChan, _vResult );
					}
					case PW_F_PCM : {
						return Pcm32ToF64( static_cast<uint32_t>(_stFrom), static_cast<uint32_t>(_stTo), _uiChan, _vResult );
					}
				}
			}

		}
		return false;
	}

	/**
	 * Fills an array of vectors.  There is an array of vectors for each channel, and each vector contains all
	 *	of the samples for that channel.
	 *
	 * \param _vResult The array of vectors to be filled with all samples in this file.
	 * \return Returns true if the vector(s) was/were able to fit all samples.
	 */
	bool CWavFile::GetAllSamples( lwaudio &_vResult ) const {
		try {
			_vResult.resize( m_uiNumChannels );
			if ( _vResult.size() != m_uiNumChannels ) { return false; }
			for ( auto I = m_uiNumChannels; I--; ) {
				if ( !GetAllSamples( I, _vResult[I] ) ) { return false; }
			}
			return true;
		}
		catch ( ... ) { return false; }
	}

	/**
	 * Adds a LIST entry.
	 *
	 * \param _uiId The ID of the entry.
	 * \param _sVal The value of the entry.
	 * \return Returns true if the entry was added.
	 */
	bool CWavFile::AddListEntry( uint32_t _uiId, const std::u8string &_sVal ) {
		try {
			std::u8string sCopy = _sVal;
			const struct PW_TABLE {
				const char8_t *					pcReplaceMe;
				const char8_t *					pcWithMe;
			} tTable[] = {
				{ u8"’", u8"'" },
				{ u8"‘", u8"'" },
				{ u8"…", u8"..." },
				{ u8"Ō", u8"O" },
				{ u8" ", u8" " },
				{ u8"é", u8"e" },
				{ u8"“", u8"\"" },
				{ u8"”", u8"\"" },
				{ u8"⅓", u8"(1/3rd)" },
				{ u8"\u016B", u8"u" },
				{ u8"\u014D", u8"o" },

				/*{ u8"ぁ", "a" },
				{ u8"あ", "A" },

				{ u8"ぃ", "i" },
				{ u8"い", "I" },

				{ u8"ぅ", "u" },
				{ u8"う", "U" },

				{ u8"ぇ", "e" },
				{ u8"え", "E" },

				{ u8"ぉ", "o" },
				{ u8"お", "O" },

				{ u8"か", "Ka" },
				{ u8"が", "Ga" },

				{ u8"き", "Ki" },
				{ u8"ぎ", "Gi" },

				{ u8"く", "Ku" },
				{ u8"ぐ", "Gu" },

				{ u8"け", "Ke" },
				{ u8"げ", "Ge" },

				{ u8"こ", "Ko" },
				{ u8"ご", "Go" },

				{ u8"さ", "Sa" },
				{ u8"ざ", "Za" },

				{ u8"し", "Shi" },
				{ u8"じ", "Ji" },

				{ u8"す", "Su" },
				{ u8"ず", "Zu" },

				{ u8"せ", "Se" },
				{ u8"ぜ", "Ze" },

				{ u8"そ", "So" },
				{ u8"ぞ", "Zo" },

				{ u8"た", "Ta" },
				{ u8"だ", "Da" },

				{ u8"ち", "Chi" },
				{ u8"ぢ", "Ji" },

				{ u8"っ", "-" },
				{ u8"つ", "Tsu" },
				{ u8"づ", "Zu" },

				{ u8"て", "Te" },
				{ u8"で", "De" },

				{ u8"と", "To" },
				{ u8"ど", "Do" },

				{ u8"な", "Na" },
				{ u8"に", "Ni" },
				{ u8"ぬ", "Nu" },
				{ u8"ね", "Ne" },
				{ u8"の", "No" },

				{ u8"は", "Ha" },
				{ u8"ば", "Ba" },
				{ u8"ぱ", "Pa" },

				{ u8"ひ", "Hi" },
				{ u8"び", "Bi" },
				{ u8"ぴ", "Pi" },

				{ u8"ふ", "Hu" },
				{ u8"ぶ", "Bu" },
				{ u8"ぷ", "Pu" },

				{ u8"へ", "He" },
				{ u8"べ", "Be" },
				{ u8"ぺ", "Pe" },

				{ u8"ほ", "Ho" },
				{ u8"ぼ", "Bo" },
				{ u8"ぽ", "Po" },

				{ u8"ま", "Ma" },
				{ u8"み", "Mi" },
				{ u8"む", "Mu" },
				{ u8"め", "Me" },
				{ u8"も", "Mo" },

				{ u8"ゃ", "ya" },
				{ u8"や", "Ya" },

				{ u8"ゅ", "yu" },
				{ u8"ゆ", "Yu" },

				{ u8"ょ", "yo" },
				{ u8"よ", "Yo" },

				{ u8"ら", "Ra" },
				{ u8"り", "Ri" },
				{ u8"る", "Ru" },
				{ u8"れ", "Re" },
				{ u8"ろ", "Ro" },

				{ u8"ゎ", "wa" },
				{ u8"わ", "Wa" },

				{ u8"ゐ", "Wi" },

				{ u8"ゑ", "We" },

				{ u8"を", "Wo" },

				{ u8"ん", "N" },

				{ u8"ゔ", "Vu" },

				{ u8"ゕ", "ka" },
				{ u8"ゖ", "ke" },			

				{ u8"ぷ", "Pu" },
				{ u8"よ", "Yo" },


				{ u8"ワ", "Wa" },
				{ u8"ン", "N" },
				{ u8"ダ", "Da" },
				{ u8"ー", "-" },
				{ u8"プ", "Pu" },
				{ u8"ロ", "Ro" },
				{ u8"ジ", "Ji" },
				{ u8"ェ", "e" },

				{ u8"ジェ", "Je" },

				{ u8"ク", "Ku" },
				{ u8"ト", "To" },
				{ u8"コ", "Ko" },
				{ u8"ル", "Ru" },
				{ u8"ジョ", "Jo" },
				{ u8"ゼ", "Ze" },
				{ u8"ゼ", "Ze" },
				{ u8"ゼッ", "Ze" },
				{ u8"ゼッ", "Ze" },
				{ u8"ズ", "Zu" },

				{ u8"森", "Mori" },
				{ u8"彰", "Aki" },
				{ u8"彦", "Hiko" },
				{ u8"彰彦", "Akihiko" },
				{ u8"魔獣使い伝説", "Maju Tsukai Densetsu" },
				{ u8"一郎", "Ichiro" },
				{ u8"根本", "Nemoto" },

				{ u8"Ｒ", "R" },
				{ u8"Ｅ", "E" },
				{ u8"Ｓ", "S" },
				{ u8"Ｕ", "U" },
				{ u8"Ｌ", "L" },
				{ u8"Ｔ", "T" },*/
			};
			for ( auto I = sizeof( tTable ) / sizeof( tTable[0] ); I--; ) {
				sCopy = CUtilities::Replace<std::u8string>( sCopy, tTable[I].pcReplaceMe, tTable[I].pcWithMe );
			};

			PW_LIST_ENTRY lsEntry;
			lsEntry.u.uiIfoId = _uiId;
			lsEntry.sText = sCopy;
			lsEntry.sText.push_back( '\0' );
			if ( lsEntry.sText.size() & 1 ) {
				// Make it an odd number of characters because a hard-coded 0 will be printed into the file.
				lsEntry.sText.push_back( '\0' );
			}
			for ( auto I = m_vListEntries.size(); I--; ) {
				if ( m_vListEntries[I].u.uiIfoId == _uiId ) {
					m_vListEntries[I] = lsEntry;
					return true;
				}
			}
			m_vListEntries.push_back( lsEntry );
			return true;
		}
		catch ( ... ) { return false; }
	}

	/**
	 * Adds a DISP image.
	 * 
	 * \param _ui32Type The image type.  One of the CF_* values.
	 * \param _vImage The image data.
	 * \return Returns true if the entry was added.
	 **/
	bool CWavFile::AddImage( uint32_t _ui32Type, const std::vector<uint8_t> &_vImage ) {
		try {
			if ( _vImage.size() != uint32_t( _vImage.size() ) ) { return false; }
			PW_DISP_ENTRY deEntry;
			deEntry.u.uiIfoId = PW_C_DISP;
			deEntry.ui32Size = uint32_t( _vImage.size() );
			deEntry.ui32Type = _ui32Type;
			deEntry.vValue = _vImage;
			m_vDisp.push_back( deEntry );
			return true;
		}
		catch ( ... ) { return false; }
	}

	/**
	 * Loads data from the "fmt ".
	 *
	 * \param _pfcChunk The chunk of data to load
	 * \return Returns true if everything loaded fine.
	 */
	bool CWavFile::LoadFmt( const PW_FMT_CHUNK * _pfcChunk ) {
		m_uiNumChannels = _pfcChunk->uiNumChannels;
		m_uiSampleRate = _pfcChunk->uiSampleRate;
		m_uiBitsPerSample = _pfcChunk->uiBitsPerSample;
		m_uiBytesPerSample = m_uiBitsPerSample / 8;
		m_fFormat = static_cast<PW_FORMAT>(_pfcChunk->uiAudioFormat);
		return true;
	}

	/**
	 * Loads the "data" chunk.
	 *
	 * \param _pdcChunk The chunk of data to load
	 * \return Returns true if everything loaded fine.
	 */
	bool CWavFile::LoadData( const PW_DATA_CHUNK * _pdcChunk ) {
		m_vSamples.resize( _pdcChunk->chHeader.uiSize );
		if ( m_vSamples.size() != _pdcChunk->chHeader.uiSize ) { return false; }
		std::memcpy( m_vSamples.data(), _pdcChunk->ui8Data, m_vSamples.size() );
		return true;
	}

	/**
	 * Loads the "smpl" chunk.
	 *
	 * \param _pscChunk The chunk of data to load
	 * \return Returns true if everything loaded fine.
	 */
	bool CWavFile::LoadSmpl( const PW_SMPL_CHUNK * _pscChunk ) {
		m_uiBaseNote = _pscChunk->uiMIDIUnityNote;
		for ( size_t I = 0; I < _pscChunk->uiNumSampleLoops; ++I ) {
			if ( _pscChunk->lpLoops[I].uiStart < m_vSamples.size() && _pscChunk->lpLoops[I].uiEnd < m_vSamples.size() ) {
				m_vLoops.push_back( _pscChunk->lpLoops[I] );
			}
		}
		return true;
	}

	/**
	 * Loads a "LIST" chunk.
	 *
	 * \param _plcChunk The chunk of data to load
	 * \return Returns true if everything loaded fine.
	 */
	bool CWavFile::LoadList( const PW_LIST_CHUNK * _plcChunk ) {
		const uint8_t * pui8Data = _plcChunk->ui8Data;
		const uint8_t * pui8End = pui8Data + _plcChunk->chHeader.uiSize - sizeof( _plcChunk->u );
		switch ( _plcChunk->u.uiTypeId ) {
			case PW_C_INFO : {				// INFO
				while ( pui8Data < pui8End ) {
					PW_LIST_ENTRY lsThis;
					if ( (pui8Data + sizeof( uint32_t )) > pui8End ) { return false; }
					lsThis.u.uiIfoId = (*reinterpret_cast<const uint32_t *>(pui8Data));
					pui8Data += sizeof( uint32_t );
					if ( (pui8Data + sizeof( uint32_t )) > pui8End ) { return false; }
					uint32_t ui32Size = (*reinterpret_cast<const uint32_t *>(pui8Data));
					pui8Data += sizeof( uint32_t );
					for ( uint32_t I = 0; I < ui32Size; ++I ) {
						if ( (pui8Data + sizeof( uint8_t )) > pui8End ) { return false; }
						lsThis.sText.push_back( (*pui8Data++) );
					}
					m_vListEntries.push_back( lsThis );
				}
				break;
			}
			case PW_C_ADTL : {
				break;
			}
			default : {
				return false;
			}
		}
		

		return true;
	}

	/**
	 * Loads an "id3 " chunk.
	 *
	 * \param _plcChunk The chunk of data to load
	 * \return Returns true if everything loaded fine.
	 */
	bool CWavFile::LoadId3( const PW_ID3_CHUNK * _picChunk ) {
		if ( _picChunk->ui16Version == 0x3 ) {
			uint32_t ui32Len = DecodeSize(_picChunk->ui32Size );

			for ( uint32_t ui32Offset = 0; ui32Offset < ui32Len; ) {
				uint32_t ui32Id = (*reinterpret_cast<const uint32_t *>(&_picChunk->ui8Data[ui32Offset]));
				ui32Offset += sizeof( uint32_t );
				uint32_t ui32Size = DecodeSize( (*reinterpret_cast<const uint32_t *>(&_picChunk->ui8Data[ui32Offset])) );
				ui32Offset += sizeof( uint32_t );
				uint16_t ui16Flags = (*reinterpret_cast<const uint16_t *>(&_picChunk->ui8Data[ui32Offset]));
				ui32Offset += sizeof( uint16_t );
				PW_ID3_ENTRY ieEntry;
				ieEntry.u.uiIfoId = ui32Id;
				ieEntry.ui16Flags = ui16Flags;
				for ( uint32_t J = 0; J < ui32Size; ++J ) {
					ieEntry.sValue.push_back( _picChunk->ui8Data[ui32Offset++] );
				}
				m_vId3Entries.push_back( ieEntry );
			}
			return true;
		}

		return false;
	}

	/**
	 * Loads an "inst" chunk.
	 *
	 * \param _plcChunk The chunk of data to load
	 * \return Returns true if everything loaded fine.
	 */
	bool CWavFile::LoadInst( const PW_INST_CHUNK * _picChunk ) {
#define PW_COPY( VAL )		m_ieInstEntry.VAL = _picChunk->VAL
		PW_COPY( ui8UnshiftedNote );
		PW_COPY( ui8FineTune );
		PW_COPY( ui8Gain );
		PW_COPY( ui8LowNote );
		PW_COPY( ui8HiNote );
		PW_COPY( ui8LowVel );
		PW_COPY( ui8HiVel );

#undef PW_COPY
		return true;
	}

	/**
	 * Converts a bunch of 8-bit PCM samples to double.
	 *
	 * \param _ui32From Starting sample index.
	 * \param _ui32To Ending sample index.
	 * \param _uiChan The channel whose sample data is to be obtained.
	 * \param _vResult The vector containing the samples.
	 * \return Returns true if the vector was able to hold all of the values.
	 */
	bool CWavFile::Pcm8ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, lwtrack &_vResult ) const {
		size_t sFinalSize = _vResult.size() + (_ui32To - _ui32From);
		_vResult.reserve( sFinalSize );
		uint32_t uiStride;
		size_t sIdx = CalcOffsetsForSample( _uiChan, _ui32From, uiStride );
		const int8_t * pi8Samples = reinterpret_cast<const int8_t *>(&m_vSamples.data()[sIdx]);
		while ( _ui32From < _ui32To ) {
			_vResult.push_back( (static_cast<int32_t>((*pi8Samples)) - 128) / 127.0 );

			pi8Samples += m_uiNumChannels;
			++_ui32From;
		}
		return true;
	}

	/**
	 * Converts a bunch of 16-bit PCM samples to double.
	 *
	 * \param _ui32From Starting sample index.
	 * \param _ui32To Ending sample index.
	 * \param _uiChan The channel whose sample data is to be obtained.
	 * \param _vResult The vector containing the samples.
	 * \return Returns true if the vector was able to hold all of the values.
	 */
	bool CWavFile::Pcm16ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, lwtrack &_vResult ) const {
		const double dFactor = std::pow( 2.0, 16.0 - 1.0 ) - 1.0;
		size_t sFinalSize = _vResult.size() + (_ui32To - _ui32From);
		_vResult.reserve( sFinalSize );
		uint32_t uiStride;
		size_t sIdx = CalcOffsetsForSample( _uiChan, _ui32From, uiStride );
		const int16_t * pi16Samples = reinterpret_cast<const int16_t *>(&m_vSamples.data()[sIdx]);
		while ( _ui32From < _ui32To ) {
			_vResult.push_back( (*pi16Samples) / dFactor );

			pi16Samples += m_uiNumChannels;
			++_ui32From;
		}
		return true;
	}

	/**
	 * Converts a bunch of 24-bit PCM samples to double.
	 *
	 * \param _ui32From Starting sample index.
	 * \param _ui32To Ending sample index.
	 * \param _uiChan The channel whose sample data is to be obtained.
	 * \param _vResult The vector containing the samples.
	 * \return Returns true if the vector was able to hold all of the values.
	 */
	bool CWavFile::Pcm24ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, lwtrack &_vResult ) const {
		const double dFactor = (std::pow( 2.0, 24.0 - 1.0 ) - 1.0) * 256.0;
		size_t sFinalSize = _vResult.size() + (_ui32To - _ui32From);
		_vResult.reserve( sFinalSize );
		uint32_t uiStride;
		size_t sIdx = CalcOffsetsForSample( _uiChan, _ui32From, uiStride );
		while ( _ui32From < _ui32To ) {
			if ( sIdx >= m_vSamples.size() ) {
				_vResult.push_back( 0.0 );
			}
			else {
				const int32_t * pi32Samples = reinterpret_cast<const int32_t *>(&m_vSamples.data()[sIdx]);

				_vResult.push_back( ((*pi32Samples) << 8) / dFactor );
			}

			sIdx += uiStride;
			++_ui32From;
		}
		return true;
	}

	/**
	 * Converts a bunch of 32-bit PCM samples to double.
	 *
	 * \param _ui32From Starting sample index.
	 * \param _ui32To Ending sample index.
	 * \param _uiChan The channel whose sample data is to be obtained.
	 * \param _vResult The vector containing the samples.
	 * \return Returns true if the vector was able to hold all of the values.
	 */
	bool CWavFile::Pcm32ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, lwtrack &_vResult ) const {
		const double dFactor = std::pow( 2.0, 32.0 - 1.0 ) - 1.0;
		size_t sFinalSize = _vResult.size() + (_ui32To - _ui32From);
		_vResult.reserve( sFinalSize );
		uint32_t uiStride;
		size_t sIdx = CalcOffsetsForSample( _uiChan, _ui32From, uiStride );
		const int32_t * pi32Samples = reinterpret_cast<const int32_t *>(&m_vSamples.data()[sIdx]);
		while ( _ui32From < _ui32To ) {
			_vResult.push_back( (*pi32Samples) / dFactor );

			pi32Samples += m_uiNumChannels;
			++_ui32From;
		}
		return true;
	}

	/**
	 * Converts a bunch of 32-bit float samples to double.
	 *
	 * \param _ui32From Starting sample index.
	 * \param _ui32To Ending sample index.
	 * \param _uiChan The channel whose sample data is to be obtained.
	 * \param _vResult The vector containing the samples.
	 * \return Returns true if the vector was able to hold all of the values.
	 */
	bool CWavFile::F32ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, lwtrack &_vResult ) const {
		size_t sFinalSize = _vResult.size() + (_ui32To - _ui32From);
		_vResult.reserve( sFinalSize );
		uint32_t uiStride;
		size_t sIdx = CalcOffsetsForSample( _uiChan, _ui32From, uiStride );
		const float * pfSamples = reinterpret_cast<const float *>(&m_vSamples.data()[sIdx]);
		while ( _ui32From < _ui32To ) {
			_vResult.push_back( (*pfSamples) );

			pfSamples += m_uiNumChannels;
			++_ui32From;
		}
		return true;
	}

	/**
	 * Converts a batch of F64 samples to PCM samples.
	 *
	 * \param _vSrc The samples to convert.
	 * \param _vDst The buffer to which to convert the samples.
	 * \return Returns trye if all samples were added to the buffer.
	 */
	bool CWavFile::BatchF64ToPcm8( const lwaudio &_vSrc, std::vector<uint8_t> &_vDst ) {
		for ( size_t I = 0; I < _vSrc[0].size(); ++I ) {
			for ( size_t J = 0; J < _vSrc.size(); ++J ) {
				int8_t iSample = static_cast<int8_t>(std::round( std::clamp( _vSrc[J][I], -1.0, 1.0 ) * 127.0 + 128.0 ));
				_vDst.push_back( static_cast<uint8_t>(iSample) );
			}
		}
		return true;
	}

	/**
	 * Converts a batch of F64 samples to PCM samples.
	 *
	 * \param _vSrc The samples to convert.
	 * \param _vDst The buffer to which to convert the samples.
	 * \return Returns trye if all samples were added to the buffer.
	 */
	bool CWavFile::BatchF64ToPcm16( const lwaudio &_vSrc, std::vector<uint8_t> &_vDst ) {
		try {
			const double dFactor = std::pow( 2.0, 16.0 - 1.0 ) - 1.0;
			auto stNumSamples = _vSrc[0].size();
			auto stNumChannels = _vSrc.size();
			auto aSize = _vDst.size();
			_vDst.reserve( _vDst.size() + stNumSamples * stNumChannels * sizeof( int16_t ) );
			int16_t * pi16Dst = reinterpret_cast<int16_t *>(_vDst.data() + aSize);

#ifdef __AVX512BW__
			if ( CUtilities::IsAvx512BWSupported() ) {
				// Constants for AVX-512 operations
				__m512d vdFactor = _mm512_set1_pd( dFactor );
				__m512d vdMin = _mm512_set1_pd( -1.0 );
				__m512d vdMax = _mm512_set1_pd( 1.0 );
				std::vector<std::vector<uint16_t>, CAlignmentAllocator<std::vector<uint16_t>, 64>> vSamples;
				vSamples.resize( stNumChannels );
				for ( auto I = stNumChannels; I--; ) {
					vSamples[I].resize( stNumSamples );
				}
				
				for ( auto C = stNumChannels; C--; ) {
					lwaudio::size_type stIdx = 0;
					while ( stIdx < stNumSamples - (8 - 1) ) {
						__m512d vdSamples = _mm512_load_pd( &_vSrc[C][stIdx] );

						// Clamp samples between -1.0 and 1.0.
						vdSamples = _mm512_max_pd( vdSamples, vdMin );
						vdSamples = _mm512_min_pd( vdSamples, vdMax );

						// Multiply by scaling factor.
						vdSamples = _mm512_mul_pd( vdSamples, vdFactor );

						// Round to nearest integer and convert to int32_t
						__m256i viInt32 = _mm512_cvt_roundpd_epi32( vdSamples, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );

						// Convert int32_t to int16_t with saturation
						__m128i viInt16 = _mm256_cvtsepi32_epi16( viInt32 );

						_mm_store_si128( reinterpret_cast<__m128i *>(&vSamples[C][stIdx]), viInt16 );


						stIdx += sizeof( __m512d ) / sizeof( double );
					}
				}

				return true;
			}
#endif	// #ifdef __AVX512BW__

			for ( size_t I = 0; I < _vSrc[0].size(); ++I ) {
				for ( size_t J = 0; J < _vSrc.size(); ++J ) {
					(*pi16Dst++) = static_cast<int16_t>(std::round( std::clamp( _vSrc[J][I], -1.0, 1.0 ) * dFactor ));
				}
			}
			return true;
		}
		catch ( ... ) { return false; }
	}

	/**
	 * Converts a batch of F64 samples to PCM samples.
	 *
	 * \param _vSrc The samples to convert.
	 * \param _vDst The buffer to which to convert the samples.
	 * \return Returns trye if all samples were added to the buffer.
	 */
	bool CWavFile::BatchF64ToPcm24( const lwaudio &_vSrc, std::vector<uint8_t> &_vDst ) {
		const double dFactor = std::pow( 2.0, 24.0 - 1.0 ) - 1.0;
		for ( size_t I = 0; I < _vSrc[0].size(); ++I ) {
			for ( size_t J = 0; J < _vSrc.size(); ++J ) {
				int32_t iSample = static_cast<int32_t>(std::round( std::clamp( _vSrc[J][I], -1.0, 1.0 ) * dFactor ));
				_vDst.push_back( static_cast<uint8_t>(iSample) );
				_vDst.push_back( static_cast<uint8_t>(iSample >> 8) );
				_vDst.push_back( static_cast<uint8_t>(iSample >> 16) );
			}
		}
		return true;
	}

	/**
	 * Converts a batch of F64 samples to PCM samples.
	 *
	 * \param _vSrc The samples to convert.
	 * \param _vDst The buffer to which to convert the samples.
	 * \return Returns trye if all samples were added to the buffer.
	 */
	bool CWavFile::BatchF64ToPcm32( const lwaudio &_vSrc, std::vector<uint8_t> &_vDst ) {
		try {
			const double dFactor = std::pow( 2.0, 32.0 - 1.0 ) - 1.0;
			auto stNumSamples = _vSrc[0].size();
			auto stNumChannels = _vSrc.size();
			auto aSize = _vDst.size();
			_vDst.resize( _vDst.size() + stNumSamples * stNumChannels * sizeof( int32_t ) );
			int32_t * pi32Dst = reinterpret_cast<int32_t *>(_vDst.data() + aSize);

#ifdef __AVX512BW__
			if ( CUtilities::IsAvx512BWSupported() ) {
				// Constants for AVX-512 operations
				__m512d vdFactor = _mm512_set1_pd( dFactor );
				__m512d vdMin = _mm512_set1_pd( -1.0 );
				__m512d vdMax = _mm512_set1_pd( 1.0 );
				std::vector<std::vector<uint32_t>, CAlignmentAllocator<std::vector<uint32_t>, 64>> vSamples;
				vSamples.resize( stNumChannels );
				for ( auto I = stNumChannels; I--; ) {
					vSamples[I].resize( stNumSamples );
				}
				
				for ( auto C = stNumChannels; C--; ) {
					lwaudio::size_type stIdx = 0;
					while ( stIdx < stNumSamples - (8 - 1) ) {
						__m512d vdSamples = _mm512_load_pd( &_vSrc[C][stIdx] );

						// Clamp samples between -1.0 and 1.0.
						vdSamples = _mm512_max_pd( vdSamples, vdMin );
						vdSamples = _mm512_min_pd( vdSamples, vdMax );

						// Multiply by scaling factor.
						vdSamples = _mm512_mul_pd( vdSamples, vdFactor );

						// Round to nearest integer and convert to int32_t
						__m256i viInt32 = _mm512_cvt_roundpd_epi32( vdSamples, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );

						// Store the int32_t samples.
						_mm256_store_si256( reinterpret_cast<__m256i*>(&vSamples[C][stIdx]), viInt32 );


						stIdx += sizeof( __m512d ) / sizeof( double );
					}

					while ( stIdx < stNumSamples ) {
						vSamples[C][stIdx++] = static_cast<int32_t>(std::round( std::clamp( _vSrc[C][stIdx], -1.0, 1.0 ) * dFactor ));
					}
				}

				for ( size_t I = 0; I < _vSrc[0].size(); ++I ) {
					for ( size_t J = 0; J < _vSrc.size(); ++J ) {
						(*pi32Dst++) = vSamples[J][I];
					}
				}

				return true;
			}
#endif	// #ifdef __AVX512BW__

			for ( size_t I = 0; I < _vSrc[0].size(); ++I ) {
				for ( size_t J = 0; J < _vSrc.size(); ++J ) {
					(*pi32Dst++) = static_cast<int32_t>(std::round( std::clamp( _vSrc[J][I], -1.0, 1.0 ) * dFactor ));
				}
			}
			return true;
		}
		catch ( ... ) { return false; }
	}

	/**
	 * Gets the byte indices of PCM data given an offset and channel.
	 *
	 * \param _uiChan The channel.
	 * \param _uiIdx The sample index.
	 * \param _uiStride The returned stride.
	 * \return Returns the byte index calculated using the given information and the sample depth.
	 */
	size_t CWavFile::CalcOffsetsForSample( uint16_t _uiChan, uint32_t _uiIdx, uint32_t &_uiStride ) const {
		_uiStride = m_uiNumChannels * m_uiBytesPerSample;
		return _uiStride * _uiIdx + (_uiChan * m_uiBytesPerSample);
	}

	/**
	 * Converts a 28-bit size value from ID3 into regular 32-bit.
	 *
	 * \param _uiSize The size value to decode.
	 * \return Returns the decoded size value.
	 */
	uint32_t CWavFile::DecodeSize( uint32_t _uiSize ) {
		return ((_uiSize >> 24) & 0x7F) |
			(((_uiSize >> 16) & 0x7F) << 7) |
			(((_uiSize >> 8) & 0x7F) << 14) |
			(((_uiSize >> 0) & 0x7F) << 21);
	}

	/**
	 * Creates an "fmt " chunk based off either this object's parameters or optional given overrides.
	 *
	 * \param _fFormat The format to set.
	 * \param _uiChannels The total channels.
	 * \param _psdSaveSettings Optional overrides.
	 * \return Returns a created object.
	 */
	CWavFile::PW_FMT_CHUNK CWavFile::CreateFmt( PW_FORMAT _fFormat, uint16_t _uiChannels, const PW_SAVE_DATA * _psdSaveSettings ) const {
		PW_FMT_CHUNK fcChunk = { 0 };
		fcChunk.chHeader.u.uiId = PW_C_FMT_;
		fcChunk.chHeader.uiSize = 16;
		fcChunk.uiAudioFormat = _fFormat;
		fcChunk.uiBitsPerSample = (_psdSaveSettings && _psdSaveSettings->uiBitsPerSample) ? _psdSaveSettings->uiBitsPerSample : BitsPerSample();
		fcChunk.uiNumChannels = _uiChannels;
		fcChunk.uiSampleRate = (_psdSaveSettings && _psdSaveSettings->uiHz) ? _psdSaveSettings->uiHz : Hz();
		fcChunk.uiBlockAlign = fcChunk.uiBitsPerSample * fcChunk.uiNumChannels / 8;
		fcChunk.uiByteRate = fcChunk.uiBlockAlign * fcChunk.uiSampleRate;

		return fcChunk;
	}

	/**
	 * Writes file-image loop-point bytes to a vector.
	 *
	 * \return Returns the bytes that represent the "smpl" chunk in a file.
	 */
	std::vector<uint8_t> CWavFile::CreateSmpl() const {
		std::vector<uint8_t> vRet;
#define PW_PUSH32( VAL )		vRet.push_back( static_cast<uint8_t>((VAL) >> 0) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 8) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 16) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 24) )
		PW_PUSH32( PW_C_SMPL );			// "smpl"
		uint32_t uiVal = static_cast<uint32_t>(36 + m_vLoops.size() * 24 + 0);
		PW_PUSH32( uiVal );				// 36 + (Num Sample Loops * 24) + Sampler Data
		PW_PUSH32( 0 );					// Manufacturer.
		PW_PUSH32( 0 );					// Product.
		PW_PUSH32( 0 );					// Sample Period.
		PW_PUSH32( m_uiBaseNote );		// MIDI Unity Note.
		PW_PUSH32( 0 );					// MIDI Pitch Fraction.
		PW_PUSH32( 0 );					// SMPTE Format.
		PW_PUSH32( 0 );					// SMPTE Offset.
		PW_PUSH32( m_vLoops.size() );	// Num Sample Loops.
		PW_PUSH32( 0 );					// Sampler Data.
		for ( size_t I = 0; I < m_vLoops.size(); ++I ) {
			PW_PUSH32( m_vLoops[I].uiCuePointID );			// Cue Point ID.

			/**
			0	Loop forward (normal)
			1	Alternating loop (forward/backward, also known as Ping Pong)
			2	Loop backward (reverse)
			3 - 31	Reserved for future standard types
			32 - 0xFFFFFFFF	Sampler specific types (defined by manufacturer) */
			PW_PUSH32( m_vLoops[I].uiType );				// Type.
			PW_PUSH32( m_vLoops[I].uiStart );				// Start.
			PW_PUSH32( m_vLoops[I].uiEnd );					// End.
			PW_PUSH32( m_vLoops[I].uiFraction );			// Fraction.
			PW_PUSH32( m_vLoops[I].uiPlayCount );			// Play Count.
		}
#undef PW_PUSH32
		return vRet;
	}

	/**
	 * Writes file-image "LIST" chunk to a vector.
	 *
	 * \return Returns the bytes that represent the "LIST" chunk in a file.
	 */
	std::vector<uint8_t> CWavFile::CreateList() const {
		std::vector<uint8_t> vRet;
#define PW_PUSH32( VAL )		vRet.push_back( static_cast<uint8_t>((VAL) >> 0) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 8) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 16) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 24) )
		PW_PUSH32( PW_C_LIST );			// "LIST"
		uint32_t ui32Size = 4;
		for ( auto I = m_vListEntries.size(); I--; ) {
			ui32Size += static_cast<uint32_t>(m_vListEntries[I].sText.size()) + 8;
		}
		PW_PUSH32( ui32Size );			// Size.
		PW_PUSH32( PW_C_INFO );			// "INFO"
		for ( size_t I = 0; I < m_vListEntries.size(); ++I ) {
			PW_PUSH32( m_vListEntries[I].u.uiIfoId );
			PW_PUSH32( static_cast<uint32_t>(m_vListEntries[I].sText.size()) );
			for ( size_t J = 0; J < m_vListEntries[I].sText.size(); ++J ) {
				vRet.push_back( static_cast<uint8_t>(m_vListEntries[I].sText[J]) );
			}
		}
#undef PW_PUSH32
		return vRet;
	}

}	// namespace pw
