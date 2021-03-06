--- absl/base/internal/unscaledcycleclock.cc.orig	2019-07-24 12:47:23 UTC
+++ absl/base/internal/unscaledcycleclock.cc
@@ -20,10 +20,15 @@
 #include <intrin.h>
 #endif
 
-#if defined(__powerpc__) || defined(__ppc__)
+#if (defined(__powerpc__) || defined(__ppc__)) && !defined(__FreeBSD__)
 #include <sys/platform/ppc.h>
 #endif
 
+#ifdef __FreeBSD__
+#include <sys/types.h>
+#include <sys/sysctl.h>
+#endif
+
 #include "absl/base/internal/sysinfo.h"
 
 namespace absl {
@@ -56,11 +61,34 @@ double UnscaledCycleClock::Frequency() {
 #elif defined(__powerpc__) || defined(__ppc__)
 
 int64_t UnscaledCycleClock::Now() {
+#ifndef __FreeBSD__
   return __ppc_get_timebase();
+#else
+  union { long long complete; unsigned int part[2]; } ticks;
+  unsigned int tmp;
+  asm volatile(
+    "0:\n"
+    "mftbu %[hi32]\n"
+    "mftb %[lo32]\n"
+    "mftbu %[tmp]\n"
+    "cmpw %[tmp],%[hi32]\n"
+    "bne 0b\n"
+    : [hi32] "=r"(ticks.part[0]), [lo32] "=r"(ticks.part[1]),
+    [tmp] "=r"(tmp)
+  );
+  return ticks.complete;
+#endif
 }
 
 double UnscaledCycleClock::Frequency() {
+#ifndef __FreeBSD__
   return __ppc_get_timebase_freq();
+#else
+  long timebaseFrequency = 0;
+  size_t length = sizeof(timebaseFrequency);
+  sysctlbyname("kern.timecounter.tc.timebase.frequency", &timebaseFrequency, &length, NULL, 0);
+  return timebaseFrequency;
+#endif
 }
 
 #elif defined(__aarch64__)
