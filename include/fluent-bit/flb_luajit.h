/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Fluent Bit
 *  ==========
 *  Copyright (C) 2019-2021 The Fluent Bit Authors
 *  Copyright (C) 2015-2018 Treasure Data Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef FLB_LUAJIT_H
#define FLB_LUAJIT_H

#include <fluent-bit/flb_info.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <monkey/mk_core/mk_list.h>

/* Lua Context */
struct flb_luajit {
    lua_State *state;      /* LuaJIT VM environment   */
    void *config;          /* Fluent Bit context      */
    struct mk_list _head;  /* Link to flb_config->lua */
};

struct flb_luajit *flb_luajit_create();
int flb_luajit_load_script(struct flb_luajit *lj, char *script);
void flb_luajit_destroy(struct flb_luajit *lj);
int flb_luajit_destroy_all(struct flb_config *ctx);

#endif
