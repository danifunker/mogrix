/*
 * dso_handle.c — __dso_handle, which IRIX provides nowhere.
 *
 * __cxa_atexit() takes a handle identifying the object registering the
 * destructor, so that unloading a shared object can run just that object's
 * destructors. GCC emits it from crtstuff.c; IRIX's own CRT files predate the
 * Itanium C++ ABI and have no such symbol, so it has to come from here.
 *
 * Its address is the value, which is the definition GCC uses for shared
 * objects (crtstuffs.o). GCC uses 0 for executables instead, but a unique
 * non-null address is correct for both -- an executable's handle only has to
 * differ from every DSO's, and nothing on IRIX treats null as "the main
 * program". One definition serving both is what lets irix-ld link this same
 * object into executables and shared libraries alike.
 *
 * Hidden visibility so each object gets its own, which is the entire point:
 * if the dynamic linker merged them, two DSOs would share a handle and
 * unloading one would run the other's destructors.
 */

extern void *__dso_handle __attribute__((__visibility__("hidden")));
void *__dso_handle __attribute__((__visibility__("hidden"))) = &__dso_handle;
