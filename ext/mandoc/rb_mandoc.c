/*
 * Copyright (C) 2026  gemmaro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "rb_mandoc.h"

#include "main.h"
#include "mandoc.h"
#include "roff.h"

/* This must be after roff.h. */
#include "mandoc_parse.h"

#include <ruby.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/* m_ is for mandoc prefix */
static VALUE rb_mMandoc, rb_cParser, rb_cMeta, rb_cMdocMeta, rb_cManMeta,
    rb_cNode, rb_cRootNode, rb_cBlockNode, rb_cHeadNode, rb_cBodyNode,
    rb_cTailNode, rb_cElemNode, rb_cTextNode, rb_cCommentNode, rb_cTBLNode,
    rb_cEQNNode, rb_eError;

/* parser */

static void
rb_parser_free (void *parser)
{
  struct mparse *parse = parser;
  mparse_free (parse);
  mchars_free ();
}

static const rb_data_type_t rb_parser_type
    = { .wrap_struct_name = "mandoc parser",
        .function = { .dfree = rb_parser_free },
        .flags = RUBY_TYPED_FREE_IMMEDIATELY };

static VALUE
rb_parser_alloc (VALUE klass)
{
  mchars_alloc ();
  /* MPARSE_VALIDATE is always required (as in mandoc(3) of course)
     since it fills title and other attributes in meta.  I don't find
     any reason to disable this. */
  struct mparse *parse = mparse_alloc (
      MPARSE_VALIDATE /* TODO: support other options */,
      MANDOC_OS_OTHER /* TODO: support explicitly set? */,
      NULL /* TODO: support default string for the mdoc Os macro? */);
  return rb_data_typed_object_wrap (klass, parse, &rb_parser_type);
}

static const rb_data_type_t rb_meta_type;
static VALUE
rb_parser_m_parse_file (VALUE self, VALUE fname)
{
  char *name = rb_string_value_cstr (&fname);

  struct mparse *parser = RTYPEDDATA_DATA (self);

  /* Previous memories are kept.  It will be freed when parsed
     will, not now. */
  mparse_reset (parser);

  int descriptor = mparse_open (parser, name);
  if (descriptor == -1)
    rb_raise (rb_eError, "failed to open");
  mparse_readfd (parser, descriptor, name);
  if (close (descriptor) == -1)
    rb_raise (rb_eError, "failed to close");
  struct roff_meta *meta = mparse_result (parser);
  /* TODO: fetch validity of the input with mparse_updaterc */

  VALUE klass;
  switch (meta->macroset)
    {
    case MACROSET_NONE:
      rb_raise (rb_eError, "unknown macroset");
    case MACROSET_MDOC:
      klass = rb_cMdocMeta;
      break;
    case MACROSET_MAN:
      klass = rb_cManMeta;
      break;
    }

  VALUE val = rb_data_typed_object_wrap (klass, meta, &rb_meta_type);

  /* To prevent garbage collect parser and for man-to-man output. */
  rb_iv_set (val, "@parser", self);

  return val;
}

/* roff meta */

void
rb_meta_free (void *meta)
{
  (void)meta;
}

static const rb_data_type_t rb_meta_type
    = { .wrap_struct_name = "mandoc meta",
        .function = { .dfree = rb_meta_free },
        .flags = RUBY_TYPED_FREE_IMMEDIATELY };

static VALUE
rb_meta_alloc (VALUE klass)
{
  return rb_data_typed_object_wrap (rb_cMeta, NULL, &rb_meta_type);
}

static VALUE
rb_meta_m_deroff (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);
  char *text = NULL;
  deroff (&text, meta->first);
  VALUE str
      = rb_str_new_cstr (text); /* user's responsibility to decide encoding */
  free (text);
  return str;
}

static VALUE
rb_meta_m_section (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);
  return rb_str_new_cstr (meta->msec);
}

static VALUE
rb_meta_m_volume (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);
  return rb_str_new_cstr (meta->vol);
}

static VALUE
rb_meta_m_os (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);
  return rb_str_new_cstr (meta->os);
}

static VALUE
rb_meta_m_arch (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);
  return meta->arch ? rb_str_new_cstr (meta->arch) : RUBY_Qnil;
}

static VALUE
rb_meta_m_title (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);
  return rb_str_new_cstr (meta->title);
}

static VALUE
rb_meta_m_name (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);
  return rb_str_new_cstr (meta->name);
}

static VALUE
rb_meta_m_date (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);
  return rb_str_new_cstr (meta->date);
}

static VALUE
rb_meta_m_so_target (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);
  return meta->sodest ? rb_str_new_cstr (meta->sodest) : RUBY_Qnil;
}

static VALUE
rb_meta_m_has_body (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);
  return meta->hasbody ? RUBY_Qtrue : RUBY_Qfalse;
}

/* Return read end of pipe. */
static int
rb_backup_stdout (int *original_stdout)
{
  int pipefd[2];
  if (pipe (pipefd) == -1)
    rb_raise (rb_eError, "failed to create pipe");
  *original_stdout = dup (STDOUT_FILENO);
  if (*original_stdout == -1)
    rb_raise (rb_eError, "failed to dup stdout");
  if (fflush (stdout) == EOF)
    rb_raise (rb_eError, "failed to flush stdout");
  if (dup2 (pipefd[1], STDOUT_FILENO) == -1)
    rb_raise (rb_eError, "failed to set stdout to pipe");
  if (close (pipefd[1]) == -1)
    rb_raise (rb_eError, "failed to close pipe write end");
  return pipefd[0];
}

static VALUE
rb_restore_stdout (int original_stdout, int read_end)
{
  if (fflush (stdout) == EOF)
    rb_raise (rb_eError, "failed to flush stdout after convert");
  if (dup2 (original_stdout, STDOUT_FILENO) == -1)
    rb_raise (rb_eError, "failed to restore stdout");
  if (close (original_stdout) == -1)
    rb_raise (rb_eError, "failed to close original stdout");

  /* $ find mandoc-1.14.6 -name '*.1' | while read -r file; do wc $file; done
   *   158     764    4593 mandoc-1.14.6/regress/regress.pl.1
   *   513    1971   10983 mandoc-1.14.6/apropos.1
   *   108     471    2707 mandoc-1.14.6/demandoc.1
   *   431    1783    9833 mandoc-1.14.6/man.1
   *  1332    4355   21554 mandoc-1.14.6/man.options.1
   *  2412   10705   58653 mandoc-1.14.6/mandoc.1
   *    86     441    2661 mandoc-1.14.6/soelim.1
   */
  VALUE md = rb_str_buf_new (3000);

  char tmp[1 << 12];
  ssize_t n;
  while ((n = read (read_end, tmp, sizeof (tmp))) > 0)
    rb_str_cat (md, tmp, n);

  if (close (read_end) == -1)
    rb_raise (rb_eError, "failed to close pipe read end");
  return md;
}

static VALUE
rb_mdocmeta_m_tree (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);

  int original_stdout;
  int read_end = rb_backup_stdout (&original_stdout);

  tree_mdoc (NULL, meta);

  return rb_restore_stdout (original_stdout, read_end);
}

static VALUE
rb_manmeta_m_tree (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);

  int original_stdout;
  int read_end = rb_backup_stdout (&original_stdout);

  tree_man (NULL, meta);

  return rb_restore_stdout (original_stdout, read_end);
}

static VALUE
rb_mdocmeta_m_man (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);

  int original_stdout;
  int read_end = rb_backup_stdout (&original_stdout);

  man_mdoc (NULL, meta);

  return rb_restore_stdout (original_stdout, read_end);
}

/**
 * The result depends on <tt>struct mparse</tt>, which means this
 * method must be called BEFORE the next +parse_file+ method calling.
 */
static VALUE
rb_manmeta_m_man (VALUE self)
{
  int original_stdout;
  int read_end = rb_backup_stdout (&original_stdout);

  VALUE parser = rb_iv_get (self, "@parser");
  struct mparse *p = RTYPEDDATA_DATA (parser);
  mparse_copy (p);

  return rb_restore_stdout (original_stdout, read_end);
}

static VALUE
rb_mdocmeta_m_markdown (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);

  int original_stdout;
  int read_end = rb_backup_stdout (&original_stdout);

  markdown_mdoc (NULL, meta);

  return rb_restore_stdout (original_stdout, read_end);
}

static const rb_data_type_t rb_node_type;
static VALUE
rb_wrap_node (struct roff_node *node)
{
  VALUE klass;
  switch (node->type)
    {
    case ROFFT_ROOT:
      klass = rb_cRootNode;
      break;
    case ROFFT_BLOCK:
      klass = rb_cBlockNode;
      break;
    case ROFFT_HEAD:
      klass = rb_cHeadNode;
      break;
    case ROFFT_BODY:
      klass = rb_cBodyNode;
      break;
    case ROFFT_TAIL:
      klass = rb_cTailNode;
      break;
    case ROFFT_ELEM:
      klass = rb_cElemNode;
      break;
    case ROFFT_TEXT:
      klass = rb_cTextNode;
      break;
    case ROFFT_COMMENT:
      klass = rb_cCommentNode;
      break;
    case ROFFT_TBL:
      klass = rb_cTBLNode;
      break;
    case ROFFT_EQN:
      klass = rb_cEQNNode;
      break;
    }
  return rb_data_typed_object_wrap (klass, node, &rb_node_type);
}

static VALUE
rb_meta_m_first_node (VALUE self)
{
  struct roff_meta *meta = RTYPEDDATA_DATA (self);
  return rb_wrap_node (meta->first);
}

/* roff node */

static void
rb_node_free (void *node)
{
  (void)node;
}

static const rb_data_type_t rb_node_type
    = { .wrap_struct_name = "mandoc node",
        .function = { .dfree = rb_node_free },
        .flags = RUBY_TYPED_FREE_IMMEDIATELY };

static VALUE
rb_node_alloc (VALUE klass)
{
  return rb_data_typed_object_wrap (klass, NULL, &rb_node_type);
}

static VALUE
rb_node_m_parent (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return node->parent ? rb_wrap_node (node->parent) : RUBY_Qnil;
}

static VALUE
rb_node_m_first_child (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return node->child ? rb_wrap_node (node->child) : RUBY_Qnil;
}

static VALUE
rb_node_m_last_child (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return node->last ? rb_wrap_node (node->last) : RUBY_Qnil;
}

static VALUE
rb_node_m_next_sibling (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return node->next ? rb_wrap_node (node->next) : RUBY_Qnil;
}

static VALUE
rb_node_m_prev_sibling (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return node->prev ? rb_wrap_node (node->prev) : RUBY_Qnil;
}

static VALUE
rb_node_m_head (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return node->head ? rb_wrap_node (node->head) : RUBY_Qnil;
}

static VALUE
rb_node_m_body (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return node->body ? rb_wrap_node (node->body) : RUBY_Qnil;
}

static VALUE
rb_node_m_tail (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return node->tail ? rb_wrap_node (node->tail) : RUBY_Qnil;
}

static VALUE
rb_node_m_text (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return node->string ? rb_str_new_cstr (node->string) : RUBY_Qnil;
}

static VALUE
rb_node_m_tag (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return node->string ? rb_str_new_cstr (node->tag) : RUBY_Qnil;
}

static VALUE
rb_node_m_line (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return rb_int2num_inline (node->line);
}

static VALUE
rb_node_m_column (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return rb_int2num_inline (node->pos);
}

static VALUE
rb_node_m_name (VALUE self)
{
  struct roff_node *node = RTYPEDDATA_DATA (self);
  return rb_usascii_str_new_cstr (roff_name[node->tok]);
}

RUBY_FUNC_EXPORTED void
Init_mandoc (void)
{
  rb_mMandoc = rb_define_module ("Mandoc");

  rb_cParser = rb_define_class_under (rb_mMandoc, "Parser", rb_cObject);
  rb_define_alloc_func (rb_cParser, rb_parser_alloc);
  rb_define_method (rb_cParser, "parse_file", rb_parser_m_parse_file, 1);

  rb_cMeta = rb_define_class_under (rb_mMandoc, "Meta", rb_cObject);
  rb_define_alloc_func (rb_cMeta, rb_meta_alloc);
  rb_define_method (rb_cMeta, "deroff", rb_meta_m_deroff, 0);
  rb_define_method (rb_cMeta, "first_node", rb_meta_m_first_node, 0);

  rb_define_method (rb_cMeta, "section", rb_meta_m_section, 0);
  rb_define_method (rb_cMeta, "volume", rb_meta_m_volume, 0);
  rb_define_method (rb_cMeta, "os", rb_meta_m_os, 0);
  rb_define_method (rb_cMeta, "arch", rb_meta_m_arch, 0);
  rb_define_method (rb_cMeta, "title", rb_meta_m_title, 0);
  rb_define_method (rb_cMeta, "name", rb_meta_m_name, 0);
  rb_define_method (rb_cMeta, "date", rb_meta_m_date, 0);
  rb_define_method (rb_cMeta, "so_target", rb_meta_m_so_target, 0);
  rb_define_method (rb_cMeta, "has_body?", rb_meta_m_has_body, 0);

  /* TODO: Converters for HTML, PDF, ASCII, UTF8, LOCALE, PS
     (PostScript) are tricky, since it expects outdata (void*) of
     struct outstate pointer. */

  rb_cMdocMeta = rb_define_class_under (rb_mMandoc, "MdocMeta", rb_cMeta);
  rb_define_method (rb_cMdocMeta, "markdown", rb_mdocmeta_m_markdown, 0);
  rb_define_method (rb_cMdocMeta, "tree", rb_mdocmeta_m_tree, 0);
  rb_define_method (rb_cMdocMeta, "man", rb_mdocmeta_m_man, 0);

  rb_cManMeta = rb_define_class_under (rb_mMandoc, "ManMeta", rb_cMeta);
  rb_define_method (rb_cManMeta, "tree", rb_manmeta_m_tree, 0);
  rb_define_method (rb_cManMeta, "man", rb_manmeta_m_man, 0);

  rb_cNode = rb_define_class_under (rb_mMandoc, "Node", rb_cObject);
  rb_define_alloc_func (rb_cNode, rb_node_alloc);

  rb_define_method (rb_cNode, "parent", rb_node_m_parent, 0);
  rb_define_method (rb_cNode, "first_child", rb_node_m_first_child, 0);
  rb_define_method (rb_cNode, "last_child", rb_node_m_last_child, 0);
  rb_define_method (rb_cNode, "next_sibling", rb_node_m_next_sibling, 0);
  rb_define_method (rb_cNode, "prev_sibling", rb_node_m_prev_sibling, 0);
  rb_define_method (rb_cNode, "head", rb_node_m_head, 0);
  rb_define_method (rb_cNode, "body", rb_node_m_body, 0);
  rb_define_method (rb_cNode, "tail", rb_node_m_tail, 0);

  rb_define_method (rb_cNode, "text", rb_node_m_text, 0);
  rb_define_method (rb_cNode, "tag", rb_node_m_tag, 0);

  rb_define_method (rb_cNode, "line", rb_node_m_line, 0);
  rb_define_method (rb_cNode, "column", rb_node_m_column, 0);

  rb_define_method (rb_cNode, "name", rb_node_m_name, 0);

  rb_cRootNode = rb_define_class_under (rb_mMandoc, "RootNode", rb_cNode);
  rb_cBlockNode = rb_define_class_under (rb_mMandoc, "BlockNode", rb_cNode);
  rb_cHeadNode = rb_define_class_under (rb_mMandoc, "HeadNode", rb_cNode);
  rb_cBodyNode = rb_define_class_under (rb_mMandoc, "BodyNode", rb_cNode);
  rb_cTailNode = rb_define_class_under (rb_mMandoc, "TailNode", rb_cNode);
  rb_cElemNode = rb_define_class_under (rb_mMandoc, "ElemNode", rb_cNode);
  rb_cTextNode = rb_define_class_under (rb_mMandoc, "TextNode", rb_cNode);
  rb_cCommentNode
      = rb_define_class_under (rb_mMandoc, "CommentNode", rb_cNode);
  rb_cTBLNode = rb_define_class_under (rb_mMandoc, "TBLNode", rb_cNode);
  rb_cEQNNode = rb_define_class_under (rb_mMandoc, "EQNNode", rb_cNode);

  rb_eError = rb_define_class_under (rb_mMandoc, "Error", rb_eStandardError);
}
