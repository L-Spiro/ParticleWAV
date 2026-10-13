/**
 * Copyright L. Spiro 2024
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: A WAV file.
 */

#include "PWWavFile.h"
#include "../Files/PWFileStream.h"
#include "../OS/PWOs.h"
#include "../Utilities/PWStream.h"
#include "../Utilities/PWUtilities.h"

#include <algorithm>
#include <codecvt>
#include <filesystem>
#include <string>

// warning C4309: 'static_cast': truncation of constant value
#pragma warning( disable : 4309 )


namespace pw {

	CWavFile::CWavFile() :
		m_fFormat( PW_F_PCM ),
		m_ui32OriginalSampleCount( 0 ),
		m_uiNumChannels( 0 ),
		m_uiSampleRate( 0 ),
		m_uiBitsPerSample( 0 ),
		m_uiBytesPerSample( 0 ),
		m_uiBaseNote( 64 ),
		m_bHasInst( false ),
		m_ui8Id3Version( 3 ) {
	}
	CWavFile::~CWavFile() {
		StopStream();
		Reset();
	}

	// == Functions.
	/**
	 * Loads a WAV file.
	 *
	 * \param _pcPath The UTF-8 path to open.
	 * \param _ui32LoadFlags The loading flags, which allow to not load large portions of the file.
	 * \param _ui32StartSample The first sample to load.
	 * \param _ui32EndSample The last sample (exclusive) to load.
	 * \return Returns true if the file was opened.
	 */
	bool CWavFile::Open( const char8_t * _pcPath, uint32_t _ui32LoadFlags, uint32_t _ui32StartSample, uint32_t _ui32EndSample ) {
		CStdFile sfFile;
		if ( !sfFile.Open( _pcPath ) ) { return false; }
		CFileStream sStream( sfFile );
		return LoadFromStream( sStream, _ui32LoadFlags, _ui32StartSample, _ui32EndSample );
		/*std::vector<uint8_t> vFile;
		if ( !CStdFile::LoadToMemory( _pcPath, vFile ) ) { return false; }
		return LoadFromMemory( vFile, _ui32LoadFlags, _ui32StartSample, _ui32EndSample );*/
		//return Open( std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes( _pcPath ).c_str() );
	}

	/**
	 * Loads a WAV file.
	 *
	 * \param _pwcPath The UTF-16 path to open.
	 * \param _ui32LoadFlags The loading flags, which allow to not load large portions of the file.
	 * \param _ui32StartSample The first sample to load.
	 * \param _ui32EndSample The last sample (exclusive) to load.
	 * \return Returns true if the file was opened.
	 */
	bool CWavFile::Open( const char16_t * _pwcPath, uint32_t _ui32LoadFlags, uint32_t _ui32StartSample, uint32_t _ui32EndSample ) {
		CStdFile sfFile;
		if ( !sfFile.Open( _pwcPath ) ) { return false; }
		CFileStream sStream( sfFile );
		return LoadFromStream( sStream, _ui32LoadFlags, _ui32StartSample, _ui32EndSample );
		/*std::vector<uint8_t> vFile;
		if ( !CStdFile::LoadToMemory( _pwcPath, vFile ) ) { return false; }
		return LoadFromMemory( vFile, _ui32LoadFlags, _ui32StartSample, _ui32EndSample );*/
	}

	/**
	 * Loads a WAV file from a stream.  Will usually be a file stream but can be any stream.
	 * 
	 * \param _sbStream The stream from which to load the file.
	 * \param _ui32LoadFlags The loading flags, which allow to not load large portions of the file.
	 * \param _ui32StartSample The first sample to load.
	 * \param _ui32EndSample The last sample (exclusive) to load.
	 * \return Returns true if the file is a valid WAV file and there are no memory problems loading it.
	 **/
	bool CWavFile::LoadFromStream( CStreamBase &_sbStream, uint32_t _ui32LoadFlags, uint32_t _ui32StartSample, uint32_t _ui32EndSample ) {
		Reset();

		//do {
			PW_CHUNK cCurChunk;
			if ( !_sbStream.ReadUi32( cCurChunk.u.uiName ) ) { return false; }
			if ( cCurChunk.u.uiName != PW_C_RIFF ) { return false; }
			if ( !_sbStream.ReadUi32( cCurChunk.uiSize ) ) { return false; }
			if ( !_sbStream.ReadUi32( cCurChunk.u2.uiFormat ) ) { return false; }
			if ( cCurChunk.u2.uiFormat != PW_C_WAVE ) { return false; }
			size_t stStartOff = _sbStream.Pos();
			std::vector<PW_CHUNK_ENTRY> ceChunks;
			PW_CHUNK_ENTRY ceThis = { 0 };
			while ( (_sbStream.Pos() - stStartOff) < cCurChunk.uiSize && _sbStream.Remaining() >= sizeof( PW_CHUNK_ENTRY ) ) {
				ceThis.uiOffset = static_cast<uint32_t>(_sbStream.Pos());
				if ( !_sbStream.ReadUi32( ceThis.u.uiName ) ) { break; }
				if ( ceThis.u.uiName == 0 ) { break; }
				if ( !_sbStream.ReadUi32( ceThis.uiSize ) ) { break; }

				ceChunks.push_back( ceThis );
				//stOffset += ceThis.uiSize;
				if ( ceThis.uiSize & 1 ) { ++ceThis.uiSize; }
				// A chunk that runs past the end of the file (a truncated file) is the last one; never seek beyond the end.
				if ( ceThis.uiSize >= _sbStream.Remaining() ) { break; }
				_sbStream.MovePointerBy( ceThis.uiSize );
			}

#define PW_LAOD_SECTION																							\
	std::vector<uint8_t> vBuffer;																					\
	_sbStream.MovePointerTo( ceChunks[I].uiOffset );																\
	if ( _sbStream.Remaining() < ceChunks[I].uiSize + sizeof( PW_CHUNK_HEADER ) ) { return false; }				\
	try { vBuffer.resize( ceChunks[I].uiSize + sizeof( PW_CHUNK_HEADER ) ); }										\
	catch ( ... ) { return false; }																					\
	if ( !_sbStream.Read( vBuffer.data(), vBuffer.size() ) ) { return false; }

			// Do the format chunk first.
			for ( size_t I = 0; I < ceChunks.size(); ++I ) {
				switch ( ceChunks[I].u.uiName ) {
					case PW_C_FMT_ : {		// "fmt "
						PW_LAOD_SECTION;
						const PW_FMT_CHUNK * pfcFmt = reinterpret_cast<const PW_FMT_CHUNK *>(vBuffer.data());
						if ( !pfcFmt ) { return false; }

						if ( !LoadFmt( pfcFmt ) ) { return false; }
						break;
					}
				}
			}
			const uint32_t ui32FrameSize = uint32_t( m_uiNumChannels ) * m_uiBytesPerSample;

			// Size the "data" chunk before anything else so that "smpl" loop points can be validated against it, whatever the chunk order.
			//	A truncated file (for example a recording that was never closed) only uses the bytes that actually exist.
			for ( size_t I = 0; I < ceChunks.size(); ++I ) {
				if ( ceChunks[I].u.uiName == PW_C_DATA ) {
					if ( !ui32FrameSize ) { return false; }		// No (valid) "fmt " chunk.
					_sbStream.MovePointerTo( ceChunks[I].uiOffset );
					if ( _sbStream.Remaining() < sizeof( PW_CHUNK_HEADER ) ) { return false; }
					ceChunks[I].uiSize = static_cast<uint32_t>(std::min<uint64_t>( ceChunks[I].uiSize, _sbStream.Remaining() - sizeof( PW_CHUNK_HEADER ) ));
					m_ui32OriginalSampleCount = ceChunks[I].uiSize / ui32FrameSize;
				}
			}
			
			for ( size_t I = 0; I < ceChunks.size(); ++I ) {
				switch ( ceChunks[I].u.uiName ) {
					case PW_C_DATA : {		// "data"
						if ( !(_ui32LoadFlags & PW_LF_DATA) ) { break; }

						
						_ui32EndSample = std::min<uint32_t>( _ui32EndSample, ceChunks[I].uiSize / (m_uiNumChannels * m_uiBytesPerSample) );
						_ui32StartSample = std::min<uint32_t>( _ui32StartSample, _ui32EndSample );
						size_t sTotalSamples = std::min<size_t>( _ui32EndSample - _ui32StartSample, ceChunks[I].uiSize / (m_uiNumChannels * m_uiBytesPerSample) - _ui32StartSample );

						ceChunks[I].uiOffset += _ui32StartSample * (m_uiNumChannels * m_uiBytesPerSample);
						ceChunks[I].uiSize = static_cast<uint32_t>(sTotalSamples * (m_uiNumChannels * m_uiBytesPerSample));

						PW_LAOD_SECTION;
						PW_DATA_CHUNK * pfcData = reinterpret_cast<PW_DATA_CHUNK *>(vBuffer.data());
						if ( !pfcData ) { return false; }
						pfcData->chHeader.u.uiId = ceChunks[I].u.uiName;
						pfcData->chHeader.uiSize = ceChunks[I].uiSize;
						
						if ( !LoadData( pfcData, 0, 0xFFFFFFFF ) ) { return false; }
						break;
					}
					case PW_C_SMPL : {		// "smpl"
						if ( !(_ui32LoadFlags & PW_LF_SMPL) ) { break; }
						PW_LAOD_SECTION;
						const PW_SMPL_CHUNK * pfcSmpl = reinterpret_cast<const PW_SMPL_CHUNK *>(vBuffer.data());
						if ( !pfcSmpl ) { return false; }

						if ( !LoadSmpl( pfcSmpl ) ) { return false; }
						break;
					}
					case PW_C_LIST : {		// "LIST"
						if ( !(_ui32LoadFlags & PW_LF_LIST) ) { break; }
						PW_LAOD_SECTION;
						const PW_LIST_CHUNK * plcList = reinterpret_cast<const PW_LIST_CHUNK *>(vBuffer.data());
						if ( !plcList ) { return false; }

						if ( !LoadList( plcList ) ) { return false; }
						break;
					}
					case PW_C_ID3_ : {		// "id3 "
						if ( !(_ui32LoadFlags & PW_LF_ID3) ) { break; }
						PW_LAOD_SECTION;
						const PW_ID3_CHUNK * picId3 = reinterpret_cast<const PW_ID3_CHUNK *>(vBuffer.data());
						if ( !picId3 ) { return false; }

						if ( !LoadId3( picId3 ) ) { return false; }
						break;
					}
					case PW_C_INST : {		// "inst"
						if ( !(_ui32LoadFlags & PW_LF_INST) ) { break; }
						PW_LAOD_SECTION;
						const PW_INST_CHUNK * picInst = reinterpret_cast<const PW_INST_CHUNK *>(vBuffer.data());
						if ( !picInst ) { return false; }

						if ( !LoadInst( picInst ) ) { return false; }
						break;
					}
				}
			}
			
		//} while ( _sbStream.Remaining() );
		
		return true;
	}

	/**
	 * Loads a WAV file from memory.  This is just an in-memory version of the file.
	 *
	 * \param _vData The in-memory file to load.
	 * \param _ui32LoadFlags The loading flags, which allow to not load large portions of the file.
	 * \param _ui32StartSample The first sample to load.
	 * \param _ui32EndSample The last sample (exclusive) to load.
	 * \return Returns true if the file is a valid WAV file.
	 */
	bool CWavFile::LoadFromMemory( const std::vector<uint8_t> &_vData, uint32_t _ui32LoadFlags, uint32_t _ui32StartSample, uint32_t _ui32EndSample ) {
#if 0
		// Since the whole file is already in memory, using a stream would be less efficient, since it will make another copy of the samples
		//	before loading them.
		CStream sStream( const_cast<std::vector<uint8_t> &>(_vData) );
		return LoadFromStream( sStream, _ui32LoadFlags, _ui32StartSample, _ui32EndSample );
#else
		Reset();
		size_t stOffset = 0;
		const uint32_t * pui32Scratch;

#define PW_PTR_SIZE( TYPE, OFFSET, SIZE )			((OFFSET) + SIZE) > _vData.size() ? nullptr : reinterpret_cast<const TYPE *>(&_vData[OFFSET])
#define PW_PTR( TYPE, OFFSET )						PW_PTR_SIZE( TYPE, OFFSET, sizeof( TYPE ) )
#define PW_READ_32( VAL )							pui32Scratch = PW_PTR( uint32_t, stOffset ); if ( !pui32Scratch ) { return false; } stOffset += sizeof( uint32_t ); VAL = (*pui32Scratch)
#define PW_READ_STRUCT( TYPE, VAL )				VAL = PW_PTR( TYPE, stOffset ); if ( !VAL ) { return false; } stOffset += sizeof( TYPE )


		//do {
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
				stOffset += ceThis.uiSize + (ceThis.uiSize & 1);	// Chunks are word-aligned; the pad byte is not part of the size.
			}

			// Do the format chunk first.
			for ( size_t I = 0; I < ceChunks.size(); ++I ) {
				switch ( ceChunks[I].u.uiName ) {
					case PW_C_FMT_ : {		// "fmt "
						const PW_FMT_CHUNK * pfcFmt = PW_PTR_SIZE( PW_FMT_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize + sizeof( PW_CHUNK_HEADER ) );
						if ( !pfcFmt ) { return false; }

						if ( !LoadFmt( pfcFmt ) ) { return false; }
						break;
					}
				}
			}

			for ( size_t I = 0; I < ceChunks.size(); ++I ) {
				switch ( ceChunks[I].u.uiName ) {
					case PW_C_DATA : {		// "data"
						const PW_DATA_CHUNK * pfcData = PW_PTR_SIZE( PW_DATA_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize + sizeof( PW_CHUNK_HEADER ) );
						if ( !pfcData ) { return false; }
						if ( !(m_uiNumChannels * m_uiBytesPerSample) ) { return false; }		// No (valid) "fmt " chunk.
						m_ui32OriginalSampleCount = pfcData->chHeader.uiSize / (m_uiNumChannels * m_uiBytesPerSample);

						if ( !(_ui32LoadFlags & PW_LF_DATA) ) { break; }
						if ( !LoadData( pfcData, _ui32StartSample, _ui32EndSample ) ) { return false; }
						break;
					}
					case PW_C_SMPL : {		// "smpl"
						if ( !(_ui32LoadFlags & PW_LF_SMPL) ) { break; }
						const PW_SMPL_CHUNK * pfcSmpl = PW_PTR_SIZE( PW_SMPL_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize + sizeof( PW_CHUNK_HEADER ) );
						if ( !pfcSmpl ) { return false; }

						if ( !LoadSmpl( pfcSmpl ) ) { return false; }
						break;
					}
					case PW_C_LIST : {		// "LIST"
						if ( !(_ui32LoadFlags & PW_LF_LIST) ) { break; }
						const PW_LIST_CHUNK * plcList = PW_PTR_SIZE( PW_LIST_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize + sizeof( PW_CHUNK_HEADER ) );
						if ( !plcList ) { return false; }

						if ( !LoadList( plcList ) ) { return false; }
						break;
					}
					case PW_C_ID3_ : {		// "id3 "
						if ( !(_ui32LoadFlags & PW_LF_ID3) ) { break; }
						const PW_ID3_CHUNK * picList = PW_PTR_SIZE( PW_ID3_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize + sizeof( PW_CHUNK_HEADER ) );
						if ( !picList ) { return false; }

						if ( !LoadId3( picList ) ) { return false; }
						break;
					}
					case PW_C_INST : {		// "inst"
						if ( !(_ui32LoadFlags & PW_LF_INST) ) { break; }
						const PW_INST_CHUNK * picList = PW_PTR_SIZE( PW_INST_CHUNK, ceChunks[I].uiOffset, ceChunks[I].uiSize + sizeof( PW_CHUNK_HEADER ) );
						if ( !picList ) { return false; }

						if ( !LoadInst( picList ) ) { return false; }
						break;
					}
				}
			}

		//} while ( stOffset < _vData.size() );


#undef PW_READ_STRUCT
#undef PW_READ_32
#undef PW_PTR
		return true;
#endif
	}

	/**
	 * Opens a stream to a WAV file.  Samples can be written over time.
	 * 
	 * \param _stfoFileOptions Contains all of the settings for streaming to a file.
	 * \param _ui32Hz The WAV Hz.
	 * \param _stBufferSize The buffer size (in samples).
	 * \return Returns true if the file could initially be created and all parameters are correct.
	 **/
	bool CWavFile::StreamToFile( const PW_STREAM_TO_FILE_OPTIONS &_stfoFileOptions, uint32_t _ui32Hz,
		size_t _stBufferSize ) {
		PW_STREAM_COND_PARM scpStartParm;
		scpStartParm.ui64Parm = 0;
		switch ( _stfoFileOptions.scStartCondition ) {
			case PW_SC_START_AT_SAMPLE : {
				scpStartParm.ui64Parm = _stfoFileOptions.ui64StartParm;
				break;
			}
			case PW_SC_ZERO_FOR_DURATION : {
				scpStartParm.dParm = _stfoFileOptions.dStartParm;
				break;
			}
		}
		PW_STREAM_COND_PARM scpStopParm;
		scpStopParm.ui64Parm = 0;
		switch ( _stfoFileOptions.seEndCondition ) {
			case PW_EC_END_AT_SAMPLE : {
				scpStopParm.ui64Parm = _stfoFileOptions.ui64EndParm;
				break;
			}
			case PW_EC_ZERO_FOR_DURATION : {
				scpStopParm.dParm = _stfoFileOptions.dEndParm;
				break;
			}
			case PW_EC_DURATION : {
				scpStopParm.dParm = _stfoFileOptions.dEndParm;
				break;
			}
		}

		std::filesystem::path pAbsolutePath = std::filesystem::absolute( std::filesystem::path( _stfoFileOptions.wsPath ) );

		StopStream();
		m_sStream.bStreaming = false;
		m_sStream.bMeta = false;

		std::unique_lock<std::mutex> ulLock( m_sStream.mMutex );
		
		switch ( _stfoFileOptions.scStartCondition ) {
			case PW_SC_NONE : {
				m_sStream.pfStartCondFunc = &CWavFile::StartCondFunc_None;
				break;
			}
			case PW_SC_START_AT_SAMPLE : {
				m_sStream.pfStartCondFunc = &CWavFile::StartCondFunc_StartAtSample;
				m_sStream.cdStartData.ui64Parm0 = scpStartParm.ui64Parm;
				break;
			}
			case PW_SC_FIRST_NON_ZERO : {
				m_sStream.pfStartCondFunc = &CWavFile::StartCondFunc_FirstNonZero;
				break;
			}
			case PW_SC_ZERO_FOR_DURATION : {
				m_sStream.pfStartCondFunc = &CWavFile::StartCondFunc_ZeroForDuration;
				m_sStream.cdStartData.ui64Counter = 0;
				m_sStream.cdStartData.ui64Parm0 = uint64_t( std::round( scpStartParm.dParm * _ui32Hz ) );
				break;
			}
			default : {
				std::wprintf( L"Unrecognized start condition %X: %s.\r\n", _stfoFileOptions.scStartCondition, _stfoFileOptions.wsPath.c_str() );
				return false;
			}
		}
		
		switch ( _stfoFileOptions.seEndCondition ) {
			case PW_EC_NONE : {
				m_sStream.pfEndCondFunc = &CWavFile::EndCondFunc_None;
				break;
			}
			case PW_EC_END_AT_SAMPLE : {
				m_sStream.pfEndCondFunc = &CWavFile::EndCondFunc_EndAtSample;
				m_sStream.cdStopData.ui64Parm0 = scpStopParm.ui64Parm;
				break;
			}
			case PW_EC_ZERO_FOR_DURATION : {
				m_sStream.pfEndCondFunc = &CWavFile::EndCondFunc_ZeroForDuration;
				m_sStream.cdStopData.ui64Counter = 0;
				m_sStream.cdStopData.ui64Parm0 = uint64_t( std::round( scpStopParm.dParm * _ui32Hz ) );
				break;
			}
			case PW_EC_DURATION : {
				m_sStream.pfEndCondFunc = &CWavFile::EndCondFunc_Duration;
				m_sStream.cdStopData.ui64Parm0 = uint64_t( std::round( scpStopParm.dParm * _ui32Hz ) );
				break;
			}
			default : {
				std::wprintf( L"Unrecognized stop condition %X: %s.\r\n", _stfoFileOptions.seEndCondition, _stfoFileOptions.wsPath.c_str() );
				return false;
			}
		}
		m_sStream.pfAddSampleFunc = &CWavFile::AddSample_CheckStartCond;

		m_sStream.fFormat = _stfoFileOptions.fFormat;
		m_sStream.ui16Bits = static_cast<uint16_t>(_stfoFileOptions.ui32Bits);
		m_sStream.ui16Channels = 1;
		m_sStream.bDither = _stfoFileOptions.bDither;
		m_sStream.ui32Hz = _ui32Hz;
		m_sStream.dDitherError = 0.0;

		if ( m_sStream.fFormat == PW_F_IEEE_FLOAT ) {
			m_sStream.ui16Bits = 32;
		}
		
		if ( !CreateStreamFile( pAbsolutePath.generic_u8string().c_str() ) ) {
			m_sStream.sfFile.Close();
			std::wprintf( L"Failed to create WAV stream file: %s.\r\n", _stfoFileOptions.wsPath.c_str() );
			return false;
		}

		
		if ( _stfoFileOptions.bMetaEnabled && _stfoFileOptions.wsMetaPath.size() && _stfoFileOptions.pfMetaFunc && _stfoFileOptions.pfMetaThreadFunc ) {
			std::filesystem::path pAbsoluteMetaPath = std::filesystem::absolute( std::filesystem::path( _stfoFileOptions.wsMetaPath ) );
			if ( !CreateStreamMetaFile( pAbsoluteMetaPath.generic_u8string().c_str() ) ) {
				m_sStream.sfFile.Close();
				m_sStream.sfMetaFile.Close();
				return false;
			}
			m_sStream.pfMetaFunc = _stfoFileOptions.pfMetaFunc;
			m_sStream.pfMetaThreadFunc = _stfoFileOptions.pfMetaThreadFunc;
			m_sStream.pvMetaParm = _stfoFileOptions.pvMetaParm;
			m_sStream.ui64MetaParm = _stfoFileOptions.ui64MetaParm;
			m_sStream.i32MetaFormat = _stfoFileOptions.i32MetaFormat;
			m_sStream.ui64MetaWritten = 0;
			m_sStream.ui64MetaThreadParm = 0;
			m_sStream.vMetaThreadScratch.clear();
			m_sStream.bMeta = true;
		}
		
		try {
			m_sStream.stBufferSize = _stBufferSize * m_sStream.ui16Channels;
			m_sStream.vCurBuffer.clear();
			m_sStream.vCurBuffer.reserve( m_sStream.stBufferSize );
		}
		catch ( ... ) {
			m_sStream.sfFile.Close();
			m_sStream.sfMetaFile.Close();
			std::wprintf( L"Out of memory creating buffers for WAV stream: %s.\r\n", _stfoFileOptions.wsPath.c_str() );
			return false;
		}
		m_sStream.bEnd = false;
		switch ( m_sStream.fFormat ) {
			case PW_F_PCM : {
				switch ( m_sStream.ui16Bits ) {
					case 8 : {
						m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToPcm8;
#ifdef __AVX2__
						if ( CUtilities::IsAvx2Supported() ) {
							m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToPcm8_AVX2;
						}
#endif	// #ifdef __AVX2__
#ifdef __AVX512F__
						if ( CUtilities::IsAvx512FSupported() ) {
							m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToPcm8_AVX512;
						}
#endif	// #ifdef __AVX512F__
						break;
					}
					case 16 : {
						m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToPcm16;
#ifdef __AVX2__
						if ( CUtilities::IsAvx2Supported() ) {
							m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToPcm16_AVX2;
						}
#endif	// #ifdef __AVX2__
#ifdef __AVX512F__
						if ( CUtilities::IsAvx512FSupported() ) {
							m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToPcm16_AVX512;
						}
#endif	// #ifdef __AVX512F__
						if ( m_sStream.bDither ) {
							m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToPcm16_Dither;
						}
						break;
					}
					case 32 : {
						m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToPcm32;
						break;
					}
					case 24 : {
						m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToPcm24;
#ifdef __AVX2__
						if ( CUtilities::IsAvx2Supported() ) {
							m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToPcm24_AVX2;
						}
#endif	// #ifdef __AVX2__
#ifdef __AVX512F__
						if ( CUtilities::IsAvx512FSupported() ) {
							m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToPcm24_AVX512;
						}
#endif	// #ifdef __AVX512F__
						break;
					}
					default : {
						m_sStream.sfFile.Close();
						m_sStream.sfMetaFile.Close();
						std::wprintf( L"Unrecognized PCM bits %X: %s.\r\n", m_sStream.ui16Bits, _stfoFileOptions.wsPath.c_str() );
						return false;
					}
				}
				break;
			}
			case PW_F_IEEE_FLOAT : {
				m_sStream.pfCvtAndWriteFunc = &CWavFile::BatchF32ToF32;
				break;
			}
			default : {
				m_sStream.sfFile.Close();
				m_sStream.sfMetaFile.Close();
				std::wprintf( L"Unrecognized WAV format %X: %s.\r\n", m_sStream.fFormat, _stfoFileOptions.wsPath.c_str() );
				return false;
			}
		}
		
		m_sStream.wsPath = _stfoFileOptions.wsPath;

		m_sStream.tThread = std::thread( &CWavFile::StreamWriterThread, this );
		if ( m_sStream.bMeta ) {
			m_sStream.tMetaThread = std::thread( &CWavFile::MetadataWriterThread, this );
		}

		m_sStream.ui64SamplesReceived = m_sStream.ui64SamplesWritten = 0;
		m_sStream.bAdding = false;
		m_sStream.ui64MetaWritten = 0;
		m_sStream.bStreaming = true;
		return true;
	}

	/**
	 * Stops the streaming file.
	 **/
	void CWavFile::StopStream() {
		{
			std::unique_lock<std::mutex> ulLock( m_sStream.mMutex );
			if ( m_sStream.sfFile.IsOpen() ) {
				if ( m_sStream.bStreaming ) {
					if ( !m_sStream.vCurBuffer.empty() ) {
						m_sStream.qBufferQueue.push( std::move( m_sStream.vCurBuffer ) );
						m_sStream.vCurBuffer.clear();
					}
					m_sStream.bEnd = true;	// File will be closed in the writer thread.
				}
			}
		}

		m_sStream.cvCondition.notify_one();
        if ( m_sStream.tThread.joinable()) {
            m_sStream.tThread.join();
        }


		std::unique_lock<std::mutex> ulLock( m_sStream.mMutex );
		{
			m_sStream.bStreaming = false;
		}
	}

	/**
	 * Adds a sample to the stream.
	 * 
	 * \param _fSample The sample to add.
	 **/
	void CWavFile::AddStreamSample( float _fSample ) {
		std::unique_lock<std::mutex> ulLock( m_sStream.mMutex );

		if PW_LIKELY( m_sStream.bStreaming && !m_sStream.bEnd ) {
			if ( (*m_sStream.pfAddSampleFunc)( _fSample, m_sStream ) ) {
				m_sStream.bAdding = true;
				m_sStream.vCurBuffer.push_back( _fSample );
				uint64_t ui64TotalWillWrite = ++m_sStream.ui64SamplesWritten;

				if ( m_sStream.bMeta && m_sStream.pfMetaFunc ) {
					std::unique_lock<std::mutex> ulLockMeta( m_sStream.mMetaMutex );
					AddMetaData();
				}

				const bool bFileFull = ui64TotalWillWrite >= m_sStream.ui64MaxSamples;									// The RIFF size cannot grow past 32 bits.
				if PW_UNLIKELY( bFileFull ||
					m_sStream.vCurBuffer.size() == m_sStream.stBufferSize ) {													// Buffer limit reached.
					// Efficiently pass the buffer off to the writer thread.
					m_sStream.qBufferQueue.push( std::move( m_sStream.vCurBuffer ) );
					// Create a new buffer and reserve space for efficiency.
					m_sStream.vCurBuffer.clear();
					m_sStream.vCurBuffer.reserve( m_sStream.stBufferSize );
					if PW_UNLIKELY( bFileFull ) {
						// No more samples fit in a WAV file.  End the stream; the writer thread finalizes and closes the file.
						m_sStream.bEnd = true;
					}
					// Notify the writer thread that a full buffer is ready.
					m_sStream.cvCondition.notify_one();
				}
			}
			else if ( m_sStream.bEnd ) {
				m_sStream.qBufferQueue.push( std::move( m_sStream.vCurBuffer ) );
				// Create a new buffer and reserve space for efficiency.
				m_sStream.vCurBuffer.clear();
				// Notify the writer thread that a full buffer is ready.
				m_sStream.cvCondition.notify_one();
			}
			++m_sStream.ui64SamplesReceived;
		}
	}

	/**
	 * Called to update the metadata stream output.  Call immediately after calling AddStreamSample().
	 **/
	void CWavFile::AddMetaData() {
		//if PW_LIKELY( m_sStream.bAdding && m_sStream.bMeta ) {
			if PW_LIKELY( m_sStream.bStreaming && !m_sStream.bEnd ) {
				if ( (*m_sStream.pfMetaFunc)( m_sStream.pvMetaParm, m_sStream ) ) {
					if PW_UNLIKELY( m_sStream.vMetaBuffer.size() >= (1024 * 0x20) ) {
						m_sStream.qMetaBufferQueue.push( std::move( m_sStream.vMetaBuffer ) );
						// Create a new buffer and reserve space for efficiency.
						m_sStream.vMetaBuffer.clear();
						m_sStream.vMetaBuffer.reserve( (1024 * 0x20) );
						// Notify the writer thread that a full buffer is ready.
						m_sStream.cvMetaCondition.notify_one();
					}

					++m_sStream.ui64MetaWritten;
				}
			}
			else {
				// Stream is closing.  Push anything remaining.
				if ( m_sStream.vMetaBuffer.size() ) {
					m_sStream.qMetaBufferQueue.push( std::move( m_sStream.vMetaBuffer ) );
					// Create a new buffer and reserve space for efficiency.
					m_sStream.vMetaBuffer.clear();
					// Notify the writer thread that a full buffer is ready.
					m_sStream.cvMetaCondition.notify_one();
				}
			}
		//}
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
		m_ui32OriginalSampleCount = 0;
		m_uiNumChannels = 0;
		m_uiSampleRate = 0;
		m_uiBytesPerSample = 0;
		m_uiBitsPerSample = 0;
		m_uiBaseNote = 64;
		m_fFormat = PW_F_PCM;
		m_ieInstEntry = PW_INST_ENTRY();
		m_bHasInst = false;
		m_ui8Id3Version = 3;
	}

#pragma optimize( "gt", on )

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
			{
			//const struct PW_TABLE {
			//	const char8_t *					pcReplaceMe;
			//	const char8_t *					pcWithMe;
			//} tTable[] = {
			//	{ u8"\u2019", u8"'" },			// ’
			//	{ u8"\u2018", u8"'" },			// ‘
			//	{ u8"\u2026", u8"..." },		// …
			//	{ u8"\u014C", u8"O" },			// Ō
			//	{ u8"\u00A0", u8" " },
			//	{ u8"\u00E9", u8"e" },			// é
			//	{ u8"\u201C", u8"\"" },			// “
			//	{ u8"\u201D", u8"\"" },			// ”
			//	{ u8"\u2153", u8"(1/3rd)" },	// ⅓
			//	{ u8"\u016B", u8"u" },			// ū
				//	{ u8"\u014D", u8"o" },			// ō

				//	/*{ u8"ぁ", "a" },
				//	{ u8"あ", "A" },

				//	{ u8"ぃ", "i" },
				//	{ u8"い", "I" },

				//	{ u8"ぅ", "u" },
				//	{ u8"う", "U" },

				//	{ u8"ぇ", "e" },
				//	{ u8"え", "E" },

				//	{ u8"ぉ", "o" },
				//	{ u8"お", "O" },

				//	{ u8"か", "Ka" },
				//	{ u8"が", "Ga" },

				//	{ u8"き", "Ki" },
				//	{ u8"ぎ", "Gi" },

				//	{ u8"く", "Ku" },
				//	{ u8"ぐ", "Gu" },

				//	{ u8"け", "Ke" },
				//	{ u8"げ", "Ge" },

				//	{ u8"こ", "Ko" },
				//	{ u8"ご", "Go" },

				//	{ u8"さ", "Sa" },
				//	{ u8"ざ", "Za" },

				//	{ u8"し", "Shi" },
				//	{ u8"じ", "Ji" },

				//	{ u8"す", "Su" },
				//	{ u8"ず", "Zu" },

				//	{ u8"せ", "Se" },
				//	{ u8"ぜ", "Ze" },

				//	{ u8"そ", "So" },
				//	{ u8"ぞ", "Zo" },

				//	{ u8"た", "Ta" },
				//	{ u8"だ", "Da" },

				//	{ u8"ち", "Chi" },
				//	{ u8"ぢ", "Ji" },

				//	{ u8"っ", "-" },
				//	{ u8"つ", "Tsu" },
				//	{ u8"づ", "Zu" },

				//	{ u8"て", "Te" },
				//	{ u8"で", "De" },

				//	{ u8"と", "To" },
				//	{ u8"ど", "Do" },

				//	{ u8"な", "Na" },
				//	{ u8"に", "Ni" },
				//	{ u8"ぬ", "Nu" },
				//	{ u8"ね", "Ne" },
				//	{ u8"の", "No" },

				//	{ u8"は", "Ha" },
				//	{ u8"ば", "Ba" },
				//	{ u8"ぱ", "Pa" },

				//	{ u8"ひ", "Hi" },
				//	{ u8"び", "Bi" },
				//	{ u8"ぴ", "Pi" },

				//	{ u8"ふ", "Hu" },
				//	{ u8"ぶ", "Bu" },
				//	{ u8"ぷ", "Pu" },

				//	{ u8"へ", "He" },
				//	{ u8"べ", "Be" },
				//	{ u8"ぺ", "Pe" },

				//	{ u8"ほ", "Ho" },
				//	{ u8"ぼ", "Bo" },
				//	{ u8"ぽ", "Po" },

				//	{ u8"ま", "Ma" },
				//	{ u8"み", "Mi" },
				//	{ u8"む", "Mu" },
				//	{ u8"め", "Me" },
				//	{ u8"も", "Mo" },

				//	{ u8"ゃ", "ya" },
				//	{ u8"や", "Ya" },

				//	{ u8"ゅ", "yu" },
				//	{ u8"ゆ", "Yu" },

				//	{ u8"ょ", "yo" },
				//	{ u8"よ", "Yo" },

				//	{ u8"ら", "Ra" },
				//	{ u8"り", "Ri" },
				//	{ u8"る", "Ru" },
				//	{ u8"れ", "Re" },
				//	{ u8"ろ", "Ro" },

				//	{ u8"ゎ", "wa" },
				//	{ u8"わ", "Wa" },

				//	{ u8"ゐ", "Wi" },

				//	{ u8"ゑ", "We" },

				//	{ u8"を", "Wo" },

				//	{ u8"ん", "N" },

				//	{ u8"ゔ", "Vu" },

				//	{ u8"ゕ", "ka" },
				//	{ u8"ゖ", "ke" },			

				//	{ u8"ぷ", "Pu" },
				//	{ u8"よ", "Yo" },


				//	{ u8"ワ", "Wa" },
				//	{ u8"ン", "N" },
				//	{ u8"ダ", "Da" },
				//	{ u8"ー", "-" },
				//	{ u8"プ", "Pu" },
				//	{ u8"ロ", "Ro" },
				//	{ u8"ジ", "Ji" },
				//	{ u8"ェ", "e" },

				//	{ u8"ジェ", "Je" },

				//	{ u8"ク", "Ku" },
				//	{ u8"ト", "To" },
				//	{ u8"コ", "Ko" },
				//	{ u8"ル", "Ru" },
				//	{ u8"ジョ", "Jo" },
				//	{ u8"ゼ", "Ze" },
				//	{ u8"ゼ", "Ze" },
				//	{ u8"ゼッ", "Ze" },
				//	{ u8"ゼッ", "Ze" },
				//	{ u8"ズ", "Zu" },

				//	{ u8"森", "Mori" },
				//	{ u8"彰", "Aki" },
				//	{ u8"彦", "Hiko" },
				//	{ u8"彰彦", "Akihiko" },
				//	{ u8"魔獣使い伝説", "Maju Tsukai Densetsu" },
				//	{ u8"一郎", "Ichiro" },
				//	{ u8"根本", "Nemoto" },

				//	{ u8"Ｒ", "R" },
				//	{ u8"Ｅ", "E" },
				//	{ u8"Ｓ", "S" },
				//	{ u8"Ｕ", "U" },
				//	{ u8"Ｌ", "L" },
				//	{ u8"Ｔ", "T" },*/
				//};
				//for ( auto I = sizeof( tTable ) / sizeof( tTable[0] ); I--; ) {
				//	sCopy = CUtilities::Replace<std::u8string>( sCopy, tTable[I].pcReplaceMe, tTable[I].pcWithMe );
				//};
			}
			PW_LIST_ENTRY lsEntry;
			lsEntry.u.uiIfoId = _uiId;
			lsEntry.sText = sCopy;
			lsEntry.sText.push_back( '\0' );
			if ( lsEntry.sText.size() & 1 ) {
				// Make it an even number of characters because a hard-coded 0 will be printed into the file.
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
		if ( _pfcChunk->chHeader.uiSize < 16 ) { return false; }
		uint16_t ui16Format = _pfcChunk->uiAudioFormat;
		if ( ui16Format == PW_F_EXTENSIBLE ) {
			// WAVE_FORMAT_EXTENSIBLE: cbSize >= 22 is followed by wValidBitsPerSample, dwChannelMask, and the SubFormat GUID, whose first 2 bytes
			//	are the actual format tag (KSDATAFORMAT_SUBTYPE_PCM = 1, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = 3).
			if ( _pfcChunk->chHeader.uiSize < 40 || _pfcChunk->uiExtraParamSize < 22 ) { return false; }
			std::memcpy( &ui16Format, reinterpret_cast<const uint8_t *>(_pfcChunk) + sizeof( PW_CHUNK_HEADER ) + 24, sizeof( ui16Format ) );
		}
		// Sample sizes that are not whole bytes (ADPCM etc.) cannot be addressed by this class.
		if ( !_pfcChunk->uiNumChannels || !_pfcChunk->uiBitsPerSample || (_pfcChunk->uiBitsPerSample & 7) ) { return false; }
		m_uiNumChannels = _pfcChunk->uiNumChannels;
		m_uiSampleRate = _pfcChunk->uiSampleRate;
		m_uiBitsPerSample = _pfcChunk->uiBitsPerSample;
		m_uiBytesPerSample = m_uiBitsPerSample / 8;
		m_fFormat = static_cast<PW_FORMAT>(ui16Format);
		return true;
	}

	/**
	 * Loads the "data" chunk.
	 *
	 * \param _pdcChunk The chunk of data to load
	 * \return Returns true if everything loaded fine.
	 */
	bool CWavFile::LoadData( const PW_DATA_CHUNK * _pdcChunk, uint32_t _ui32StartSample, uint32_t _ui32EndSample ) {
		const uint32_t ui32FrameSize = uint32_t( m_uiNumChannels ) * m_uiBytesPerSample;
		if ( !ui32FrameSize ) { return false; }		// No (valid) "fmt " chunk.
		_ui32EndSample = std::min<uint32_t>( _ui32EndSample, _pdcChunk->chHeader.uiSize / ui32FrameSize );
		_ui32StartSample = std::min<uint32_t>( _ui32StartSample, _ui32EndSample );
		size_t sTotalSamples = std::min<size_t>( _ui32EndSample - _ui32StartSample, _pdcChunk->chHeader.uiSize / ui32FrameSize - _ui32StartSample ) * m_uiNumChannels;

		try {
			m_vSamples.resize( sTotalSamples * m_uiBytesPerSample );
		}
		catch ( ... ) { return false; }
		if ( m_vSamples.size() != sTotalSamples * m_uiBytesPerSample ) { return false; }
		
		if ( m_vSamples.size() ) {
			std::memcpy( m_vSamples.data(), _pdcChunk->ui8Data + (size_t( _ui32StartSample ) * ui32FrameSize), m_vSamples.size() );
		}
		return true;
	}

	/**
	 * Loads the "smpl" chunk.
	 *
	 * \param _pscChunk The chunk of data to load
	 * \return Returns true if everything loaded fine.
	 */
	bool CWavFile::LoadSmpl( const PW_SMPL_CHUNK * _pscChunk ) {
		constexpr uint32_t ui32HeaderSize = sizeof( PW_SMPL_CHUNK ) - sizeof( PW_CHUNK_HEADER ) - sizeof( PW_LOOP_POINT );	// 36.
		if ( _pscChunk->chHeader.uiSize < ui32HeaderSize ) { return true; }	// Malformed; ignore it rather than failing the whole file.
		m_uiBaseNote = _pscChunk->uiMIDIUnityNote;
		// Never read more loops than the chunk actually holds.
		const size_t stLoops = std::min<size_t>( _pscChunk->uiNumSampleLoops, (_pscChunk->chHeader.uiSize - ui32HeaderSize) / sizeof( PW_LOOP_POINT ) );
		for ( size_t I = 0; I < stLoops; ++I ) {
			PW_LOOP_POINT lpLoop;
			std::memcpy( &lpLoop, reinterpret_cast<const uint8_t *>(_pscChunk->lpLoops) + I * sizeof( PW_LOOP_POINT ), sizeof( lpLoop ) );
			// Loop points are sample frames, not bytes.
			if ( lpLoop.uiStart <= lpLoop.uiEnd && (!m_ui32OriginalSampleCount || lpLoop.uiEnd < m_ui32OriginalSampleCount) ) {
				m_vLoops.push_back( lpLoop );
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
		if ( _plcChunk->chHeader.uiSize < sizeof( _plcChunk->u ) ) { return true; }
		const uint8_t * pui8Data = _plcChunk->ui8Data;
		const uint8_t * pui8End = pui8Data + (_plcChunk->chHeader.uiSize - sizeof( _plcChunk->u ));
		switch ( _plcChunk->u.uiTypeId ) {
			case PW_C_INFO : {				// INFO
				while ( pui8End - pui8Data >= 8 ) {
					PW_LIST_ENTRY lsThis;
					std::memcpy( &lsThis.u.uiIfoId, pui8Data, sizeof( uint32_t ) );
					pui8Data += sizeof( uint32_t );
					uint32_t ui32Size;
					std::memcpy( &ui32Size, pui8Data, sizeof( uint32_t ) );
					pui8Data += sizeof( uint32_t );
					if ( ui32Size > size_t( pui8End - pui8Data ) ) { break; }	// Malformed; keep what was read.
					lsThis.sText.assign( reinterpret_cast<const char8_t *>(pui8Data), ui32Size );
					pui8Data += ui32Size;
					// Sub-chunks are word-aligned: skip the pad byte after an odd-sized entry.
					if ( (ui32Size & 1) && pui8Data < pui8End ) { ++pui8Data; }
					m_vListEntries.push_back( lsThis );
				}
				break;
			}
			default : {
				// "adtl" and other list types are not used by this class; ignore them rather than failing the whole file.
				break;
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
		// Anything this loader cannot represent faithfully is ignored rather than failing the whole file.
		constexpr uint32_t ui32TagHeader = 10;
		const uint32_t ui32ChunkSize = _picChunk->chHeader.uiSize;
		if ( ui32ChunkSize < ui32TagHeader ) { return true; }
		if ( _picChunk->sName[0] != u8'I' || _picChunk->sName[1] != u8'D' || _picChunk->sName[2] != u8'3' ) { return true; }
		const uint8_t ui8Major = static_cast<uint8_t>(_picChunk->ui16Version & 0xFF);
		if ( ui8Major != 3 && ui8Major != 4 ) { return true; }
		if ( _picChunk->ui8Flags & 0x80 ) { return true; }				// Unsynchronisation is not supported.
		const uint32_t ui32Len = std::min<uint32_t>( DecodeSize( _picChunk->ui32Size ), ui32ChunkSize - ui32TagHeader );
		const uint8_t * pui8Data = _picChunk->ui8Data;
		auto ReadBe32 = [&]( uint32_t _ui32Offset ) {
			return (uint32_t( pui8Data[_ui32Offset] ) << 24) | (uint32_t( pui8Data[_ui32Offset+1] ) << 16) | (uint32_t( pui8Data[_ui32Offset+2] ) << 8) | uint32_t( pui8Data[_ui32Offset+3] );
		};
		auto ReadRaw32 = [&]( uint32_t _ui32Offset ) {
			uint32_t ui32Val;
			std::memcpy( &ui32Val, &pui8Data[_ui32Offset], sizeof( ui32Val ) );
			return ui32Val;
		};

		uint32_t ui32Offset = 0;
		if ( _picChunk->ui8Flags & 0x40 ) {
			// Extended header.  v2.3: the size excludes its own 4 bytes.  v2.4: the size is sync-safe and includes itself.
			if ( ui32Len < 4 ) { return true; }
			ui32Offset = (ui8Major == 4) ? DecodeSize( ReadRaw32( 0 ) ) : (ReadBe32( 0 ) + 4);
		}

		std::vector<PW_ID3_ENTRY> vEntries;
		while ( ui32Offset <= ui32Len && ui32Len - ui32Offset >= ui32TagHeader ) {
			if ( pui8Data[ui32Offset] == 0 ) { break; }						// Padding.
			PW_ID3_ENTRY ieEntry;
			std::memcpy( &ieEntry.u.uiIfoId, &pui8Data[ui32Offset], sizeof( uint32_t ) );
			// Frame sizes are plain big-endian in v2.3 and sync-safe in v2.4.
			const uint32_t ui32Size = (ui8Major == 4) ? DecodeSize( ReadRaw32( ui32Offset + 4 ) ) : ReadBe32( ui32Offset + 4 );
			std::memcpy( &ieEntry.ui16Flags, &pui8Data[ui32Offset+8], sizeof( uint16_t ) );
			ui32Offset += ui32TagHeader;
			if ( ui32Size > ui32Len - ui32Offset ) { break; }
			ieEntry.sValue.assign( reinterpret_cast<const char8_t *>(&pui8Data[ui32Offset]), ui32Size );
			ui32Offset += ui32Size;
			vEntries.push_back( ieEntry );
		}
		m_vId3Entries.insert( m_vId3Entries.end(), vEntries.begin(), vEntries.end() );
		m_ui8Id3Version = ui8Major;
		return true;
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
		m_bHasInst = true;
		return true;
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
		fcChunk.uiAudioFormat = _psdSaveSettings ? _psdSaveSettings->fFormat : _fFormat;
		fcChunk.uiBitsPerSample = (_psdSaveSettings && _psdSaveSettings->uiBitsPerSample) ? _psdSaveSettings->uiBitsPerSample : BitsPerSample();
		if ( PW_F_IEEE_FLOAT == fcChunk.uiAudioFormat ) {
			if ( fcChunk.uiBitsPerSample != 64 ) { fcChunk.uiBitsPerSample = 32; }
			// Every non-PCM "fmt " chunk carries cbSize (0 here), making it 18 bytes.
			fcChunk.chHeader.uiSize = 18;
			fcChunk.uiExtraParamSize = 0;
		}
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
		PW_PUSH32( m_uiSampleRate ? static_cast<uint32_t>(std::round( 1.0e9 / m_uiSampleRate )) : 0 );	// Sample Period (nanoseconds).
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
		if ( !m_vListEntries.size() ) { return vRet; }
#define PW_PUSH32( VAL )		vRet.push_back( static_cast<uint8_t>((VAL) >> 0) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 8) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 16) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 24) )
		PW_PUSH32( PW_C_LIST );			// "LIST"
		uint32_t ui32Size = 4;
		for ( auto I = m_vListEntries.size(); I--; ) {
			const uint32_t ui32Text = static_cast<uint32_t>(m_vListEntries[I].sText.size());
			ui32Size += 8 + ui32Text + (ui32Text & 1);	// Sub-chunks are word-aligned.
		}
		PW_PUSH32( ui32Size );				// Size.
		PW_PUSH32( PW_C_INFO );			// "INFO"
		for ( size_t I = 0; I < m_vListEntries.size(); ++I ) {
			const uint32_t ui32Text = static_cast<uint32_t>(m_vListEntries[I].sText.size());
			PW_PUSH32( m_vListEntries[I].u.uiIfoId );
			PW_PUSH32( ui32Text );
			for ( size_t J = 0; J < m_vListEntries[I].sText.size(); ++J ) {
				vRet.push_back( static_cast<uint8_t>(m_vListEntries[I].sText[J]) );
			}
			if ( ui32Text & 1 ) { vRet.push_back( 0 ); }
		}
#undef PW_PUSH32
		return vRet;
	}

	/**
	 * Writes a file-image "inst" chunk to a vector, if an "inst" chunk was loaded.
	 *
	 * \return Returns the bytes that represent the "inst" chunk in a file, or an empty vector.
	 */
	std::vector<uint8_t> CWavFile::CreateInst() const {
		std::vector<uint8_t> vRet;
		if ( !m_bHasInst ) { return vRet; }
#define PW_PUSH32( VAL )		vRet.push_back( static_cast<uint8_t>((VAL) >> 0) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 8) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 16) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 24) )
		PW_PUSH32( PW_C_INST );			// "inst"
		PW_PUSH32( 7 );
		vRet.push_back( m_ieInstEntry.ui8UnshiftedNote );
		vRet.push_back( m_ieInstEntry.ui8FineTune );
		vRet.push_back( m_ieInstEntry.ui8Gain );
		vRet.push_back( m_ieInstEntry.ui8LowNote );
		vRet.push_back( m_ieInstEntry.ui8HiNote );
		vRet.push_back( m_ieInstEntry.ui8LowVel );
		vRet.push_back( m_ieInstEntry.ui8HiVel );
		vRet.push_back( 0 );				// Pad byte.
#undef PW_PUSH32
		return vRet;
	}

	/**
	 * Writes file-image "DISP" chunks to a vector, one per image added with AddImage().
	 *
	 * \return Returns the bytes that represent the "DISP" chunks in a file, or an empty vector.
	 */
	std::vector<uint8_t> CWavFile::CreateDisp() const {
		std::vector<uint8_t> vRet;
#define PW_PUSH32( VAL )		vRet.push_back( static_cast<uint8_t>((VAL) >> 0) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 8) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 16) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 24) )
		for ( size_t I = 0; I < m_vDisp.size(); ++I ) {
			const uint32_t ui32Size = static_cast<uint32_t>(m_vDisp[I].vValue.size()) + 4;
			PW_PUSH32( PW_C_DISP );			// "DISP"
			PW_PUSH32( ui32Size );
			PW_PUSH32( m_vDisp[I].ui32Type );
			vRet.insert( vRet.end(), m_vDisp[I].vValue.begin(), m_vDisp[I].vValue.end() );
			if ( ui32Size & 1 ) { vRet.push_back( 0 ); }
		}
#undef PW_PUSH32
		return vRet;
	}

	/**
	 * Writes a file-image "id3 " chunk to a vector containing the loaded ID3v2 frames, using the loaded tag's version.
	 *
	 * \return Returns the bytes that represent the "id3 " chunk in a file, or an empty vector.
	 */
	std::vector<uint8_t> CWavFile::CreateId3() const {
		std::vector<uint8_t> vRet;
		if ( !m_vId3Entries.size() ) { return vRet; }
		auto PushSyncSafe = []( std::vector<uint8_t> &_vDst, uint32_t _ui32Val ) {
			_vDst.push_back( static_cast<uint8_t>((_ui32Val >> 21) & 0x7F) );
			_vDst.push_back( static_cast<uint8_t>((_ui32Val >> 14) & 0x7F) );
			_vDst.push_back( static_cast<uint8_t>((_ui32Val >> 7) & 0x7F) );
			_vDst.push_back( static_cast<uint8_t>(_ui32Val & 0x7F) );
		};
		std::vector<uint8_t> vFrames;
		for ( size_t I = 0; I < m_vId3Entries.size(); ++I ) {
			const uint32_t ui32Size = static_cast<uint32_t>(m_vId3Entries[I].sValue.size());
			if ( ui32Size >= (1U << 28) ) { return std::vector<uint8_t>(); }
			vFrames.insert( vFrames.end(), m_vId3Entries[I].u.cName, m_vId3Entries[I].u.cName + 4 );
			if ( m_ui8Id3Version == 4 ) { PushSyncSafe( vFrames, ui32Size ); }
			else {
				vFrames.push_back( static_cast<uint8_t>(ui32Size >> 24) );
				vFrames.push_back( static_cast<uint8_t>(ui32Size >> 16) );
				vFrames.push_back( static_cast<uint8_t>(ui32Size >> 8) );
				vFrames.push_back( static_cast<uint8_t>(ui32Size) );
			}
			vFrames.push_back( static_cast<uint8_t>(m_vId3Entries[I].ui16Flags) );
			vFrames.push_back( static_cast<uint8_t>(m_vId3Entries[I].ui16Flags >> 8) );
			vFrames.insert( vFrames.end(), m_vId3Entries[I].sValue.begin(), m_vId3Entries[I].sValue.end() );
		}
		if ( vFrames.size() >= (1U << 28) ) { return std::vector<uint8_t>(); }

#define PW_PUSH32( VAL )		vRet.push_back( static_cast<uint8_t>((VAL) >> 0) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 8) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 16) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 24) )
		const uint32_t ui32TagSize = static_cast<uint32_t>(vFrames.size()) + 10;
		PW_PUSH32( PW_C_ID3_ );			// "id3 "
		PW_PUSH32( ui32TagSize );
		vRet.push_back( 'I' );
		vRet.push_back( 'D' );
		vRet.push_back( '3' );
		vRet.push_back( m_ui8Id3Version );
		vRet.push_back( 0 );				// Revision.
		vRet.push_back( 0 );				// Flags.
		PushSyncSafe( vRet, static_cast<uint32_t>(vFrames.size()) );
		vRet.insert( vRet.end(), vFrames.begin(), vFrames.end() );
		if ( ui32TagSize & 1 ) { vRet.push_back( 0 ); }
#undef PW_PUSH32
		return vRet;
	}

	/**
	 * Fixes the output file name of a given full path.
	 * 
	 * \param _pcFullpath The full path whose file name is to fix.
	 * \return Returns the fixed file path.
	 **/
	std::u8string CWavFile::FixFileName( const char8_t * _pcFullpath ) {
		std::u8string sPath = _pcFullpath;
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

		return sFolder + sCopy;
	}

	/**
	 * Creates the file for streaming and writes the header data to it, preparing it for writing samples.
	 * 
	 * \param _pcPath Uses data loaded into m_sStream to create a new file.
	 * \return Returns true if the file was created and the header was written to it.
	 **/
	bool CWavFile::CreateStreamFile( const char8_t * _pcPath ) {
		PW_SAVE_DATA sdSaveSettings( m_sStream.ui32Hz, m_sStream.ui16Bits );
		sdSaveSettings.fFormat = m_sStream.fFormat;
		
		if ( !m_sStream.sfFile.Create( _pcPath ) ) {
			std::wprintf( L"Failed to create stream file: %s.\r\n", reinterpret_cast<const wchar_t *>(CUtilities::Utf8ToUtf16( _pcPath ).c_str()) );
			return false;
		}

		PW_FMT_CHUNK fcChunk = CreateFmt( m_sStream.fFormat, m_sStream.ui16Channels,
			&sdSaveSettings );
		if ( !fcChunk.uiBlockAlign ) { return false; }

		const uint32_t uiFmtSize = fcChunk.chHeader.uiSize + 8;

		if ( !m_sStream.sfFile.WriteUi32( PW_C_RIFF ) ) { return false; }

		m_sStream.ui64WavFileOffset_Size = m_sStream.sfFile.GetPos();
		if ( !m_sStream.sfFile.Write( uint32_t( 0 ) ) ) { return false; }		// Filled in when the stream is closed.
		
		if ( !m_sStream.sfFile.WriteUi32( PW_C_WAVE ) ) { return false; }

		if ( !m_sStream.sfFile.WriteToFile( reinterpret_cast<const uint8_t *>(&fcChunk), uiFmtSize ) ) { return false; }

		// Everything in the RIFF chunk except the sample data and its pad byte.
		m_sStream.ui32WavFile_Size = 4 + uiFmtSize + 8;							// "WAVE", "fmt ", and the "data" chunk header.
		m_sStream.ui64WavFileOffset_Fact = 0;
		if ( fcChunk.uiAudioFormat != PW_F_PCM ) {
			// Every non-PCM format requires a "fact" chunk holding the number of sample frames.
			if ( !m_sStream.sfFile.WriteUi32( PW_C_FACT ) ) { return false; }
			if ( !m_sStream.sfFile.Write( uint32_t( 4 ) ) ) { return false; }
			m_sStream.ui64WavFileOffset_Fact = m_sStream.sfFile.GetPos();
			if ( !m_sStream.sfFile.Write( uint32_t( 0 ) ) ) { return false; }		// Filled in when the stream is closed.
			m_sStream.ui32WavFile_Size += 12;
		}

		// Append the "data" chunk.
		if ( !m_sStream.sfFile.WriteUi32( PW_C_DATA ) ) { return false; }
		
		m_sStream.ui64WavFileOffset_DSize = m_sStream.sfFile.GetPos();
		if ( !m_sStream.sfFile.Write( uint32_t( 0 ) ) ) { return false; }		// Filled in when the stream is closed.
		
		// The RIFF size (ui32WavFile_Size + data + pad byte) must fit in 32 bits.
		m_sStream.ui64MaxSamples = (0xFFFFFFFFULL - m_sStream.ui32WavFile_Size - 1) / fcChunk.uiBlockAlign * m_sStream.ui16Channels;

		// File now ready for streaming.
		return true;
	}

	/**
	 * Closes the current streaming file.
	 **/
	void CWavFile::CloseStreamFile() {
		if PW_LIKELY( m_sStream.sfFile.IsOpen() ) {
			if PW_LIKELY( m_sStream.bStreaming ) {
				// AddStreamSample() never accepts more samples than fit (ui64MaxSamples), so these sizes cannot overflow.
				const uint32_t ui32DataSize = static_cast<uint32_t>(m_sStream.ui64SamplesWritten * (m_sStream.ui16Bits / 8));
				const uint32_t ui32Pad = ui32DataSize & 1;
				if ( ui32Pad ) {
					m_sStream.sfFile.Write<uint8_t>( 0 );			// Pad byte; not part of the "data" size.
				}

				m_sStream.sfFile.MovePointerTo( m_sStream.ui64WavFileOffset_Size );
				m_sStream.sfFile.Write( uint32_t( m_sStream.ui32WavFile_Size + ui32DataSize + ui32Pad ) );

				m_sStream.sfFile.MovePointerTo( m_sStream.ui64WavFileOffset_DSize );
				m_sStream.sfFile.Write( ui32DataSize );

				if ( m_sStream.ui64WavFileOffset_Fact ) {
					m_sStream.sfFile.MovePointerTo( m_sStream.ui64WavFileOffset_Fact );
					m_sStream.sfFile.Write( static_cast<uint32_t>(m_sStream.ui64SamplesWritten / m_sStream.ui16Channels) );
				}
			}
		
			m_sStream.sfFile.Close();
		}
		

		{
			std::unique_lock<std::mutex> ulLockMeta( m_sStream.mMetaMutex );
			if ( m_sStream.bMeta && m_sStream.pfMetaFunc ) {
				AddMetaData();
			}
		}

		if ( m_sStream.bMeta ) {
			m_sStream.cvMetaCondition.notify_one();
			if ( m_sStream.tMetaThread.joinable()) {
				m_sStream.tMetaThread.join();
			}
			m_sStream.bMeta = false;
		}
	}

	/**
	 * Creates the file for streaming metadata.  All writes to the file are handled by a callback.
	 * 
	 * \param _pcPath Uses data loaded into m_sStream to create a new file.
	 * \return Returns true if the file was created.
	 **/
	bool CWavFile::CreateStreamMetaFile( const char8_t * _pcPath ) {
		if ( !m_sStream.sfMetaFile.Create( _pcPath ) ) {
			std::wprintf( L"Failed to create stream metadata file: %s.\r\n", reinterpret_cast<const wchar_t *>(CUtilities::Utf8ToUtf16( _pcPath ).c_str()) );
			return false;
		}
		return true;
	}

	/**
	 * Closes the current streaming metadata file.
	 **/
	void CWavFile::CloseStreamMetaFile() {
		if ( m_sStream.sfMetaFile.IsOpen() ) {
			m_sStream.sfMetaFile.Close();
		}
	}

	/**
	 * The stream-to-file writer thread.
	 **/
	void CWavFile::StreamWriterThread() {
		std::vector<uint8_t> vConversionBuffer;
		while ( true ) {
			std::vector<float> vBufferToWrite;
			{
				std::unique_lock<std::mutex> ulLock( m_sStream.mMutex );
				// Wait until there is a full buffer or a shutdown is signaled.
                m_sStream.cvCondition.wait( ulLock, [this] {
                    return !m_sStream.qBufferQueue.empty() || m_sStream.bEnd;
                });

				// If there’s data to write, retrieve the next buffer.
                if ( !m_sStream.qBufferQueue.empty() ) {
                    vBufferToWrite = std::move( m_sStream.qBufferQueue.front() );
                    m_sStream.qBufferQueue.pop();
                }
				else if ( m_sStream.bEnd ) {
                    // No more buffers and shutdown requested.
                    break;
                }
			}
			// Write the buffer to disk (if any).
            if PW_UNLIKELY( !vBufferToWrite.empty() ) {
				(*m_sStream.pfCvtAndWriteFunc)( vBufferToWrite, vConversionBuffer, m_sStream );
            }
		}
		CloseStreamFile();
	}

	/**
	 * The metadata-to-file writer thread.
	 **/
	void CWavFile::MetadataWriterThread() {
		std::vector<uint8_t> vConversionBuffer;
		while ( true ) {
			std::vector<uint8_t, CAlignmentAllocator<uint8_t, 64>> vTmpBuffer;
			{
				std::unique_lock<std::mutex> ulLock( m_sStream.mMetaMutex );
				// Wait until there is a full buffer or a shutdown is signaled.
				m_sStream.cvMetaCondition.wait( ulLock, [this] {
					return !m_sStream.qMetaBufferQueue.empty() || m_sStream.bEnd;
				});

				// If there’s data to write, retrieve the next buffer.
				if ( !m_sStream.qMetaBufferQueue.empty() ) {
					vTmpBuffer = std::move( m_sStream.qMetaBufferQueue.front() );
					m_sStream.qMetaBufferQueue.pop();
				}
				else if ( m_sStream.bEnd ) {
					// No more buffers and shutdown requested.
					break;
				}
			}
			// Write the buffer to disk (if any).
            if PW_UNLIKELY( !vTmpBuffer.empty() ) {
				(*m_sStream.pfMetaThreadFunc)( this, m_sStream.pvMetaParm, m_sStream, vTmpBuffer, vConversionBuffer );
            }
		}
		CloseStreamMetaFile();
	}

	/**
	 * The None start condition.  Returns true.
	 * 
	 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
	 * \param _fSample The value of the sample being tested.
	 * \param _cdData Function-specific data.
	 * \return Returns true if the start condition has been met.
	 **/
	bool PW_STDCALL CWavFile::StartCondFunc_None( uint64_t /*_ui64AbsoluteIdx*/, float /*_fSample*/, PW_CONDITIONS_DATA &/*_cdData*/ ) { return true; }

	/**
	 * The Start-at-Sample start condition.  Returns true if _ui64AbsoluteIdx >= _cdData.ui64Parm0.
	 * 
	 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
	 * \param _fSample The value of the sample being tested.
	 * \param _cdData Function-specific data.
	 * \return Returns true if the start condition has been met.
	 **/
	bool PW_STDCALL CWavFile::StartCondFunc_StartAtSample( uint64_t _ui64AbsoluteIdx, float /*_fSample*/, PW_CONDITIONS_DATA &_cdData ) { return _ui64AbsoluteIdx >= _cdData.ui64Parm0; }

	/**
	 * The First-Non-Zero start condition.  Returns true if _fSample != 0.0f.
	 * 
	 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
	 * \param _fSample The value of the sample being tested.
	 * \param _cdData Function-specific data.
	 * \return Returns true if the start condition has been met.
	 **/
	bool PW_STDCALL CWavFile::StartCondFunc_FirstNonZero( uint64_t /*_ui64AbsoluteIdx*/, float _fSample, PW_CONDITIONS_DATA &/*_cdData*/ ) { return _fSample != 0.0f; }

	/**
	 * The Zero-For-Duration start condition.  Returns true if _cdData.ui64Counter >= _cdData.ui64Parm0, with _cdData.ui64Counter being counted on each 0 sample.
	 * 
	 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
	 * \param _fSample The value of the sample being tested.
	 * \param _cdData Function-specific data.
	 * \return Returns true if the start condition has been met.
	 **/
	bool PW_STDCALL CWavFile::StartCondFunc_ZeroForDuration( uint64_t /*_ui64AbsoluteIdx*/, float _fSample, PW_CONDITIONS_DATA &_cdData ) {
		if ( _fSample && _cdData.ui64Counter >= _cdData.ui64Parm0 ) { return true; }
		if ( _fSample ) { _cdData.ui64Counter = 0; }
		else { ++_cdData.ui64Counter; }
		return false;
	}
		
	/**
	 * The None end condition.  Returns true.
	 * 
	 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
	 * \param _ui64IdxSinceStart The index of the sample being tested since recording began.
	 * \param _fSample The value of the sample being tested.
	 * \param _cdData Function-specific data.
	 * \return Returns false if the end condition has been met, at which point recording should stop.
	 **/
	bool PW_STDCALL CWavFile::EndCondFunc_None( uint64_t /*_ui64AbsoluteIdx*/, uint64_t /*_ui64IdxSinceStart*/, float /*_fSample*/, PW_CONDITIONS_DATA &/*_cdData*/ ) { return true; }

	/**
	 * The End-At-Sample end condition.  Returns true if _ui64AbsoluteIdx < _cdData.ui64Parm0.
	 * 
	 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
	 * \param _ui64IdxSinceStart The index of the sample being tested since recording began.
	 * \param _fSample The value of the sample being tested.
	 * \param _cdData Function-specific data.
	 * \return Returns false if the end condition has been met, at which point recording should stop.
	 **/
	bool PW_STDCALL CWavFile::EndCondFunc_EndAtSample( uint64_t _ui64AbsoluteIdx, uint64_t /*_ui64IdxSinceStart*/, float /*_fSample*/, PW_CONDITIONS_DATA &_cdData ) { return _ui64AbsoluteIdx < _cdData.ui64Parm0; }

	/**
	 * The Zero-For-Duration end condition.  Returns true if _cdData.ui64Counter < _cdData.ui64Parm0, with _cdData.ui64Counter counting consecutive 0's.
	 * 
	 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
	 * \param _ui64IdxSinceStart The index of the sample being tested since recording began.
	 * \param _fSample The value of the sample being tested.
	 * \param _cdData Function-specific data.
	 * \return Returns false if the end condition has been met, at which point recording should stop.
	 **/
	bool PW_STDCALL CWavFile::EndCondFunc_ZeroForDuration( uint64_t /*_ui64AbsoluteIdx*/, uint64_t /*_ui64IdxSinceStart*/, float _fSample, PW_CONDITIONS_DATA &_cdData ) {
		if PW_LIKELY( _fSample ) { _cdData.ui64Counter = 0; return false; }
		return ++_cdData.ui64Counter < _cdData.ui64Parm0;
	}

	/**
	 * The Duration end condition.  Returns true if _ui64IdxSinceStart < _cdData.ui64Parm0.
	 * 
	 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
	 * \param _ui64IdxSinceStart The index of the sample being tested since recording began.
	 * \param _fSample The value of the sample being tested.
	 * \param _cdData Function-specific data.
	 * \return Returns false if the end condition has been met, at which point recording should stop.
	 **/
	bool PW_STDCALL CWavFile::EndCondFunc_Duration( uint64_t /*_ui64AbsoluteIdx*/, uint64_t _ui64IdxSinceStart, float /*_fSample*/, PW_CONDITIONS_DATA &_cdData ) { return _ui64IdxSinceStart < _cdData.ui64Parm0; }

	/**
	 * Checks the starting condition before adding a sample.  Once the starting condition has been reached, _sStream.pfAddSampleFunc is changed to AddSample_CheckStopCond().
	 * 
	 * \param _fSample The sampple to add.
	 * \param _sStream The stream data.
	 * \return Returns true if the starting condition has been reached.
	 **/
	bool PW_STDCALL CWavFile::AddSample_CheckStartCond( float _fSample, PW_STREAMING &_sStream ) {
		if ( (*_sStream.pfStartCondFunc)( _sStream.ui64SamplesReceived, _fSample, _sStream.cdStartData ) ) {
			_sStream.pfAddSampleFunc = &CWavFile::AddSample_CheckStopCond;
			return (*_sStream.pfAddSampleFunc)( _fSample, _sStream );
		}
		return false;
	}

	/**
	 * Checks the stopping condition before adding a sample.  Once the stopping condition has been reached, the stream ends. 
	 * 
	 * \param _fSample The sampple to add.
	 * \param _sStream The stream data.
	 * \return Returns true for as long as samples should be output.  Once false is returned, the ending condition has been reached and the stream should close without adding the sample.
	 **/
	bool PW_STDCALL CWavFile::AddSample_CheckStopCond( float _fSample, PW_STREAMING &_sStream ) {
		if PW_UNLIKELY( !(*_sStream.pfEndCondFunc)( _sStream.ui64SamplesReceived, _sStream.ui64SamplesWritten, _fSample, _sStream.cdStopData ) ) {
			_sStream.bEnd = true;
			return false;
		}
		return true;
	}

}	// namespace pw
