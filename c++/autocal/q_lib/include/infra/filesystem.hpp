/*=============================================================================
   Copyright (c) 2016-2020 Joel de Guzman

   Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
=============================================================================*/
#if !defined(INFRA_FILESYSTEM_APRIL_6_2020)
#define INFRA_FILESYSTEM_APRIL_6_2020

#if defined(__apple_build_version__) && (__apple_build_version__ <= 11000033L)
# define INFRA_HAS_NO_STD_FS
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1920)
# define INFRA_USE_STD_FS
#endif

#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include)
# if __has_include(<filesystem>) && !defined(INFRA_HAS_NO_STD_FS)
#  define INFRA_USE_STD_FS
# endif
#endif

#if defined(INFRA_USE_STD_FS)
# include <filesystem>
namespace cycfi {
  namespace fs = std::filesystem;
}
#else
# include <ghc/filesystem.hpp>
namespace cycfi {
  namespace fs = ghc::filesystem;
}
#endif

#endif
