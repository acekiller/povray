// fatal errors
enum
{
	kNoError              = 0,
	kNoErr                = kNoError,
	kParamErr             = -1,
	kMemFullErr           = -2,
	kOutOfMemoryErr       = kMemFullErr,
	kInvalidDataSizeErr   = -3,
	kCannotHandleDataErr  = -4,
	kNullPointerErr       = -5,
	kChecksumErr          = -6,
	kParseErr             = -7,
	kCannotOpenFileErr    = -8,
	kInvalidDestAddrErr   = -9,
	kCannotConnectErr     = -10,
	kDisconnectedErr      = -11,
	kHostDisconnectedErr  = -12,
	kNetworkDataErr       = -13,
	kNetworkConnectionErr = -14,
	kObjectAccessErr      = -15,
	kVersionErr           = -16,
	kFileDataErr          = -17,
	kAuthorisationErr     = -18,
	kDataTypeErr          = -19,
	kTimeoutErr           = -20
};

// non fatal errors
enum
{
	kFalseErr             = 1,
	kOutOfSyncErr         = 2,
	kNotNowErr            = kOutOfSyncErr,
	kQueueFullErr         = 3
};
