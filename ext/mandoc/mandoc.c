#include "mandoc.h"

VALUE rb_mMandoc;

RUBY_FUNC_EXPORTED void
Init_mandoc(void)
{
  rb_mMandoc = rb_define_module("Mandoc");
}
