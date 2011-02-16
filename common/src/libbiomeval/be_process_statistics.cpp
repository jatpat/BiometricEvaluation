/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <fstream>
#include <sstream>

#include <sys/resource.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <be_error.h>
#include <be_text.h>
#include <be_time.h>
#include <be_process_statistics.h>
#include <be_io_utility.h>

using namespace BiometricEvaluation;

/*
 * There is no standard method to obtain process statistics from the OS.
 * So we'll define static-local functions here for each OS that is supported.
 * Unfortunately, these functions may be dependent not only on the OS, but
 * a specfic version of the OS, but we'll try to avoid that.
 * The alternative is to either link with an OS library that provides what
 * we need, or import the source code; either way adds its own set of
 * complications.
 */

/*
 * Define a common struct used by all OS-dependent functions to pass
 * back process statistics.
 */
struct _pstats {
	string procname;
	uint64_t vmrss;
	uint64_t vmsize;
	uint64_t vmpeak;
	uint64_t vmdata;
	uint64_t vmstack;
	uint32_t threads;
};
typedef struct _pstats PSTATS;
static const string LogSheetHeader =
    "Entry Usertime Systime RSS VMSize VMPeak VMData VMStack Threads";

/*
 * Define a function to be used for Linux, to grab the OS statistics.
 */
#ifdef Linux

static const string ProcNameProp = "Name";
static const string VmRSSProp = "VmRSS";
static const string VmSizeProp = "VmSize";
static const string VmPeakProp = "VmPeak";
static const string VmDataProp = "VmData";
static const string VmStackProp = "VmStk";
static const string ThreadsProp = "Threads";

static PSTATS
_internalGetPstats()
    throw (Error::StrategyError, Error::NotImplemented)
{
	pid_t pid = getpid();
	ostringstream tp;
	tp <<  "/proc/" << pid << "/status";

	if (!IO::Utility::fileExists(tp.str()))
		throw Error::StrategyError("Could not find " + tp.str() + ".");

	/* Read the process status into a string so we can parse it. */
	std::ifstream ifs(tp.str().c_str());
	if (!ifs)
		throw Error::StrategyError("Could not open " + tp.str() + ".");

	/*
	 * The status info for a process is composed on n lines in this form:
	 *	key: value <units>
	 * so, for example:
	 *	VmSize:    2164 kB
	 */
	string oneline, key, value;
	string::size_type idx;
	PSTATS stats;
	while (!ifs.eof()) {
		std::getline(ifs, oneline);
		idx = oneline.find(":");
		key = oneline.substr(0, idx);
		value = oneline.substr(idx + 1, oneline.length());
		Text::removeLeadingTrailingWhitespace(key);

		//XXX May want to remove non-digits in value string

		/*
		 * The call to strtoll() works for all the stat values here
		 * because the entry in /proc/<pid>/status has the number
		 * separated from the units by a whitespace.
		 */
		if (key == ProcNameProp) {
			Text::removeLeadingTrailingWhitespace(value);
			stats.procname = value;
			continue;
		}
		if (key == VmRSSProp) {
			stats.vmrss = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
		if (key == VmSizeProp) {
			stats.vmsize = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
		if (key == VmPeakProp) {
			stats.vmpeak = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
		if (key == VmDataProp) {
			stats.vmdata = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
		if (key == VmStackProp) {
			stats.vmstack = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
		if (key == ThreadsProp) {
			stats.threads = (uint64_t)std::strtoll(value.c_str(),
			    NULL, 10);;
			continue;
		}
	}
	return (stats);
}

/*
 * Local function to get usage stats from the Darwin (Mac OS-X) OS.
 */
#elif Darwin
static PSTATS
_internalGetPstats()
    throw (Error::StrategyError, Error::NotImplemented)
{
//XXX Replace with Darwin-specific info retrieval
	throw Error::NotImplemented();
}
#else

/*
 * The default, not-implemented-here stats function.
 */
static PSTATS
_internalGetPstats()
    throw (Error::StrategyError, Error::NotImplemented)
{
	throw Error::NotImplemented();
}
#endif

BiometricEvaluation::Process::Statistics::Statistics()
{
	_logCabinet = NULL;
	_logging = false;
}

BiometricEvaluation::Process::Statistics::Statistics(
    IO::LogCabinet * const logCabinet)
    throw (Error::NotImplemented,
	Error::ObjectExists,
	Error::StrategyError)
{
	_logCabinet = logCabinet;
	IO::LogSheet *tempLS;
	ostringstream lsname, descr;
	PSTATS ps = _internalGetPstats();
	lsname << ps.procname << "-" << getpid() << ".stats.log";
	descr << "Statistics for " << ps.procname << "(PID " << getpid() << ")";
	try {
		tempLS = logCabinet->newLogSheet(lsname.str(), descr.str());
	} catch (Error::ObjectExists &e) {
		throw Error::StrategyError("Logsheet already exists.");
	} catch (Error::StrategyError &e) {
		throw e;
	}
	_logging = true;
	tempLS->writeComment(LogSheetHeader);
	_logSheet.reset(tempLS);
}

void
BiometricEvaluation::Process::Statistics::getCPUTimes(
    uint64_t *usertime,
    uint64_t *systemtime)
    throw (Error::StrategyError, Error::NotImplemented)
{
	struct rusage ru;
	int ret = getrusage(RUSAGE_SELF, &ru);
	if (ret != 0)
		throw Error::StrategyError("OS call failed: " +
		    Error::errorStr());

	if (usertime != NULL)
		*usertime = (uint64_t)(ru.ru_utime.tv_sec *
		    Time::MicrosecondsPerSecond + ru.ru_utime.tv_usec);
	if (systemtime != NULL)
		*systemtime = (uint64_t)(ru.ru_stime.tv_sec *
		    Time::MicrosecondsPerSecond + ru.ru_stime.tv_usec);
}

void
BiometricEvaluation::Process::Statistics::getMemorySizes(
    uint64_t *vmrss,
    uint64_t *vmsize,
    uint64_t *vmpeak,
    uint64_t *vmdata,
    uint64_t *vmstack)
    throw (Error::StrategyError, Error::NotImplemented)
{
	/* Let exceptions from this call float out */
	PSTATS ps = _internalGetPstats();
	if (vmrss != NULL)
		*vmrss = ps.vmrss;
	if (vmsize != NULL)
		*vmsize = ps.vmsize;
	if (vmpeak != NULL)
		*vmpeak = ps.vmpeak;
	if (vmdata != NULL)
		*vmdata = ps.vmdata;
	if (vmstack != NULL)
		*vmstack = ps.vmstack;
}

uint32_t
BiometricEvaluation::Process::Statistics::getNumThreads()
    throw (Error::StrategyError, Error::NotImplemented)
{
	PSTATS ps = _internalGetPstats();
	return (ps.threads);
}

void
BiometricEvaluation::Process::Statistics::logStats()
    throw (Error::ObjectDoesNotExist,
    Error::StrategyError, Error::NotImplemented)
{
	if (!_logging)
		throw Error::ObjectDoesNotExist();

	PSTATS ps = _internalGetPstats();
	uint64_t usertime, systemtime;
	this->getCPUTimes(&usertime, &systemtime);
	*_logSheet << usertime << " " << systemtime << " ";
	*_logSheet << ps.vmrss << " " << ps.vmsize << " " << ps.vmpeak << " ";
	*_logSheet << ps.vmdata << " " << ps.vmstack << " " << ps.threads;
	_logSheet->newEntry();
}