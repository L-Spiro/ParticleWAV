/**
 * Copyright L. Spiro 2024
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: Useful utility functions.
 */


#pragma once

#include "../OS/PWOs.h"
#include "../OS/PWFeatureSet.h"
#include "../Filters/PWHpfFilter.h"
#include "../Filters/PWLpfFilter.h"

#include <algorithm>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
//#include <intrin.h>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>


#ifndef PW_ELEMENTS
#define PW_ELEMENTS( x )									((sizeof( x ) / sizeof( 0[x] )) / (static_cast<size_t>(!(sizeof( x ) % sizeof(0[x])))))
#endif	// #ifndef PW_ELEMENTS

#ifndef PW_PI
#define PW_PI												3.14159265358979323846264338327950288419716939937510	// You can tell how cool a programmer is by how many digits she puts in the PI macro.  True story.
#endif	// #ifndef PW_PI

#ifndef M_PI
#define M_PI												PW_PI
#endif	// #ifndef M_PI


namespace pw {

	/**
	 * Class CUtilities
	 * \brief Useful utility functions.
	 *
	 * Description: Useful utility functions.
	 */
	class CUtilities {
	public :
		// == Types.
		/** The result of fitting a chain of LPF's to a step response. */
		struct PW_LPF_FIT {
			std::vector<double>								vCutOffs;											/**< The fitted cut-offs in Hz (CLpfFilter units), sorted from lowest to highest. */
			double											dStepTime = 0.0;									/**< The fitted time of the step in samples from the start of the window. */
			double											dOffset = 0.0;										/**< The fitted level at the start of the window (before the step). */
			double											dDrift = 0.0;										/**< The fitted linear drift across the whole window. */
			double											dStepSize = 0.0;									/**< The fitted size of the step (before filtering). */
			double											dFit = 0.0;											/**< The final mean-squared error between the model and the samples. */
		};


		// == Functions.
		/**
		 * Creates a string with _cReplaceMe replaced with _cWithMe inside _s16String.
		 *
		 * \param _s16String The string in which replacements are to be made.
		 * \param _cReplaceMe The character to replace.
		 * \param _cWithMe The character with which to replace _cReplaceMe.
		 * \return Returns the new string with the given replacements made.
		 */
		template <typename _tType = std::u16string>
		static _tType										Replace( const _tType &_s16String, _tType::value_type _cReplaceMe, _tType::value_type _cWithMe ) {
			_tType s16Copy = _s16String;
			auto aFound = s16Copy.find( _cReplaceMe );
			while ( aFound != _tType::npos ) {
				s16Copy[aFound] = _cWithMe;
				aFound = s16Copy.find( _cReplaceMe, aFound + 1 );
			}
			return s16Copy;
		}

		/**
		 * Creates a string with _cReplaceMe replaced with _cWithMe inside _s16String.
		 *
		 * \param _s16String The string in which replacements are to be made.
		 * \param _cReplaceMe The character to replace.
		 * \param _cWithMe The character with which to replace _cReplaceMe.
		 * \return Returns the new string with the given replacements made.
		 */
		template <typename _tType = std::u16string>
		static _tType										Replace( const _tType &_s16String, const _tType &_cReplaceMe, const _tType &_cWithMe ) {
			_tType sCopy = _s16String;
			const size_t sLen = _cReplaceMe.size();
			size_t sIdx = sCopy.find( _cReplaceMe );
			while ( _tType::npos != sIdx ) {
				sCopy = sCopy.replace( sIdx, sLen, _cWithMe );
				sIdx = sCopy.find( _cReplaceMe );
			}
			return sCopy;
		}

		/**
		 * Replaces a string inside a data vector.
		 * 
		 * \param _vData The buffer of data in which to replace a string.
		 * \param _sReplaceMe The string to replace.
		 * \param _sWithMe The string with which to replace _sReplaceMe inside _vData.
		 * \return Returns a reference to _vData.
		 **/
		static std::vector<uint8_t> &						Replace( std::vector<uint8_t> &_vData, const std::string &_sReplaceMe, const std::string &_sWithMe );

		/**
		 * Converts a UTF-8 string to a UTF-16 string.  The resulting string may have allocated more characters than necessary but will be terminated with a NULL.
		 *
		 * \param _pcString String to convert.
		 * \param _pbErrored If not nullptr, holds a returned boolean indicating success or failure of the conversion.
		 * \return Returns the converted UTF-16 string.
		 */
		static std::u16string								Utf8ToUtf16( const char8_t * _pcString, bool * _pbErrored = nullptr );

		/**
		 * Converts a UTF-16 string to a UTF-8 string.  The resulting string may have allocated more characters than necessary but will be terminated with a NULL.
		 *
		 * \param _pcString String to convert.
		 * \param _pbErrored If not nullptr, holds a returned boolean indicating success or failure of the conversion.
		 * \return Returns the converted UTF-8 string.
		 */
		static std::u8string								Utf16ToUtf8( const char16_t * _pcString, bool * _pbErrored = nullptr );

		/**
		 * Converts a value to a string.
		 * 
		 * \param _tVal The value to convert.
		 * \param _stDigits The number of digits to which to pad.
		 * \return Returns the string form of the given value.
		 **/
		template <typename _tType>
		static std::u16string								ToString( _tType _tVal, size_t _stDigits = 0 ) {
			std::wstring wsNumber = std::to_wstring( _tVal );
			std::u16string usNumber( wsNumber.begin(), wsNumber.end() );
			while ( usNumber.size() < _stDigits ) {
				usNumber.insert( usNumber.begin(), u'0' );
			}
			return usNumber;
		}

		/**
		 * Converts a value to a string.
		 * 
		 * \param _pwcVal The value to convert.
		 * \param _stDigits The number of digits to which to pad.
		 * \return Returns the string form of the given value.
		 **/
		static std::u16string								ToString( const wchar_t * _pwcVal ) {
			std::wstring wsNumber = _pwcVal;
			std::u16string usNumber( wsNumber.begin(), wsNumber.end() );
			return usNumber;
		}

		/**
		 * Converts a value to a string.
		 * 
		 * \param _pcVal The value to convert.
		 * \param _stDigits The number of digits to which to pad.
		 * \return Returns the string form of the given value.
		 **/
		static std::u16string								ToString( const char * _pcVal ) {
			std::string wsNumber = _pcVal;
			std::u16string usNumber( wsNumber.begin(), wsNumber.end() );
			return usNumber;
		}

		/**
		 * Converts an * string to a std::wstring.  Call inside try{}catch(...){}.
		 * 
		 * \param _pwcStr The string to convert.
		 * \param _sLen The length of the string or 0.
		 * \return Returns the converted string.
		 **/
		template <typename _tCharType>
		static inline std::wstring							XStringToWString( const _tCharType * _pwcStr, size_t _sLen ) {
			std::wstring u16Tmp;
			if ( _sLen ) {
				u16Tmp.reserve( _sLen );
			}
			for ( size_t I = 0; (I < _sLen) || (_sLen == 0 && !_pwcStr[I]); ++I ) {
				u16Tmp.push_back( static_cast<wchar_t>(_pwcStr[I]) );
			}
			return u16Tmp;
		}

		/**
		 * Converts an * string to a std::u16string.  Call inside try{}catch(...){}.
		 * 
		 * \param _pwcStr The string to convert.
		 * \param _sLen The length of the string or 0.
		 * \return Returns the converted string.
		 **/
		template <typename _tCharType>
		static inline std::u16string						XStringToU16String( const _tCharType * _pwcStr, size_t _sLen ) {
			std::u16string u16Tmp;
			if ( _sLen ) {
				u16Tmp.reserve( _sLen );
			}
			for ( size_t I = 0; (I < _sLen) || (_sLen == 0 && !_pwcStr[I]); ++I ) {
				u16Tmp.push_back( static_cast<char16_t>(_pwcStr[I]) );
			}
			return u16Tmp;
		}

		/**
		 * Converts an * string to a std::u8string.  Call inside try{}catch(...){}.
		 * 
		 * \param _pwcStr The string to convert.
		 * \param _sLen The length of the string or 0.
		 * \return Returns the converted string.
		 **/
		template <typename _tCharType>
		static inline std::u8string							XStringToU8String( const _tCharType * _pwcStr, size_t _sLen ) {
			std::u8string u16Tmp;
			if ( _sLen ) {
				u16Tmp.reserve( _sLen );
			}
			for ( size_t I = 0; (I < _sLen) || (_sLen == 0 && !_pwcStr[I]); ++I ) {
				u16Tmp.push_back( static_cast<char8_t>(_pwcStr[I]) );
			}
			return u16Tmp;
		}

		/**
		 * Reads a line from a buffer.
		 * 
		 * \param _vBuffer The buffer from which to read a line.
		 * \param _stPos The current position inside the buffer, updated on return.
		 * \return Returns the line read from the file.
		 **/
		static std::string									ReadLine( const std::vector<uint8_t> &_vBuffer, size_t &_stPos );

		/**
		 * Tokenizes a string by a given character.
		 * 
		 * \param _sString The string to tokenize.
		 * \param _vtDelimiter The character by which to deliminate the string into sections.
		 * \param _bAllowEmptyStrings If true, the return value could contain empty strings when the delimiter is found more than once in a row.
		 * \return DESC
		 **/
		static std::vector<std::string>						Tokenize( const std::string &_sString, std::string::value_type _vtDelimiter, bool _bAllowEmptyStrings );

#ifdef __AVX512F__
		/**
		 * Horizontally adds all the floats in a given AVX-512 register.
		 * 
		 * \param _mReg The register containing all of the values to sum.
		 * \return Returns the sum of all the floats in the given register.
		 **/
		static inline float									HorizontalSum( __m512 _mReg ) {
			return _mm512_reduce_add_ps( _mReg );
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (unaligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_AVX512_U( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 16 floats at a time.  But like, 2 16-at-a-times at a time.
			for ( ; I + 31 < _sTotal; I += 32 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 32 );
				PW_PREFETCH_LINE( _pfOp1 + 32 );

				// Load and add the first 16 floats.
				__m512 mA1 = _mm512_loadu_ps( &_pfOp0[I] );
				__m512 mB1 = _mm512_loadu_ps( &_pfOp1[I] );
				__m512 mC1 = _mm512_add_ps( mA1, mB1 );
				_mm512_storeu_ps( &_pfOut[I], mC1 );

				// Load and add the next 16 floats.
				__m512 mA2 = _mm512_loadu_ps( &_pfOp0[I+16] );
				__m512 mB2 = _mm512_loadu_ps( &_pfOp1[I+16] );
				__m512 mC2 = _mm512_add_ps( mA2, mB2 );
				_mm512_storeu_ps( &_pfOut[I+16], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (aligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_AVX512( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 16 floats at a time.  But like, 2 16-at-a-times at a time.
			for ( ; I + 31 < _sTotal; I += 32 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 32 );
				PW_PREFETCH_LINE( _pfOp1 + 32 );

				// Load and add the first 16 floats.
				__m512 mA1 = _mm512_load_ps( &_pfOp0[I] );
				__m512 mB1 = _mm512_load_ps( &_pfOp1[I] );
				__m512 mC1 = _mm512_add_ps( mA1, mB1 );
				_mm512_store_ps( &_pfOut[I], mC1 );

				// Load and add the next 16 floats.
				__m512 mA2 = _mm512_load_ps( &_pfOp0[I+16] );
				__m512 mB2 = _mm512_load_ps( &_pfOp1[I+16] );
				__m512 mC2 = _mm512_add_ps( mA2, mB2 );
				_mm512_store_ps( &_pfOut[I+16], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}
#endif	// #ifdef __AVX512F__

#ifdef __AVX__
		/**
		 * Horizontally adds all the floats in a given AVX register.
		 * 
		 * \param _mReg The register containing all of the values to sum.
		 * \return Returns the sum of all the floats in the given register.
		 **/
		static inline float									HorizontalSum( __m256 &_mReg ) {
			__m256 mTmp = _mm256_add_ps( _mReg, _mm256_permute2f128_ps( _mReg, _mReg, 1 ) );
			mTmp = _mm256_hadd_ps( mTmp, mTmp );
			mTmp = _mm256_hadd_ps( mTmp, mTmp );
			return _mm256_cvtss_f32( mTmp );
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (unaligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_AVX_U( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 8 floats at a time.  But like, 2 8-at-a-times at a time
			for ( ; I + 15 < _sTotal; I += 16 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 16 );
				PW_PREFETCH_LINE( _pfOp1 + 16 );

				// Load and add the first 8 floats
				__m256 mA1 = _mm256_loadu_ps( &_pfOp0[I] );
				__m256 mB1 = _mm256_loadu_ps( &_pfOp1[I] );
				__m256 mC1 = _mm256_add_ps( mA1, mB1 );
				_mm256_storeu_ps( &_pfOut[I], mC1 );

				// Load and add the next 8 floats
				__m256 mA2 = _mm256_loadu_ps( &_pfOp0[I+8] );
				__m256 mB2 = _mm256_loadu_ps( &_pfOp1[I+8] );
				__m256 mC2 = _mm256_add_ps( mA2, mB2 );
				_mm256_storeu_ps( &_pfOut[I+8], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (aligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_AVX( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 8 floats at a time.  But like, 2 8-at-a-times at a time.
			for ( ; I + 15 < _sTotal; I += 16 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 16 );
				PW_PREFETCH_LINE( _pfOp1 + 16 );

				// Load and add the first 8 floats.
				__m256 mA1 = _mm256_load_ps( &_pfOp0[I] );
				__m256 mB1 = _mm256_load_ps( &_pfOp1[I] );
				__m256 mC1 = _mm256_add_ps( mA1, mB1 );
				_mm256_store_ps( &_pfOut[I], mC1 );

				// Load and add the next 8 floats.
				__m256 mA2 = _mm256_load_ps( &_pfOp0[I+8] );
				__m256 mB2 = _mm256_load_ps( &_pfOp1[I+8] );
				__m256 mC2 = _mm256_add_ps( mA2, mB2 );
				_mm256_store_ps( &_pfOut[I+8], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}
#endif	// #ifdef __AVX__

#ifdef __SSE4_1__
		/**
		 * Horizontally adds all the floats in a given SSE register.
		 * 
		 * \param _mReg The register containing all of the values to sum.
		 * \return Returns the sum of all the floats in the given register.
		 **/
		static inline float									HorizontalSum( __m128 &_mReg ) {
			__m128 mAddH1 = _mm_hadd_ps( _mReg, _mReg );
			__m128 mAddH2 = _mm_hadd_ps( mAddH1, mAddH1 );
			return _mm_cvtss_f32( mAddH2 );
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (unaligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_SSE4_U( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 4 floats at a time.  But like, 2 4-at-a-times at a time.
			for ( ; I + 7 < _sTotal; I += 8 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 8 );
				PW_PREFETCH_LINE( _pfOp1 + 8 );

				// Load and add the first 4 floats.
				__m128 mA1 = _mm_loadu_ps( &_pfOp0[I] );
				__m128 mB1 = _mm_loadu_ps( &_pfOp1[I] );
				__m128 mC1 = _mm_add_ps( mA1, mB1 );
				_mm_storeu_ps( &_pfOut[I], mC1 );

				// Load and add the next 4 floats.
				__m128 mA2 = _mm_loadu_ps( &_pfOp0[I+4] );
				__m128 mB2 = _mm_loadu_ps( &_pfOp1[I+4] );
				__m128 mC2 = _mm_add_ps( mA2, mB2 );
				_mm_storeu_ps( &_pfOut[I+4], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (aligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_SSE4( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 4 floats at a time.  But like, 2 4-at-a-times at a time.
			for ( ; I + 7 < _sTotal; I += 8 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 8 );
				PW_PREFETCH_LINE( _pfOp1 + 8 );

				// Load and add the first 4 floats.
				__m128 mA1 = _mm_load_ps( &_pfOp0[I] );
				__m128 mB1 = _mm_load_ps( &_pfOp1[I] );
				__m128 mC1 = _mm_add_ps( mA1, mB1 );
				_mm_store_ps( &_pfOut[I], mC1 );

				// Load and add the next 4 floats.
				__m128 mA2 = _mm_load_ps( &_pfOp0[I+4] );
				__m128 mB2 = _mm_load_ps( &_pfOp1[I+4] );
				__m128 mC2 = _mm_add_ps( mA2, mB2 );
				_mm_store_ps( &_pfOut[I+4], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}
#endif	// #ifdef __SSE4_1__

		/**
		 * Sums a pair of arrays of floats to a given destination.
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			while ( _sTotal >= 8 ) {
				// Let's do a loop of 8 at a time.
				_pfOut[0] = _pfOp0[0] + _pfOp1[0];
				_pfOut[1] = _pfOp0[1] + _pfOp1[1];
				_pfOut[2] = _pfOp0[2] + _pfOp1[2];
				_pfOut[3] = _pfOp0[3] + _pfOp1[3];
				_pfOut[4] = _pfOp0[4] + _pfOp1[4];

				PW_PREFETCH_LINE( _pfOp0 + 8 );
				PW_PREFETCH_LINE( _pfOp1 + 8 );

				_pfOut[5] = _pfOp0[5] + _pfOp1[5];
				_pfOut[6] = _pfOp0[6] + _pfOp1[6];
				_pfOut[7] = _pfOp0[7] + _pfOp1[7];
				_pfOp0 += 8;
				_pfOp0 += 8;
				_pfOp1 += 8;
				_sTotal -= 8;
			}
			// Finish the rest.
			for ( size_t I = 0; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}

		/**
		 * Standard sinc() function.
		 * 
		 * \param _dX The operand.
		 * \return Returns sin(x) / x.
		 **/
		static inline double								Sinc( double _dX ) {
			_dX *= std::numbers::pi;
			if ( _dX < 0.01 && _dX > -0.01 ) {
				return 1.0 + _dX * _dX * (-1.0 / 6.0 + _dX * _dX * 1.0 / 120.0);
			}

			return std::sin( _dX ) / _dX;
		}

		/**
		 * Is AVX supported?
		 *
		 * \return Returns true if AVX is supported.
		 **/
		static inline bool									IsAvxSupported() {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER )
			return CFeatureSet::AVX();
#else
			return false;
#endif	// #if defined( __i386__ ) || defined( __x86_64__ )
		}

		/**
		 * Is AVX 2 supported?
		 *
		 * \return Returns true if AVX is supported.
		 **/
		static inline bool									IsAvx2Supported() {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER )
			return CFeatureSet::AVX2();
#else
			return false;
#endif	// #if defined( __i386__ ) || defined( __x86_64__ )
		}

		/**
		 * Is AVX-512F supported?
		 *
		 * \return Returns true if AVX-512F is supported.
		 **/
		static inline bool									IsAvx512FSupported() {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER )
			return CFeatureSet::AVX512F();
#else
			return false;
#endif	// #if defined( __i386__ ) || defined( __x86_64__ )
		}

		/**
		 * Is AVX-512BW supported?
		 *
		 * \return Returns true if AVX-512BW is supported.
		 **/
		static inline bool									IsAvx512BWSupported() {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER )
			return CFeatureSet::AVX512BW();
#else
			return false;
#endif	// #if defined( __i386__ ) || defined( __x86_64__ )
		}

		/**
		 * Is SSE 4 supported?
		 *
		 * \return Returns true if SSE 4 is supported.
		 **/
		static inline bool									IsSse4Supported() {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER )
			return CFeatureSet::SSE41();
#else
			return false;
#endif	// #if defined( __i386__ ) || defined( __x86_64__ )
		}
		
		/**
		 * Converts a sample to an integer: scales it, rounds half away from zero (exactly as std::round()), then clamps to the given integer range.
		 *	NaN converts to 0.  Clamping happens in the integer domain, not to [-1,1], so a value read from a file (for example -32768 / 32767.0)
		 *	is written back unchanged, while generated audio clamped to [-1,1] still never produces the asymmetric minimum (for example -32768).
		 *	This is the reference conversion; every SIMD path must match it bit-for-bit.
		 *
		 * \param _dSample The sample to convert.
		 * \param _dScale The value by which to scale the sample (32767.0 for 16-bit PCM, etc.)
		 * \param _i32Min The smallest allowed result.
		 * \param _i32Max The largest allowed result.
		 * \return Returns the converted sample.
		 **/
		static inline int32_t								SampleToInt( double _dSample, double _dScale, int32_t _i32Min, int32_t _i32Max ) {
			double dScaled = std::round( _dSample * _dScale );
			if PW_UNLIKELY( !(dScaled == dScaled) ) { return 0; }
			return static_cast<int32_t>(std::clamp( dScaled, double( _i32Min ), double( _i32Max ) ));
		}

		/**
		 * Converts a sample from a floating-point format to a uint8_t.  8-bit PCM data is expressed as an unsigned value over the range 0 to 255, 128 being an
		 *	audio output level of zero.
		 *
		 * \param _dSample The sample to convert.
		 * \return Returns the converted sample.
		 **/
		static inline uint8_t								SampleToUi8( double _dSample ) {
			double dClampedSample = std::clamp( _dSample, -1.0, 1.0 );
			double dScaledSample = (dClampedSample + 1.0) * 0.5 * 255.0;
			return static_cast<uint8_t>(std::round( dScaledSample ));
		}

		/**
		 * Converts a sample from a floating-point format to an int16_t.  16-bit PCM data is expressed as a signed value over the
		 *	range -32768 to 32767, 0 being an audio output level of zero.  Note that both -32768 and -32767 are -1.0; a proper
		 *	conversion never generates -32768.
		 *
		 * \param _dSample The sample to convert.
		 * \return Returns the converted sample.
		 **/
		static inline int16_t								SampleToI16( double _dSample ) {
			double dClampedSample = std::clamp( _dSample, -1.0, 1.0 );
			double dScaledSample = dClampedSample * 32767.0;
			return static_cast<int16_t>(std::round( dScaledSample ));
		}

		/**
		 * Converts a sample from a floating-point format to an int16_t while applying error-diffusion dithering.  16-bit PCM data is expressed as a signed value over the
		 *	range -32768 to 32767, 0 being an audio output level of zero.  The sample plus the running error is clamped to [-1,1] so that the error cannot grow
		 *	without bound while the signal is clipping.
		 * 
		 * \param _dSample The sample to convert.
		 * \param _dError The running error state.
		 * \return Returns the converted sample.
		 **/
		static inline int16_t								SampleToI16_Dither( double _dSample, double &_dError ) {
			double dThis = _dSample + _dError;
			if PW_UNLIKELY( !(dThis == dThis) ) { dThis = 0.0; }
			if PW_UNLIKELY( dThis < -1.0 ) { dThis = -1.0; }
			else if PW_UNLIKELY( dThis > 1.0 ) { dThis = 1.0; }
		
			int16_t i16Final = static_cast<int16_t>(std::round( static_cast<double>(dThis * 32767.0) ));

			double dScaled = double( i16Final );
			double dQuantized = dScaled * (1.0 / 32767.0);
			_dError = double( dThis - dQuantized );

			return i16Final;
		}
		
		/**
		 * Converts a sample from a floating-point format to an int32_t.  24-bit PCM data is expressed as a signed value over the
		 *	range -8388607 to 8388607, 0 being an audio output level of zero.  Note that both -8388608 and -8388607 are -1.0; a proper
		 *	conversion never generates -8388608.
		 *
		 * \param _dSample The sample to convert.
		 * \return Returns the converted sample.
		 **/
		static inline int32_t								SampleToI24( double _dSample ) {
			double dClampedSample = std::clamp( _dSample, -1.0, 1.0 );
			double dScaledSample = dClampedSample * 8388607.0;
			return static_cast<int32_t>(std::round( dScaledSample ));
		}

		/**
		 * Converts a sample from a floating-point format to an int32_t.  32-bit PCM data is expressed as a signed value over the
		 *	range -2147483648 to 2147483647, 0 being an audio output level of zero.  -1.0 maps to -2147483647; only values below -1.0 map to -2147483648.
		 *
		 * \param _dSample The sample to convert.
		 * \return Returns the converted sample.
		 **/
		static inline int32_t								SampleToI32( double _dSample ) {
			return SampleToInt( _dSample, 2147483647.0, INT32_MIN, INT32_MAX );
		}

#ifdef __AVX2__
		/**
		 * Converts 4 doubles to int32_t exactly as SampleToInt() does (scale, round half away from zero, clamp, NaN -> 0).
		 * 
		 * \param _vdSamples The samples to convert.
		 * \param _dScale The value by which to scale the samples.
		 * \param _dMin The smallest allowed result.
		 * \param _dMax The largest allowed result.
		 * \return Returns the 4 converted samples.
		 **/
		static inline __m128i								SampleToInt_AVX2( __m256d _vdSamples, double _dScale, double _dMin, double _dMax ) {
			const __m256d vdSignMask = _mm256_set1_pd( -0.0 );
			__m256d vdX		= _mm256_mul_pd( _vdSamples, _mm256_set1_pd( _dScale ) );
			__m256d vdT		= _mm256_round_pd( vdX, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC );
			__m256d vdFrac	= _mm256_andnot_pd( vdSignMask, _mm256_sub_pd( vdX, vdT ) );										// |x - trunc( x )|, exact.
			__m256d vdBump	= _mm256_and_pd( _mm256_cmp_pd( vdFrac, _mm256_set1_pd( 0.5 ), _CMP_GE_OQ ),
				_mm256_or_pd( _mm256_and_pd( vdX, vdSignMask ), _mm256_set1_pd( 1.0 ) ) );									// +-1 where |frac| >= 0.5.
			__m256d vdR		= _mm256_add_pd( vdT, vdBump );
			vdR				= _mm256_min_pd( _mm256_max_pd( vdR, _mm256_set1_pd( _dMin ) ), _mm256_set1_pd( _dMax ) );
			vdR				= _mm256_and_pd( vdR, _mm256_cmp_pd( vdX, vdX, _CMP_ORD_Q ) );										// NaN -> 0.
			return _mm256_cvtpd_epi32( vdR );
		}

		/**
		 * Converts 8 floats to 8 int32_t values (in order) exactly as SampleToInt() does.
		 * 
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _dScale The value by which to scale the samples.
		 * \param _dMin The smallest allowed result.
		 * \param _dMax The largest allowed result.
		 * \param _viLo Holds the returned first 4 results.
		 * \param _viHi Holds the returned last 4 results.
		 **/
		static inline void									SampleToInt_AVX2( const float * _pfSample, double _dScale, double _dMin, double _dMax, __m128i &_viLo, __m128i &_viHi ) {
			__m256 vSamples	= _mm256_loadu_ps( _pfSample );
			_viLo			= SampleToInt_AVX2( _mm256_cvtps_pd( _mm256_castps256_ps128( vSamples ) ), _dScale, _dMin, _dMax );
			_viHi			= SampleToInt_AVX2( _mm256_cvtps_pd( _mm256_extractf128_ps( vSamples, 1 ) ), _dScale, _dMin, _dMax );
		}

		/**
		 * Converts 8 samples from a floating-point format to uint8_t.  Bit-exact with SampleToUi8().
		 * 
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pui8Dst Pointer to the output.  Exactly 8 bytes are written.
		 **/
		static inline void									SampleToUi8_AVX2( const float * _pfSample, uint8_t * _pui8Dst ) {
			__m128i viLo, viHi;
			SampleToInt_AVX2( _pfSample, 127.0, -128.0, 127.0, viLo, viHi );
			__m128i vi16	= _mm_add_epi16( _mm_packs_epi32( viLo, viHi ), _mm_set1_epi16( 128 ) );						// 0..255 as int16_t.
			_mm_storel_epi64( reinterpret_cast<__m128i *>(_pui8Dst), _mm_packus_epi16( vi16, vi16 ) );
		}

		/**
		 * Converts 8 samples from a floating-point format to int16_t.  Bit-exact with SampleToI16().
		 * 
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pi16Dst Pointer to the output.  Exactly 16 bytes are written.
		 **/
		static inline void									SampleToI16_AVX2( const float * _pfSample, int16_t * _pi16Dst ) {
			__m128i viLo, viHi;
			SampleToInt_AVX2( _pfSample, 32767.0, -32768.0, 32767.0, viLo, viHi );
			_mm_storeu_si128( reinterpret_cast<__m128i *>(_pi16Dst), _mm_packs_epi32( viLo, viHi ) );
		}

		/**
		 * Converts 8 samples from a floating-point format to 24-bit values held in int32_t.  Bit-exact with SampleToI24().
		 *
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pi32Dst Pointer to the output.  No alignment is required.
		 **/
		static inline void									SampleToI24_AVX2( const float * _pfSample, int32_t * _pi32Dst ) {
			__m128i viLo, viHi;
			SampleToInt_AVX2( _pfSample, 8388607.0, -8388608.0, 8388607.0, viLo, viHi );
			_mm_storeu_si128( reinterpret_cast<__m128i *>(_pi32Dst), viLo );
			_mm_storeu_si128( reinterpret_cast<__m128i *>(_pi32Dst + 4), viHi );
		}
#endif	// #ifdef __AVX2__

#ifdef __AVX512F__
		/**
		 * Converts 8 doubles to int32_t exactly as SampleToInt() does (scale, round half away from zero, clamp, NaN -> 0).  Uses only AVX-512F.
		 * 
		 * \param _vdSamples The samples to convert.
		 * \param _dScale The value by which to scale the samples.
		 * \param _dMin The smallest allowed result.
		 * \param _dMax The largest allowed result.
		 * \return Returns the 8 converted samples.
		 **/
		static inline __m256i								SampleToInt_AVX512( __m512d _vdSamples, double _dScale, double _dMin, double _dMax ) {
			const __m512i viSignMask = _mm512_set1_epi64( INT64_MIN );
			__m512d vdX		= _mm512_mul_pd( _vdSamples, _mm512_set1_pd( _dScale ) );
			__m512d vdT		= _mm512_roundscale_pd( vdX, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC );
			__m512d vdFrac	= _mm512_castsi512_pd( _mm512_andnot_si512( viSignMask, _mm512_castpd_si512( _mm512_sub_pd( vdX, vdT ) ) ) );
			__mmask8 mBump	= _mm512_cmp_pd_mask( vdFrac, _mm512_set1_pd( 0.5 ), _CMP_GE_OQ );
			__m512d vdOne	= _mm512_castsi512_pd( _mm512_or_si512( _mm512_and_si512( _mm512_castpd_si512( vdX ), viSignMask ),
				_mm512_castpd_si512( _mm512_set1_pd( 1.0 ) ) ) );																	// +-1 with the sign of x.
			__m512d vdR		= _mm512_mask_add_pd( vdT, mBump, vdT, vdOne );
			vdR				= _mm512_min_pd( _mm512_max_pd( vdR, _mm512_set1_pd( _dMin ) ), _mm512_set1_pd( _dMax ) );
			vdR				= _mm512_maskz_mov_pd( _mm512_cmp_pd_mask( vdX, vdX, _CMP_ORD_Q ), vdR );							// NaN -> 0.
			return _mm512_cvtpd_epi32( vdR );
		}

		/**
		 * Converts 16 floats to 16 int32_t values (in order) exactly as SampleToInt() does.
		 * 
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _dScale The value by which to scale the samples.
		 * \param _dMin The smallest allowed result.
		 * \param _dMax The largest allowed result.
		 * \return Returns the 16 converted samples.
		 **/
		static inline __m512i								SampleToInt_AVX512( const float * _pfSample, double _dScale, double _dMin, double _dMax ) {
			__m512 vSamples	= _mm512_loadu_ps( _pfSample );
			__m256i viLo	= SampleToInt_AVX512( _mm512_cvtps_pd( _mm512_castps512_ps256( vSamples ) ), _dScale, _dMin, _dMax );
			__m256i viHi	= SampleToInt_AVX512( _mm512_cvtps_pd( _mm256_castsi256_ps( _mm512_extracti64x4_epi64( _mm512_castps_si512( vSamples ), 1 ) ) ),
				_dScale, _dMin, _dMax );
			return _mm512_inserti64x4( _mm512_castsi256_si512( viLo ), viHi, 1 );
		}

		/**
		 * Converts 16 samples from a floating-point format to uint8_t.  Bit-exact with SampleToUi8().
		 * 
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pui8Dst Pointer to the output.  Exactly 16 bytes are written.
		 **/
		static inline void									SampleToUi8_AVX512( const float * _pfSample, uint8_t * _pui8Dst ) {
			__m512i vi32	= _mm512_add_epi32( SampleToInt_AVX512( _pfSample, 127.0, -128.0, 127.0 ), _mm512_set1_epi32( 128 ) );	// 0..255.
			_mm_storeu_si128( reinterpret_cast<__m128i *>(_pui8Dst), _mm512_cvtusepi32_epi8( vi32 ) );
		}

		/**
		 * Converts 16 samples from a floating-point format to int16_t.  Bit-exact with SampleToI16().
		 * 
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pi16Dst Pointer to the output.  Exactly 32 bytes are written.
		 **/
		static inline void									SampleToI16_AVX512( const float * _pfSample, int16_t * _pi16Dst ) {
			__m512i vi32	= SampleToInt_AVX512( _pfSample, 32767.0, -32768.0, 32767.0 );
			_mm256_storeu_si256( reinterpret_cast<__m256i *>(_pi16Dst), _mm512_cvtsepi32_epi16( vi32 ) );
		}

		/**
		 * Converts 16 samples from a floating-point format to 24-bit values held in int32_t.  Bit-exact with SampleToI24().
		 *
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pi32Dst Pointer to the output.  No alignment is required.
		 **/
		static inline void									SampleToI24_AVX512( const float * _pfSample, int32_t * _pi32Dst ) {
			_mm512_storeu_si512( _pi32Dst, SampleToInt_AVX512( _pfSample, 8388607.0, -8388608.0, 8388607.0 ) );
		}
#endif	// #ifdef __AVX512F__

		/**
		 * Converts a block of doubles to int32_t exactly as SampleToInt() does, using AVX-512 or AVX 2 when available.
		 *
		 * \param _pdSrc The samples to convert.
		 * \param _stTotal The number of samples to convert.
		 * \param _pi32Dst The output.  Must hold _stTotal values.
		 * \param _dScale The value by which to scale the samples.
		 * \param _i32Min The smallest allowed result.
		 * \param _i32Max The largest allowed result.
		 **/
		static inline void									SamplesToInt( const double * _pdSrc, size_t _stTotal, int32_t * _pi32Dst, double _dScale, int32_t _i32Min, int32_t _i32Max ) {
			size_t I = 0;
#ifdef __AVX512F__
			if ( IsAvx512FSupported() ) {
				for ( ; I + 8 <= _stTotal; I += 8 ) {
					_mm256_storeu_si256( reinterpret_cast<__m256i *>(_pi32Dst + I), SampleToInt_AVX512( _mm512_loadu_pd( _pdSrc + I ), _dScale, double( _i32Min ), double( _i32Max ) ) );
				}
			}
#endif	// #ifdef __AVX512F__
#ifdef __AVX2__
			if ( IsAvx2Supported() ) {
				for ( ; I + 4 <= _stTotal; I += 4 ) {
					_mm_storeu_si128( reinterpret_cast<__m128i *>(_pi32Dst + I), SampleToInt_AVX2( _mm256_loadu_pd( _pdSrc + I ), _dScale, double( _i32Min ), double( _i32Max ) ) );
				}
			}
#endif	// #ifdef __AVX2__
			for ( ; I < _stTotal; ++I ) {
				_pi32Dst[I] = SampleToInt( _pdSrc[I], _dScale, _i32Min, _i32Max );
			}
		}

		/**
		 * Computes the mean squared error between two waveforms.
		 * 
		 * \param _vA First waveform samples.
		 * \param _vB Second waveform samples.
		 * \returns Mean squared error.
		 */
		template <typename _tType = std::vector<double>>
		static double										GradeSimilarity( const _tType &_vA, const _tType &_vB ) {
			size_t sN = std::min( _vA.size(), _vB.size() );
			double dMSE = 0.0;
			for ( size_t I = 0; I < sN; ++I ) {
				double dErr		= _vA[I] - _vB[I];
				dMSE		   += dErr * dErr;
			}
			return dMSE / double( sN );
		}

		/**
		 * Finds the next fuzzy DC crossing cluster starting at a given sample index.
		 * 
		 * \param _vSamples Waveform samples.
		 * \param _sStartIndex Index to start search.
		 * \param _dThreshold Absolute amplitude threshold for considering zero.
		 * \returns Pair of (firstIndex, lastIndex) of the cluster, or (n,n) if none.
		 */
		template <typename _tType = std::vector<double>>
		static std::pair<size_t, size_t>					FindFuzzyDCCrossing( const _tType &_vSamples, size_t _sStartIndex, double _dThreshold ) {
			size_t sN = _vSamples.size();
			for ( size_t I = _sStartIndex + 1; I < sN; ++I ) {
				double dY0 = _vSamples[I-1];
				double dY1 = _vSamples[I];
				if ( (dY0 > 0 && dY1 < 0) || (dY0 < 0 && dY1 > 0) || (std::abs( dY1 ) <= _dThreshold) ) {
					size_t sFirst	= I - 1;
					size_t sLast	= I;
					size_t J		= I + 1;
					for ( ; J < sN; ++J ) {
						if ( std::abs( _vSamples[J] ) <= _dThreshold ) {
							sLast = J;
						}
						else { break; }
					}
					return std::make_pair( sFirst, sLast );
				}
			}
			return std::make_pair( sN, sN );
		}

		/**
		 * Loads a given region of a given WAV file and analyzes for the given number of HPF’s.  Call within try/catch.
		 * 
		 * \param _pwcPath The WAV file to load.
		 * \param _sNumPoles The number of HPF influences to find.
		 * \param _ui32Start The starting sample from the file to analyze.
		 * \param _ui64Total The number of samples to analyze.
		 * \return Returns true if the file was loaded and analyzed.  False indicates a missing or unloadable file.
		 * \throws std::runtime_error in cases of solver failures, std::bad_alloc on memory failure.
		 **/
		static bool											SolveForHpfs( const char16_t * _pwcPath, size_t _sNumPoles, uint32_t _ui32Start, uint32_t _ui32Total );

		/**
		 * Fits a chain of 1-pole LPF's (CLpfFilter) to a recorded step that also passed through a known chain of HPF's.
		 * 
		 * The model is Offset + Drift * u[n] + Step * G[n], where G is a unit step that occurs at a fractional sample time (StepTime) run from
		 *	rest through the LPF's and then the known HPF's, and u[n] runs linearly from -0.5 to 0.5 across the window (it absorbs the slowly
		 *	decaying HPF tails of earlier edges).  The input to the filters at sample n is clamp( n - StepTime, 0, 1 ), so a step that lands partway
		 *	through a sample period is represented.  The window should start a few samples before the edge.
		 * 
		 * The filters run at _dFilterHz (for example the NES CPU rate), and the result is sampled at _dHz by linear interpolation.  CLpfFilter's
		 *	cut-off parameter depends on the rate at which the filter runs, so _dFilterHz should be the rate at which the fitted filter will be used.
		 * 
		 * The LPF cut-offs and StepTime are found by Levenberg-Marquardt; Offset, Drift, and Step are solved exactly by linear least squares for
		 *	every trial.
		 *
		 * \param _vSamples The recorded step response.
		 * \param _dHz The sample rate of _vSamples.
		 * \param _sNumPoles The number of LPF influences to find.
		 * \param _vHpfs The cut-offs of the HPF's known to be in the signal path, in the same units as CHpfFilter::CreateHpf() (and SolveForHpfs()).  May be empty.
		 * \param _dFilterHz The rate at which the LPF's and HPF's run.  0 (or less) means _dHz.
		 * \param _bPrintProgress If true, intermediate results are printed for every accepted iteration.
		 * \return Returns the fitted cut-offs (sorted from lowest to highest), step time, levels, and final mean-squared error.
		 * \throws std::runtime_error if _sNumPoles is 0 or there are fewer than 4 samples, std::bad_alloc on memory failure.
		 **/
		template <typename _tType = std::vector<double>>
		static PW_LPF_FIT									FitLpfs( const _tType &_vSamples, double _dHz, size_t _sNumPoles, const std::vector<double> &_vHpfs, double _dFilterHz = 0.0, bool _bPrintProgress = false ) {
			if PW_UNLIKELY( !_sNumPoles ) { throw std::runtime_error( "At least 1 LPF must be requested." ); }
			if PW_UNLIKELY( _vSamples.size() < 4 ) { throw std::runtime_error( "At least 4 samples are required to fit LPF's." ); }

			const size_t sTotal = _vSamples.size();
			const double dFilterHz = (_dFilterHz > 0.0) ? _dFilterHz : _dHz;
			const double dRatio = dFilterHz / _dHz;																	// Filter samples per input sample.
			const size_t sFilterTotal = size_t( std::ceil( double( sTotal - 1 ) * dRatio ) ) + 2;
			// CLpfFilter is stable only while 2 * pi * Fc / Hz < 2.
			const double dMaxFc = std::min( dFilterHz * 0.5, dFilterHz / std::numbers::pi ) * 0.9999;
			const double dMinFc = 1.0;
			const double dLogMin = std::log( dMinFc ), dLogMax = std::log( dMaxFc );
			const size_t sNumParms = _sNumPoles + 1;																// ln( Fc ) per pole, then the step time.
			const size_t sStepTimeIdx = _sNumPoles;

			// The linear terms (Offset, Drift, Step) are solved for each trial; these hold the most recent solution.
			double dOffset = 0.0, dDrift = 0.0, dStepSize = 0.0;
			std::vector<double> vResponse( sTotal ), vFilterOut( sFilterTotal ), vU( sTotal );
			for ( size_t I = 0; I < sTotal; ++I ) { vU[I] = double( I ) / double( sTotal - 1 ) - 0.5; }

			/**
			 * Evaluates the model for a set of parameters, solving the linear terms.
			 * 
			 * \param _vParms The parameters (ln( Fc ) per pole, then the step time).
			 * \param _pvResiduals If not nullptr, receives sample - model for every sample.
			 * \return Returns the mean-squared error.
			 **/
			auto Evaluate = [&]( const std::vector<double> &_vParms, std::vector<double> * _pvResiduals ) {
				std::vector<CLpfFilter> vLpfFilters( _sNumPoles );
				std::vector<CHpfFilter> vHpfFilters( _vHpfs.size() );
				for ( size_t P = 0; P < _sNumPoles; ++P ) {
					vLpfFilters[P].CreateLpfD( std::exp( _vParms[P] ), dFilterHz );
					vLpfFilters[P].ResetState();
				}
				for ( size_t P = 0; P < vHpfFilters.size(); ++P ) {
					vHpfFilters[P].CreateHpf( float( _vHpfs[P] ), float( dFilterHz ) );
				}

				// Run the step through the filters at the filter rate.
				const double dStepTime = _vParms[sStepTimeIdx] * dRatio;
				for ( size_t I = 0; I < sFilterTotal; ++I ) {
					double dSample = std::clamp( double( I ) - dStepTime, 0.0, 1.0 );
					for ( size_t J = 0; J < vLpfFilters.size(); ++J ) {
						dSample = vLpfFilters[J].Process( dSample );
					}
					for ( size_t J = 0; J < vHpfFilters.size(); ++J ) {
						dSample = vHpfFilters[J].Process( dSample );
					}
					vFilterOut[I] = dSample;
				}
				// Sample the filtered signal at the input rate.
				for ( size_t I = 0; I < sTotal; ++I ) {
					const double dPos = double( I ) * dRatio;
					const size_t sIdx = std::min( size_t( dPos ), sFilterTotal - 2 );
					const double dFrac = dPos - double( sIdx );
					vResponse[I] = vFilterOut[sIdx] + (vFilterOut[sIdx+1] - vFilterOut[sIdx]) * dFrac;
				}

				// Linear least squares for [1, u, G] via the 3x3 normal equations (Cramer's rule).
				double dS11 = double( sTotal ), dS1u = 0.0, dS1g = 0.0, dSuu = 0.0, dSug = 0.0, dSgg = 0.0;
				double dB1 = 0.0, dBu = 0.0, dBg = 0.0;
				for ( size_t I = 0; I < sTotal; ++I ) {
					const double dU = vU[I], dG = vResponse[I], dY = _vSamples[I];
					dS1u += dU; dS1g += dG; dSuu += dU * dU; dSug += dU * dG; dSgg += dG * dG;
					dB1 += dY; dBu += dU * dY; dBg += dG * dY;
				}
				auto Det3 = []( double _dA, double _dB, double _dC, double _dD, double _dE, double _dF, double _dG, double _dH, double _dI ) {
					return _dA * (_dE * _dI - _dF * _dH) - _dB * (_dD * _dI - _dF * _dG) + _dC * (_dD * _dH - _dE * _dG);
				};
				const double dDet = Det3( dS11, dS1u, dS1g, dS1u, dSuu, dSug, dS1g, dSug, dSgg );
				if ( std::abs( dDet ) > 0.0 ) {
					dOffset = Det3( dB1, dS1u, dS1g, dBu, dSuu, dSug, dBg, dSug, dSgg ) / dDet;
					dDrift = Det3( dS11, dB1, dS1g, dS1u, dBu, dSug, dS1g, dBg, dSgg ) / dDet;
					dStepSize = Det3( dS11, dS1u, dB1, dS1u, dSuu, dBu, dS1g, dSug, dBg ) / dDet;
				}
				else {
					dOffset = dB1 / dS11;
					dDrift = dStepSize = 0.0;
				}

				double dMse = 0.0;
				for ( size_t I = 0; I < sTotal; ++I ) {
					const double dErr = _vSamples[I] - (dOffset + dDrift * vU[I] + dStepSize * vResponse[I]);
					if ( _pvResiduals ) { (*_pvResiduals)[I] = dErr; }
					dMse += dErr * dErr;
				}
				return dMse / double( sTotal );
			};

			// Initial step time: the largest sample-to-sample jump.  The step lies between samples sEdge and sEdge + 1.
			size_t sEdge = 0;
			double dBiggest = -1.0;
			for ( size_t I = 0; I + 1 < sTotal; ++I ) {
				const double dJump = std::abs( _vSamples[I+1] - _vSamples[I] );
				if ( dJump > dBiggest ) {
					dBiggest = dJump;
					sEdge = I;
				}
			}

			// Initial guess: scan step times around the edge and a logarithmic grid of cut-offs (all poles equal) and keep the best.
			std::vector<double> vParms( sNumParms );
			std::vector<double> vBest( sNumParms );
			double dBestFit = DBL_MAX;
			const size_t sFreqSteps = 64;
			for ( double dT = double( sEdge ) - 1.0; dT <= double( sEdge ) + 1.0; dT += 0.125 ) {
				for ( size_t I = 0; I <= sFreqSteps; ++I ) {
					const double dLogFc = dLogMin + (dLogMax - dLogMin) * (I / double( sFreqSteps ));
					std::fill( vParms.begin(), vParms.begin() + _sNumPoles, dLogFc );
					vParms[sStepTimeIdx] = dT;
					const double dThisFit = Evaluate( vParms, nullptr );
					if ( dThisFit < dBestFit ) {
						dBestFit = dThisFit;
						vBest = vParms;
					}
				}
			}
			vParms = vBest;
			for ( size_t I = 0; I < _sNumPoles; ++I ) {
				// Spread the poles over 1 octave around the best scanned frequency so they are free to separate.
				const double dFrac = (_sNumPoles > 1) ? (I / (_sNumPoles - 1.0)) : 0.5;
				vParms[I] = std::clamp( vBest[I] + (dFrac - 0.5) * std::numbers::ln2, dLogMin, dLogMax );
			}

			auto ClampParms = [&]( std::vector<double> &_vP ) {
				for ( size_t I = 0; I < _sNumPoles; ++I ) { _vP[I] = std::clamp( _vP[I], dLogMin, dLogMax ); }
				_vP[sStepTimeIdx] = std::clamp( _vP[sStepTimeIdx], -1.0, double( sTotal - 1 ) );
			};

			// Levenberg-Marquardt with central-difference Jacobians.
			std::vector<double> vRes( sTotal ), vResP( sTotal ), vResM( sTotal );
			std::vector<double> vJac( sTotal * sNumParms );
			double dCurFit = Evaluate( vParms, &vRes );
			double dLambda = 1.0e-3;
			const double dDeriv = 1.0e-6;
			for ( size_t K = 0; K < 500; ++K ) {
				for ( size_t P = 0; P < sNumParms; ++P ) {
					std::vector<double> vPp = vParms, vPm = vParms;
					vPp[P] += dDeriv;
					vPm[P] -= dDeriv;
					Evaluate( vPp, &vResP );
					Evaluate( vPm, &vResM );
					for ( size_t I = 0; I < sTotal; ++I ) {
						vJac[I*sNumParms+P] = (vResP[I] - vResM[I]) / (2.0 * dDeriv);
					}
				}
				std::vector<double> vJtJ( sNumParms * sNumParms, 0.0 ), vJtR( sNumParms, 0.0 );
				for ( size_t I = 0; I < sTotal; ++I ) {
					for ( size_t A = 0; A < sNumParms; ++A ) {
						vJtR[A] += vJac[I*sNumParms+A] * vRes[I];
						for ( size_t B = 0; B < sNumParms; ++B ) {
							vJtJ[A*sNumParms+B] += vJac[I*sNumParms+A] * vJac[I*sNumParms+B];
						}
					}
				}

				bool bAccepted = false;
				double dMaxRelStep = 0.0;
				while ( dLambda < 1.0e16 ) {
					// Solve (JtJ + lambda * diag( JtJ )) * Delta = -JtR by Gaussian elimination with partial pivoting.
					std::vector<double> vM = vJtJ, vDelta( sNumParms );
					for ( size_t A = 0; A < sNumParms; ++A ) {
						vM[A*sNumParms+A] += dLambda * std::max( vJtJ[A*sNumParms+A], 1.0e-30 );
						vDelta[A] = -vJtR[A];
					}
					for ( size_t C = 0; C < sNumParms; ++C ) {
						size_t sPivot = C;
						for ( size_t R = C + 1; R < sNumParms; ++R ) {
							if ( std::abs( vM[R*sNumParms+C] ) > std::abs( vM[sPivot*sNumParms+C] ) ) { sPivot = R; }
						}
						if ( sPivot != C ) {
							for ( size_t X = 0; X < sNumParms; ++X ) { std::swap( vM[C*sNumParms+X], vM[sPivot*sNumParms+X] ); }
							std::swap( vDelta[C], vDelta[sPivot] );
						}
						const double dDiag = vM[C*sNumParms+C];
						if ( dDiag == 0.0 ) { continue; }
						for ( size_t R = C + 1; R < sNumParms; ++R ) {
							const double dF = vM[R*sNumParms+C] / dDiag;
							for ( size_t X = C; X < sNumParms; ++X ) { vM[R*sNumParms+X] -= dF * vM[C*sNumParms+X]; }
							vDelta[R] -= dF * vDelta[C];
						}
					}
					for ( size_t C = sNumParms; C--; ) {
						for ( size_t X = C + 1; X < sNumParms; ++X ) { vDelta[C] -= vM[C*sNumParms+X] * vDelta[X]; }
						vDelta[C] = (vM[C*sNumParms+C] != 0.0) ? (vDelta[C] / vM[C*sNumParms+C]) : 0.0;
					}

					// vJac holds d( residual ) / d( parameter ), so -(JtJ)^-1 * JtR is the Gauss-Newton step.
					std::vector<double> vTry = vParms;
					for ( size_t A = 0; A < sNumParms; ++A ) { vTry[A] += vDelta[A]; }
					ClampParms( vTry );
					std::vector<double> vTryRes( sTotal );
					const double dTryFit = Evaluate( vTry, &vTryRes );
					if ( dTryFit < dCurFit ) {
						dMaxRelStep = 0.0;
						for ( size_t A = 0; A < sNumParms; ++A ) {
							dMaxRelStep = std::max( dMaxRelStep, std::abs( vTry[A] - vParms[A] ) / std::max( std::abs( vParms[A] ), 1.0 ) );
						}
						vParms = vTry;
						vRes = vTryRes;
						dCurFit = dTryFit;
						dLambda = std::max( dLambda / 3.0, 1.0e-12 );
						bAccepted = true;
						break;
					}
					dLambda *= 4.0;
				}

				if ( _bPrintProgress && bAccepted ) {
					std::string sPrintMe = std::format( "{}:", K );
					for ( size_t P = 0; P < _sNumPoles; ++P ) {
						sPrintMe += std::format( " {:.15f}", std::exp( vParms[P] ) );
					}
					sPrintMe += std::format( "  t={:.6f}  MSE={:.6e}\r\n", vParms[sStepTimeIdx], dCurFit );
					std::cout << sPrintMe.c_str();
#ifdef _WIN32
					::OutputDebugStringA( sPrintMe.c_str() );
#endif	// #ifdef _WIN32
				}

				// Stop when no step improves the fit or the steps have shrunk to the limit of double precision.
				if ( !bAccepted || dMaxRelStep < 4.0 * DBL_EPSILON ) { break; }
			}
			dCurFit = Evaluate( vParms, &vRes );		// Re-sync the linear terms with the final parameters.

			PW_LPF_FIT lfRet;
			for ( size_t P = 0; P < _sNumPoles; ++P ) { lfRet.vCutOffs.push_back( std::exp( vParms[P] ) ); }
			std::sort( lfRet.vCutOffs.begin(), lfRet.vCutOffs.end() );
			lfRet.dStepTime = vParms[sStepTimeIdx];
			lfRet.dOffset = dOffset - 0.5 * dDrift;				// Level at the first sample of the window (u = -0.5).
			lfRet.dDrift = dDrift;
			lfRet.dStepSize = dStepSize;
			lfRet.dFit = dCurFit;
			return lfRet;
		}

		/**
		 * Loads a given region of a given WAV file and analyzes for the given number of LPF’s, taking into account a given set of HPF’s already
		 *	known to be in the signal path.  Call within try/catch.
		 *
		 * \param _pwcPath The WAV file to load.
		 * \param _sNumPoles The number of LPF influences to find.
		 * \param _ui32Start The starting sample from the file to analyze.
		 * \param _ui32Total The number of samples to analyze.
		 * \param _vHpfs The cut-offs of the HPF’s known to be in the signal path (as found by SolveForHpfs()).  May be empty.
		 * \param _dFilterHz The rate at which the filters run (for example the NES CPU rate).  0 means the WAV file’s rate.
		 * \return Returns true if the file was loaded and analyzed.  False indicates a missing or unloadable file.
		 * \throws std::runtime_error in cases of solver failures, std::bad_alloc on memory failure.
		 **/
		static bool											SolveForLpfs( const char16_t * _pwcPath, size_t _sNumPoles, uint32_t _ui32Start, uint32_t _ui32Total, const std::vector<double> &_vHpfs, double _dFilterHz = 0.0 );

		/**
		 * Starts at a given sample index and finds the size and position of the next peak.
		 * 
		 * \param _vSamples Waveform samples.
		 * \param _sStart The sample at which to start the search.
		 * \param _dSize Holds the returned size of the peak.
		 * \return Returns the index where the next peak ends.
		 **/
		template <typename _tType = std::vector<double>>
		static size_t										GetPeak( const _tType &_vSamples, size_t _sStart, double &_dSize ) {
			if ( _vSamples.size() - _sStart < 2 ) { return _vSamples.size(); }
			size_t sIdx = _sStart + 1;
			double dDiff = _vSamples[sIdx] - _vSamples[sIdx-1];
			
			size_t J = sIdx + 1;
			while ( J < _vSamples.size() ) {
				double dTmpDiff = _vSamples[J] - _vSamples[J-1];
				if ( (dTmpDiff > 0.0 && dDiff > 0.0) ||
					(dTmpDiff < 0.0 && dDiff < 0.0) ) {
					dDiff += dTmpDiff;
					++sIdx;
					++J;
				}
				else { break; }
			}
			_dSize = std::abs( dDiff );
			return sIdx;
		}

		/**
		 * Finds a rising or falling edge of at least the given magnitude from the starting point.  Returns the sample of the found sample and the given sample.
		 * 
		 * \param _vSamples Waveform samples.
		 * \param _sStart The sample at which to start the search.
		 * \param _dThresh The threshold for considering a ris or fall part of a square wave.
		 * \param _dFoundValue The returned value unless _vSamples.size() is returned.
		 * \return Returns the index of the found edge or _vSamples.size().
		 **/
		template <typename _tType = std::vector<double>>
		static size_t										FindNextEdge( const _tType &_vSamples, size_t _sStart, double _dThresh, double &_dFoundValue ) {
			if ( _vSamples.size() - _sStart < 2 ) { return _vSamples.size(); }

			for ( auto I = _sStart; I < _vSamples.size(); ++I ) {
				I = GetPeak( _vSamples, I, _dFoundValue );
				if ( _dFoundValue >= _dThresh ) { return I; }
			}
			return _vSamples.size();
		}

		/**
		 * Prints the levels of found square-wav rising and falling edges.
		 * 
		 * \param _pwcPath The WAV file to load.
		 * \param _dThresh The threshold for differentiating between noise and square-wave edges.
		 * \param _ui32Start The starting sample from the file to analyze.
		 * \param _ui64Total The number of samples to analyze.
		 * \return Returns true if the file was loaded and analyzed.  False indicates a missing or unloadable file.
		 * \throws std::runtime_error in cases of solver failures, std::bad_alloc on memory failure.
		 **/
		static bool											FindSquareVolumes( const char16_t * _pwcPath, double _dThresh, uint32_t _ui32Start, uint32_t _ui32Total );

		/**
		 * Analyses a change in volume relative to an absolute volume (found in _pwcPath1).
		 * 
		 * \param _pwcPath1 The first WAV file to load.
		 * \param _ui32Start1 The starting sample from the first file to analyze.
		 * \param _ui32Total1 The number of samples to analyze.
		 * \param _pwcPath2 The second WAV file to load.
		 * \param _ui32Start2 The starting sample from the second file to analyze.
		 * \return Returns true if the files have been loaded and analyzed.  False indicates a missing or unloadable file.
		 * \throws std::runtime_error in cases of solver failures, std::bad_alloc on memory failure.
		 **/
		static bool											FindDiffInVolumeByVolume( const char16_t * _pwcPath1, uint32_t _ui32Start1, uint32_t _ui32Total1, const char16_t * _pwcPath2, uint32_t _ui32Start2 );
	};

}	// namespace pw
