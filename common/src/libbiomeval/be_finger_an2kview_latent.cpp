/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_finger_an2kview_latent.h>
#include <be_io_utility.h>
extern "C" {
#include <an2k.h>
}

namespace BE = BiometricEvaluation;

BiometricEvaluation::Finger::AN2KViewLatent::AN2KViewLatent(
    const std::string &filename,
    const uint32_t recordNumber) :
    AN2KViewVariableResolution(filename, RecordType::Type_13, recordNumber)
{
	/* Parent classes handle all fields */
}

BiometricEvaluation::Finger::AN2KViewLatent::AN2KViewLatent(
    Memory::uint8Array &buf,
    const uint32_t recordNumber) :
    AN2KViewVariableResolution(buf, RecordType::Type_13, recordNumber)
{
	/* Parent classes handle all fields */
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/

BiometricEvaluation::Finger::PositionDescriptors
BiometricEvaluation::Finger::AN2KViewLatent::getSearchPositionDescriptors()
    const
{
	return (getPositionDescriptors());
}

BiometricEvaluation::View::AN2KViewVariableResolution::QualityMetricSet
BiometricEvaluation::Finger::AN2KViewLatent::getLatentQualityMetric()
    const
{
	return (getQualityMetric());
}

