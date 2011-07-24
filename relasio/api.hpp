#ifndef RELASIO_API_HPP
#define RELASIO_API_HPP

#if defined(_MSC_VER)
  #error MSVC is untested; remove at your own risk
  /* The __declspec stuff for ensuring symbols are exported from DLLs and
   * imported back into libraries */
  #ifdef RELASIO_EXPORTS
    #define RELASIO_API __declspec(dllexport)
  #else
    #define RELASIO_API __declspec(dllimport)
  #endif //RELASIO_EXPORTS
#else
  #if defined(__GNUC__) && (__GNUC__ >= 4)
    #define RELASIO_API __attribute__ ((visibility ("default")))
  #else
    #define RELASIO_API
  #endif
#endif

#endif // RELASIO_API_HPP

