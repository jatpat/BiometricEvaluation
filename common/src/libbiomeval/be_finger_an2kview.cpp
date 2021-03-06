/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <be_finger_an2kview.h>
#include <be_memory_autobuffer.h>
#include <be_io_utility.h>
extern "C" {
#include <an2k.h>
}

namespace BE = BiometricEvaluation;

BiometricEvaluation::Finger::AN2KView::AN2KView(
    const std::string filename,
    const RecordType typeID,
    const uint32_t recordNumber) :
    BiometricEvaluation::View::AN2KView(filename, typeID, recordNumber)
{
	readImageRecord(typeID, recordNumber);
}

BiometricEvaluation::Finger::AN2KView::AN2KView(
    Memory::uint8Array &buf,
    const RecordType typeID,
    const uint32_t recordNumber) :
    BiometricEvaluation::View::AN2KView(buf, typeID, recordNumber)
{
	readImageRecord(typeID, recordNumber);
}

/******************************************************************************/
/* Public functions.                                                          */
/******************************************************************************/

BiometricEvaluation::Finger::PositionSet
BiometricEvaluation::Finger::AN2KView::getPositions() const
{
	return (_positions);
}

BiometricEvaluation::Finger::Impression
BiometricEvaluation::Finger::AN2KView::getImpressionType() const
{
	return (_imp);
}

std::vector<BiometricEvaluation::Finger::AN2KMinutiaeDataRecord>
BiometricEvaluation::Finger::AN2KView::getMinutiaeDataRecordSet()
    const
{
	return (_minutiaeDataRecordSet);
}

BiometricEvaluation::Finger::Position
BiometricEvaluation::Finger::AN2KView::convertPosition(int an2kFGP)
{
	Finger::Position fgp;
	switch (an2kFGP) {
	case 0: fgp = Finger::Position::Unknown; break;
	case 1: fgp = Finger::Position::RightThumb; break;
	case 2: fgp = Finger::Position::RightIndex; break;
	case 3: fgp = Finger::Position::RightMiddle; break;
	case 4: fgp = Finger::Position::RightRing; break;
	case 5: fgp = Finger::Position::RightLittle; break;
	case 6: fgp = Finger::Position::LeftThumb; break;
	case 7: fgp = Finger::Position::LeftIndex; break;
	case 8: fgp = Finger::Position::LeftMiddle; break;
	case 9: fgp = Finger::Position::LeftRing; break;
	case 10: fgp = Finger::Position::LeftLittle; break;
	case 11: fgp = Finger::Position::PlainRightThumb; break;
	case 12: fgp = Finger::Position::PlainLeftThumb; break;
	case 13: fgp = Finger::Position::PlainRightFourFingers; break;
	case 14: fgp = Finger::Position::PlainLeftFourFingers; break;
	case 15: fgp = Finger::Position::LeftRightThumbs; break;
	case 19: fgp = Finger::Position::EJI; break;
	default:
    		throw Error::DataError("Invalid Position Code");
	}
	return (fgp);
}

BiometricEvaluation::Finger::PositionSet
BiometricEvaluation::Finger::AN2KView::populateFGP(FIELD* field)
{
	int an2kFGP;
	Finger::PositionSet fgp;
	
	for (int i = 0; i < field->num_subfields; i++) {
		an2kFGP = atoi((char *)field->subfields[i]->items[0]->value);
		fgp.push_back(Finger::AN2KView::convertPosition(an2kFGP));
	}
	return (fgp);
}

BiometricEvaluation::Finger::Impression
BiometricEvaluation::Finger::AN2KView::convertImpression(
    const unsigned char *str)
{
	long an2k_imp;
	an2k_imp = strtol((const char *)str, nullptr, 10);
	switch (an2k_imp) {
	case 0: return (Finger::Impression::LiveScanPlain);
	case 1: return (Finger::Impression::LiveScanRolled);
	case 2: return (Finger::Impression::NonLiveScanPlain);
	case 3: return (Finger::Impression::NonLiveScanRolled);
	case 4: return (Finger::Impression::LatentImpression);
	case 5: return (Finger::Impression::LatentTracing);
	case 6: return (Finger::Impression::LatentPhoto);
	case 7: return (Finger::Impression::LatentLift);
	case 8: return (Finger::Impression::LiveScanVerticalSwipe);
	case 20: return (Finger::Impression::LiveScanOpticalContactPlain);
	case 21: return (Finger::Impression::LiveScanOpticalContactRolled);
	case 22: return (Finger::Impression::LiveScanNonOpticalContactPlain);
	case 23: return (Finger::Impression::LiveScanNonOpticalContactRolled);
	case 24: return (Finger::Impression::LiveScanOpticalContactlessPlain);
	case 25: return (Finger::Impression::LiveScanOpticalContactlessRolled);
	case 26: return (Finger::Impression::LiveScanNonOpticalContactlessPlain);
	case 27: return (Finger::Impression::LiveScanNonOpticalContactlessRolled);
	case 28: return (Finger::Impression::Other);
	case 29: return (Finger::Impression::Unknown);
	default:
		throw Error::DataError("Invalid IMP value");
	}
}

BiometricEvaluation::Finger::FingerImageCode
BiometricEvaluation::Finger::AN2KView::convertFingerImageCode(
    const char *str)
{
	if (strncmp(str, "EJI", 3) == 0)
		return (BE::Finger::FingerImageCode::EJI);
	else if (strncmp(str, "TIP", 3) == 0)
		return (BE::Finger::FingerImageCode::RolledTip);
	else if (strncmp(str, "FV1", 3) == 0)
		return (BE::Finger::FingerImageCode::FullFingerRolled);
	else if (strncmp(str, "FV2", 3) == 0)
		return (BE::Finger::FingerImageCode::FullFingerPlainLeft);
	else if (strncmp(str, "FV3", 3) == 0)
		return (BE::Finger::FingerImageCode::FullFingerPlainCenter);
	else if (strncmp(str, "FV4", 3) == 0)
		return (BE::Finger::FingerImageCode::FullFingerPlainRight);
	else if (strncmp(str, "PRX", 3) == 0)
		return (BE::Finger::FingerImageCode::ProximalSegment);
	else if (strncmp(str, "DST", 3) == 0)
		return (BE::Finger::FingerImageCode::DistalSegment);
	else if (strncmp(str, "MED", 3) == 0)
		return (BE::Finger::FingerImageCode::MedialSegment);
	else if (strncmp(str, "NA", 2) == 0)
		return (BE::Finger::FingerImageCode::NA);
	else
		throw BE::Error::DataError("Invalid finger image code value");
}

void
BiometricEvaluation::Finger::AN2KView::addMinutiaeDataRecord(
    BiometricEvaluation::Finger::AN2KMinutiaeDataRecord &mdr)
{
	_minutiaeDataRecordSet.push_back(mdr);
}

/******************************************************************************/
/* Local functions.                                                           */
/******************************************************************************/

/******************************************************************************/
/* Protected functions.                                                       */
/******************************************************************************/

/******************************************************************************/
/* Private functions.                                                         */
/******************************************************************************/

void
BiometricEvaluation::Finger::AN2KView::readImageRecord(
    const RecordType typeID,
    const uint32_t recordNumber)
{
	switch (typeID) {
		case RecordType::Type_3:
		case RecordType::Type_4:	
		case RecordType::Type_5:
		case RecordType::Type_6:
			break;
		default:
			throw Error::ParameterError("Invalid Record Type ID");
	}
	RECORD *record = AN2KView::getAN2KRecord();

	FIELD *field;
	int idx;

	if (lookup_ANSI_NIST_field(&field, &idx, IMP_ID, record) != TRUE)
		throw Error::DataError("Field IMP not found");
	_imp = convertImpression(field->subfields[0]->items[0]->value);
							
	if (lookup_ANSI_NIST_field(&field, &idx, FGP_ID, record) != TRUE)
		throw Error::DataError("Field FGP not found");
	_positions = populateFGP(field);
}

