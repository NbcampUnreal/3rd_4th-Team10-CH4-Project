#pragma once

#include "Logging/LogMacros.h"

class UObject;

CATCHMEIFYOUCAN_API DECLARE_LOG_CATEGORY_EXTERN(LogCY, Log, All);

CATCHMEIFYOUCAN_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
