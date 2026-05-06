#pragma once

/// Log once per session only 
#define STEVES_LOG_ONCE(CategoryName, Verbosity, Format, ...) \
{ \
	static bool bLogged = false; \
	UE_CLOG(!bLogged, CategoryName, Verbosity, Format, ##__VA_ARGS__); \
	bLogged = true; \
}

/// Suppress repeat logs if the value is the same as last time at the call site
#define STEVES_LOG_NOREPEAT(TypeName, Value, CategoryName, Verbosity, Format, ...) \
{ \
	static TOptional<TypeName> LastValue; \
	UE_CLOG(!LastValue.IsSet() || LastValue != Value, CategoryName, Verbosity, Format, ##__VA_ARGS__); \
	LastValue = Value; \
}
