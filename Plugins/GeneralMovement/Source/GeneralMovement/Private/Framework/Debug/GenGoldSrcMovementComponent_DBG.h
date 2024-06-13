// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#undef FLog
#undef CFLog

#if !NO_LOGGING

// Utility macros to quickly log messages from goldsrc movement component functions.
#define FLog(Verbosity, Format, ...)\
  GMC_LOG(Verbosity, TEXT("UGenGoldSrcMovementComponent::%s: ") TEXT(Format), *FString(__func__), ##__VA_ARGS__)
#define CFLog(Condition, Verbosity, Format, ...)\
  GMC_CLOG(Condition, Verbosity, TEXT("UGenGoldSrcMovementComponent::%s: ") TEXT(Format), *FString(__func__), ##__VA_ARGS__)

#else

#define FLog(Verbosity, Format, ...)
#define CFLog(Condition, Verbosity, Format, ...)

#endif
