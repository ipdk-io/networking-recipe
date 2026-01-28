// Thread safety annotation compatibility header for GCC
// This header provides compatibility macros for thread safety annotations
// when using abseil's ABSL_ prefixed macros with code that expects non-prefixed macros.
#ifndef THREAD_ANNOTATIONS_COMPAT_H_
#define THREAD_ANNOTATIONS_COMPAT_H_

#include "absl/base/thread_annotations.h"

#ifndef GUARDED_BY
#define GUARDED_BY(x) ABSL_GUARDED_BY(x)
#endif

#ifndef PT_GUARDED_BY
#define PT_GUARDED_BY(x) ABSL_PT_GUARDED_BY(x)
#endif

#ifndef SHARED_LOCKS_REQUIRED
#define SHARED_LOCKS_REQUIRED(...) ABSL_SHARED_LOCKS_REQUIRED(__VA_ARGS__)
#endif

#ifndef EXCLUSIVE_LOCKS_REQUIRED
#define EXCLUSIVE_LOCKS_REQUIRED(...) ABSL_EXCLUSIVE_LOCKS_REQUIRED(__VA_ARGS__)
#endif

#ifndef LOCKS_EXCLUDED
#define LOCKS_EXCLUDED(...) ABSL_LOCKS_EXCLUDED(__VA_ARGS__)
#endif

#ifndef LOCK_RETURNED
#define LOCK_RETURNED(x) ABSL_LOCK_RETURNED(x)
#endif

#ifndef LOCKABLE
#define LOCKABLE ABSL_LOCKABLE
#endif

#ifndef SCOPED_LOCKABLE
#define SCOPED_LOCKABLE ABSL_SCOPED_LOCKABLE
#endif

#ifndef EXCLUSIVE_LOCK_FUNCTION
#define EXCLUSIVE_LOCK_FUNCTION(...) ABSL_EXCLUSIVE_LOCK_FUNCTION(__VA_ARGS__)
#endif

#ifndef SHARED_LOCK_FUNCTION
#define SHARED_LOCK_FUNCTION(...) ABSL_SHARED_LOCK_FUNCTION(__VA_ARGS__)
#endif

#ifndef UNLOCK_FUNCTION
#define UNLOCK_FUNCTION(...) ABSL_UNLOCK_FUNCTION(__VA_ARGS__)
#endif

#ifndef NO_THREAD_SAFETY_ANALYSIS
#define NO_THREAD_SAFETY_ANALYSIS ABSL_NO_THREAD_SAFETY_ANALYSIS
#endif

#endif  // THREAD_ANNOTATIONS_COMPAT_H_
