/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IMAGE_PNG__
#define __BE_IMAGE_PNG__

#include <be_image_image.h>

/* png.h forward-declares */
extern "C" {
	typedef size_t png_size_t;
	typedef unsigned char png_byte;
	typedef png_byte * png_bytep;
	
	struct png_struct_def;
	typedef struct png_struct_def png_struct;
	typedef png_struct * png_structp;
	
	typedef const char * png_const_charp;
}

namespace BiometricEvaluation
{
	namespace Image
	{
		/**
		 * @brief
		 * A PNG-encoded image.
		 */
		class PNG : public Image
		{
		public:
			PNG(
			    const uint8_t *data,
			    const uint64_t size);

			PNG(
			    const Memory::uint8Array &data);

			~PNG() = default;

			Memory::uint8Array
			getRawData()
			    const;

			Memory::uint8Array
			getRawGrayscaleData(
			    uint8_t depth) const;

			/**
			 * Whether or not data is a PNG image.
			 *
			 * @param[in] data
			 *	The buffer to check.
			 * @param[in] size
			 *	The size of data.
			 *
			 * @return
			 *	true if data appears to be a PNG image, false
			 *	otherwise
			 */
			static bool
			isPNG(
			    const uint8_t *data,
			    uint64_t size);

		protected:

		private:
			/**
			 * @brief
			 * Wrapper for reading PNG-encoded data from an
			 * AutoArray with libpng.
			 */
			struct png_buffer
			{
				/** PNG-encoded buffer */
				const uint8_t *data;
				/** Size of data */
				const uint64_t size;
				/** Number of bytes currently read by libpng */
				png_size_t offset;

			};
			using png_buffer = struct png_buffer;
		
			/**
			 * @brief
			 * libpng callback to read data from an AutoArray.
			 *
			 * @param png_ptr
			 *	Pointer to a PNG struct for the image.
			 * @param buffer
			 *	Pointer to a png_buffer struct.
			 * @param length
			 *	Amount of data to copy from buffer->data
			 *	into buffer, starting from buffer->offset.
			 *
			 * @throw Error::StrategyError
			 *	Input buffer given to libpng is nullptr or
			 *	libpng wants to read more data from input 
			 *	buffer than available.
			 */
			static void
			png_read_mem_src(
			    png_structp png_ptr,
			    png_bytep buffer,
			    png_size_t length);

		        /**
		         * @brief
		         * Convert libpng errors into C++ exceptions.
		         *
		         * @param png_ptr
		         *	Pointer to a PNG struct for the image.
		         * @param msg
		         *	C-style string containing an error message,
		         *	generated by libpng.
		         *
		         * @throw Error::StrategyError
		         *	Always thrown with msg.
		         */
		        static void
			png_error(
			    png_structp png_ptr,
			    png_const_charp msg);
		};
	}
}

#endif /* __BE_IMAGE_PNG__ */

