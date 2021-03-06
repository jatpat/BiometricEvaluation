/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_VIEW_AN2KVIEW_VARRES_H__
#define __BE_VIEW_AN2KVIEW_VARRES_H__

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <be_view_an2kview.h>
#include <be_error_exception.h>
#include <be_finger_an2kminutiae_data_record.h>

/* an2k.h forward declares */
struct field;
typedef field FIELD;

namespace BiometricEvaluation 
{
	namespace View
	{
		/**
		 * @brief
		 * A class to represent single view based on an ANSI/NIST
		 * record.
		 * @details
		 * The view represents a variable resolution (Type-13/14/15)
		 * AN2K record.
		 */
		class AN2KViewVariableResolution : public AN2KView {
		public:
		
			/**
			* @brief
			* A structure to represent an AN2K quality metric.
			* @details
			* The quality metric is an optional field in the Type-13
			* (Latent), Type-14 (Fingerprint and Segmentation) and 
			* Type-15 (Palmprint). The NIST Quality Metric is also
			* returned via this structure.
			*/
			struct AN2KQualityMetric
			{
				Finger::Position	position;
				uint8_t			score;
				uint16_t		vendorID;
				uint16_t		productCode;
			};
			using AN2KQualityMetric = struct AN2KQualityMetric;
			using QualityMetricSet = std::vector<AN2KQualityMetric>;
			
			/**
			 * @brief
			 * Read a Quality Metric Set from a variable resolution
			 * AN2K record.
			 *
			 * @param[in] field
			 *	A pointer to the field within the AN2K record.
			 *
			 * @throws Error::DataError
			 *	The data contains an invalid value.
			 */
			static QualityMetricSet
			extractQuality(FIELD *field);

			/**
			 * @return
			 *	The source agency.
			 */
			std::string getSourceAgency() const;

			/**
			 * @return
			 *	The capture date.
			 */
			std::string getCaptureDate() const;

			/**
			 * @brief
			 * Obtain the comment field.
			 * @details
			 * The comment field is optional in an AN2K record.
			 * @return
			 *	The comment field, empty string if not present.
			 */
			std::string getComment() const;
			
			/**
			 * @brief
			 * Obtain a user-defined field.
			 * @details
			 * Fields are retrieved on-demand and then cached.
			 *
			 * @param[in] field
			 *	The field number to retrieve.
			 *
			 * @return
			 *	Raw bytes read from the field.
			 *
			 * @throw Error::ObjectDoesNotExist
			 * There is no user-defined field with the requested
			 * field number.
			 * @throw Error::ParameterError
			 *	Invalid value for field.
			 * @throw Error::StrategyError
			 * Field could not be cached.
			 */
			Memory::uint8Array
			getUserDefinedField(
			    const uint16_t field) const;

			/**
			 * @brief
			 * Read raw bytes from a user-defined AN2K field.
			 *
			 * @param[in] record
			 *	Pointer to a RECORD containing the
			 *	user-defined field.
			 * @param[in] fieldID
			 *	The user-defined field number.
			 *
			 * @return
			 *	Raw bytes from field.
			 *
 			 * @throw Error::ObjectDoesNotExist
			 * There is no user-defined field with the requested
			 * field number.
			 * @throw Error::ParameterError
			 *	Invalid value for fieldID.
			 */
			static Memory::uint8Array
			parseUserDefinedField(
			    const RECORD* const record,
			    int fieldID);

		protected:
			/**
			 * @brief
			 * Construct an AN2K finger view from a file.
			 * @details
			 * The file must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 */
			AN2KViewVariableResolution(
			    const std::string &filename,
			    const RecordType typeID,
			    const uint32_t recordNumber);

			/**
			 * @brief
			 * Construct an AN2K finger view using from a memory
			 * buffer.
			 * @details
			 * The buffer must contain the entire AN2K record, not
			 * just the finger image and/or minutiae records.
			 */
			AN2KViewVariableResolution(
			    Memory::uint8Array &buf,
			    const RecordType typeID,
			    const uint32_t recordNumber);

			/** 
			 * @brief
			 * Obtain quality metrics for associated image record
			 *
			 * @return
			 * Quality metrics
			 */
			QualityMetricSet
			getQualityMetric()
			    const;

		private:
			void readImageRecord(
			    const RecordType typeID);

			std::string _sourceAgency;
			std::string _captureDate;
			std::string _comment;
			/** Metrics of image quality score data */
			QualityMetricSet _qms;
			/** User-defined Fields (populated on access) */
			mutable std::map<uint16_t, Memory::uint8Array> _udf;
		};
		
		/**
		 * @brief
		 * Output stream overload for AN2KQualityMetric.
		 *
		 * @param[in] stream
		 *	Stream on which to append formatted AN2KQualityMetric
		 *	information.
		 * @param[in] qm
		 *	AN2KQualityMetric information to append to stream.
		 *
		 * @return
		 *	stream with a qm textual representation appended.
		 */
		std::ostream&
		operator<<(
		    std::ostream &stream,
		    const AN2KViewVariableResolution::AN2KQualityMetric &qm);

	}
}
#endif /* __BE_VIEW_AN2KVIEW_VARRES_H__ */

