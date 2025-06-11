#ifndef  _GNU_SOURCE
#define  _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define ALIGN_DOWN_BY(val, unit)	((val) - (val) % (unit))

typedef void (*glfwSetWindowIcon_fnptr_t) (void *, int, const void *);

static unsigned char new_codes_[] = {
#if defined(__i386__) || defined(__x86_64__)
  0xc3  /* ret */
#elif defined(__arm__)
  0x1e, 0xff, 0x2f, 0xe1  /* bx lr */
#elif defined(__aarch64__)
  0xc0, 0x03, 0x5f, 0xd6  /* ret */
#else
#error please add support for this architecture
#endif
};

__attribute__((constructor)) static void
do_patch_glfw_funcs_ (void)
{
  void *lib_handle;
  glfwSetWindowIcon_fnptr_t fn_addr;
  long page_size;
  char *page_start;

  lib_handle = dlopen ("libglfw.so", RTLD_NOW);
  if (!lib_handle)
    abort ();

  fn_addr = (typeof(fn_addr))(size_t)dlsym (lib_handle, "glfwSetWindowIcon");
  if (!fn_addr)
    abort ();

  page_size = sysconf (_SC_PAGE_SIZE);
  if (page_size < 0)
    abort ();

  page_start = (void *)ALIGN_DOWN_BY((size_t)fn_addr, page_size);

  // let us modify this chunk of memory
  if (mprotect (page_start, page_size,
                PROT_READ | PROT_WRITE | PROT_EXEC) < 0)
    abort ();

  memcpy ((void *)(size_t)fn_addr, new_codes_, sizeof(new_codes_));

  // remove write access from it again
  if (mprotect (page_start, page_size,
                PROT_READ | PROT_EXEC) < 0)
    abort ();

  // don't close lib handle since the ref count will drop to zero
  // because the game haven't loaded it at this moment
}
